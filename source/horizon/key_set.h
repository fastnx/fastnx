#pragma once
#include <unordered_map>
#include <boost/container_hash/hash.hpp>
#include <fs_sys/refs/editable_directory.h>

#include <crypto/types.h>
namespace FastNx::Horizon {
    enum class KeyMode {
        None,
        Dev,
        Prod
    };

    enum class KeyType {
        Unknown,
        Production,
        Title
    };

    enum class KeyIndexType : U64 {
        Unknown,
        HeaderKey,
    };
    struct KeyIdentifier {
        KeyIndexType type;
        U32 index;
        bool operator==(const KeyIdentifier& _keyid) const {
            return type == _keyid.type && index == _keyid.index;
        }
    };

    struct KeyIndexableHash {
        U64 operator()(const KeyIdentifier& key) const {
            U64 result{};
            boost::hash_combine(result, std::to_underlying(key.type));
            boost::hash_combine(result, key.index);
            return result;
        }
    };

    enum class ProductionTaste {
        Named,
        Indexable
    };

    class KeySet {
    public:
        explicit KeySet(FsSys::ReFs::EditableDirectory &dirKeys);

        void ParserKeys(std::vector<std::string> &&pairs, KeyType type);

        bool AddTitleKey(const std::string_view &keyname, const std::string_view &keyvalue);
        bool AddProdKey(const std::string_view &keyname, const std::string_view &keyvalue);

        std::vector<std::pair<Crypto::Key128, Crypto::Key128>> titles;
        std::unordered_map<std::string_view, std::string_view> prods;

        std::unordered_map<KeyIndexType, Crypto::Key256> namedkeys256;
        std::unordered_map<KeyIdentifier, std::string_view, KeyIndexableHash> indexablekeys;

        std::optional<Crypto::Key256*> headerKey;
        std::vector<std::string> keyring; // <- Store the keys to avoid unnecessary allocations
    };
}
