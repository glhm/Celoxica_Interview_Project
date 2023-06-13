#ifndef UNIQUE_ID_GENERATOR
#define UNIQUE_ID_GENERATOR

#include <cstdint>
#include <atomic>
#include <random>

/*
 * Provides a method to get a unique 32 bits,
 * it assures a random number for up to 6 threads calling the method every seconds for 24h
 */
class UniqueIdGenerator
{
public:
    /*
     * At construction the object will generates a random number
     */
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
    /**
     *  Counter to be iterated to provide a unique number for differents threads, max value 2^19-1
     *  Atomic type so that two threads can't get the same value
     */
    std::atomic<uint32_t> m_counter{0};

    /**
     *  Random value which will be initialized with a value between 0 and 2^13 - 1
     * Atomic type so that two threads will not reset its value when max size is reached
     */
    std::atomic<uint32_t> m_randomNumber;

    std::default_random_engine m_randomEngine;                    // Random number generator engine
    std::uniform_int_distribution<uint32_t> m_randomDistribution; // Distribution for random number generation
    const uint32_t m_randomNumberBits{13};                        // number of bits of the random number
    const uint32_t m_counterBits{19};                             // number of bits of the counter
    const uint32_t m_maxValueCounter;                             // max value for the counter (2^)
    const uint32_t m_maxValueRandom;                              // max value for the random number
};

#endif
