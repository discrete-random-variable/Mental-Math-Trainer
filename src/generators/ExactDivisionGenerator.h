#pragma once

#include "adaptive_math/core/IQuestionGenerator.h"
#include <random>

namespace adaptive_math {

// Generates exact division questions (no remainder).
// Easy:   single-digit / single-digit (from tables)
// Medium: 2-digit / single-digit
// Hard:   3-digit / single-digit
class ExactDivisionGenerator : public IQuestionGenerator {
public:
    ExactDivisionGenerator() : rng_(std::random_device{}()) {}

    Question generate(DifficultyLevel difficulty) override {
        switch (difficulty) {
            case DifficultyLevel::Easy:   return generateEasy();
            case DifficultyLevel::Medium: return generateMedium();
            case DifficultyLevel::Hard:   return generateHard();
        }
        return generateEasy();
    }

    SkillID getSkillID() const override {
        return SkillID::ExactDivision;
    }

private:
    mutable std::mt19937 rng_;

    // O(1) exact division: generate divisor and quotient, compute dividend
    Question generateEasy() {
        // quotient (1-9), divisor (1-9) => dividend = q * d
        std::uniform_int_distribution<int> dist(1, 9);
        int quotient = dist(rng_);
        int divisor  = dist(rng_);
        int dividend = quotient * divisor;
        return {dividend, divisor, Operation::Divide, quotient,
                SkillID::ExactDivision, DifficultyLevel::Easy};
    }

    Question generateMedium() {
        // 2-digit dividend, 1-digit divisor, exact
        std::uniform_int_distribution<int> distQ(2, 20);
        std::uniform_int_distribution<int> distD(2, 9);
        int quotient = distQ(rng_);
        int divisor  = distD(rng_);
        int dividend = quotient * divisor;
        return {dividend, divisor, Operation::Divide, quotient,
                SkillID::ExactDivision, DifficultyLevel::Medium};
    }

    Question generateHard() {
        // 3-digit dividend, 1-digit divisor, exact
        std::uniform_int_distribution<int> distQ(12, 111);
        std::uniform_int_distribution<int> distD(2, 9);
        int quotient = distQ(rng_);
        int divisor  = distD(rng_);
        int dividend = quotient * divisor;
        return {dividend, divisor, Operation::Divide, quotient,
                SkillID::ExactDivision, DifficultyLevel::Hard};
    }
};

} // namespace adaptive_math
