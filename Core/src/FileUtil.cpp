#include "FileUtil.h"

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

namespace CDBTo3DTiles
{
    namespace Utilities
    {
        void writeBinaryFile(const fs::path& filename, const char* binary, uint64_t byteLength)
        {
            if (!fs::is_directory(filename.parent_path())) 
            {
                fs::create_directories(filename.parent_path());
            }
            std::ofstream outputStream(filename, std::ios_base::out | std::ios_base::binary);
            outputStream.write(binary, static_cast<int64_t>(byteLength));
            outputStream.close();
        }

        void writeBinaryFile(std::ostream& outputStream, const char* binary, uint64_t byteLength)
        {
            outputStream.write(binary, static_cast<int64_t>(byteLength));
        }

        void readBinaryFile(const fs::path& filename, uint8_t* binary, uint64_t byteLength)
        {
            std::ifstream inputStream(filename, std::ios_base::in | std::ios_base::binary);
            inputStream.read((char*)binary, static_cast<std::streamsize>(byteLength));
        }

        void writeStringToFile(const fs::path& filename, const std::string& string)
        {
            if (!fs::is_directory(filename.parent_path())) 
            {
                fs::create_directories(filename.parent_path());
            }
            std::ofstream outputStream(filename, std::ios_base::out);
            outputStream << string;
            outputStream.close();
        }

        std::string readFileToString(const fs::path& filename)
        {
            std::ifstream inputStream(filename);
            inputStream.seekg (0, inputStream.end);
            std::streampos fileSize = inputStream.tellg();
            inputStream.seekg (0, inputStream.beg);

            std::string fileData;
            fileData.resize(static_cast<size_t>(fileSize));
            inputStream.read(&fileData[0], fileSize);
            return fileData;
        }

        size_t getDirectorySize(const fs::path& directory)
        {
            size_t totalSize = 0;

            if (fs::is_directory(directory))
            {
                for (auto &entry : fs::recursive_directory_iterator(directory))
                {
                    if (fs::is_regular_file(entry.status()))
                    {
                        totalSize += fs::file_size(entry);
                    }
                }
            }

            return totalSize;
        }
    } // namespace Utilities
} // namespace CDBTo3DTiles
