#include "adaptive_math/statistics/Statistics.h"
#include "adaptive_math/core/SkillDAG.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace adaptive_math {

Statistics::Statistics(SkillManager& skillManager)
    : skillManager_(skillManager) {}

std::string Statistics::formatPercent(double ratio) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << (ratio * 100.0) << "%";
    return oss.str();
}

std::string Statistics::formatTime(double seconds) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << seconds << "s";
    return oss.str();
}

std::string Statistics::progressBar(double ratio, int width) {
    ratio = std::clamp(ratio, 0.0, 1.0);
    int filled = static_cast<int>(ratio * width);
    std::string bar;
    for (int i = 0; i < width; ++i) {
        bar += (i < filled) ? "#" : "-";
    }
    return "[" + bar + "]";
}

std::string Statistics::skillReport(SkillID id) const {
    const auto& stats = skillManager_.getSkillStats(id);
    const auto& record = skillManager_.getSkillRecord(id);
    bool unlocked = skillManager_.getUnlockedSkills().end() !=
        std::find(skillManager_.getUnlockedSkills().begin(),
                  skillManager_.getUnlockedSkills().end(), id);

    std::ostringstream oss;
    oss << "=== " << skillIDToString(id) << " ===\n";
    oss << "  Status:    " << (unlocked ? "UNLOCKED" : "LOCKED") << "\n";
    oss << "  Attempts:  " << stats.attempts << "\n";
    oss << "  Correct:   " << stats.correct << "\n";
    oss << "  Accuracy:  " << formatPercent(stats.accuracy())
        << " " << progressBar(stats.accuracy()) << "\n";
    oss << "  Avg Time:  " << formatTime(stats.averageTime())
        << " (par: " << formatTime(stats.parTime) << ")\n";
    oss << "  Priority:  " << std::fixed << std::setprecision(3)
        << stats.priorityScore() << "\n";

    if (stats.attempts > 0) {
        oss << "  Records:\n";
        oss << "    Best Accuracy: " << formatPercent(record.bestAccuracy) << "\n";
        oss << "    Best Streak:   " << record.bestStreak << "\n";
        if (record.fastestAvgTime > 0) {
            oss << "    Fastest Avg:   " << formatTime(record.fastestAvgTime) << "\n";
        }
    }

    // Show prerequisites
    oss << "  " << skillManager_.dag().describePath(id) << "\n";

    return oss.str();
}

std::string Statistics::fullReport() const {
    std::ostringstream oss;
    oss << "\n========================================\n";
    oss << "       FULL SKILL ANALYSIS REPORT\n";
    oss << "========================================\n\n";

    auto order = skillManager_.dag().topologicalOrder();
    for (SkillID id : order) {
        oss << skillReport(id) << "\n";
    }

    // Global records
    const auto& gr = skillManager_.globalRecord();
    oss << "========================================\n";
    oss << "         GLOBAL RECORDS\n";
    oss << "========================================\n";
    oss << "  Total Sessions:  " << gr.totalSessions << "\n";
    oss << "  Total Questions: " << gr.totalQuestions << "\n";
    if (gr.overallBestAccuracy > 0) {
        oss << "  Best Accuracy:   " << formatPercent(gr.overallBestAccuracy) << "\n";
    }
    if (gr.overallBestStreak > 0) {
        oss << "  Best Streak:     " << gr.overallBestStreak << "\n";
    }

    return oss.str();
}

std::string Statistics::sessionSummary(const SessionSummary& summary) const {
    std::ostringstream oss;
    oss << "\n========================================\n";
    oss << "          SESSION SUMMARY\n";
    oss << "========================================\n";
    oss << "  Questions:  " << summary.totalQuestions << "\n";
    oss << "  Correct:    " << summary.correctAnswers << "\n";
    oss << "  Accuracy:   " << formatPercent(summary.accuracy)
        << " " << progressBar(summary.accuracy) << "\n";
    oss << "  Time:       " << formatTime(summary.totalTime) << "\n";
    oss << "  Streak:     " << summary.currentStreak
        << " (best: " << summary.bestStreak << ")\n";
    oss << "========================================\n";
    return oss.str();
}

std::string Statistics::skillComparisonTable() const {
    std::ostringstream oss;
    oss << "\n+------------------------------+----------+----------+----------+\n";
    oss << "| Skill                        | Accuracy | Avg Time | Priority |\n";
    oss << "+------------------------------+----------+----------+----------+\n";

    auto order = skillManager_.dag().topologicalOrder();
    for (SkillID id : order) {
        const auto& stats = skillManager_.getSkillStats(id);
        std::string name = skillIDToString(id);

        // Pad/truncate name to 28 chars
        if (name.size() > 28) name = name.substr(0, 28);
        name += std::string(28 - name.size(), ' ');

        oss << "| " << name << " | "
            << std::setw(8) << formatPercent(stats.accuracy()) << " | "
            << std::setw(8) << formatTime(stats.averageTime()) << " | "
            << std::setw(8) << std::fixed << std::setprecision(3)
            << stats.priorityScore() << " |\n";
    }

    oss << "+------------------------------+----------+----------+----------+\n";
    return oss.str();
}

std::string Statistics::skillTreeDisplay() const {
    std::ostringstream oss;
    oss << "\n========================================\n";
    oss << "           SKILL TREE (DAG)\n";
    oss << "========================================\n\n";

    auto unlocked = skillManager_.getUnlockedSkills();

    // Display in topological order with indentation based on depth
    auto topoOrder = skillManager_.dag().topologicalOrder();

    for (SkillID id : topoOrder) {
        bool isUnlocked = std::find(unlocked.begin(), unlocked.end(), id) != unlocked.end();
        const auto& stats = skillManager_.getSkillStats(id);
        const auto& prereqs = skillManager_.dag().getPrerequisites(id);

        // Determine depth by number of prerequisite levels
        std::string indent(prereqs.size() * 2, ' ');

        std::string status = isUnlocked ? "[UNLOCKED]" : "[LOCKED]  ";
        std::string accStr = (stats.attempts > 0)
            ? formatPercent(stats.accuracy())
            : "---";

        oss << indent << status << " " << skillIDToString(id)
            << " (" << accStr << ")\n";
    }

    oss << "\n  Unlock threshold: "
        << static_cast<int>(skillManager_.dag().masteryThreshold() * 100)
        << "% accuracy with min 5 attempts\n";

    return oss.str();
}

} // namespace adaptive_math
