#pragma once

#include "adaptive_math/core/SkillManager.h"
#include <string>
#include <vector>

namespace adaptive_math {

// Analytics wrapper over SkillManager.
// Provides high-level queries and formatted reports.
// Session never edits maps directly — it goes through this class.
class Statistics {
public:
    explicit Statistics(SkillManager& skillManager);

    // Per-skill formatted report
    std::string skillReport(SkillID id) const;

    // Full report of all skills
    std::string fullReport() const;

    // Session summary (questions answered, accuracy, time)
    struct SessionSummary {
        int totalQuestions;
        int correctAnswers;
        double accuracy;
        double totalTime;
        int currentStreak;
        int bestStreak;
    };

    std::string sessionSummary(const SessionSummary& summary) const;

    // Skill comparison table
    std::string skillComparisonTable() const;

    // Skill tree display showing locked/unlocked status
    std::string skillTreeDisplay() const;

    // Get the underlying manager (for persistence)
    SkillManager& manager() { return skillManager_; }
    const SkillManager& manager() const { return skillManager_; }

private:
    SkillManager& skillManager_;

    static std::string formatPercent(double ratio);
    static std::string formatTime(double seconds);
    static std::string progressBar(double ratio, int width = 20);
};

} // namespace adaptive_math
