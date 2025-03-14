#include <algorithm>
#include <print>

#include <boost/algorithm/string.hpp>
#include <boost/container/small_vector.hpp>
#include <fmt/format.h>

#include <common/exception.h>
#include <common/bytes.h>
#include <fs_sys/regex_file.h>
#include <horizon/key_set.h>

#include <common/async_logger.h>

namespace FastNx::Horizon {
    std::pair<KeyMode, KeyType> GetKeyModeByName(const FsSys::VfsBackingFilePtr &key) {
        const auto &keyname{key->path.filename()};
        if (keyname== "prod.keys")
            return {KeyMode::Prod, KeyType::Production};
        if (keyname == "title.keys")
            return {KeyMode::Prod, KeyType::Title};

        return {};
    }

    KeySet::KeySet(FsSys::ReFs::EditableDirectory &dirKeys) {
        if (dirKeys.GetFilesCount() < 2)
            return;
        const auto keys{dirKeys.GlobAllFiles("*.keys")};

        for (const auto &keyname: keys) {
            const auto keyfile{dirKeys.OpenFile(keyname)};
            assert(*keyfile);
            AsyncLogger::Success("Importing keys from file {}", GetPathStr(keyfile));
            if (const auto [_, _ktype] = GetKeyModeByName(keyfile); _ktype != KeyType::Unknown) {
                const auto pattern = [_ktype] -> std::string {
                    if (_ktype == KeyType::Production)
                        return "^\\w+\\s*=\\s*[a-fA-F0-9]+$";
                    return "^[a-fA-F0-9]{32}\\s*=\\s*[a-fA-F0-9]{32}$";
                }();

                if (const auto kfile{std::make_shared<FsSys::RegexFile>(std::move(keyfile), pattern)}; *kfile)
                    ParserKeys(kfile->GetAllMatches(), _ktype);
            }
        }
        AsyncLogger::Info("Total keys read and stored: {}", prods.size() + titles.size());

        if (!keys.empty())
            if (!prods.empty() || !saveall)
                if (!titles.empty())
                    return;
        throw exception{"No valid key file was found in the path {}", GetPathStr(dirKeys)};
    }

    void KeySet::ParserKeys(std::vector<std::string> &&pairs, const KeyType type) {
        const auto callback = [&] {
            if (type == KeyType::Production)
                return &KeySet::AddProdKey;
            return &KeySet::AddTitleKey;
        }();

        for (auto &&key: pairs) {
            std::vector<std::string_view> keyview;

            split(keyview, key, boost::is_any_of("="));
            keyview.front().remove_suffix(1);
            keyview.back().remove_prefix(1);

            if ((this->*callback)(keyview.front(), keyview.back()))
                keyring.emplace_back(std::move(key));
        }
    }

    bool KeySet::AddTitleKey(const std::string_view &keyname, const std::string_view &keyvalue) {
        const auto keyid{ToObjectOf<Crypto::Key128>(keyname)};
        const auto keyval{ToObjectOf<Crypto::Key128>(keyvalue)};

        titles.emplace_back(keyid, keyval);
        return {}; // Since we copy the key, there's no need to store it
    }

    ProductionTaste GetKeyResidence(const KeyIndexType _type) {
        switch (_type) {
            case KeyIndexType::HeaderKey:
                return ProductionTaste::Named;
            default:
                return ProductionTaste::Indexable;
        }
    }

    bool KeySet::AddProdKey(const std::string_view &keyname, const std::string_view &keyvalue) {
        const auto _type = [&] -> KeyIndexType {
            if (keyname == "header_key")
                return KeyIndexType::HeaderKey;
            if (keyname.starts_with("titlekek"))
                return KeyIndexType::Titlekek;
            return {};
        }();

        if (_type == KeyIndexType::Undefined) {
            if (saveall)
                prods.emplace(keyname, keyvalue);
            return saveall;
        }
        boost::container::small_vector<std::string_view, 2> indexstr;
        split(indexstr, keyname, boost::is_any_of("_"));

        bool persistent{saveall};
        if (GetKeyResidence(_type) == ProductionTaste::Named) {
            const auto _key256{ToObjectOf<Crypto::Key256>(keyvalue)};

            if (!namedkeys256.contains(_type))
                namedkeys256.emplace(_type, _key256);
            if (_type == KeyIndexType::HeaderKey)
                headerKey.emplace(&namedkeys256.find(_type)->second);
        } else if (std::isdigit(*indexstr.back().begin())) {
            const auto keyindex{strtoll(indexstr.back().begin(), nullptr, 16)};
            if (const auto idkey{KeyIdentifier(_type, keyindex)}; !indexablekeys.contains(idkey))
                indexablekeys.emplace(idkey, keyvalue);

            persistent = true;
        }
        return persistent;
    }
}
