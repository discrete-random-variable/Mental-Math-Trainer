#pragma once

#include "adaptive_math/core/IQuestionGenerator.h"
#include <random>
#include <algorithm>

namespace adaptive_math {

// Generates subtraction questions requiring borrowing.
// Easy:   2-digit - 2-digit, 1 borrow position
// Medium: 3-digit - 3-digit, 2 borrow positions
// Hard:   3-digit - 3-digit, all positions borrow
class BorrowSubtractionGenerator : public IQuestionGenerator {
public:
    BorrowSubtractionGenerator() : rng_(std::random_device{}()) {}

    Question generate(DifficultyLevel difficulty) override {
        switch (difficulty) {
            case DifficultyLevel::Easy:   return generateEasy();
            case DifficultyLevel::Medium: return generateMedium();
            case DifficultyLevel::Hard:   return generateHard();
        }
        return generateEasy();
    }

    SkillID getSkillID() const override {
        return SkillID::BorrowSubtraction;
    }

private:
    mutable std::mt19937 rng_;

    Question generateEasy() {
        // 2-digit - 2-digit, borrow at ones but not at tens
        // Ones: o1 < o2 (triggers borrow)
        // Tens: t1 > t2 (after borrow, t1-1 >= t2)
        int o1 = std::uniform_int_distribution<int>(0, 8)(rng_);
        int o2 = std::uniform_int_distribution<int>(o1 + 1, 9)(rng_);

        // t1 must be > t2 + 0 (since borrow reduces t1 by 1)
        // so t1 - 1 >= t2, meaning t1 >= t2 + 1
        int t2 = std::uniform_int_distribution<int>(0, 7)(rng_);
        int t1 = std::uniform_int_distribution<int>(t2 + 1, 9)(rng_);

        int a = t1 * 10 + o1;
        int b = t2 * 10 + o2;
        return {a, b, Operation::Subtract, a - b,
                SkillID::BorrowSubtraction, DifficultyLevel::Easy};
    }

    Question generateMedium() {
        // 3-digit - 3-digit, borrow at ones and tens, not hundreds
        int o1 = std::uniform_int_distribution<int>(0, 8)(rng_);
        int o2 = std::uniform_int_distribution<int>(o1 + 1, 9)(rng_);

        // Borrow at tens: after ones borrow, tens digit reduces by 1
        // Then tens digit (t1-1) < t2 triggers another borrow
        // So t1 - 1 < t2, meaning t1 < t2 + 1, i.e., t1 <= t2
        int t2 = std::uniform_int_distribution<int>(1, 9)(rng_);
        int t1 = std::uniform_int_distribution<int>(0, t2)(rng_);
        // After borrow: effective t1 = t1 - 1 - 10 + 10 = handled by algorithm
        // But we need t1-1 < t2 for borrow, which is t1 <= t2 (already ensured)

        // Hundreds: no borrow (h1 > h2 after tens borrow)
        // h1 - 1 >= h2 (hundreds borrow from tens reduces h1)
        int h2 = std::uniform_int_distribution<int>(0, 7)(rng_);
        int h1 = std::uniform_int_distribution<int>(h2 + 2, 9)(rng_);
        // h1 >= h2 + 2 ensures h1 - 1 >= h2 + 1 > h2

        int a = h1 * 100 + t1 * 10 + o1;
        int b = h2 * 100 + t2 * 10 + o2;
        return {a, b, Operation::Subtract, a - b,
                SkillID::BorrowSubtraction, DifficultyLevel::Medium};
    }

    Question generateHard() {
        // 3-digit - 3-digit, all positions borrow
        int o1 = std::uniform_int_distribution<int>(0, 8)(rng_);
        int o2 = std::uniform_int_distribution<int>(o1 + 1, 9)(rng_);

        // Tens borrow (after ones borrow): t1 <= t2
        int t2 = std::uniform_int_distribution<int>(0, 9)(rng_);
        int t1 = std::uniform_int_distribution<int>(0, std::min(t2, 9))(rng_);

        // Hundreds borrow (after tens borrow): h1 <= h2 (or h1-1 < h2)
        // But we need result >= 0, so a > b overall
        // Generate and verify
        int h2 = std::uniform_int_distribution<int>(1, 9)(rng_);
        // h1 must be such that h1 <= h2 but overall a > b
        // Since ones borrow and tens borrow, we need h1*100 > h2*100 - some_amount
        // For safety, set h1 = h2 and ensure lower digits make a > b
        int h1 = h2; // same hundreds, borrow still happens if t1-1 < t2

        int a = h1 * 100 + t1 * 10 + o1;
        int b = h2 * 100 + t2 * 10 + o2;

        // Ensure a > b; if not, adjust
        if (a <= b) {
            h1 = h2 + 1;
            a = h1 * 100 + t1 * 10 + o1;
        }

        return {a, b, Operation::Subtract, a - b,
                SkillID::BorrowSubtraction, DifficultyLevel::Hard};
    }
};

} // namespace adaptive_math
