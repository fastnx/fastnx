#include <algorithm>
#include <print>

#include <fmt/format.h>

#include <common/exception.h>
#include <fs_sys/regex_file.h>
#include <horizon/key_set.h>
namespace FastNx::Horizon {
    KeySet::KeySet(FsSys::ReFs::EditableDirectory &dirKeys) {
        if (dirKeys.GetFilesCount() < 2)
            return;
        const auto keys{dirKeys.GlobAllFiles("*.keys")};

        bool prod{}, title{};
        for (const auto &keyname: keys) {
            const auto keyfile{dirKeys.OpenFile(keyname)};
            if (!keyfile)
                continue;

            if (!prod && keyname.filename() == "prod.keys")
                if (const auto prodfile{std::make_shared<FsSys::RegexFile>(std::move(keyfile), "^\\w+\\s*=\\s*[a-fA-F0-9]+$")}; static_cast<bool>(*prodfile))
                    prod = ParserKeys(prodfile->matches, KeyType::Production);
            if (!title && keyname.filename() == "title.keys")
                if (const auto titlefile{std::make_shared<FsSys::RegexFile>(std::move(keyfile), "^[a-fA-F0-9]{32}\\s*=\\s*[a-fA-F0-9]{32}$")}; static_cast<bool>(*titlefile))
                    title = ParserKeys(titlefile->matches, KeyType::Title);
        }

        if (keys.empty() || !prod || !title)
            throw exception{"No valid key file was found in the path {}", GetPathStr(dirKeys)};
    }

    bool KeySet::ParserKeys(std::vector<std::string> &pairs, [[maybe_unused]] const KeyType type) const {
        if (pairs.empty())
            return {};
        std::ranges::for_each(pairs, [](const auto &strkey) {
            std::println("{}", strkey);
        });

        return true;
    }
}
