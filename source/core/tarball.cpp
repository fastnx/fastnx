#include <regex>
#include <fs_sys/refs/buffered_file.h>
#include <core/tarball.h>

#include <common/exception.h>
#include <common/container.h>


namespace FastNx::Core {
    constexpr auto TarBlockSize{512};
    Tarball::Tarball(const FsSys::FsPath &tarpath) :
        tarfile(std::make_shared<FsSys::ReFs::BufferedFile>(tarpath, 0, FsSys::FileModeType::ReadWrite, true)) {
    }

    void Setname(Tapefile &tar, const FsSys::FsPath &filepath) {
        auto path{filepath};
        if (path.is_absolute())
            path = path.lexically_relative(std::filesystem::current_path());
        auto *result{tar.filename.begin()};
        const auto size{std::strlen(path.c_str())};
        if (path.empty() || size > tar.filename.size())
            throw exception{"Invalid filename"};
        std::strncpy(result, path.c_str(), size);
    }
    void WriteZeros(const FsSys::VfsBackingFilePtr &file, const U64 offset, const U64 count) {
        boost::container::small_vector<char, TarBlockSize> bytes;
        bytes.resize(count);
        std::ranges::fill(bytes, 0);
        file->WriteSome(ToSpan(bytes), offset);

    }

    void CopyToFile(const FsSys::VfsBackingFilePtr &output, const U64 offset,
        const FsSys::VfsBackingFilePtr &source) {

        std::vector<U8> buffer(2_MBYTES);
        for (U64 outoff{}; outoff < source->GetSize(); ) {
            const auto size{std::min(source->GetSize() - outoff, buffer.size())};
            if (buffer.size() > size)
                buffer.resize(size);
            source->ReadSome(ToSpan(buffer), outoff);
            output->WriteSome(ToSpan(buffer), offset + outoff);

            outoff += size;
        }
    }
    void Start(Tapefile &tar) {
        std::strcpy(tar.magic.begin(), "ustar");
        std::strcpy(tar.version.begin(), " ");
        std::sprintf(tar.mode.begin(), "%07o", 0644);
        std::sprintf(tar.gname.begin(), "users");
        const auto now = std::chrono::system_clock::now();
        std::sprintf(tar.mtime.begin(), "%011lo", std::chrono::system_clock::to_time_t(now));
    }

    void Checksum(Tapefile &tar) {
        U32 result{};
        const auto *begin{reinterpret_cast<U8 *>(&tar)};
        const auto *end{begin + sizeof(tar)};
        const auto field{begin + offsetof(Tapefile, checksum)};

        while (begin < field)
            result += *begin++ & 0xff;
        for (; begin < field + 8; ++begin)
            result += ' ';
        while (begin != end)
            result += *begin++ & 0xff;

        std::sprintf(tar.checksum.begin(), "%06o", result);
    }

    void Tarball::Include(const FsSys::VfsBackingFilePtr &file) {
        Tapefile tar{};
        Start(tar);

        Setname(tar, file->path);
        std::sprintf(tar.size.begin(), "%011lo", file->GetSize());
        Checksum(tar);

        tarfile->Write(tar, offset);
        offset += sizeof(tar);

        CopyToFile(tarfile, offset, file);
        offset += file->GetSize();

        const auto block{offset % TarBlockSize};
        WriteZeros(tarfile, offset, TarBlockSize - block);
        offset += TarBlockSize - block;

        // Testing alignment
        NX_ASSERT((offset % TarBlockSize) == 0);
    }

    void Tarball::Finish() {
        NX_ASSERT(writemode);
        writemode = {};
        Tapefile padding{};
        for (U64 count{}; count < 2; count++)
            tarfile->Write(padding, offset += sizeof(padding));
    }
}
