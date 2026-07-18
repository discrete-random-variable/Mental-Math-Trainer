#pragma once

#include "adaptive_math/core/IQuestionGenerator.h"
#include <random>

namespace adaptive_math {

// Generates subtraction questions where no borrowing is needed.
// Easy:   1-digit - 1-digit
// Medium: 2-digit - 2-digit (no borrow)
// Hard:   3-digit - 3-digit (no borrow)
class BasicSubtractionGenerator : public IQuestionGenerator {
public:
    BasicSubtractionGenerator() : rng_(std::random_device{}()) {}

    Question generate(DifficultyLevel difficulty) override {
        switch (difficulty) {
            case DifficultyLevel::Easy:   return generateEasy();
            case DifficultyLevel::Medium: return generateMedium();
            case DifficultyLevel::Hard:   return generateHard();
        }
        return generateEasy();
    }

    SkillID getSkillID() const override {
        return SkillID::BasicSubtraction;
    }

private:
    mutable std::mt19937 rng_;

    Question generateEasy() {
        // 1-digit - 1-digit, result >= 0
        std::uniform_int_distribution<int> dist_a(1, 9);
        int a = dist_a(rng_);
        std::uniform_int_distribution<int> dist_b(0, a);
        int b = dist_b(rng_);
        return {a, b, Operation::Subtract, a - b,
                SkillID::BasicSubtraction, DifficultyLevel::Easy};
    }

    Question generateMedium() {
        // 2-digit - 2-digit, no borrow at any position
        // Each digit of minuend >= corresponding digit of subtrahend
        std::uniform_int_distribution<int> digit(0, 9);

        int t1 = std::uniform_int_distribution<int>(1, 9)(rng_);
        int o1 = digit(rng_);
        int t2 = std::uniform_int_distribution<int>(0, t1)(rng_);
        int o2 = std::uniform_int_distribution<int>(0, o1)(rng_);

        int a = t1 * 10 + o1;
        int b = t2 * 10 + o2;
        return {a, b, Operation::Subtract, a - b,
                SkillID::BasicSubtraction, DifficultyLevel::Medium};
    }

    Question generateHard() {
        // 3-digit - 3-digit, no borrow
        std::uniform_int_distribution<int> digit(0, 9);

        int h1 = std::uniform_int_distribution<int>(1, 9)(rng_);
        int t1 = digit(rng_);
        int o1 = digit(rng_);
        int h2 = std::uniform_int_distribution<int>(0, h1)(rng_);
        int t2 = std::uniform_int_distribution<int>(0, t1)(rng_);
        int o2 = std::uniform_int_distribution<int>(0, o1)(rng_);

        int a = h1 * 100 + t1 * 10 + o1;
        int b = h2 * 100 + t2 * 10 + o2;
        return {a, b, Operation::Subtract, a - b,
                SkillID::BasicSubtraction, DifficultyLevel::Hard};
    }
};

} // namespace adaptive_math
