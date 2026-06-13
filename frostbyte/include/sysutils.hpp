#pragma once

namespace frostbyte {

class SysUtils {
    static double sample_frequency_s;
public:
    static double physical_memory_total;
    static double physical_memory_used;

    static void run();
};

}; // namespace frostbyte
