#pragma once

#include "SkillID.h"
#include "SkillStats.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace adaptive_math {

// Models skill prerequisite relationships as a Directed Acyclic Graph.
// A skill is "unlocked" when the user achieves sufficient accuracy
// on all of its prerequisite skills.
class SkillDAG {
public:
    SkillDAG();

    // Returns the prerequisites for a given skill
    const std::vector<SkillID>& getPrerequisites(SkillID id) const;

    // Returns skills that depend on the given skill
    const std::vector<SkillID>& getDependents(SkillID id) const;

    // Checks whether a skill is unlocked given current per-skill stats
    bool isUnlocked(SkillID id,
                    const std::unordered_map<SkillID, SkillStats>& stats) const;

    // Returns all skills that are currently unlocked
    std::vector<SkillID> getUnlockedSkills(
        const std::unordered_map<SkillID, SkillStats>& stats) const;

    // Returns all skills with no prerequisites (always unlocked)
    std::vector<SkillID> getRootSkills() const;

    // Returns all skills in topological order
    std::vector<SkillID> topologicalOrder() const;

    // Returns a human-readable prerequisite chain for a skill
    std::string describePath(SkillID id) const;

    // Mastery threshold to unlock next skill (0.0 - 1.0)
    double masteryThreshold() const { return masteryThreshold_; }
    void setMasteryThreshold(double t) { masteryThreshold_ = t; }

private:
    // Adjacency list: skill -> its prerequisites
    std::unordered_map<SkillID, std::vector<SkillID>> prerequisites_;

    // Reverse adjacency: skill -> skills that depend on it
    std::unordered_map<SkillID, std::vector<SkillID>> dependents_;

    double masteryThreshold_ = 0.70; // 70% accuracy unlocks next skill

    void addEdge(SkillID from, SkillID to);
    void buildDefaultDAG();
};

} // namespace adaptive_math
