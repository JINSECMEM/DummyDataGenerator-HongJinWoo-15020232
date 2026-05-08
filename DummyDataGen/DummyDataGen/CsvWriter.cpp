#include "CsvWriter.h"
#include <fstream>
#include <ctime>
#include <filesystem>

CsvWriter::CsvWriter(const std::string& out_dir) : out_dir_(out_dir) {}

std::string CsvWriter::make_filename(int count) const {
    time_t now = time(nullptr);
    struct tm t = {};
    gmtime_s(&t, &now);
    char buf[64];
    strftime(buf, sizeof(buf), "dummy_%Y%m%d_%H%M%S", &t);
    return out_dir_ + "/" + std::string(buf) + "_" + std::to_string(count) + ".csv";
}

std::string CsvWriter::write(const std::vector<OrderEvent>& events) {
    std::filesystem::create_directories(out_dir_);

    std::string path = make_filename(static_cast<int>(events.size()));
    std::ofstream ofs(path);

    ofs << "order_id,order_time,requester_id,priority,due_date,status,"
           "sample_id,lot_id,material,diameter_mm,thickness_um,process_step,quantity\n";

    for (const auto& ev : events) {
        const Order&  o = ev.order;
        const Sample& s = ev.sample;
        ofs << o.order_id              << ','
            << o.order_time            << ','
            << o.requester_id          << ','
            << to_string(o.priority)   << ','
            << o.due_date              << ','
            << to_string(o.status)     << ','
            << s.sample_id             << ','
            << s.lot_id                << ','
            << to_string(s.material)   << ','
            << s.diameter_mm           << ','
            << s.thickness_um          << ','
            << to_string(s.process_step) << ','
            << s.quantity              << '\n';
    }

    return path;
}
