#pragma once

#include "AdaptiveSelector.h"
#include "SkillManager.h"
#include "adaptive_math/statistics/Statistics.h"
#include "adaptive_math/persistence/Persistence.h"
#include "adaptive_math/ui/TerminalUI.h"
#include <chrono>
#include <memory>

namespace adaptive_math {

// Top-level orchestrator.
// Main simply creates a Session and calls run().
// Session owns all modules and manages their lifecycle.
//
// Architecture:
//   main -> Session -> AdaptiveSelector -> Generator
//                           |
//                      SkillManager -> Statistics
//                           |
//                      Persistence
//                           |
//                      TerminalUI
class Session {
public:
    Session();

    // Main entry point — runs the interactive session loop
    void run();

private:
    SkillManager skillManager_;
    AdaptiveSelector selector_;
    Statistics statistics_;
    Persistence persistence_;
    TerminalUI ui_;

    // Session state
    int currentStreak_ = 0;
    int bestStreak_    = 0;

    // Register all generators with the selector
    void registerGenerators();

    // Run a practice session
    void runPracticeSession();

    // Main menu loop
    void mainMenuLoop();

    // Display statistics
    void showStatistics();

    // Display skill tree
    void showSkillTree();

    // Display personal records
    void showPersonalRecords();

    // Reset progress
    void resetProgress();

    // Save current progress
    void saveProgress();

    // Load progress on startup
    void loadProgress();
};

} // namespace adaptive_math
