#pragma once

#include <string>
#include <array>
#include <cstddef>

namespace adaptive_math {

enum class SkillID {
    BasicAddition,
    CarryAddition,
    BasicSubtraction,
    BorrowSubtraction,
    TablesMultiplication,
    LargerMultiplication,
    ExactDivision,
    LargerDivision,
    COUNT  // sentinel for iteration
};

inline std::string skillIDToString(SkillID id) {
    switch (id) {
        case SkillID::BasicAddition:        return "Basic Addition";
        case SkillID::CarryAddition:        return "Carry Addition";
        case SkillID::BasicSubtraction:     return "Basic Subtraction";
        case SkillID::BorrowSubtraction:    return "Borrow Subtraction";
        case SkillID::TablesMultiplication: return "Tables Multiplication";
        case SkillID::LargerMultiplication: return "Larger Multiplication";
        case SkillID::ExactDivision:        return "Exact Division";
        case SkillID::LargerDivision:       return "Larger Division";
        case SkillID::COUNT:                return "COUNT";
    }
    return "Unknown";
}

inline constexpr std::size_t skillCount() {
    return static_cast<std::size_t>(SkillID::COUNT);
}

inline SkillID indexToSkillID(std::size_t i) {
    return static_cast<SkillID>(i);
}

} // namespace adaptive_math
