#include "umepch.h"
#include "Utils.h"

#include <random>

namespace Ume
{
	static std::uniform_real_distribution<float> s_Distribution(0.0f, 1.0f);
	static std::default_random_engine s_Generator;

	float RandomFloat()
	{
		return s_Distribution(s_Generator);
	}
	float Lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}
}