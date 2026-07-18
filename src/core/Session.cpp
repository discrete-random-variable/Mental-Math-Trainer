#include "adaptive_math/core/Session.h"
#include "adaptive_math/core/Operation.h"

// Include all generator headers
#include "../../src/generators/BasicAdditionGenerator.h"
#include "../../src/generators/CarryAdditionGenerator.h"
#include "../../src/generators/BasicSubtractionGenerator.h"
#include "../../src/generators/BorrowSubtractionGenerator.h"
#include "../../src/generators/TablesMultiplicationGenerator.h"
#include "../../src/generators/LargerMultiplicationGenerator.h"
#include "../../src/generators/ExactDivisionGenerator.h"
#include "../../src/generators/LargerDivisionGenerator.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

namespace adaptive_math {

Session::Session()
    : selector_(skillManager_),
      statistics_(skillManager_),
      persistence_("data") {
    registerGenerators();
    loadProgress();
}

void Session::registerGenerators() {
    selector_.registerGenerator(std::make_unique<BasicAdditionGenerator>());
    selector_.registerGenerator(std::make_unique<CarryAdditionGenerator>());
    selector_.registerGenerator(std::make_unique<BasicSubtractionGenerator>());
    selector_.registerGenerator(std::make_unique<BorrowSubtractionGenerator>());
    selector_.registerGenerator(std::make_unique<TablesMultiplicationGenerator>());
    selector_.registerGenerator(std::make_unique<LargerMultiplicationGenerator>());
    selector_.registerGenerator(std::make_unique<ExactDivisionGenerator>());
    selector_.registerGenerator(std::make_unique<LargerDivisionGenerator>());
}

void Session::run() {
    ui_.showWelcome();

    if (persistence_.hasSaveData()) {
        ui_.showMessage("Welcome back! Your progress has been loaded.");
    } else {
        ui_.showMessage("New user? Start with a practice session to begin!");
    }

    mainMenuLoop();
}

void Session::mainMenuLoop() {
    while (true) {
        ui_.showMainMenu();
        int choice = ui_.getMenuChoice();

        switch (choice) {
            case 1:
                runPracticeSession();
                break;
            case 2:
                showStatistics();
                break;
            case 3:
                showSkillTree();
                break;
            case 4:
                showPersonalRecords();
                break;
            case 5:
                resetProgress();
                break;
            case 6:
                saveProgress();
                ui_.showMessage("Progress saved. Goodbye!");
                return;
            default:
                ui_.showError("Invalid choice. Please try again.");
                break;
        }
    }
}

void Session::runPracticeSession() {
    auto config = ui_.getSessionConfig();

    skillManager_.startSession();

    int totalQuestions = 0;
    int correctAnswers = 0;
    int sessionStreak  = 0;
    int sessionBestStreak = 0;
    //double totalResponseTime = 0.0;

    auto sessionStart = std::chrono::steady_clock::now();
    bool sessionEnded = false;

    ui_.showMessage("\nSession starting... Type 'quit' to end early.\n");

    while (!sessionEnded) {
        // Check time limit for timed sessions
        if (config.mode == TerminalUI::SessionMode::Timed) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - sessionStart).count();
            if (elapsed >= config.durationSeconds) {
                ui_.showMessage("Time's up!");
                break;
            }
        }

        // Check question count limit
        if (config.mode == TerminalUI::SessionMode::QuestionCount) {
            if (totalQuestions >= config.questionCount) {
                ui_.showMessage("All questions completed!");
                break;
            }
        }

        // Generate next question adaptively
        Question q = selector_.nextQuestion();
        totalQuestions++;

        // Show progress
        if (config.mode == TerminalUI::SessionMode::Timed) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - sessionStart).count();
            ui_.showProgress(0, 0, elapsed, config.durationSeconds);
        } else if (config.mode == TerminalUI::SessionMode::QuestionCount) {
            ui_.showProgress(totalQuestions, config.questionCount, 0, 0);
        }

        // Display question and get answer
        ui_.showQuestion(q, totalQuestions, sessionStreak);
        auto answerStart = std::chrono::steady_clock::now();

        auto answer = ui_.getAnswer();
        if (!answer.has_value()) {
            ui_.showMessage("Session ended early.");
            break;
        }

        auto answerEnd = std::chrono::steady_clock::now();
        double responseTime = std::chrono::duration<double>(answerEnd - answerStart).count();

        bool correct = q.checkAnswer(answer.value());

        // Update statistics
        skillManager_.recordAttempt(q.skillId, correct, responseTime);
        //totalResponseTime += responseTime;

        if (correct) {
            correctAnswers++;
            sessionStreak++;
            if (sessionStreak > sessionBestStreak) sessionBestStreak = sessionStreak;
            if (sessionStreak > bestStreak_) bestStreak_ = sessionStreak;
            ui_.showCorrect(sessionStreak);
        } else {
            sessionStreak = 0;
            ui_.showIncorrect(q, answer.value());
        }

        // Update global streak record
        if (bestStreak_ > skillManager_.globalRecord().overallBestStreak) {
            skillManager_.globalRecord().overallBestStreak = bestStreak_;
        }
    }

    // Session end
    ui_.showSessionEnd();

    double totalSessionTime = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - sessionStart).count();

    Statistics::SessionSummary summary{
        totalQuestions,
        correctAnswers,
        (totalQuestions > 0) ? static_cast<double>(correctAnswers) / totalQuestions : 0.0,
        totalSessionTime,
        sessionStreak,
        sessionBestStreak
    };

    std::cout << statistics_.sessionSummary(summary);
    std::cout << statistics_.skillComparisonTable();

    // Auto-save after session
    saveProgress();

    skillManager_.endSession(totalQuestions);
    currentStreak_ = sessionStreak;
}

void Session::showStatistics() {
    std::cout << statistics_.fullReport();
    std::cout << "\n  Press Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

void Session::showSkillTree() {
    std::cout << statistics_.skillTreeDisplay();
    std::cout << "\n  Press Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

void Session::showPersonalRecords() {
    const auto& gr = skillManager_.globalRecord();

    std::cout << "\n" << TerminalUI::Color::Yellow << TerminalUI::Color::Bold;
    std::cout << "========================================\n";
    std::cout << "         PERSONAL RECORDS\n";
    std::cout << "========================================\n";
    std::cout << TerminalUI::Color::Reset;

    std::cout << "  Total Sessions:   " << gr.totalSessions << "\n";
    std::cout << "  Total Questions:  " << gr.totalQuestions << "\n";

    if (gr.overallBestAccuracy > 0) {
        std::cout << "  Best Accuracy:    "
                  << std::fixed << std::setprecision(1)
                  << (gr.overallBestAccuracy * 100) << "%\n";
    }
    if (gr.overallBestStreak > 0) {
        std::cout << "  Best Streak:      " << gr.overallBestStreak << "\n";
    }
    if (gr.overallFastestAvg > 0) {
        std::cout << "  Fastest Avg Time: "
                  << std::fixed << std::setprecision(1)
                  << gr.overallFastestAvg << "s\n";
    }

    // Per-skill records
    std::cout << "\n  Per-Skill Records:\n";
    std::cout << "  ┌────────────────────────┬──────────┬────────┬──────────┐\n";
    std::cout << "  │ Skill                  │ Best Acc │ Streak │ Best Time│\n";
    std::cout << "  ├────────────────────────┼──────────┼────────┼──────────┤\n";

    for (std::size_t i = 0; i < skillCount(); ++i) {
        SkillID id = indexToSkillID(i);
        const auto& rec = skillManager_.getSkillRecord(id);
        std::string name = skillIDToString(id);

        if (name.size() > 22) name = name.substr(0, 22);
        name += std::string(22 - name.size(), ' ');

        std::cout << "  │ " << name << " │ "
                  << std::setw(7) << std::fixed << std::setprecision(1)
                  << (rec.bestAccuracy * 100) << "% │ "
                  << std::setw(6) << rec.bestStreak << " │ "
                  << std::setw(7) << std::fixed << std::setprecision(1)
                  << rec.fastestAvgTime << "s │\n";
    }

    std::cout << "  └────────────────────────┴──────────┴────────┴──────────┘\n";
    std::cout << "\n  Press Enter to continue...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

void Session::resetProgress() {
    if (ui_.getYesNo("Are you sure you want to reset ALL progress?")) {
        skillManager_.reset();
        persistence_.deleteSaveData();
        currentStreak_ = 0;
        bestStreak_ = 0;
        ui_.showMessage("All progress has been reset.");
    } else {
        ui_.showMessage("Reset cancelled.");
    }
}

void Session::saveProgress() {
    if (persistence_.saveStatistics(skillManager_)) {
        // Silently saved — no need to spam the user
    } else {
        ui_.showError("Failed to save progress.");
    }
}

void Session::loadProgress() {
    if (persistence_.hasSaveData()) {
        if (persistence_.loadStatistics(skillManager_)) {
            // Loaded successfully
        } else {
            ui_.showError("Failed to load saved progress. Starting fresh.");
        }
    }
}

} // namespace adaptive_math
