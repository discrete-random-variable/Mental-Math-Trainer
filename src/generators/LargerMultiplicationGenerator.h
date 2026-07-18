#pragma once

#include "adaptive_math/core/IQuestionGenerator.h"
#include <random>

namespace adaptive_math {

// Generates larger multiplication questions.
// Easy:   2-digit x 1-digit
// Medium: 2-digit x 2-digit
// Hard:   3-digit x 2-digit
class LargerMultiplicationGenerator : public IQuestionGenerator {
public:
    LargerMultiplicationGenerator() : rng_(std::random_device{}()) {}

    Question generate(DifficultyLevel difficulty) override {
        switch (difficulty) {
            case DifficultyLevel::Easy:   return generateEasy();
            case DifficultyLevel::Medium: return generateMedium();
            case DifficultyLevel::Hard:   return generateHard();
        }
        return generateEasy();
    }

    SkillID getSkillID() const override {
        return SkillID::LargerMultiplication;
    }

private:
    mutable std::mt19937 rng_;

    Question generateEasy() {
        // 2-digit x 1-digit
        std::uniform_int_distribution<int> d2(10, 99);
        std::uniform_int_distribution<int> d1(2, 9);
        int a = d2(rng_);
        int b = d1(rng_);
        return {a, b, Operation::Multiply, a * b,
                SkillID::LargerMultiplication, DifficultyLevel::Easy};
    }

    Question generateMedium() {
        // 2-digit x 2-digit
        std::uniform_int_distribution<int> d2(11, 49);
        int a = d2(rng_);
        int b = d2(rng_);
        return {a, b, Operation::Multiply, a * b,
                SkillID::LargerMultiplication, DifficultyLevel::Medium};
    }

    Question generateHard() {
        // 3-digit x 2-digit
        std::uniform_int_distribution<int> d3(100, 499);
        std::uniform_int_distribution<int> d2(11, 49);
        int a = d3(rng_);
        int b = d2(rng_);
        return {a, b, Operation::Multiply, a * b,
                SkillID::LargerMultiplication, DifficultyLevel::Hard};
    }
};

} // namespace adaptive_math
