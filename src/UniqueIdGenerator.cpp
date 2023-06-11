#include <chrono>
#include <iostream>
#include "UniqueIdGenerator.hpp"

UniqueIdGenerator::UniqueIdGenerator() : m_maxValueCounter((1u << m_counterBits) - 1), m_maxValueRandom((1u << m_randomNumberBits) - 1)
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto seed = currentTime.time_since_epoch().count();
    m_randomEngine.seed(static_cast<uint32_t>(seed));                                                  // Seed the random engine with the current time
    m_randomDistribution = std::uniform_int_distribution<uint32_t>(0, (1u << m_randomNumberBits) - 1); // Set up the distribution for random number generation (0 to 2^13 - 1)
    m_randomNumber = m_randomDistribution(m_randomEngine);                                             // Generate the initial random value
    std::cout << "Random Number for this execution : " << m_randomNumber << std::endl;
}

uint32_t UniqueIdGenerator::generateID()
{
    auto start = std::chrono::high_resolution_clock::now();
    /*- Shift the value of m_randomNumber to the left by m_counterBits positions. This effectively creates space to welcome m_counter.
      - Bitwise OR operator to combine the shifted random number with the counter value.
      - m_counter.fetch_add(1):  increments it by 1, and returns the original value.
    */
    uint32_t id = (m_randomNumber << m_counterBits) | m_counter.fetch_add(1);
    if (m_counter >= m_maxValueCounter)
    {
        m_counter = 0;
        m_randomNumber = m_randomDistribution(m_randomEngine);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    std::cout << "Id Generated " << m_randomNumber << ": Runtime duration of generateID(): " << duration.count() << " nanoseconds" << std::endl;

    return id;
}
