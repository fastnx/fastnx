#pragma once
#include <fs_sys/types.h>
namespace FastNx::Core {
    // https://www.gnu.org/software/tar/manual/html_node/Standard.html
    struct alignas(512) Tapefile {
        std::array<char, 100> filename;
        std::array<char, 8> mode;
        U64 uid;
        U64 gid;
        std::array<char, 12> size;
        std::array<char, 12> mtime;
        std::array<char, 8> checksum;
        char typeflag;
        std::array<char, 100> linkname;
        std::array<char, 6> magic;
        std::array<char, 2> version;
        std::array<char, 32> uname;
        std::array<char, 32> gname;
        U64 devmajor;
        U64 devminor;
        std::array<char, 155> prefix;
    };
    class Tarball {
        public:
        explicit Tarball(const FsSys::FsPath &tarpath);
        void Include(const FsSys::VfsBackingFilePtr &file);

        // After this function is called, no more files can be added to the tarball
        void Finish();
    private:
        bool writemode{true};
        U64 offset{};
        FsSys::VfsBackingFilePtr tarfile;
    };
}
