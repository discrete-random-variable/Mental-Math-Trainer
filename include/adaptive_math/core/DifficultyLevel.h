#pragma once

#include <string>

namespace adaptive_math {

enum class DifficultyLevel {
    Easy,
    Medium,
    Hard
};

inline std::string difficultyToString(DifficultyLevel d) {
    switch (d) {
        case DifficultyLevel::Easy:   return "Easy";
        case DifficultyLevel::Medium: return "Medium";
        case DifficultyLevel::Hard:   return "Hard";
    }
    return "Unknown";
}

} // namespace adaptive_math
