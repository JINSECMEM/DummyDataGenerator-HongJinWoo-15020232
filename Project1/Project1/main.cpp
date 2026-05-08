#include "Config.h"
#include "Generator.h"
#include "CsvWriter.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <stdexcept>
#include <cstdio>

// ── helpers ──────────────────────────────────────────────────────────────────

static void print_usage(const char* prog) {
    std::cout <<
        "Usage: " << prog << " [options]\n"
        "\n"
        "  --count N          Records to generate               (default: 100)\n"
        "  --seed S           Random seed for repeatability     (default: current time)\n"
        "  --start-time T     Order time range start, ISO 8601  (default: 2026-01-01T00:00:00)\n"
        "  --end-time T       Order time range end,   ISO 8601  (default: now)\n"
        "  --out-dir PATH     Output directory                   (default: ./output)\n"
        "  --verbose          Print generation statistics\n"
        "  --preview N        Print first N records to console\n"
        "  --help             Show this message\n";
}

static bool validate_datetime(const std::string& s) {
    int year = 0, mon = 0, mday = 0, hour = 0, min = 0, sec = 0;
    if (sscanf_s(s.c_str(), "%d-%d-%dT%d:%d:%d",
                 &year, &mon, &mday, &hour, &min, &sec) != 6)
        return false;
    return year >= 1970 && year <= 9999
        && mon  >= 1    && mon  <= 12
        && mday >= 1    && mday <= 31
        && hour >= 0    && hour <= 23
        && min  >= 0    && min  <= 59
        && sec  >= 0    && sec  <= 59;
}

// ── stats ─────────────────────────────────────────────────────────────────────

static void print_stats(const std::vector<OrderEvent>& events) {
    std::map<std::string, int> priority_cnt, material_cnt, process_cnt;

    for (const auto& ev : events) {
        priority_cnt[to_string(ev.order.priority)]++;
        material_cnt[to_string(ev.sample.material)]++;
        process_cnt [to_string(ev.sample.process_step)]++;
    }

    int n = static_cast<int>(events.size());

    auto print_group = [&](const char* label,
                           const std::map<std::string, int>& m) {
        std::cout << "  " << std::left << std::setw(13) << label << ":";
        for (const auto& [k, v] : m) {
            std::cout << "  " << k << " " << v
                      << " (" << std::fixed << std::setprecision(1)
                      << (100.0 * v / n) << "%)";
        }
        std::cout << "\n";
    };

    std::cout << "\n--- Statistics (" << n << " records) ---\n";
    print_group("Priority",    priority_cnt);
    print_group("Material",    material_cnt);
    print_group("ProcessStep", process_cnt);

    if (!events.empty()) {
        std::cout << "  Time range   :  "
                  << events.front().order.order_time << "  ~  "
                  << events.back().order.order_time  << "\n";
    }
    std::cout << "\n";
}

// ── preview ───────────────────────────────────────────────────────────────────

static void print_preview(const std::vector<OrderEvent>& events, int n) {
    int show = std::min(n, static_cast<int>(events.size()));

    // column widths
    const int w_oid  = 10, w_time = 19, w_req = 8, w_pri = 7,
              w_due  = 11, w_sid  = 10, w_lot = 14, w_mat = 5,
              w_dia  =  6, w_thk  =  7, w_prc = 12, w_qty = 4;

    auto sep = [&]() {
        std::cout
            << std::string(w_oid+1,'-') << '+'
            << std::string(w_time+1,'-') << '+'
            << std::string(w_req+1,'-') << '+'
            << std::string(w_pri+1,'-') << '+'
            << std::string(w_due+1,'-') << '+'
            << std::string(w_sid+1,'-') << '+'
            << std::string(w_lot+1,'-') << '+'
            << std::string(w_mat+1,'-') << '+'
            << std::string(w_dia+1,'-') << '+'
            << std::string(w_thk+1,'-') << '+'
            << std::string(w_prc+1,'-') << '+'
            << std::string(w_qty,'-')   << "\n";
    };

    std::cout << "\n--- Preview (first " << show << " records) ---\n";
    sep();
    std::cout << std::left
        << std::setw(w_oid)  << "order_id"   << ' ' << '|'
        << std::setw(w_time) << "order_time" << ' ' << '|'
        << std::setw(w_req)  << "requester"  << ' ' << '|'
        << std::setw(w_pri)  << "priority"   << ' ' << '|'
        << std::setw(w_due)  << "due_date"   << ' ' << '|'
        << std::setw(w_sid)  << "sample_id"  << ' ' << '|'
        << std::setw(w_lot)  << "lot_id"     << ' ' << '|'
        << std::setw(w_mat)  << "mat"        << ' ' << '|'
        << std::setw(w_dia)  << "diam"       << ' ' << '|'
        << std::setw(w_thk)  << "thick"      << ' ' << '|'
        << std::setw(w_prc)  << "process"    << ' ' << '|'
        << std::setw(w_qty)  << "qty"        << "\n";
    sep();

    for (int i = 0; i < show; ++i) {
        const Order&  o = events[i].order;
        const Sample& s = events[i].sample;
        std::cout << std::left
            << std::setw(w_oid)  << o.order_id              << ' ' << '|'
            << std::setw(w_time) << o.order_time             << ' ' << '|'
            << std::setw(w_req)  << o.requester_id           << ' ' << '|'
            << std::setw(w_pri)  << to_string(o.priority)    << ' ' << '|'
            << std::setw(w_due)  << o.due_date               << ' ' << '|'
            << std::setw(w_sid)  << s.sample_id              << ' ' << '|'
            << std::setw(w_lot)  << s.lot_id                 << ' ' << '|'
            << std::setw(w_mat)  << to_string(s.material)    << ' ' << '|'
            << std::setw(w_dia)  << s.diameter_mm            << ' ' << '|'
            << std::setw(w_thk)  << s.thickness_um           << ' ' << '|'
            << std::setw(w_prc)  << to_string(s.process_step)<< ' ' << '|'
            << std::setw(w_qty)  << s.quantity               << "\n";
    }
    sep();
    std::cout << "\n";
}

// ── main ──────────────────────────────────────────────────────────────────────

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
        else if (arg == "--verbose")    { cfg.verbose    = true; }
        else if (arg == "--preview")    { cfg.preview    = std::stoi(need_next()); }
        else if (arg == "--help" || arg == "-h") { print_usage(argv[0]); return 0; }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    // ── validation ────────────────────────────────────────────────────────────
    if (cfg.count <= 0) {
        std::cerr << "Error: --count must be a positive integer\n";
        return 1;
    }
    if (!validate_datetime(cfg.start_time)) {
        std::cerr << "Error: --start-time must be ISO 8601 (YYYY-MM-DDTHH:MM:SS), got: "
                  << cfg.start_time << "\n";
        return 1;
    }
    if (!cfg.end_time.empty() && !validate_datetime(cfg.end_time)) {
        std::cerr << "Error: --end-time must be ISO 8601 (YYYY-MM-DDTHH:MM:SS), got: "
                  << cfg.end_time << "\n";
        return 1;
    }
    if (cfg.preview < 0) {
        std::cerr << "Error: --preview must be a non-negative integer\n";
        return 1;
    }

    // ── generate & write ──────────────────────────────────────────────────────
    try {
        Generator gen(cfg);
        auto events = gen.generate();

        if (cfg.preview > 0)
            print_preview(events, cfg.preview);

        if (cfg.verbose)
            print_stats(events);

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
