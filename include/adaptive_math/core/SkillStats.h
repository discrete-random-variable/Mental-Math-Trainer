#pragma once

#include "SkillID.h"
#include <nlohmann/json.hpp>
#include <cstddef>
#include <ctime>
#include <chrono>
#include <string>

namespace adaptive_math {

struct SkillStats {
    int attempts       = 0;
    int correct        = 0;
    double totalTime   = 0.0;   // total seconds spent answering
    std::time_t lastPracticed = 0;
    double parTime      = 5.0;  // expected time for a skilled person (seconds)

    double accuracy() const {
        return (attempts == 0) ? 0.0 : static_cast<double>(correct) / attempts;
    }

    double averageTime() const {
        return (attempts == 0) ? 0.0 : totalTime / attempts;
    }

    double normalizedResponseTime() const {
        if (attempts == 0) return 1.0;
        double capped = std::min(averageTime(), parTime);
        return capped / parTime;
    }

    // Priority score: higher means more need for practice
    double priorityScore() const {
        return 0.6 * (1.0 - accuracy()) + 0.4 * normalizedResponseTime();
    }

    void recordAttempt(bool wasCorrect, double responseTime) {
        attempts++;
        if (wasCorrect) correct++;
        totalTime += responseTime;
        lastPracticed = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
    }
};

// nlohmann/json serialization
inline void to_json(nlohmann::json& j, const SkillStats& s) {
    j = nlohmann::json{
        {"attempts",       s.attempts},
        {"correct",        s.correct},
        {"totalTime",      s.totalTime},
        {"lastPracticed",  static_cast<std::int64_t>(s.lastPracticed)},
        {"parTime",        s.parTime}
    };
}

inline void from_json(const nlohmann::json& j, SkillStats& s) {
    j.at("attempts").get_to(s.attempts);
    j.at("correct").get_to(s.correct);
    j.at("totalTime").get_to(s.totalTime);
    s.lastPracticed = static_cast<std::time_t>(j.at("lastPracticed").get<std::int64_t>());
    j.at("parTime").get_to(s.parTime);
}

} // namespace adaptive_math
