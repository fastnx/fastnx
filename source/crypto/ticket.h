#pragma once
#include <crypto/types.h>

namespace FastNx::Horizon {
    class KeySet;
}

namespace FastNx::Crypto {
#pragma pack(push, 1)
    struct TicketData {
        std::array<char, 0x40> issuer;
        std::array<U8, 0x100> keyblock;
        U8 constant; // Always 2 for Switch (ES) Tickets
        U8 keytype;
        U16 version; // Ticket Version
        U8 license;
        U8 revision;
        U16 properties;
        U64 reserved;
        U64 ticketid;
        U64 deviceid;
        RightsId rights;
        U32 accountid;
        std::array<U8, 0x140 + 0xC> reserved1;
    };
    static_assert(IsSizeOf<TicketData, 0x180 + 0x140>);
#pragma pack(pop)

    enum class SignatureType : U32 {
        Rsa4096Sha1 = 0x010000,
        Rsa2048Sha1,
        EcdsaSha1,
        Rsa4096Sha256,
        Rsa2048Sha256,
        EcdsaSha256,
        HmacSha1
    };
    class Ticket {
    public:
        explicit Ticket(const FsSys::VfsBackingFilePtr &tik);

        SignatureType type;
        std::vector<U8> signature;
        TicketData content;
        void Export(const FsSys::VfsBackingFilePtr &file);
        std::array<U8, 16> DecryptTitleKey(const std::array<U8, 16> &kek) const;
    };
}
