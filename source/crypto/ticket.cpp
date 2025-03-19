#include <common/async_logger.h>
#include <horizon/key_set.h>

#include <crypto/ticket.h>
namespace FastNx::Crypto {
    Ticket::Ticket(const FsSys::VfsBackingFilePtr &tik) {
        if (tik->GetSize() < sizeof(TicketData))
            return;
        std::memset(&content, 0, sizeof(content));

        type = tik->Read<SignatureType>();
        U64 offset{4};
        const auto size = [&] {
            switch (type) {
                case SignatureType::Rsa4096Sha1:
                case SignatureType::Rsa4096Sha256:
                    return 0x200;
                case SignatureType::Rsa2048Sha1:
                case SignatureType::Rsa2048Sha256:
                    return 0x100;
                case SignatureType::EcdsaSha1:
                case SignatureType::EcdsaSha256:
                    return 0x3C;
                default:
                    return 0x14;
            }
        }();
        signature = tik->ReadSome(size, offset);

        offset += signature.size();
        if (signature.size() == 0x3C)
            offset += 0x40;
        else if (signature.size() == 0x14)
            offset += 0x14;
        else
            offset += 0x3C;

        std::span ticket{reinterpret_cast<char* >(&content), sizeof(content) - offset};
        if (tik->ReadSome(ticket, offset) != ticket.size())
            return;

        if (const std::string_view issuer{ticket.data(), strlen(ticket.data())}; !issuer.empty())
            AsyncLogger::Info("Ticket issuer: {}", issuer);
    }

    void Ticket::Export(const FsSys::VfsBackingFilePtr &file) {
        file->Write(type);
        file->WriteSome(std::span(signature), 0x4);

        const auto offset{signature.size() + 0x4};
        const std::span ticket{reinterpret_cast<U8 *>(&content), sizeof(content) - offset};
        file->WriteSome(ticket, offset);
    }
}
