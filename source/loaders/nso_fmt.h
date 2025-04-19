#pragma once
#include <unordered_map>
#include <loaders/types.h>

namespace FastNx::Loaders {

#pragma pack(push, 1)
    struct BinarySection {
        U32 fileoffset;
        U32 memoryoffset;
        U32 size;
    };
    struct BinarySegment {
        U32 offset;
        U32 size;
    };

    struct NsoHeader {
        U32 magic; // Signature ("NSO0")
        U32 version;
        U32 reserved;
        U32 flags;
        BinarySection textsection;
        U32 modulenameoffset;
        BinarySection rosection;
        U32 modulenamesize;
        BinarySection datasection;

        U32 bssSize;

        std::array<U8, 0x20> moduleid;
        // Size of the compressed sections on disk
        U32 textfilesize;
        U32 rofilesize;
        U32 datafilesize;

        std::array<U8, 0x1C> reserved1;

        BinarySegment embedded;
        BinarySegment dynstr;
        BinarySegment dynsym;

        std::array<std::array<U8, 0x20>, 3> sectionshash;
    };
#pragma pack(pop)

    enum class NsoSectionType : U64 {
        Text,
        Ro,
        Data,
    };

    static_assert(IsSizeOf<NsoHeader, 0x100>);
    class NsoFmt final : public AppLoader {
    public:
        NsoFmt(const FsSys::VfsBackingFilePtr &nso, bool &loaded);

        void LoadApplication(std::shared_ptr<Kernel::Types::KProcess> &kprocess) override;

        std::unordered_map<NsoSectionType, std::vector<U8>> secsmap;

        static void LoadModules(const std::shared_ptr<Kernel::Types::KProcess> &kprocess, const FsSys::VfsReadOnlyDirectoryPtr &exefs);
    private:
        void PrintRo(const std::string &strings) const;
        void GetSection(const BinarySection &section, U32 compressed, NsoSectionType type);
    };
}
