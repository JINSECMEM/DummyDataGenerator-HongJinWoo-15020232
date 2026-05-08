#pragma once
#include <string>

enum class Material    { Si, SiC, GaAs, GaN };
enum class ProcessStep { BARE, OXIDATION, LITHOGRAPHY, ETCH, DEPOSITION, CMP, INSPECTION };
enum class Priority    { LOW, NORMAL, HIGH, URGENT };
enum class Status      { PENDING, IN_PROGRESS, COMPLETED, CANCELLED };

struct Sample {
    std::string sample_id;
    std::string lot_id;
    Material    material;
    int         diameter_mm;
    int         thickness_um;
    ProcessStep process_step;
    int         quantity;
};

struct Order {
    std::string order_id;
    std::string order_time;   // ISO 8601: YYYY-MM-DDTHH:MM:SS
    std::string requester_id;
    Priority    priority;
    std::string due_date;     // YYYY-MM-DD
    Status      status;
};

struct OrderEvent {
    Order  order;
    Sample sample;
};

inline const char* to_string(Material m) {
    switch (m) {
    case Material::Si:   return "Si";
    case Material::SiC:  return "SiC";
    case Material::GaAs: return "GaAs";
    case Material::GaN:  return "GaN";
    }
    return "";
}

inline const char* to_string(ProcessStep s) {
    switch (s) {
    case ProcessStep::BARE:        return "BARE";
    case ProcessStep::OXIDATION:   return "OXIDATION";
    case ProcessStep::LITHOGRAPHY: return "LITHOGRAPHY";
    case ProcessStep::ETCH:        return "ETCH";
    case ProcessStep::DEPOSITION:  return "DEPOSITION";
    case ProcessStep::CMP:         return "CMP";
    case ProcessStep::INSPECTION:  return "INSPECTION";
    }
    return "";
}

inline const char* to_string(Priority p) {
    switch (p) {
    case Priority::LOW:    return "LOW";
    case Priority::NORMAL: return "NORMAL";
    case Priority::HIGH:   return "HIGH";
    case Priority::URGENT: return "URGENT";
    }
    return "";
}

inline const char* to_string(Status s) {
    switch (s) {
    case Status::PENDING:     return "PENDING";
    case Status::IN_PROGRESS: return "IN_PROGRESS";
    case Status::COMPLETED:   return "COMPLETED";
    case Status::CANCELLED:   return "CANCELLED";
    }
    return "";
}
