#pragma once

#include "adaptive_math/core/IQuestionGenerator.h"
#include <random>

namespace adaptive_math {

// Generates multiplication questions focusing on times tables.
// Easy:   single-digit x single-digit (tables 1-5)
// Medium: single-digit x single-digit (tables 6-9)
// Hard:   2-digit x 1-digit
class TablesMultiplicationGenerator : public IQuestionGenerator {
public:
    TablesMultiplicationGenerator() : rng_(std::random_device{}()) {}

    Question generate(DifficultyLevel difficulty) override {
        switch (difficulty) {
            case DifficultyLevel::Easy:   return generateEasy();
            case DifficultyLevel::Medium: return generateMedium();
            case DifficultyLevel::Hard:   return generateHard();
        }
        return generateEasy();
    }

    SkillID getSkillID() const override {
        return SkillID::TablesMultiplication;
    }

private:
    mutable std::mt19937 rng_;

    Question generateEasy() {
        // Single-digit x single-digit, factors 1-5
        std::uniform_int_distribution<int> dist(1, 5);
        int a = dist(rng_);
        int b = dist(rng_);
        return {a, b, Operation::Multiply, a * b,
                SkillID::TablesMultiplication, DifficultyLevel::Easy};
    }

    Question generateMedium() {
        // Single-digit x single-digit, factors 6-9
        std::uniform_int_distribution<int> dist(6, 9);
        int a = dist(rng_);
        int b = dist(rng_);
        return {a, b, Operation::Multiply, a * b,
                SkillID::TablesMultiplication, DifficultyLevel::Medium};
    }

    Question generateHard() {
        // 2-digit x 1-digit
        std::uniform_int_distribution<int> dist2d(10, 99);
        std::uniform_int_distribution<int> dist1d(2, 9);
        int a = dist2d(rng_);
        int b = dist1d(rng_);
        return {a, b, Operation::Multiply, a * b,
                SkillID::TablesMultiplication, DifficultyLevel::Hard};
    }
};

} // namespace adaptive_math
