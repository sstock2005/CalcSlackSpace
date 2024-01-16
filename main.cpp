#include <iostream>
#include <string>
#include <algorithm>
#include "Utils.h"

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