#include "adaptive_math/core/SkillDAG.h"
#include <sstream>

namespace adaptive_math {

SkillDAG::SkillDAG() {
    buildDefaultDAG();
}

void SkillDAG::addEdge(SkillID prerequisite, SkillID dependent) {
    prerequisites_[dependent].push_back(prerequisite);
    dependents_[prerequisite].push_back(dependent);
}

void SkillDAG::buildDefaultDAG() {
    // Initialize empty vectors for all skills
    for (std::size_t i = 0; i < skillCount(); ++i) {
        SkillID id = indexToSkillID(i);
        prerequisites_[id];
        dependents_[id];
    }

    // Define the prerequisite DAG:
    //
    // BasicAddition ──────► CarryAddition ──────► BorrowSubtraction
    //     │                                         (needs carry experience)
    //     ▼
    // BasicSubtraction
    //
    // TablesMultiplication ──► LargerMultiplication
    //
    // ExactDivision ─────────► LargerDivision
    //
    // The DAG ensures foundational skills are mastered before
    // more complex variants are introduced.

    addEdge(SkillID::BasicAddition,    SkillID::CarryAddition);
    addEdge(SkillID::BasicAddition,    SkillID::BasicSubtraction);
    addEdge(SkillID::CarryAddition,    SkillID::BorrowSubtraction);
    addEdge(SkillID::TablesMultiplication, SkillID::LargerMultiplication);
    addEdge(SkillID::ExactDivision,    SkillID::LargerDivision);
}

const std::vector<SkillID>& SkillDAG::getPrerequisites(SkillID id) const {
    auto it = prerequisites_.find(id);
    if (it != prerequisites_.end()) return it->second;
    static const std::vector<SkillID> empty;
    return empty;
}

const std::vector<SkillID>& SkillDAG::getDependents(SkillID id) const {
    auto it = dependents_.find(id);
    if (it != dependents_.end()) return it->second;
    static const std::vector<SkillID> empty;
    return empty;
}

bool SkillDAG::isUnlocked(SkillID id,
                           const std::unordered_map<SkillID, SkillStats>& stats) const {
    const auto& prereqs = getPrerequisites(id);
    if (prereqs.empty()) return true; // root skills always unlocked

    for (SkillID prereq : prereqs) {
        auto it = stats.find(prereq);
        if (it == stats.end()) return false;
        if (it->second.attempts < 5) return false; // need minimum attempts
        if (it->second.accuracy() < masteryThreshold_) return false;
    }
    return true;
}

std::vector<SkillID> SkillDAG::getUnlockedSkills(
    const std::unordered_map<SkillID, SkillStats>& stats) const {
    std::vector<SkillID> result;
    for (std::size_t i = 0; i < skillCount(); ++i) {
        SkillID id = indexToSkillID(i);
        if (isUnlocked(id, stats)) {
            result.push_back(id);
        }
    }
    return result;
}

std::vector<SkillID> SkillDAG::getRootSkills() const {
    std::vector<SkillID> roots;
    for (std::size_t i = 0; i < skillCount(); ++i) {
        SkillID id = indexToSkillID(i);
        if (getPrerequisites(id).empty()) {
            roots.push_back(id);
        }
    }
    return roots;
}

std::vector<SkillID> SkillDAG::topologicalOrder() const {
    // Kahn's algorithm
    std::unordered_map<SkillID, int> inDegree;
    for (std::size_t i = 0; i < skillCount(); ++i) {
        SkillID id = indexToSkillID(i);
        inDegree[id] = static_cast<int>(getPrerequisites(id).size());
    }

    std::vector<SkillID> queue;
    for (const auto& [id, deg] : inDegree) {
        if (deg == 0) queue.push_back(id);
    }

    std::vector<SkillID> order;
    while (!queue.empty()) {
        SkillID current = queue.back();
        queue.pop_back();
        order.push_back(current);

        for (SkillID dep : getDependents(current)) {
            inDegree[dep]--;
            if (inDegree[dep] == 0) {
                queue.push_back(dep);
            }
        }
    }

    return order;
}

std::string SkillDAG::describePath(SkillID id) const {
    std::ostringstream oss;
    oss << skillIDToString(id);

    const auto& prereqs = getPrerequisites(id);
    if (!prereqs.empty()) {
        oss << " (requires: ";
        for (std::size_t i = 0; i < prereqs.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << skillIDToString(prereqs[i]) << " >= "
                << static_cast<int>(masteryThreshold_ * 100) << "%";
        }
        oss << ")";
    } else {
        oss << " (always available)";
    }
    return oss.str();
}

} // namespace adaptive_math
