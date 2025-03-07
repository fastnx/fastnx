#include <algorithm>
#include <print>

#include <fmt/format.h>

#include <common/exception.h>
#include <fs_sys/regex_file.h>
#include <horizon/key_set.h>
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

        bool prod{}, title{};
        for (const auto &keyname: keys) {
            const auto keyfile{dirKeys.OpenFile(keyname)};
            assert(*keyfile);
            std::println("Importing keys from file {}", GetPathStr(keyfile));
            if (const auto [_, _ktype] = GetKeyModeByName(keyfile); _ktype != KeyType::Unknown) {
                const auto pattern = [_ktype] -> std::string {
                    if (_ktype == KeyType::Production)
                        return "^\\w+\\s*=\\s*[a-fA-F0-9]+$";
                    return "^[a-fA-F0-9]{32}\\s*=\\s*[a-fA-F0-9]{32}$";
                }();

                bool result{};
                if (const auto kfile{std::make_shared<FsSys::RegexFile>(std::move(keyfile), pattern)}; *kfile)
                    result = ParserKeys(kfile->matches, _ktype);
                if (_ktype == KeyType::Production)
                    prod = result;
                else if (_ktype == KeyType::Title)
                    title = result;
            }
        }

        if (keys.empty() || !prod || !title)
            throw exception{"No valid key file was found in the path {}", GetPathStr(dirKeys)};
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    bool KeySet::ParserKeys(std::vector<std::string> &pairs, [[maybe_unused]] const KeyType type) const {
        if (pairs.empty())
            return {};
        std::ranges::for_each(pairs, [](const auto &strkey) {
           assert(!strkey.empty());
        });

        return true;
    }
}
