#include <cstdint>
#include <random>

class mwc256xxa64
{
	const uint64_t MULT = 0xfeb3'4465'7c0a'f413;

	inline static uint64_t x2;
	inline static uint64_t x3;
	inline static __uint128_t c_x1 = __uint128_t(0x1405'7B7E'F767'814F) << 64 | 23456;

public:
	mwc256xxa64()
	{
		x2 = std::random_device{}();
		x3 = std::random_device{}();
	}
	mwc256xxa64(uint64_t seed1, uint64_t seed2)
	{
		x2 = seed1;
		x3 = seed2;
	}

	using result_type = uint64_t;

	uint64_t operator()()
	{
		__uint128_t x = (__uint128_t)(x3)*MULT;
		uint64_t result = (x3 ^ x2) + ((uint64_t)(c_x1) ^ (uint64_t)(x >> 64));
		x3 = x2;
		x2 = (uint64_t)(c_x1);
		c_x1 = x + (c_x1 >> 64);
		return result;
	}
};
