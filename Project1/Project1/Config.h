#pragma once
#include <string>

struct Config {
    int         count          = 100;
    unsigned    seed           = 0;
    bool        use_fixed_seed = false;
    std::string start_time     = "2026-01-01T00:00:00";
    std::string end_time;        // empty = current time
    std::string out_dir        = "./output";
    bool        verbose        = false;
    int         preview        = 0;   // 0 = off; N > 0 → print first N rows to console
};
