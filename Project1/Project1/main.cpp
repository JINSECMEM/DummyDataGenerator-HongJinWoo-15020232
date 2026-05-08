#include "Config.h"
#include "Generator.h"
#include "CsvWriter.h"
#include <iostream>
#include <string>
#include <stdexcept>

static void print_usage(const char* prog) {
    std::cout <<
        "Usage: " << prog << " [options]\n"
        "  --count N          Records to generate          (default: 100)\n"
        "  --seed S           Random seed for repeatability (default: current time)\n"
        "  --start-time T     Order time range start, ISO 8601 (default: 2026-01-01T00:00:00)\n"
        "  --end-time T       Order time range end,   ISO 8601 (default: now)\n"
        "  --out-dir PATH     Output directory               (default: ./output)\n"
        "  --help             Show this message\n";
}

int main(int argc, char* argv[]) {
    Config cfg;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto need_next = [&]() -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << arg << " requires a value\n";
                std::exit(1);
            }
            return argv[++i];
        };

        if      (arg == "--count")      { cfg.count = std::stoi(need_next()); }
        else if (arg == "--seed")       { cfg.seed = static_cast<unsigned>(std::stoul(need_next())); cfg.use_fixed_seed = true; }
        else if (arg == "--start-time") { cfg.start_time = need_next(); }
        else if (arg == "--end-time")   { cfg.end_time   = need_next(); }
        else if (arg == "--out-dir")    { cfg.out_dir    = need_next(); }
        else if (arg == "--help" || arg == "-h") { print_usage(argv[0]); return 0; }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (cfg.count <= 0) {
        std::cerr << "Error: --count must be a positive integer\n";
        return 1;
    }

    try {
        Generator gen(cfg);
        auto events = gen.generate();

        CsvWriter writer(cfg.out_dir);
        std::string out_path = writer.write(events);

        std::cout << "Generated " << events.size() << " records -> " << out_path << "\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
