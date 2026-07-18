#pragma once

#include "SkillID.h"
#include "SkillStats.h"
#include "PersonalRecord.h"
#include "SkillDAG.h"
#include <unordered_map>
#include <map>
#include <vector>
#include <cstddef>

namespace adaptive_math {

// Owns all SkillStats and PersonalRecords.
// Provides a clean API for recording attempts and querying performance.
// Separates data management from selection logic (SRP).
class SkillManager {
public:
    SkillManager();

    // Record a practice attempt for a skill
    void recordAttempt(SkillID id, bool wasCorrect, double responseTime);

    // Access skill statistics
    SkillStats& getSkillStats(SkillID id);
    const SkillStats& getSkillStats(SkillID id) const;

    // Access personal records
    SkillRecord& getSkillRecord(SkillID id);
    const SkillRecord& getSkillRecord(SkillID id) const;

    // Global records
    GlobalRecord& globalRecord();
    const GlobalRecord& globalRecord() const;

    // Query methods
    double overallAccuracy() const;
    std::vector<std::pair<SkillID, double>> getWeakestSkills(int count) const;
    std::vector<SkillID> getUnlockedSkills() const;

    // Get all skill IDs with their stats
    const std::unordered_map<SkillID, SkillStats>& allStats() const;
    const std::map<SkillID, SkillRecord>& allRecords() const;

    // Reset all stats (fresh start)
    void reset();

    // Session tracking
    void startSession();
    void endSession(int questionsAnswered);

    // DAG access
    const SkillDAG& dag() const { return dag_; }

private:
    std::unordered_map<SkillID, SkillStats> stats_;
    std::map<SkillID, SkillRecord> records_;
    GlobalRecord globalRecord_;
    SkillDAG dag_;
};

} // namespace adaptive_math
