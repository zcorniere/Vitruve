#include "Engine/Misc/Utils.hxx"

#include <fstream>

std::string Utils::ReadFile(const std::filesystem::path& filename)
{
    if (!std::filesystem::exists(filename))
        return "";

    /// Must be opened in binary mode, so Windows won't mess with the newlines
    std::string fileContent;
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    size_t fileSize = std::filesystem::file_size(filename);

    if (!file.is_open())
        return "";

    fileContent.resize(fileSize);
    file.read(fileContent.data(), fileSize);
    file.close();
    return fileContent;
}

std::string Utils::BytesToString(uint64 bytes)
{
    constexpr uint64_t GB = 1024 * 1024 * 1024;
    constexpr uint64_t MB = 1024 * 1024;
    constexpr uint64_t KB = 1024;

    if (bytes > GB)
        return std::format("{:2f} GB", (float)bytes / (float)GB);
    else if (bytes > MB)
        return std::format("{:2f} MB", (float)bytes / (float)MB);
    else if (bytes > KB)
        return std::format("{:2f} KB", (float)bytes / (float)KB);
    else
        return std::format("{:2f} bytes", (float)bytes);
}

static struct
{
    bool bHasRequested = false;
    int ExitStatus = 0;
} GExitRequestInfo;
void Utils::RequestExit(int Status, bool bForce)
{
    if (bForce)
    {
        std::exit(Status);
    }
    GExitRequestInfo.ExitStatus = Status;
    GExitRequestInfo.bHasRequested = true;
}

bool Utils::HasRequestedExit(int& OutStatus)
{
    OutStatus = GExitRequestInfo.ExitStatus;
    return GExitRequestInfo.bHasRequested;
}
