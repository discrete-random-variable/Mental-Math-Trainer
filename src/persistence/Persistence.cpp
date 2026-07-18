#include "adaptive_math/persistence/Persistence.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace adaptive_math {

namespace fs = std::filesystem;

Persistence::Persistence(const std::string& dataDir)
    : dataDir_(dataDir) {}

std::string Persistence::saveFilePath() const {
    return dataDir_ + "/progress.json";
}

bool Persistence::ensureDataDirectory() const {
    try {
        if (!fs::exists(dataDir_)) {
            fs::create_directories(dataDir_);
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating data directory: " << e.what() << "\n";
        return false;
    }
}

bool Persistence::saveStatistics(const SkillManager& manager) const {
    if (!ensureDataDirectory()) return false;

    try {
        nlohmann::json j;

        // Save per-skill statistics
        nlohmann::json statsJson = nlohmann::json::object();
        for (const auto& [id, stats] : manager.allStats()) {
            statsJson[std::to_string(static_cast<int>(id))] = stats;
        }
        j["skillStats"] = statsJson;

        // Save per-skill records
        nlohmann::json recordsJson = nlohmann::json::object();
        for (const auto& [id, record] : manager.allRecords()) {
            recordsJson[std::to_string(static_cast<int>(id))] = record;
        }
        j["skillRecords"] = recordsJson;

        // Save global records
        j["globalRecord"] = manager.globalRecord();

        // Write to file
        std::ofstream file(saveFilePath());
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for writing: "
                      << saveFilePath() << "\n";
            return false;
        }

        file << j.dump(2);
        file.close();
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error saving statistics: " << e.what() << "\n";
        return false;
    }
}

bool Persistence::loadStatistics(SkillManager& manager) const {
    if (!hasSaveData()) return false;

    try {
        std::ifstream file(saveFilePath());
        if (!file.is_open()) return false;

        nlohmann::json j;
        file >> j;
        file.close();

        // Load per-skill statistics
        if (j.contains("skillStats")) {
            for (const auto& [key, value] : j["skillStats"].items()) {
                int idInt = std::stoi(key);
                if (idInt >= 0 && idInt < static_cast<int>(skillCount())) {
                    SkillID id = static_cast<SkillID>(idInt);
                    manager.getSkillStats(id) = value.get<SkillStats>();
                }
            }
        }

        // Load per-skill records
        if (j.contains("skillRecords")) {
            for (const auto& [key, value] : j["skillRecords"].items()) {
                int idInt = std::stoi(key);
                if (idInt >= 0 && idInt < static_cast<int>(skillCount())) {
                    SkillID id = static_cast<SkillID>(idInt);
                    manager.getSkillRecord(id) = value.get<SkillRecord>();
                }
            }
        }

        // Load global records
        if (j.contains("globalRecord")) {
            manager.globalRecord() = j["globalRecord"].get<GlobalRecord>();
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error loading statistics: " << e.what() << "\n";
        return false;
    }
}

bool Persistence::hasSaveData() const {
    return fs::exists(saveFilePath());
}

bool Persistence::deleteSaveData() const {
    try {
        if (fs::exists(saveFilePath())) {
            fs::remove(saveFilePath());
        }
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error deleting save data: " << e.what() << "\n";
        return false;
    }
}

} // namespace adaptive_math
