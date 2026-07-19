# Adaptive Mental Math Trainer

A modular, terminal-based mental math trainer built with modern C++20. The project emphasizes clean software architecture, constraint-based arithmetic generation, and an adaptive practice engine. Skill performance is tracked independently and practice is prioritized using normalized accuracy and response-time metrics, with the architecture designed to evolve toward Glicko-2 ratings and spaced repetition.

---

## Resume Description

> **Adaptive Mental Math Trainer | C++20, CMake**
>
> - Engineered a modular terminal-based mental math trainer using modern C++20, using polymorphic question generators and a layered architecture with clear separation of concerns.
> - Implemented an adaptive question selection algorithm that prioritizes weaker skills using per-category performance metrics and normalized response times.
> - Modeled arithmetic skills independently while supporting prerequisite-based progression where appropriate.
> - Built persistent analytics tracking accuracy and historical session data using `nlohmann/json`, integrated via modern CMake dependency management.
> - Organized the project as reusable modules with clear ownership boundaries, modern C++20 idioms, and extensible component interfaces.

---

## Features

### Adaptive Skill System
8 distinct skill categories, each with its own performance tracking and difficulty progression:

| Skill                 | Operation      | Constraint                                        |
| --------------------- | -------------- | ------------------------------------------------- |
| Basic Addition        | Addition       | No carry at any digit position                    |
| Carry Addition        | Addition       | At least one carry position (1, 2, or all 3)      |
| Basic Subtraction     | Subtraction    | No borrow at any digit position                   |
| Borrow Subtraction    | Subtraction    | At least one borrow position                      |
| Tables Multiplication | Multiplication | Times tables (1×1 up to 2-digit × 1-digit)        |
| Larger Multiplication | Multiplication | Multi-digit products up to 3-digit × 2-digit      |
| Exact Division        | Division       | Always produces integer quotient, 1-digit divisor |
| Larger Division       | Division       | 2-digit divisors, always exact                    |

### Skill Progression
Skills may be organized in a lightweight prerequisite graph for introducing new categories, while day-to-day practice is driven by the adaptive selector. You must demonstrate mastery (≥70% accuracy with at least 5 attempts) of prerequisite skills before advanced skills unlock:

```
BasicAddition ──────► CarryAddition ──────► BorrowSubtraction
     │
     ▼
BasicSubtraction

TablesMultiplication ──► LargerMultiplication

ExactDivision ─────────► LargerDivision
```

### Adaptive Practice Engine
The next question is not random — it is chosen based on a priority score that weighs accuracy and response speed:

```
score = 0.6 × (1 − accuracy) + 0.4 × normalizedResponseTime
```

Skills with higher scores (weaker performance) are selected more frequently via weighted random sampling.

### Constraint-Based Question Generation
Each generator produces questions that satisfy mathematical constraints (e.g., carry addition guarantees at least one carry). Generation is O(1) — digits are constructed directly from constraints rather than using rejection sampling.

### Difficulty Progression
Each skill supports 3 difficulty levels (Easy, Medium, Hard). The system automatically suggests difficulty based on your current accuracy and speed:

| Accuracy              | Speed           | Difficulty |
| --------------------- | --------------- | ---------- |
| < 70% or < 3 attempts | Any             | Easy       |
| ≥ 70%                 | ≤ 1.5× par time | Medium     |
| ≥ 85%                 | ≤ 1.2× par time | Hard       |

### Persistent Progress
All statistics, personal records, and session history are saved to `data/progress.json` using `nlohmann/json`. Progress is automatically loaded on startup and saved after each session.

### Configurable Sessions
- **Quick Practice** — 2 minutes
- **Standard Session** — 5 minutes
- **Extended Session** — 10 minutes
- **Question Count** — 20 or 50 questions
- **Endless Mode** — practice until you quit
- **Custom** — set your own duration or question count

---

## Architecture

```
main()
  └── Session
        ├── AdaptiveSelector ──→ IQuestionGenerator (polymorphic)
        │       │                     ├── BasicAdditionGenerator
        │       │                     ├── CarryAdditionGenerator
        │       │                     ├── BasicSubtractionGenerator
        │       │                     ├── BorrowSubtractionGenerator
        │       │                     ├── TablesMultiplicationGenerator
        │       │                     ├── LargerMultiplicationGenerator
        │       │                     ├── ExactDivisionGenerator
        │       │                     └── LargerDivisionGenerator
        │       │
        │       └── SkillManager
        │             ├── SkillDAG (prerequisite graph)
        │             ├── SkillStats (per-skill metrics)
        │             └── SkillRecord / GlobalRecord (personal bests)
        │
        ├── Statistics (formatted reports & analytics)
        ├── Persistence (JSON save/load)
        └── TerminalUI (all terminal I/O)
```

### Design Principles

- **Single Responsibility Principle** — each class has one job
- **Open-Closed Principle** — add new skills by creating a new `IQuestionGenerator` implementation; no existing code changes
- **Interface Segregation** — generators depend only on `IQuestionGenerator`, not on each other
- **Dependency Inversion** — `AdaptiveSelector` depends on the `IQuestionGenerator` interface, not concrete generators
- **RAII** — generators own their RNG, `Session` owns all module lifetimes

---

## Project Structure

```
adaptive-math-trainer/
├── CMakeLists.txt
├── .gitignore
│
├── include/adaptive_math/
│   ├── core/
│   │   ├── Operation.h                 # enum class Operation
│   │   ├── SkillID.h                   # enum class SkillID
│   │   ├── DifficultyLevel.h           # enum class DifficultyLevel
│   │   ├── Question.h                  # Immutable question data object
│   │   ├── SkillStats.h                # Per-skill metrics + priority formula
│   │   ├── PersonalRecord.h            # SkillRecord + GlobalRecord
│   │   ├── IQuestionGenerator.h        # Polymorphic generator interface
│   │   ├── SkillDAG.h                  # Prerequisite DAG model
│   │   ├── SkillManager.h              # Stats & records owner
│   │   ├── AdaptiveSelector.h          # Weighted priority selector
│   │   └── Session.h                   # Top-level orchestrator
│   ├── statistics/
│   │   └── Statistics.h                # Analytics & report formatting
│   ├── persistence/
│   │   └── Persistence.h               # JSON save/load
│   └── ui/
│       └── TerminalUI.h                # Terminal presentation layer
│
├── src/
│   ├── main.cpp                        # int main() — 3 lines
│   ├── core/
│   │   ├── SkillDAG.cpp
│   │   ├── SkillManager.cpp
│   │   ├── AdaptiveSelector.cpp
│   │   └── Session.cpp
│   ├── generators/
│   │   ├── BasicAdditionGenerator.h
│   │   ├── CarryAdditionGenerator.h
│   │   ├── BasicSubtractionGenerator.h
│   │   ├── BorrowSubtractionGenerator.h
│   │   ├── TablesMultiplicationGenerator.h
│   │   ├── LargerMultiplicationGenerator.h
│   │   ├── ExactDivisionGenerator.h
│   │   └── LargerDivisionGenerator.h
│   ├── statistics/
│   │   └── Statistics.cpp
│   ├── persistence/
│   │   └── Persistence.cpp
│   └── ui/
│       └── TerminalUI.cpp
│
└── data/                               # Auto-created at runtime
    └── progress.json                   # User progress (JSON)
```

---

## Screenshots

> Screenshots and an animated terminal demo will be added after the first stable release.


---

## Build Instructions

### Prerequisites

- **C++20 compiler** (GCC 12+, Clang 15+, MSVC 19.30+)
- **CMake 3.14+**
- **Internet connection** (CMake FetchContent downloads `nlohmann/json` on first build)

### Build

```bash
git clone <repo-url>
cd adaptive-math-trainer
mkdir build && cd build
cmake ..
cmake --build .
```

### Run

```bash
./adaptive-math-trainer
```

On first run, you will see the main menu. Start a practice session to begin. Your progress is saved automatically.

---

## Usage

### Main Menu

```
  1. Start Practice Session
  2. View Skill Statistics
  3. View Skill Tree
  4. View Personal Records
  5. Reset All Progress
  6. Exit
```

### During a Session

- Type your numeric answer and press Enter
- Type `quit`, `q`, or `exit` to end the session early
- Streaks of 5+ and 10+ correct answers are highlighted

### Session End

After each session you will see:
- Session summary (accuracy, time, streak)
- Skill comparison table (all skills ranked by priority)
- Progress is automatically saved

---

## Dependencies

| Library                                           | Version | Integration        | Purpose                                     |
| ------------------------------------------------- | ------- | ------------------ | ------------------------------------------- |
| [nlohmann/json](https://github.com/nlohmann/json) | v3.11.3 | CMake FetchContent | JSON serialization for progress persistence |

No other external dependencies. The project uses only C++20 standard library features.

---

## License

This project is for educational and portfolio purposes.