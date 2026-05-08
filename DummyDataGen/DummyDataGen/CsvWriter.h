#pragma once
#include "DataModel.h"
#include <string>
#include <vector>

class CsvWriter {
public:
    explicit CsvWriter(const std::string& out_dir);

    // Writes events to a timestamped CSV file and returns the full path.
    std::string write(const std::vector<OrderEvent>& events);

private:
    std::string out_dir_;
    std::string make_filename(int count) const;
};
