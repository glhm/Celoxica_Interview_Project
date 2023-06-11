#ifndef UNIQUE_ID_GENERATOR
#define UNIQUE_ID_GENERATOR

#include <cstdint>
#include <atomic>
#include <random>

/*
 * The class provides a method to get a unique 32 bits,
 * it assures a random number to up to 6 threads calling the method every seconds for 24h
 */
class UniqueIdGenerator
{
public:
    UniqueIdGenerator();

    /**
     * Return a unique 32 bits integer which is a combination of a 13 bits random number and a 19 bits counter
     * Up to 6 threads can increment the counter 3600 times per hour during 24 hours : 6*3600*24 = 518400 calls (2^19 = 524288)
     * The rest of the id (32-19 = 13 bits ) is a random number generated at construction to make sure to get a new unique id even after several lauch
     * if the counter has reach max size it returns to 0 after generation of a new random number
     *
     */
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
