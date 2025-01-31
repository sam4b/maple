#pragma once
#include <cstdint>
#include <random>

static struct random {
	random() : rd(), gen(rd()), dis() {};
	std::random_device rd;
	std::mt19937_64 gen;
	std::uniform_int_distribution<uint64_t> dis;
} my_gen;

inline uint64_t generateUUID() {
	return my_gen.dis(my_gen.gen);
}