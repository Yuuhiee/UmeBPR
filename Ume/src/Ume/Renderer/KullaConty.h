#pragma once

#include <string>

namespace Ume
{
	class LUT
	{
	public:
		enum class Type
		{
			Emu = 0,
			Eavg = 1,
		};

		static void PrecomputeEmu(const std::string& filepath, int resolution = 128);
		static void PrecomputeEavg(const std::string& filepath, const std::string& emuPath, int resolution = 128);
	private:
		static bool FileExist(const std::string& filepath, int width, int height, int channels);
	};
}