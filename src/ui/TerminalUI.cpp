#include "adaptive_math/ui/TerminalUI.h"
#include <iostream>
#include <string>
#include <limits>
#include <cstdlib>

namespace adaptive_math {

bool TerminalUI::supportsAnsi() {
    const char* term = std::getenv("TERM");
    if (!term) return false;
    std::string termStr(term);
    return termStr.find("xterm") != std::string::npos ||
           termStr.find("vt100") != std::string::npos ||
           termStr.find("ansi")  != std::string::npos ||
           termStr.find("color") != std::string::npos ||
           termStr.find("linux") != std::string::npos;
}

void TerminalUI::clearScreen() const {
    std::cout << "\033[2J\033[H";
}

void TerminalUI::showWelcome() const {
    std::cout << Color::Cyan << Color::Bold;
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════╗\n";
    std::cout << "  ║     ADAPTIVE MENTAL MATH TRAINER  v1.0      ║\n";
    std::cout << "  ║                                              ║\n";
    std::cout << "  ║     Sharpen your mental arithmetic          ║\n";
    std::cout << "  ║     with adaptive skill progression          ║\n";
    std::cout << "  ╚══════════════════════════════════════════════╝\n";
    std::cout << Color::Reset << "\n";
}

void TerminalUI::showMainMenu() const {
    std::cout << Color::Yellow << Color::Bold;
    std::cout << "┌──────────────────────────────────┐\n";
    std::cout << "│          MAIN MENU               │\n";
    std::cout << "├──────────────────────────────────┤\n";
    std::cout << Color::Reset;
    std::cout << "│  1. " << Color::Green << "Start Practice Session" << Color::Reset << "     │\n";
    std::cout << "│  2. " << Color::Cyan << "View Skill Statistics" << Color::Reset << "     │\n";
    std::cout << "│  3. " << Color::Blue << "View Skill Tree" << Color::Reset << "          │\n";
    std::cout << "│  4. " << Color::Magenta << "View Personal Records" << Color::Reset << "   │\n";
    std::cout << "│  5. " << Color::Red << "Reset All Progress" << Color::Reset << "       │\n";
    std::cout << "│  6. " << Color::Dim << "Exit" << Color::Reset << "                      │\n";
    std::cout << Color::Yellow << "└──────────────────────────────────┘\n";
    std::cout << Color::Reset;
    std::cout << "  Choose [1-6]: ";
}

void TerminalUI::showSessionConfigMenu() const {
    std::cout << Color::Yellow << Color::Bold;
    std::cout << "\n┌──────────────────────────────────┐\n";
    std::cout << "│       SESSION CONFIGURATION      │\n";
    std::cout << "├──────────────────────────────────┤\n";
    std::cout << Color::Reset;
    std::cout << "│  1. " << "Quick Practice (2 min)" << "        │\n";
    std::cout << "│  2. " << "Standard Session (5 min)" << "      │\n";
    std::cout << "│  3. " << "Extended Session (10 min)" << "     │\n";
    std::cout << "│  4. " << "20 Questions" << "                  │\n";
    std::cout << "│  5. " << "50 Questions" << "                  │\n";
    std::cout << "│  6. " << "Endless Mode" << "                  │\n";
    std::cout << "│  7. " << Color::Dim << "Custom..." << Color::Reset << "                   │\n";
    std::cout << "│  8. " << Color::Dim << "Back to Main Menu" << Color::Reset << "            │\n";
    std::cout << Color::Yellow << "└──────────────────────────────────┘\n";
    std::cout << Color::Reset;
    std::cout << "  Choose [1-8]: ";
}

void TerminalUI::showQuestion(const Question& q, int questionNum, int streak) const {
    std::cout << "\n";
    std::cout << Color::Dim << "  Q" << questionNum;
    if (streak > 0) {
        std::cout << Color::Yellow << "  Streak: " << streak;
    }
    std::cout << Color::Reset << "\n";

    std::cout << Color::Bold << Color::Cyan;
    std::cout << "  " << q.display() << "  ";
    std::cout << Color::Reset;

    std::cout << Color::Dim
              << "[" << skillIDToString(q.skillId)
              << " / " << difficultyToString(q.difficulty) << "]"
              << Color::Reset << "\n";
    std::cout << "  Your answer: ";
}

void TerminalUI::showCorrect(int streak) const {
    std::cout << "  " << Color::Green << Color::Bold
              << "CORRECT!";
    if (streak >= 5) {
        std::cout << " " << Color::Yellow << "Streak x" << streak << "!";
    }
    if (streak >= 10) {
        std::cout << " " << Color::Magenta << "AMAZING!";
    }
    std::cout << Color::Reset << "\n";
}

void TerminalUI::showIncorrect(const Question& q, int answer) const {
    std::cout << "  " << Color::Red << Color::Bold
              << "WRONG!" << Color::Reset;
    std::cout << Color::Dim << " Your answer: " << answer << Color::Reset << "\n";
    std::cout << "  " << Color::Green << "Correct: "
              << q.operand1 << " " << operationToString(q.operation)
              << " " << q.operand2 << " = " << q.correctAnswer
              << Color::Reset << "\n";
}

void TerminalUI::showSessionEnd() const {
    std::cout << "\n" << Color::Yellow << Color::Bold;
    std::cout << "  ══════════════════════════════════\n";
    std::cout << "         SESSION COMPLETE\n";
    std::cout << "  ══════════════════════════════════\n";
    std::cout << Color::Reset;
}

void TerminalUI::showMessage(const std::string& msg) const {
    std::cout << Color::Cyan << "  " << msg << Color::Reset << "\n";
}

void TerminalUI::showError(const std::string& msg) const {
    std::cout << Color::Red << "  Error: " << msg << Color::Reset << "\n";
}

void TerminalUI::showProgress(int current, int total, double elapsed, double limit) const {
    std::cout << Color::Dim << "  [";
    if (total > 0) {
        double pct = static_cast<double>(current) / total;
        int width = 20;
        int filled = static_cast<int>(pct * width);
        for (int i = 0; i < width; ++i) {
            std::cout << (i < filled ? "#" : "-");
        }
    }
    std::cout << "] ";

    // Time display
    int mins = static_cast<int>(elapsed) / 60;
    int secs = static_cast<int>(elapsed) % 60;
    std::cout << mins << ":" << (secs < 10 ? "0" : "") << secs;

    if (limit > 0) {
        int totalMins = static_cast<int>(limit) / 60;
        int totalSecs = static_cast<int>(limit) % 60;
        std::cout << "/" << totalMins << ":" << (totalSecs < 10 ? "0" : "") << totalSecs;
    }

    std::cout << Color::Reset << "\n";
}

int TerminalUI::getMenuChoice() const {
    int choice = 0;
    std::string line;
    if (std::getline(std::cin, line)) {
        try {
            choice = std::stoi(line);
        } catch (...) {
            choice = 0;
        }
    }
    return choice;
}

TerminalUI::SessionConfig TerminalUI::getSessionConfig() const {
    showSessionConfigMenu();
    int choice = getMenuChoice();

    SessionConfig config;

    switch (choice) {
        case 1:
            config.mode = SessionMode::Timed;
            config.durationSeconds = 120;  // 2 min
            break;
        case 2:
            config.mode = SessionMode::Timed;
            config.durationSeconds = 300;  // 5 min
            break;
        case 3:
            config.mode = SessionMode::Timed;
            config.durationSeconds = 600;  // 10 min
            break;
        case 4:
            config.mode = SessionMode::QuestionCount;
            config.questionCount = 20;
            break;
        case 5:
            config.mode = SessionMode::QuestionCount;
            config.questionCount = 50;
            break;
        case 6:
            config.mode = SessionMode::Endless;
            break;
        case 7: {
            // Custom
            std::cout << "  Enter duration in minutes (0 for question count mode): ";
            int mins = getMenuChoice();
            if (mins > 0) {
                config.mode = SessionMode::Timed;
                config.durationSeconds = mins * 60;
            } else {
                std::cout << "  Enter number of questions: ";
                config.mode = SessionMode::QuestionCount;
                config.questionCount = getMenuChoice();
                if (config.questionCount <= 0) config.questionCount = 20;
            }
            break;
        }
        default:
            config.mode = SessionMode::Timed;
            config.durationSeconds = 120;
            break;
    }

    return config;
}

std::optional<int> TerminalUI::getAnswer() const {
    std::string line;
    if (!std::getline(std::cin, line)) {
        return std::nullopt; // EOF
    }

    // Trim whitespace
    std::string trimmed;
    for (char c : line) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            trimmed += c;
        }
    }

    if (trimmed.empty()) return std::nullopt;
    if (trimmed == "quit" || trimmed == "q" || trimmed == "exit") return std::nullopt;

    try {
        return std::stoi(trimmed);
    } catch (...) {
        return std::nullopt;
    }
}

bool TerminalUI::getYesNo(const std::string& prompt) const {
    std::cout << "  " << prompt << " (y/n): ";
    std::string line;
    if (std::getline(std::cin, line)) {
        return !line.empty() && (line[0] == 'y' || line[0] == 'Y');
    }
    return false;
}

} // namespace adaptive_math
