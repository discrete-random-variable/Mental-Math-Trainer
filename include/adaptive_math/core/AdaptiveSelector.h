#pragma once

#include "IQuestionGenerator.h"
#include "SkillManager.h"
#include "DifficultyLevel.h"
#include "Question.h"
#include <vector>
#include <memory>
#include <random>
#include <unordered_map>

namespace adaptive_math {

// Selects the next question based on adaptive priority scoring.
// Uses the SkillManager to query per-skill performance and the
// SkillDAG to respect prerequisite unlocking.
//
// Selection algorithm:
//   1. Get all unlocked skills
//   2. Compute priority score for each:
//      score = 0.6 * (1 - accuracy) + 0.4 * normalizedResponseTime
//   3. Weighted random selection (higher score = more likely)
//   4. Choose difficulty based on current accuracy for that skill
//   5. Delegate to the appropriate IQuestionGenerator
class AdaptiveSelector {
public:
    AdaptiveSelector(SkillManager& skillManager);

    // Register a generator for a specific skill
    void registerGenerator(std::unique_ptr<IQuestionGenerator> generator);

    // Select and generate the next question
    Question nextQuestion();

    // Determine difficulty for a skill based on performance
    DifficultyLevel suggestDifficulty(SkillID id) const;

    // Get registered generators count
    std::size_t generatorCount() const { return generators_.size(); }

private:
    SkillManager& skillManager_;
    std::unordered_map<SkillID, std::unique_ptr<IQuestionGenerator>> generators_;

    // Weighted random selection
    SkillID selectSkill() const;

    // Discrete distribution for weighted selection
    mutable std::mt19937 rng_{std::random_device{}()};
};

} // namespace adaptive_math
