#pragma once

#include "adaptive_math/core/Question.h"
#include "adaptive_math/core/SkillID.h"
#include "adaptive_math/core/DifficultyLevel.h"
#include <string>
#include <chrono>
#include <optional>

namespace adaptive_math {

// Handles all terminal input/output.
// Main should never print directly — everything goes through TerminalUI.
class TerminalUI {
public:
    // ANSI color codes for polished terminal output
    struct Color {
        static constexpr const char* Reset   = "\033[0m";
        static constexpr const char* Bold    = "\033[1m";
        static constexpr const char* Dim     = "\033[2m";
        static constexpr const char* Green   = "\033[32m";
        static constexpr const char* Red     = "\033[31m";
        static constexpr const char* Yellow  = "\033[33m";
        static constexpr const char* Blue    = "\033[34m";
        static constexpr const char* Cyan    = "\033[36m";
        static constexpr const char* Magenta = "\033[35m";
        static constexpr const char* BgGreen = "\033[42m";
        static constexpr const char* BgRed   = "\033[41m";
    };

    // Session mode
    enum class SessionMode {
        Timed,
        QuestionCount,
        Endless
    };

    struct SessionConfig {
        SessionMode mode;
        int durationSeconds = 120; // default 2 minutes
        int questionCount   = 20;
    };

    // Display methods
    void showWelcome() const;
    void showMainMenu() const;
    void showSessionConfigMenu() const;
    void showQuestion(const Question& q, int questionNum, int streak) const;
    void showCorrect(int streak) const;
    void showIncorrect(const Question& q, int answer) const;
    void showSessionEnd() const;
    void showMessage(const std::string& msg) const;
    void showError(const std::string& msg) const;
    void showProgress(int current, int total, double elapsed, double limit) const;

    // Input methods
    int  getMenuChoice() const;
    SessionConfig getSessionConfig() const;
    std::optional<int> getAnswer() const;
    bool getYesNo(const std::string& prompt) const;

    // ANSI helpers
    static bool supportsAnsi();
    void clearScreen() const;
};

} // namespace adaptive_math
