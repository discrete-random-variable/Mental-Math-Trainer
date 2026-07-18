#include "adaptive_math/core/SkillManager.h"
#include <algorithm>
#include <cmath>

namespace adaptive_math {

SkillManager::SkillManager() {
    // Initialize stats and records for all skills with appropriate par times
    for (std::size_t i = 0; i < skillCount(); ++i) {
        SkillID id = indexToSkillID(i);
        stats_[id] = SkillStats{};

        // Set per-skill par times (expected time for skilled performance)
        switch (id) {
            case SkillID::BasicAddition:        stats_[id].parTime = 3.0; break;
            case SkillID::CarryAddition:        stats_[id].parTime = 5.0; break;
            case SkillID::BasicSubtraction:     stats_[id].parTime = 3.0; break;
            case SkillID::BorrowSubtraction:    stats_[id].parTime = 6.0; break;
            case SkillID::TablesMultiplication: stats_[id].parTime = 3.0; break;
            case SkillID::LargerMultiplication: stats_[id].parTime = 8.0; break;
            case SkillID::ExactDivision:        stats_[id].parTime = 4.0; break;
            case SkillID::LargerDivision:       stats_[id].parTime = 10.0; break;
            case SkillID::COUNT: break;
        }

        records_[id] = SkillRecord{};
    }
}

void SkillManager::recordAttempt(SkillID id, bool wasCorrect, double responseTime) {
    auto it = stats_.find(id);
    if (it == stats_.end()) return;

    it->second.recordAttempt(wasCorrect, responseTime);

    // Update personal records
    SkillRecord& rec = records_[id];
    double acc = it->second.accuracy();
    double avgTime = it->second.averageTime();

    if (acc > rec.bestAccuracy) rec.bestAccuracy = acc;
    if (rec.fastestAvgTime == 0.0 || (avgTime > 0.0 && avgTime < rec.fastestAvgTime)) {
        rec.fastestAvgTime = avgTime;
    }

    // Update global records
    if (wasCorrect) {
        globalRecord_.totalQuestions++;
    } else {
        globalRecord_.totalQuestions++;
    }

    double overallAcc = overallAccuracy();
    if (overallAcc > globalRecord_.overallBestAccuracy) {
        globalRecord_.overallBestAccuracy = overallAcc;
    }
}

SkillStats& SkillManager::getSkillStats(SkillID id) {
    return stats_[id];
}

const SkillStats& SkillManager::getSkillStats(SkillID id) const {
    auto it = stats_.find(id);
    if (it != stats_.end()) return it->second;
    static SkillStats dummy;
    return dummy;
}

SkillRecord& SkillManager::getSkillRecord(SkillID id) {
    return records_[id];
}

const SkillRecord& SkillManager::getSkillRecord(SkillID id) const {
    auto it = records_.find(id);
    if (it != records_.end()) return it->second;
    static SkillRecord dummy;
    return dummy;
}

GlobalRecord& SkillManager::globalRecord() {
    return globalRecord_;
}

const GlobalRecord& SkillManager::globalRecord() const {
    return globalRecord_;
}

double SkillManager::overallAccuracy() const {
    int totalAttempts = 0;
    int totalCorrect  = 0;
    for (const auto& [id, stat] : stats_) {
        totalAttempts += stat.attempts;
        totalCorrect  += stat.correct;
    }
    return (totalAttempts == 0) ? 0.0
                                : static_cast<double>(totalCorrect) / totalAttempts;
}

std::vector<std::pair<SkillID, double>> SkillManager::getWeakestSkills(int count) const {
    std::vector<std::pair<SkillID, double>> scored;
    for (const auto& [id, stat] : stats_) {
        scored.emplace_back(id, stat.priorityScore());
    }
    std::partial_sort(scored.begin(), scored.begin() + std::min(count, static_cast<int>(scored.size())),
                      scored.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });
    scored.resize(std::min(count, static_cast<int>(scored.size())));
    return scored;
}

std::vector<SkillID> SkillManager::getUnlockedSkills() const {
    return dag_.getUnlockedSkills(stats_);
}

const std::unordered_map<SkillID, SkillStats>& SkillManager::allStats() const {
    return stats_;
}

const std::map<SkillID, SkillRecord>& SkillManager::allRecords() const {
    return records_;
}

void SkillManager::reset() {
    for (auto& [id, stat] : stats_) {
        stat = SkillStats{};
        // Restore par times
        switch (id) {
            case SkillID::BasicAddition:        stat.parTime = 3.0; break;
            case SkillID::CarryAddition:        stat.parTime = 5.0; break;
            case SkillID::BasicSubtraction:     stat.parTime = 3.0; break;
            case SkillID::BorrowSubtraction:    stat.parTime = 6.0; break;
            case SkillID::TablesMultiplication: stat.parTime = 3.0; break;
            case SkillID::LargerMultiplication: stat.parTime = 8.0; break;
            case SkillID::ExactDivision:        stat.parTime = 4.0; break;
            case SkillID::LargerDivision:       stat.parTime = 10.0; break;
            case SkillID::COUNT: break;
        }
    }
    for (auto& [id, rec] : records_) {
        rec = SkillRecord{};
    }
    globalRecord_ = GlobalRecord{};
}

void SkillManager::startSession() {
    globalRecord_.totalSessions++;
}

void SkillManager::endSession(int questionsAnswered) {
    // Session end tracking handled elsewhere
    (void)questionsAnswered;
}

} // namespace adaptive_math
