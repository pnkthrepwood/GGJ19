#pragma once
#include <random>
#include <functional>
#include <stdint.h>
#include <algorithm>

#include <iostream>
using namespace std;

struct DiceEngine
{
	typedef unsigned int uint;

	std::mt19937 gen;

	unsigned int seed;

	std::uniform_real_distribution<float> uniform;

	std::uniform_int_distribution<uint> distr_coin;
	std::uniform_int_distribution<uint> distr_1d4;
	std::uniform_int_distribution<uint> distr_1d6;
	std::uniform_int_distribution<uint> distr_1d10;
	std::uniform_int_distribution<uint> distr_1d20;

	DiceEngine()
	{
		init();
	}

	void init_distributions()
	{
		uniform = std::uniform_real_distribution<float>(0, 1);

		distr_coin = std::uniform_int_distribution<uint>(1, 2);
		distr_1d4 = std::uniform_int_distribution<uint>(1, 4);
		distr_1d6 = std::uniform_int_distribution<uint>(1, 6);
		distr_1d10 = std::uniform_int_distribution<uint>(1, 10);
		distr_1d20 = std::uniform_int_distribution<uint>(1, 20);
	}

	void init()
	{
		std::random_device rd;
		init(rd());
	}

	void init(unsigned int _seed)
	{
		//seed = 1337;// _seed;
		gen = std::mt19937(seed);

		init_distributions();
	}

	void init(uint64_t _seed)
	{
		//_seed = 1337;
		//seed = 1337;
		std::mt19937_64 proxy(_seed); // cutre hash
		std::uniform_int_distribution<unsigned int> dist;
		init(dist(proxy));
	}


	inline uint roll_flipcoin() { return distr_coin(gen) - 1; }
	inline uint roll_1d4() { return distr_1d4(gen); }
	inline uint roll_1d6() { return distr_1d6(gen); }
	inline uint roll_1d10() { return distr_1d10(gen); }
	inline uint roll_1d20() { return distr_1d20(gen); }

	inline uint roll(uint min, uint max) { return std::uniform_int_distribution<uint>(min, max)(gen); }
	inline uint roll(uint max) { return roll(0, max - 1); }

	inline uint64_t roll64(uint64_t min, uint64_t max) { return std::uniform_int_distribution<uint64_t>(min, max)(gen); }
	inline uint64_t roll64(uint64_t max) { return roll(1, (uint)max); }

	inline float rollf(float min, float max) { return std::uniform_real_distribution<float>(min, max)(gen); }
	inline float rollf(float max) { return rollf(0, max); }
	inline float rollf() { return uniform(gen); }

	template<typename _T>
	void shuffle(_T *values, size_t count)
	{
		std::shuffle(values, values + (count - 1), gen);
	}

	template<typename _T, size_t _Count>
	void shuffle(_T(&values)[_Count])
	{
		return shuffle<_T>(values, _Count);
	}
};

namespace Fun
{
	extern DiceEngine Random;
	/*
	typedef unsigned int uint;
	inline uint roll_flipcoin() { return Random.roll_flipcoin(); }
	inline uint roll_1d4() { return Random.roll_1d4(); }
	inline uint roll_1d6() { return Random.roll_1d6(); }
	inline uint roll_1d10() { return Random.roll_1d10(); }
	inline uint roll_1d20() { return Random.roll_1d20(); }

	inline uint roll(uint min, uint max) { return Random.roll(min, max); }
	inline uint roll(uint max) { return roll(1, max); }

	inline uint64_t roll64(uint64_t min, uint64_t max) { return Random.roll64(min, max); }
	inline uint64_t roll64(uint64_t max) { return roll(1, (uint)max); }

	inline float rollf(float min, float max) { return Random.rollf(min, max); }
	inline float rollf(float max) { return rollf(1, max); }

	template<typename _T>
	void shuffle(_T *values, size_t count)
	{
		Random.shuffle(values, count);
	}

	template<typename _T, size_t _Count>
	void shuffle(_T(&values)[_Count])
	{
		Random.shuffle(values);
	}
	*/
}
