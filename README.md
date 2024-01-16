
# CalcSlackSpace

A simple calculator program written in C++ with the goal to calculate a file's slack space on Windows.

Note: This will only work on Windows

Try it out: [Releases Page](https://github.com/sstock2005/CalcSlackSpace/releases/latest)
## TL;DR

I am going to give a decently detailed explanation of how this program works not only for you but for myself, I am learning C++ so some of the functions used (especially WinAPI) seemed advanced to me and I want to document how to use them in the future.

### Utils.h
This is the header file for the Utils class. It contains the following:

| Type | Name | Parameter | Returns |
|---|---|---|---|
| Unsigned Long | getLogicalSize | std::string filename| Logical Size of File  |
| Unsigned Long | getSizeonDisk | std::string filename | Physical Size of File |
| std::string | wPrefix | unsigned long bytes | Formatted Storage String |
| bool | error | None | Whether or not there has been an error |

**getLogicalSize**  
Returns the logical size of the file.  

**getSizeonDisk**  
Returns the physical size or the size on disk of the file.

**wPrefix**  
Returns a formatted string representing the given bytes.

**error**  
Stores whether or not the program has had an error while running.


### Utils.cpp
This is the real part of the Utils class, it contains the logic behind the Utils functions.  

```cpp
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
```

**What does this do?**  
This function intakes an std::string named filename. On line 1, the function creates an `std::ifstream` object named file. `std::ifstream` is a class in C++ used for reading from files. The constructor takes two arguments: the name of the file to open and the mode to open the file in. `std::ifstream::ate` opens the file and seeks to the end of the file, and `std::ifstream::binary` opens the file in binary mode.  

The function then checks if the object could not be created. If the file object could not be created, the function reports the error and returns.

Finally, the function returns the static casted unsigned long from `file.tellg()`. `file.tellg()` is the current file pointer. Because the file object was created with `std::ifstream::ate`, the current file pointer is the logical size of the file.

```cpp
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
```

**Oh Boy..**  
This is the real meat of the program. This calculates the physical size of a file or the size of the file on disk. This interacts heavily with the Windows API so it can be confusing to understand.

**What does this do?**  
`std::wstring wfilename(filename.begin(), filename.end());`  
Converts the given filename to wide character string because the Windows API requires weird things.  

`WIN32_FILE_ATTRIBUTE_DATA fad;`  
Declares a `WIN32_FILE_ATTRIBUTE_DATA` structure that will hold file attribute data.  

`if (!GetFileAttributesEx(wfilename.c_str(), GetFileExInfoStandard, &fad)) { ... }`  
This calls the `GetFileAttributesEx` function to get the attributes of the file. If the function fails, it prints an error message, sets `Utils::error` to `true`, and returns `0`.  

`if (fad.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED || fad.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) { ... }`  
This checks if the file is compressed or sparse. If it is, it prints a warning message because the physical size might not be accurate for these types of files.  

`HANDLE hFile = CreateFile(wfilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);`  
This calls the `CreateFile` function to open the file and get a handle to it.  

`if (hFile == INVALID_HANDLE_VALUE) { ... }`  
This checks if the `CreateFile` function succeeded. If it failed, it prints an error message, sets `Utils::error` to `true`, and returns `0`.  

`BY_HANDLE_FILE_INFORMATION info;`  
This declares a `BY_HANDLE_FILE_INFORMATION` structure that will hold file information.  

`if (!GetFileInformationByHandle(hFile, &info)) { ... }`  
This calls the `GetFileInformationByHandle` function to get information about the file. If the function fails, it prints an error message, sets `Utils::error` to `true`, closes the file handle, and returns `0`.  

`DWORD sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;`  
This declares several `DWORD` variables that will hold disk information.  

`if (!GetDiskFreeSpace(L"C:\\", &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters)) { ... }`  
This line calls the `GetDiskFreeSpace` function to get information about the disk. If the function fails, it prints an error message, sets `Utils::error` to `true`, closes the file handle, and returns `0`.  

`unsigned long physicalSize = info.nFileSizeHigh * (MAXDWORD)+info.nFileSizeLow;`  
This calculates the size of the file in bytes.  

`physicalSize = ((physicalSize + bytesPerSector - 1) / bytesPerSector) * bytesPerSector;`  
This rounds the size of the file up to the nearest multiple of the sector size. This is the physical size of the file on disk.  

`CloseHandle(hFile);`  
Closes the file handle.  

`return physicalSize;`  
Returns the physical size of the file or the size on disk.  

### Main.cpp
This is the main function of the program, it is pretty straightforward.  

```cpp
int main(int argc, char * argv[])
{
	std::string filename;
	unsigned long logicalSize;
	unsigned long sizeOnDisk;
	unsigned long slackSpace;

	if (argv[1] == NULL)
	{
		std::cout << "File: ";
		std::getline(std::cin, filename);
		std::cout << "\n";
	}
	else 
	{
		filename = argv[1];
	}

	if (filename.find("\"") != std::string::npos) 
	{
		filename.erase(std::remove(filename.begin(), filename.end(), '\"'), filename.end());
	}

	logicalSize = Utils::getLogicalSize(filename);
	sizeOnDisk = Utils::getSizeonDisk(filename);

	if (Utils::error == true) 
	{
		return 0;
	}

	slackSpace = sizeOnDisk - logicalSize;

	std::cout << "Logical Size: " << Utils::wPrefix(logicalSize) << " (" << logicalSize << " bytes)\n";
	std::cout << "Physical Size (size on disk): " << Utils::wPrefix(sizeOnDisk) << " (" << sizeOnDisk << " bytes)\n";
	std::cout << "Slack Space: " << Utils::wPrefix(slackSpace) << " (" << slackSpace << " bytes)\n";
	return 0;
}
```

**What does this do?**  
This function creates an `std::string filename`, and 3 `unsigned long` variables named `logicalSize`, `sizeOnDisk`, and `slackSpace`. It then checks if an argument was passed when the program was executed, if there was no argument passed, the program then asks for user input and assigns that to the filename, otherwise if there was, the program assigns the value of the first argument to the `filename` variable. 

The program then removes any " from the filename because of the way dropping a file with spaces work on Windows. The program then assigns `localSize` and `sizeOnDisk` to the calculates values from `Utils::getLogicalSize` and `Utils::getSizeonDisk`. It then calculates the `slackSpace` by subtracting the physical size of the file by the logical size of the file. 

The program then checks if the `Utils::error` bool is set to `true`, the program exists if it is. Then the program prints out the formatted strings of `logicalSize`, `sizeOnDisk`, and `slackSpace`.