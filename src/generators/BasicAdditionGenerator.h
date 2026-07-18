#pragma once

#include "adaptive_math/core/IQuestionGenerator.h"
#include <random>

namespace adaptive_math {

// Generates addition questions where no digit sum exceeds 9 (no carry).
// Easy:   1-digit + 1-digit
// Medium: 2-digit + 2-digit (no carry)
// Hard:   3-digit + 3-digit (no carry)
class BasicAdditionGenerator : public IQuestionGenerator {
public:
    BasicAdditionGenerator() : rng_(std::random_device{}()) {}

    Question generate(DifficultyLevel difficulty) override {
        switch (difficulty) {
            case DifficultyLevel::Easy:   return generateEasy();
            case DifficultyLevel::Medium: return generateMedium();
            case DifficultyLevel::Hard:   return generateHard();
        }
        return generateEasy();
    }

    SkillID getSkillID() const override {
        return SkillID::BasicAddition;
    }

private:
    mutable std::mt19937 rng_;

    Question generateEasy() {
        // 1-digit + 1-digit, sum < 10 (no carry)
        std::uniform_int_distribution<int> dist_a(0, 9);
        int a = dist_a(rng_);
        std::uniform_int_distribution<int> dist_b(0, 9 - a);
        int b = dist_b(rng_);
        return {a, b, Operation::Add, a + b,
                SkillID::BasicAddition, DifficultyLevel::Easy};
    }

    Question generateMedium() {
        // 2-digit + 2-digit, no carry at any position
        std::uniform_int_distribution<int> tens(0, 4);
        std::uniform_int_distribution<int> ones(0, 9);

        int t1 = tens(rng_);
        int o1 = ones(rng_);
        int t2 = std::uniform_int_distribution<int>(0, 9 - t1)(rng_);
        int o2 = std::uniform_int_distribution<int>(0, 9 - o1)(rng_);

        int a = t1 * 10 + o1;
        int b = t2 * 10 + o2;
        return {a, b, Operation::Add, a + b,
                SkillID::BasicAddition, DifficultyLevel::Medium};
    }

    Question generateHard() {
        // 3-digit + 3-digit, no carry at any position
        std::uniform_int_distribution<int> digit(0, 9);

        int h1 = std::uniform_int_distribution<int>(0, 4)(rng_);
        int t1 = digit(rng_);
        int o1 = digit(rng_);
        int h2 = std::uniform_int_distribution<int>(0, 9 - h1)(rng_);
        int t2 = std::uniform_int_distribution<int>(0, 9 - t1)(rng_);
        int o2 = std::uniform_int_distribution<int>(0, 9 - o1)(rng_);

        int a = h1 * 100 + t1 * 10 + o1;
        int b = h2 * 100 + t2 * 10 + o2;
        return {a, b, Operation::Add, a + b,
                SkillID::BasicAddition, DifficultyLevel::Hard};
    }
};

} // namespace adaptive_math
