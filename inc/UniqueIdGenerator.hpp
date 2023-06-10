#ifndef UNIQUE_ID_GENERATOR
#define UNIQUE_ID_GENERATOR

#include <cstdint>
#include <atomic>
#include <random>

class UniqueIdGenerator
{
public:
    UniqueIdGenerator();
    uint32_t generateID();

private:
    std::atomic<uint32_t> m_counter{0};
    std::atomic<uint32_t> m_randomNumber; // Random value between 0 and 2^13 - 1

    std::default_random_engine m_randomEngine;                    // Random number generator engine
    std::uniform_int_distribution<uint32_t> m_randomDistribution; // Distribution for random number generation
    const uint32_t m_randomNumberBits{13};
    const uint32_t m_counterBits{19};
    const uint32_t m_maxValueCounter;
    const uint32_t m_maxValueRandom;
};

#endif
