#include "Utils.h"
#include <windows.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

bool Utils::error = false;

std::string Utils::wPrefix(unsigned long bytes) 
{
    std::string result;
    std::ostringstream oss;

    if (bytes > 1e12) 
    {
        oss << std::fixed << std::setprecision(1) << (static_cast<double>(bytes) / 1e12) << " TB";
        result = oss.str();
        return result;
    }
    else if (bytes > 1e9) 
    {
        oss << std::fixed << std::setprecision(1) << (static_cast<double>(bytes) / 1e9) << " GB";
        result = oss.str();
        return result;
    }
    else if (bytes > 1e6) 
    {
        oss << std::fixed << std::setprecision(1) << (static_cast<double>(bytes) / 1e6) << " MB";
        result = oss.str();
        return result;
    }
    else if (bytes > 1000) 
    {
        oss << std::fixed << std::setprecision(1) << (static_cast<double>(bytes) / 1000) << " KB";
        result = oss.str();
        return result;
    }
    else 
    {
        result = std::to_string(bytes);
        result.append(" bytes");
        return result;
    }

}
unsigned long Utils::getLogicalSize(std::string filename) 
{
    std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);
    if (!file) {
        std::cerr << "Failed to open file\n";
        Utils::error = true;
        return 1;
    }
    return static_cast<unsigned long>(file.tellg());
}

unsigned long Utils::getSizeonDisk(std::string filename)
{
    std::wstring wfilename(filename.begin(), filename.end());

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(wfilename.c_str(), GetFileExInfoStandard, &fad))
    {
        std::cerr << "Failed to get file attributes\n";
        Utils::error = true;
        return 0;
    }

    if (fad.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED || fad.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
    {
        std::cerr << "File is compressed or sparse, physical size might not be accurate\n";
    }

    HANDLE hFile = CreateFile(wfilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to open file\n";
        Utils::error = true;
        return 0;
    }

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(hFile, &info))
    {
        std::cerr << "Failed to get file information\n";
        Utils::error = true;
        CloseHandle(hFile);
        return 0;
    }

    DWORD sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;
    if (!GetDiskFreeSpace(L"C:\\", &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters))
    {
        std::cerr << "Failed to get disk information\n";
        Utils::error = true;
        CloseHandle(hFile);
        return 0;
    }

    unsigned long physicalSize = info.nFileSizeHigh * (MAXDWORD)+info.nFileSizeLow;
    physicalSize = ((physicalSize + bytesPerSector - 1) / bytesPerSector) * bytesPerSector;

    CloseHandle(hFile);
    return physicalSize;
}