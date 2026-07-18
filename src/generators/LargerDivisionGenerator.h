#pragma once

#include "adaptive_math/core/IQuestionGenerator.h"
#include <random>

namespace adaptive_math {

// Generates larger division questions with 2-digit divisors (exact).
// Easy:   2-digit / 2-digit
// Medium: 3-digit / 2-digit
// Hard:   4-digit / 2-digit
class LargerDivisionGenerator : public IQuestionGenerator {
public:
    LargerDivisionGenerator() : rng_(std::random_device{}()) {}

    Question generate(DifficultyLevel difficulty) override {
        switch (difficulty) {
            case DifficultyLevel::Easy:   return generateEasy();
            case DifficultyLevel::Medium: return generateMedium();
            case DifficultyLevel::Hard:   return generateHard();
        }
        return generateEasy();
    }

    SkillID getSkillID() const override {
        return SkillID::LargerDivision;
    }

private:
    mutable std::mt19937 rng_;

    Question generateEasy() {
        // 2-digit / 2-digit, quotient 1-9
        std::uniform_int_distribution<int> distQ(1, 9);
        std::uniform_int_distribution<int> distD(10, 19);
        int quotient = distQ(rng_);
        int divisor  = distD(rng_);
        int dividend = quotient * divisor;
        return {dividend, divisor, Operation::Divide, quotient,
                SkillID::LargerDivision, DifficultyLevel::Easy};
    }

    Question generateMedium() {
        // 3-digit / 2-digit
        std::uniform_int_distribution<int> distQ(2, 49);
        std::uniform_int_distribution<int> distD(10, 49);
        int quotient = distQ(rng_);
        int divisor  = distD(rng_);
        int dividend = quotient * divisor;
        return {dividend, divisor, Operation::Divide, quotient,
                SkillID::LargerDivision, DifficultyLevel::Medium};
    }

    Question generateHard() {
        // 4-digit / 2-digit
        std::uniform_int_distribution<int> distQ(10, 499);
        std::uniform_int_distribution<int> distD(10, 49);
        int quotient = distQ(rng_);
        int divisor  = distD(rng_);
        int dividend = quotient * divisor;
        return {dividend, divisor, Operation::Divide, quotient,
                SkillID::LargerDivision, DifficultyLevel::Hard};
    }
};

} // namespace adaptive_math
