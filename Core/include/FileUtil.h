#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace CDBTo3DTiles
{
    namespace Utilities
    {
        void writeBinaryFile(const fs::path& filename, const char* binary, uint64_t byteLength);
        void writeBinaryFile(std::ostream& outputStream, const char* binary, uint64_t byteLength);
        void writeStringToFile(const fs::path& filename, const std::string& string);
        void readBinaryFile(const fs::path& filename, uint8_t* binary, uint64_t byteLength);
        std::string readFileToString(const fs::path& filename);
        size_t getDirectorySize(const fs::path& path);
    } // namespace Utilities
} // namespace CDBTo3DTiles
