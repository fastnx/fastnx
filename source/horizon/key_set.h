#pragma once
#include <fs_sys/refs/editable_directory.h>

#include <crypto/types.h>
namespace FastNx::Horizon {
    enum class KeyMode {
        Dev,
        Prod
    };

    enum class KeyType {
        Production,
        Title
    };

    struct KeyContainer {
        KeyMode mode;
        std::variant<Crypto::Key128, Crypto::Key256> multikey;
    };
    class KeySet {
    public:
        explicit KeySet(FsSys::ReFs::EditableDirectory &dirKeys);

        bool ParserKeys(std::vector<std::string>& pairs, KeyType type) const;
        std::optional<KeyContainer> mainKey;
    };
}
