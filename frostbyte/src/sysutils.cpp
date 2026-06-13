#include "sysutils.hpp"

#include <chrono>
#include <fstream>
#ifdef __linux__
#include <unistd.h>
#endif

namespace frostbyte {

double SysUtils::sample_frequency_s = 1.0;
double SysUtils::physical_memory_total = 0.0;
double SysUtils::physical_memory_used = 0.0;

auto last_sample_time = std::chrono::system_clock::now();
void SysUtils::run() {
    auto current_time = std::chrono::system_clock::now();
    if (static_cast<std::chrono::duration<double>>(current_time - last_sample_time).count() < sample_frequency_s)
        return;
    last_sample_time = current_time;

    #ifdef __linux__
        {
        physical_memory_total = 0.0;

        std::ifstream f("/proc/meminfo");
        std::string key;
        long long memory_kb;
        while (f >> key >> memory_kb) {
            if (key == "MemTotal:") {
                physical_memory_total = memory_kb / (1024.0 * 1024.0);
                break;
            }
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        }

        {
        physical_memory_used = 0.0;

        std::ifstream f("/proc/self/status");
        std::string key;
        long long memory_kb;
        while (f >> key) {
            if (key.find("VmRSS:") != std::string::npos) {
                f >> memory_kb;
                physical_memory_used = memory_kb / (1024.0 * 1024.0);
                break;
            }
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        }
    #else

    #endif
}

}; // namespace frostbyte
