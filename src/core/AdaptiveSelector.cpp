#include "adaptive_math/core/AdaptiveSelector.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>

namespace adaptive_math {

AdaptiveSelector::AdaptiveSelector(SkillManager& skillManager)
    : skillManager_(skillManager) {}

void AdaptiveSelector::registerGenerator(std::unique_ptr<IQuestionGenerator> generator) {
    SkillID id = generator->getSkillID();
    generators_[id] = std::move(generator);
}

Question AdaptiveSelector::nextQuestion() {
    if (generators_.empty()) {
        throw std::runtime_error("No generators registered");
    }

    // Get unlocked skills that have generators
    auto unlocked = skillManager_.getUnlockedSkills();
    std::vector<SkillID> available;
    for (SkillID id : unlocked) {
        if (generators_.count(id) > 0) {
            available.push_back(id);
        }
    }

    if (available.empty()) {
        // Fallback: use root skills (should always be unlocked)
        auto roots = skillManager_.dag().getRootSkills();
        for (SkillID id : roots) {
            if (generators_.count(id) > 0) {
                available.push_back(id);
            }
        }
    }

    if (available.empty()) {
        throw std::runtime_error("No available skills with generators");
    }

    SkillID selected = selectSkill();
    DifficultyLevel diff = suggestDifficulty(selected);

    auto it = generators_.find(selected);
    if (it == generators_.end()) {
        // Fallback to first available
        selected = available[0];
        it = generators_.find(selected);
    }

    return it->second->generate(diff);
}

SkillID AdaptiveSelector::selectSkill() const {
    auto unlocked = skillManager_.getUnlockedSkills();
    std::vector<SkillID> available;
    for (SkillID id : unlocked) {
        if (generators_.count(id) > 0) {
            available.push_back(id);
        }
    }

    if (available.empty()) {
        auto roots = skillManager_.dag().getRootSkills();
        for (SkillID id : roots) {
            if (generators_.count(id) > 0) {
                available.push_back(id);
            }
        }
    }

    if (available.size() == 1) {
        return available[0];
    }

    // Compute priority scores for weighted selection
    std::vector<double> scores;
    scores.reserve(available.size());
    for (SkillID id : available) {
        double score = skillManager_.getSkillStats(id).priorityScore();
        // Add small epsilon to ensure all skills have some chance
        scores.push_back(score + 0.05);
    }

    // Weighted random selection
    std::discrete_distribution<std::size_t> dist(scores.begin(), scores.end());
    std::size_t idx = dist(rng_);
    return available[idx];
}

DifficultyLevel AdaptiveSelector::suggestDifficulty(SkillID id) const {
    const auto& stats = skillManager_.getSkillStats(id);

    // If skill hasn't been practiced much, start easy
    if (stats.attempts < 3) {
        return DifficultyLevel::Easy;
    }

    double acc = stats.accuracy();
    double avgTime = stats.averageTime();
    double parTime = stats.parTime;

    // Progressive difficulty based on accuracy and speed
    if (acc >= 0.85 && avgTime <= parTime * 1.2) {
        return DifficultyLevel::Hard;
    } else if (acc >= 0.70 && avgTime <= parTime * 1.5) {
        return DifficultyLevel::Medium;
    } else {
        return DifficultyLevel::Easy;
    }
}

} // namespace adaptive_math
