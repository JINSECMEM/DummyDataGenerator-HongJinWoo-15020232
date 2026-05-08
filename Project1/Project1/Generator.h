#pragma once
#include "DataModel.h"
#include "Config.h"
#include <vector>
#include <random>
#include <string>

class Generator {
public:
    explicit Generator(const Config& cfg);
    std::vector<OrderEvent> generate();

private:
    Config       cfg_;
    std::mt19937 rng_;
    long long    start_epoch_ = 0;
    long long    end_epoch_   = 0;

    int next_order_id_  = 1;
    int next_sample_id_ = 1;

    std::vector<std::string> lot_pool_;
    std::vector<std::string> requester_pool_;

    std::discrete_distribution<int>          priority_dist_;
    std::uniform_int_distribution<int>       material_dist_;
    std::uniform_int_distribution<int>       process_dist_;
    std::uniform_int_distribution<int>       diameter_idx_dist_;
    std::uniform_int_distribution<int>       quantity_dist_;
    std::uniform_int_distribution<long long> time_dist_;

    void        build_pools();
    Priority    pick_priority();
    Material    pick_material();
    ProcessStep pick_process_step();
    int         pick_diameter();
    int         pick_thickness(int diameter_mm);
    int         pick_quantity();
    long long   pick_epoch();
    std::string compute_due_date(long long order_epoch, Priority p);

    std::string fmt_order_id(int n);
    std::string fmt_sample_id(int n);
    std::string epoch_to_datetime(long long epoch) const;
    std::string epoch_to_date(long long epoch) const;
    long long   parse_datetime(const std::string& dt) const;
};
