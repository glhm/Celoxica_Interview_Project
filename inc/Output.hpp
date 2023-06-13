
#ifndef SHARED_COUT
#define SHARED_COUT

#include <mutex>
#include <iostream>

std::mutex cout_mtx; // mutex to avoid access to output from different threads to break integrity from messages

/**
 * Class providing synchronization for console outputing
 */
class Output
{
public:
    /*
     * Synchronised print to maintain integrity of output
     * @param message, message to display on console
     */

    static void shared_print(const std::string &str)
    {
        // lock mutex the mutex to access console ouput
        std::lock_guard<std::mutex> guard(cout_mtx);
        std::cout << str << std::endl;
        // release
    }
};

#endif