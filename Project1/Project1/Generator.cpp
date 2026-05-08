#include "Generator.h"
#include <ctime>
#include <cstdio>
#include <algorithm>

static const int k_diameters[] = { 100, 150, 200, 300 };

Generator::Generator(const Config& cfg)
    : cfg_(cfg)
    , priority_dist_({ 10.0, 60.0, 25.0, 5.0 })   // LOW, NORMAL, HIGH, URGENT
    , material_dist_(0, 3)
    , process_dist_(0, 6)
    , diameter_idx_dist_(0, 3)
    , quantity_dist_(1, 25)
{
    unsigned seed = cfg.use_fixed_seed
        ? cfg.seed
        : static_cast<unsigned>(time(nullptr));
    rng_.seed(seed);

    start_epoch_ = parse_datetime(cfg.start_time);
    end_epoch_   = cfg.end_time.empty()
        ? static_cast<long long>(time(nullptr))
        : parse_datetime(cfg.end_time);

    if (start_epoch_ > end_epoch_)
        std::swap(start_epoch_, end_epoch_);

    time_dist_ = std::uniform_int_distribution<long long>(start_epoch_, end_epoch_);

    build_pools();
}

void Generator::build_pools() {
    // ~1 lot per 5 orders — gives realistic sharing across orders
    int lot_count = std::max(1, cfg_.count / 5);

    struct tm t = {};
    time_t e = static_cast<time_t>(start_epoch_);
    gmtime_s(&t, &e);
    int year = t.tm_year + 1900;

    lot_pool_.reserve(lot_count);
    for (int i = 1; i <= lot_count; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "LOT-%04d-%04d", year, i);
        lot_pool_.push_back(buf);
    }

    for (int i = 1; i <= 15; ++i) {
        char buf[16];
        snprintf(buf, sizeof(buf), "ENG-%03d", i);
        requester_pool_.push_back(buf);
    }
}

std::vector<OrderEvent> Generator::generate() {
    std::vector<OrderEvent> events;
    events.reserve(cfg_.count);

    std::uniform_int_distribution<int> lot_dist(0, static_cast<int>(lot_pool_.size()) - 1);
    std::uniform_int_distribution<int> req_dist(0, static_cast<int>(requester_pool_.size()) - 1);

    for (int i = 0; i < cfg_.count; ++i) {
        long long epoch    = pick_epoch();
        Priority  priority = pick_priority();
        int       diam     = pick_diameter();

        Order o;
        o.order_id     = fmt_order_id(next_order_id_++);
        o.order_time   = epoch_to_datetime(epoch);
        o.requester_id = requester_pool_[req_dist(rng_)];
        o.priority     = priority;
        o.due_date     = compute_due_date(epoch, priority);
        o.status       = Status::PENDING;

        Sample s;
        s.sample_id    = fmt_sample_id(next_sample_id_++);
        s.lot_id       = lot_pool_[lot_dist(rng_)];
        s.material     = pick_material();
        s.diameter_mm  = diam;
        s.thickness_um = pick_thickness(diam);
        s.process_step = pick_process_step();
        s.quantity     = pick_quantity();

        events.push_back({ o, s });
    }

    // Sort chronologically so the output file reads naturally
    std::sort(events.begin(), events.end(), [](const OrderEvent& a, const OrderEvent& b) {
        return a.order.order_time < b.order.order_time;
    });

    return events;
}

Priority Generator::pick_priority() {
    switch (priority_dist_(rng_)) {
    case 0:  return Priority::LOW;
    case 1:  return Priority::NORMAL;
    case 2:  return Priority::HIGH;
    default: return Priority::URGENT;
    }
}

Material Generator::pick_material() {
    return static_cast<Material>(material_dist_(rng_));
}

ProcessStep Generator::pick_process_step() {
    return static_cast<ProcessStep>(process_dist_(rng_));
}

int Generator::pick_diameter() {
    return k_diameters[diameter_idx_dist_(rng_)];
}

int Generator::pick_thickness(int diameter_mm) {
    switch (diameter_mm) {
    case 100: return 525;
    case 150: return 675;
    case 200: return 725;
    case 300: {
        static const int opts[] = { 775, 800, 825 };
        std::uniform_int_distribution<int> d(0, 2);
        return opts[d(rng_)];
    }
    }
    return 725;
}

int Generator::pick_quantity() {
    return quantity_dist_(rng_);
}

long long Generator::pick_epoch() {
    return time_dist_(rng_);
}

std::string Generator::compute_due_date(long long order_epoch, Priority p) {
    int min_days, max_days;
    switch (p) {
    case Priority::URGENT: min_days = 1;  max_days = 3;  break;
    case Priority::HIGH:   min_days = 3;  max_days = 7;  break;
    case Priority::NORMAL: min_days = 7;  max_days = 21; break;
    case Priority::LOW:    min_days = 14; max_days = 30; break;
    default:               min_days = 7;  max_days = 21; break;
    }
    std::uniform_int_distribution<int> d(min_days, max_days);
    long long due_epoch = order_epoch + static_cast<long long>(d(rng_)) * 86400LL;
    return epoch_to_date(due_epoch);
}

std::string Generator::fmt_order_id(int n) {
    char buf[16];
    snprintf(buf, sizeof(buf), "ORD-%05d", n);
    return buf;
}

std::string Generator::fmt_sample_id(int n) {
    char buf[16];
    snprintf(buf, sizeof(buf), "SMP-%05d", n);
    return buf;
}

std::string Generator::epoch_to_datetime(long long epoch) const {
    struct tm t = {};
    time_t e = static_cast<time_t>(epoch);
    gmtime_s(&t, &e);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &t);
    return buf;
}

std::string Generator::epoch_to_date(long long epoch) const {
    struct tm t = {};
    time_t e = static_cast<time_t>(epoch);
    gmtime_s(&t, &e);
    char buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
    return buf;
}

long long Generator::parse_datetime(const std::string& dt) const {
    struct tm t = {};
    int year = 0, mon = 0, mday = 0, hour = 0, min = 0, sec = 0;
    sscanf_s(dt.c_str(), "%d-%d-%dT%d:%d:%d",
        &year, &mon, &mday, &hour, &min, &sec);
    t.tm_year = year - 1900;
    t.tm_mon  = mon - 1;
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min  = min;
    t.tm_sec  = sec;
    t.tm_isdst = 0;
    return static_cast<long long>(_mkgmtime(&t));
}
