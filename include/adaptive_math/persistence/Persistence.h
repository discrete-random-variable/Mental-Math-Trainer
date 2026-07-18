#pragma once

#include "adaptive_math/core/SkillManager.h"
#include <string>
#include <optional>

namespace adaptive_math {

// Handles saving and loading of user progress via JSON.
// Self-documenting API: saveStatistics/loadStatistics
// instead of generic save/load names.
class Persistence {
public:
    explicit Persistence(const std::string& dataDir = "data");

    // Save all statistics and records
    bool saveStatistics(const SkillManager& manager) const;

    // Load statistics and records into the manager
    bool loadStatistics(SkillManager& manager) const;

    // Check if save data exists
    bool hasSaveData() const;

    // Delete save data
    bool deleteSaveData() const;

    // Get the data directory path
    const std::string& dataDirectory() const { return dataDir_; }

private:
    std::string dataDir_;
    std::string saveFilePath() const;

    // Ensure the data directory exists
    bool ensureDataDirectory() const;
};

} // namespace adaptive_math
