#pragma once

#include <vector>
#include <filesystem>

class IVRPath
{
public:
	//purpose of this class is to provide a way to get cross platform paths to files used by the engine
	static std::string GetCrossPlatformPath(const std::vector<std::string>& path_vector)
	{
		//convert filepath vector into a path using filesystem
		std::filesystem::path filepath = std::filesystem::current_path();

		#ifdef __linux__
			filepath /= "..";
		#endif

		for (std::string path : path_vector)
		{
			filepath /= path;
		}

		return filepath.string();
	}
};