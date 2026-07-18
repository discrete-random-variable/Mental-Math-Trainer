#pragma once

#include "adaptive_math/core/IQuestionGenerator.h"
#include <random>
#include <algorithm>

namespace adaptive_math {

// Generates addition questions with at least one carry.
// Easy:   2-digit + 2-digit, exactly 1 carry position
// Medium: 3-digit + 3-digit, exactly 2 carry positions
// Hard:   3-digit + 3-digit, all positions carry
class CarryAdditionGenerator : public IQuestionGenerator {
public:
    CarryAdditionGenerator() : rng_(std::random_device{}()) {}

    Question generate(DifficultyLevel difficulty) override {
        switch (difficulty) {
            case DifficultyLevel::Easy:   return generateEasy();
            case DifficultyLevel::Medium: return generateMedium();
            case DifficultyLevel::Hard:   return generateHard();
        }
        return generateEasy();
    }

    SkillID getSkillID() const override {
        return SkillID::CarryAddition;
    }

private:
    mutable std::mt19937 rng_;

    Question generateEasy() {
        // 2-digit + 2-digit with exactly 1 carry
        // Choose which position carries (ones or tens)
        std::uniform_int_distribution<int> which(0, 1);
        int carryPos = which(rng_);

        int t1, o1, t2, o2;

        if (carryPos == 0) {
            // Carry at ones, no carry at tens (accounting for carry-in)
            o1 = std::uniform_int_distribution<int>(1, 9)(rng_);
            o2 = std::uniform_int_distribution<int>(10 - o1, 9)(rng_);
            // tens must not produce carry even with +1 from ones carry
            t1 = std::uniform_int_distribution<int>(0, 8)(rng_);
            t2 = std::uniform_int_distribution<int>(0, 8 - t1)(rng_);
            // ensure tens + tens + 1 < 10
            if (t1 + t2 + 1 >= 10) t2 = std::max(0, 8 - t1);
        } else {
            // No carry at ones, carry at tens
            o1 = std::uniform_int_distribution<int>(0, 8)(rng_);
            o2 = std::uniform_int_distribution<int>(0, 8 - o1)(rng_);
            // tens must carry
            t1 = std::uniform_int_distribution<int>(1, 9)(rng_);
            t2 = std::uniform_int_distribution<int>(10 - t1, 9)(rng_);
        }

        int a = t1 * 10 + o1;
        int b = t2 * 10 + o2;
        return {a, b, Operation::Add, a + b,
                SkillID::CarryAddition, DifficultyLevel::Easy};
    }

    Question generateMedium() {
        // 3-digit + 3-digit with exactly 2 carry positions
        // Ones carry, tens carry, hundreds no carry
        int o1 = std::uniform_int_distribution<int>(1, 9)(rng_);
        int o2 = std::uniform_int_distribution<int>(10 - o1, 9)(rng_);

        // Tens carry (including possible carry from ones)
        int t1 = std::uniform_int_distribution<int>(0, 9)(rng_);
        // We need t1 + t2 + 1 >= 10 (carry from ones always present)
        int minT2 = std::max(0, 9 - t1); // t1 + t2 + 1 >= 10 => t2 >= 9-t1
        if (minT2 > 9) minT2 = 9;
        int t2 = std::uniform_int_distribution<int>(minT2, 9)(rng_);

        // Hundreds must not carry (even with carry from tens)
        int h1 = std::uniform_int_distribution<int>(0, 4)(rng_);
        int maxH2 = 8 - h1; // h1 + h2 + 1 <= 9
        if (maxH2 < 0) maxH2 = 0;
        int h2 = std::uniform_int_distribution<int>(0, maxH2)(rng_);

        int a = h1 * 100 + t1 * 10 + o1;
        int b = h2 * 100 + t2 * 10 + o2;
        return {a, b, Operation::Add, a + b,
                SkillID::CarryAddition, DifficultyLevel::Medium};
    }

    Question generateHard() {
        // 3-digit + 3-digit with all positions carrying
        int o1 = std::uniform_int_distribution<int>(1, 9)(rng_);
        int o2 = std::uniform_int_distribution<int>(10 - o1, 9)(rng_);

        // Tens must carry (with carry-in from ones)
        int t1 = std::uniform_int_distribution<int>(0, 9)(rng_);
        int minT2 = std::max(0, 9 - t1);
        int t2 = std::uniform_int_distribution<int>(minT2, 9)(rng_);

        // Hundreds must carry (with carry-in from tens)
        int h1 = std::uniform_int_distribution<int>(1, 9)(rng_);
        int minH2 = std::max(0, 9 - h1);
        int h2 = std::uniform_int_distribution<int>(minH2, 9)(rng_);

        int a = h1 * 100 + t1 * 10 + o1;
        int b = h2 * 100 + t2 * 10 + o2;
        return {a, b, Operation::Add, a + b,
                SkillID::CarryAddition, DifficultyLevel::Hard};
    }
};

} // namespace adaptive_math
