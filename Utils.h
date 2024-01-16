#pragma once
#include <iostream>

class Utils
{
public:
	static unsigned long getLogicalSize(std::string file);
	static unsigned long getSizeonDisk(std::string file);
	static std::string wPrefix(unsigned long bytes);
	static bool error;
};

