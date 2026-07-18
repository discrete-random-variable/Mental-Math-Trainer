#pragma once

#include "SkillID.h"
#include <nlohmann/json.hpp>
#include <map>
#include <ctime>

namespace adaptive_math {

struct SkillRecord {
    double bestAccuracy     = 0.0;  // best accuracy ratio (0.0 - 1.0)
    int    bestStreak       = 0;
    double fastestAvgTime   = 0.0;  // in seconds, 0 means no record

    static SkillRecord computeFrom(const SkillRecord& current, double accuracy,
                                    int streak, double avgTime) {
        SkillRecord updated = current;
        if (accuracy > updated.bestAccuracy) updated.bestAccuracy = accuracy;
        if (streak > updated.bestStreak)     updated.bestStreak = streak;
        if (updated.fastestAvgTime == 0.0 || avgTime < updated.fastestAvgTime) {
            if (avgTime > 0.0) updated.fastestAvgTime = avgTime;
        }
        return updated;
    }
};

struct GlobalRecord {
    int    overallBestStreak    = 0;
    double overallBestAccuracy  = 0.0;
    double overallFastestAvg    = 0.0;
    int    totalSessions        = 0;
    int    totalQuestions       = 0;
};

// JSON serialization for SkillRecord
inline void to_json(nlohmann::json& j, const SkillRecord& r) {
    j = nlohmann::json{
        {"bestAccuracy",    r.bestAccuracy},
        {"bestStreak",      r.bestStreak},
        {"fastestAvgTime",  r.fastestAvgTime}
    };
}

inline void from_json(const nlohmann::json& j, SkillRecord& r) {
    j.at("bestAccuracy").get_to(r.bestAccuracy);
    j.at("bestStreak").get_to(r.bestStreak);
    j.at("fastestAvgTime").get_to(r.fastestAvgTime);
}

// JSON serialization for GlobalRecord
inline void to_json(nlohmann::json& j, const GlobalRecord& r) {
    j = nlohmann::json{
        {"overallBestStreak",   r.overallBestStreak},
        {"overallBestAccuracy", r.overallBestAccuracy},
        {"overallFastestAvg",   r.overallFastestAvg},
        {"totalSessions",       r.totalSessions},
        {"totalQuestions",      r.totalQuestions}
    };
}

inline void from_json(const nlohmann::json& j, GlobalRecord& r) {
    j.at("overallBestStreak").get_to(r.overallBestStreak);
    j.at("overallBestAccuracy").get_to(r.overallBestAccuracy);
    j.at("overallFastestAvg").get_to(r.overallFastestAvg);
    j.at("totalSessions").get_to(r.totalSessions);
    j.at("totalQuestions").get_to(r.totalQuestions);
}

} // namespace adaptive_math
