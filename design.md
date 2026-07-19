# Design Document — Adaptive Mental Math Trainer v1

## 1. Overview

The Adaptive Mental Math Trainer is a terminal-based C++20 application that generates arithmetic questions and adapts to the user's ability over time. Unlike a simple quiz program, it tracks per-skill performance metrics, generates mathematically constrained arithmetic problems, and uses an adaptive priority algorithm to focus practice on weaker areas. The architecture is intentionally prepared for future Glicko-2 ratings and spaced repetition.

The design prioritizes **separation of concerns**, **extensibility without modification**, and **algorithmic sophistication** — the qualities that distinguish an engineering project from a console calculator.

---

## 2. Architecture

### 2.1 Layered Architecture

The system is organized into four layers, each with a single responsibility:

```
┌─────────────────────────────────────────────────────┐
│                   Presentation Layer                 │
│                     TerminalUI                       │
├─────────────────────────────────────────────────────┤
│                   Orchestration Layer                │
│                      Session                        │
├──────────────────────┬──────────────────────────────┤
│   Selection Layer    │       Analytics Layer         │
│  AdaptiveSelector    │        Statistics             │
├──────────────────────┼──────────────────────────────┤
│    Generation Layer  │      Persistence Layer        │
│  IQuestionGenerator  │        Persistence            │
│  (8 implementations) │     (nlohmann/json)           │
├──────────────────────┴──────────────────────────────┤
│                   Domain Core                        │
│  SkillManager · SkillDAG · SkillStats · Question    │
└─────────────────────────────────────────────────────┘
```

Each layer only depends on layers below it. `Session` orchestrates the layers but contains no business logic itself.

### 2.2 Dependency Flow

```
main()
  │
  └── Session (owns everything)
        │
        ├── AdaptiveSelector ──── ref ──→ SkillManager
        │       │                              │
        │       ├── owns ──→ IQuestionGenerator│
        │       │           (8 concrete)       │
        │       │                              ├── owns ──→ SkillDAG
        │       │                              ├── owns ──→ SkillStats (per skill)
        │       │                              ├── owns ──→ SkillRecord (per skill)
        │       │                              └── owns ──→ GlobalRecord
        │       │
        ├── Statistics ──── ref ──→ SkillManager
        ├── Persistence (value type)
        └── TerminalUI (value type)
```

`main()` creates a `Session` and calls `run()`. No other wiring is required — `Session` constructs and connects all modules internally.

---

## 3. Core Domain Types

### 3.1 Operation (enum class)

```cpp
enum class Operation { Add, Subtract, Multiply, Divide };
```

**Why not `char`?** Using `char` to represent operations (e.g., `'+', '-', '*', '/'`) is type-unsafe. A `char` can hold any character — there is no compile-time guarantee that the value is a valid operation. With `enum class`, the compiler enforces that only the four defined values are used, and `switch` statements can be exhaustively checked.

### 3.2 SkillID (enum class)

```cpp
enum class SkillID {
    BasicAddition, CarryAddition,
    BasicSubtraction, BorrowSubtraction,
    TablesMultiplication, LargerMultiplication,
    ExactDivision, LargerDivision,
    COUNT  // iteration sentinel
};
```

**Why not `string`?** String-based skill identification is error-prone: typos are not caught at compile time, string comparisons are slower than integer comparisons, and `unordered_map` lookups require hashing variable-length strings. The `enum class` approach eliminates all of these problems. The `COUNT` sentinel enables compile-time iteration over all skills.

**String conversion** is deferred to the UI layer via `skillIDToString()`. The domain core never produces display strings.

### 3.3 DifficultyLevel (enum class)

```cpp
enum class DifficultyLevel { Easy, Medium, Hard };
```

Difficulty is not a property of the question itself — it is a parameter that the `AdaptiveSelector` passes to the generator based on the user's current performance for that skill.

### 3.4 Question (immutable struct)

```cpp
struct Question {
    int operand1;
    int operand2;
    Operation operation;
    int correctAnswer;
    SkillID skillId;
    DifficultyLevel difficulty;

    std::string display() const;
    bool checkAnswer(int answer) const;
};
```

A `Question` is a value object — once created, it is never modified. It carries all the information needed to display the question, validate the answer, and attribute the result to the correct skill and difficulty level. The `checkAnswer()` method encapsulates the validation logic so that no other component needs to know how to compute correctness.

### 3.5 SkillStats (per-skill metrics)

```cpp
struct SkillStats {
    int attempts = 0;
    int correct = 0;
    double totalTime = 0.0;
    std::time_t lastPracticed = 0;
    double parTime = 5.0;  // expected time for a skilled person

    double accuracy() const;
    double averageTime() const;
    double normalizedResponseTime() const;
    double priorityScore() const;
    void recordAttempt(bool wasCorrect, double responseTime);
};
```

This struct is the heart of the adaptive system. Each skill maintains its own `SkillStats`, enabling independent performance tracking.

**Par time** (`parTime`) is the expected response time for a skilled practitioner on that skill type. It varies per skill — basic addition has a par time of 3 seconds, while larger division has 10 seconds. This normalization prevents slow skills from dominating the priority formula.

**`lastPracticed`** is included even in v1 for forward compatibility. The priority formula currently uses accuracy and response time, but adding a recency component (spaced repetition) requires no redesign — simply add a recency term to `priorityScore()`.

**JSON serialization** is provided via `to_json()` / `from_json()` free functions for nlohmann/json, enabling `json j = stats` instead of manual field-by-field serialization.

---

## 4. Question Generation System

### 4.1 IQuestionGenerator (interface)

```cpp
class IQuestionGenerator {
public:
    virtual ~IQuestionGenerator() = default;
    virtual Question generate(DifficultyLevel difficulty) = 0;
    virtual SkillID getSkillID() const = 0;
};
```

This is the **Open-Closed Principle** in action. To add a new skill (e.g., "Negative Number Addition"), you create a new class implementing `IQuestionGenerator` and register it with the `AdaptiveSelector`. No existing code is modified.

The `generate(DifficultyLevel)` signature accepts difficulty as a parameter rather than having the generator choose it internally. This keeps difficulty selection under the control of the `AdaptiveSelector`, which has access to the user's performance data.

### 4.2 Constraint-Based Generation (O(1) Construction)

Each generator produces questions that satisfy mathematical constraints by **constructing digits directly** rather than generating random numbers and rejecting invalid ones.

#### Example: CarryAddition (Easy — 2-digit + 2-digit, exactly 1 carry)

Instead of:
```
while (true):
    a = random(10, 99)
    b = random(10, 99)
    if has_exactly_one_carry(a, b):
        return Question(a, b)
    // Could loop forever with bad luck
```

We do:
```
// Decide which position carries (ones or tens)
carryPos = random(0, 1)

if carryPos == 0:
    // Carry at ones position
    o1 = random(1, 9)
    o2 = random(10 - o1, 9)       // o1 + o2 >= 10 guaranteed
    t1 = random(0, 8)
    t2 = random(0, 8 - t1)        // t1 + t2 + 1 < 10 guaranteed (no second carry)
else:
    // Carry at tens position
    o1 = random(0, 8)
    o2 = random(0, 8 - o1)        // o1 + o2 < 10 guaranteed (no carry at ones)
    t1 = random(1, 9)
    t2 = random(10 - t1, 9)       // t1 + t2 >= 10 guaranteed
```

This is **guaranteed O(1)** — no loops, no rejection, no possibility of infinite iteration if constraints become impossible.

#### Example: ExactDivision (inverse construction)

Division generators never perform division at random and check for remainders. Instead:

```
quotient = random(2, 49)
divisor  = random(2, 9)
dividend = quotient × divisor     // Exact by construction
```

The displayed question is `dividend ÷ divisor = ?`, and the answer is `quotient`. This guarantees the question is always a valid exact division problem.

### 4.3 RNG Ownership

Each generator owns its own `std::mt19937` instance, initialized once in the constructor:

```cpp
class BasicAdditionGenerator : public IQuestionGenerator {
    mutable std::mt19937 rng_;
public:
    BasicAdditionGenerator() : rng_(std::random_device{}()) {}
    // ...
};
```

This avoids the anti-pattern of recreating the RNG on every `generate()` call. A `std::mt19937` engine maintains internal state that produces high-quality pseudo-random sequences; recreating it from `std::random_device` on each call wastes entropy and can produce correlated sequences if the OS random device is slow.

### 4.4 Generator Specifications

| Generator | Easy | Medium | Hard |
|-----------|------|--------|------|
| **BasicAddition** | 1-digit + 1-digit, sum < 10 | 2-digit + 2-digit, no carry | 3-digit + 3-digit, no carry |
| **CarryAddition** | 2-digit + 2-digit, 1 carry | 3-digit + 3-digit, 2 carries | 3-digit + 3-digit, all 3 carry |
| **BasicSubtraction** | 1-digit − 1-digit, result ≥ 0 | 2-digit − 2-digit, no borrow | 3-digit − 3-digit, no borrow |
| **BorrowSubtraction** | 2-digit − 2-digit, 1 borrow | 3-digit − 3-digit, 2 borrows | 3-digit − 3-digit, all 3 borrow |
| **TablesMultiplication** | factors 1–5 | factors 6–9 | 2-digit × 1-digit |
| **LargerMultiplication** | 2-digit × 1-digit | 2-digit × 2-digit | 3-digit × 2-digit |
| **ExactDivision** | 1-digit ÷ 1-digit | 2-digit ÷ 1-digit | 3-digit ÷ 1-digit |
| **LargerDivision** | 2-digit ÷ 2-digit | 3-digit ÷ 2-digit | 4-digit ÷ 2-digit |

---

## 5. Skill Progression (Prerequisite Graph)

### 5.1 Graph Structure

```
BasicAddition ──────► CarryAddition ──────► BorrowSubtraction
     │
     ▼
BasicSubtraction

TablesMultiplication ──► LargerMultiplication

ExactDivision ─────────► LargerDivision
```

**Root skills** (no prerequisites, always available):
- BasicAddition
- TablesMultiplication
- ExactDivision

**Edge semantics**: An edge `A → B` means "mastering A is required before B becomes available."

### 5.2 Unlocking Mechanism

A skill is **unlocked** when all of its prerequisite skills satisfy:

1. **Minimum attempts**: `attempts >= 5` — prevents unlocking based on luck
2. **Accuracy threshold**: `accuracy() >= 0.70` (70%) — configurable via `setMasteryThreshold()`

If a skill has multiple prerequisites, **all** must meet both conditions. This ensures the user has a solid foundation before encountering more complex skills.

### 5.3 Implementation

```cpp
class SkillDAG {
    std::unordered_map<SkillID, std::vector<SkillID>> prerequisites_;
    std::unordered_map<SkillID, std::vector<SkillID>> dependents_;
    double masteryThreshold_ = 0.70;

    void addEdge(SkillID from, SkillID to);
    void buildDefaultDAG();

public:
    const std::vector<SkillID>& getPrerequisites(SkillID id) const;
    const std::vector<SkillID>& getDependents(SkillID id) const;
    bool isUnlocked(SkillID id, const std::unordered_map<SkillID, SkillStats>& stats) const;
    std::vector<SkillID> getUnlockedSkills(...) const;
    std::vector<SkillID> getRootSkills() const;
    std::vector<SkillID> topologicalOrder() const;    // Kahn's algorithm
    std::string describePath(SkillID id) const;
};
```

Two adjacency maps (`prerequisites_` and `dependents_`) are maintained for efficient traversal in both directions. `topologicalOrder()` uses **Kahn's algorithm** to produce a valid ordering where prerequisites always appear before their dependents.

### 5.4 Why a Lightweight Prerequisite Graph?

A linear difficulty progression (Level 1 → Level 2 → ...) assumes all skills lie on a single path. In reality, arithmetic skills have independent branches — you don't need to master division before learning carry addition. The DAG allows:

- **Branching**: Addition and multiplication progress independently
- **Merging**: BorrowSubtraction requires both BasicSubtraction and CarryAddition experience (carry understanding is needed to understand multi-position borrowing)
- **Parallel development**: The user can focus on whichever branch they choose

### 5.5 Topological Order

The topological sort produced by Kahn's algorithm determines the display order in reports and the skill tree visualization:

```
1. BasicAddition
2. BasicSubtraction
3. CarryAddition
4. BorrowSubtraction
5. TablesMultiplication
6. LargerMultiplication
7. ExactDivision
8. LargerDivision
```

---

## 6. Adaptive Practice Engine

### 6.1 Priority Score Formula

Each skill has a **priority score** that quantifies how much the user needs to practice it:

```
score = 0.6 × (1 − accuracy) + 0.4 × normalizedResponseTime
```

**Intuition**:
- `(1 − accuracy)` is high when the user gets many wrong → needs more practice
- `normalizedResponseTime` is high when the user is slow → needs more practice
- The 0.6/0.4 weighting gives accuracy more influence than speed, reflecting that correctness matters more than speed for skill development

**Score range**:
- A skill with 0% accuracy and very slow responses: `0.6 × 1.0 + 0.4 × 1.0 = 1.0` (maximum)
- A skill with 100% accuracy and faster-than-par responses: `0.6 × 0.0 + 0.4 × 0.0 = 0.0` (minimum)

### 6.2 Response Time Normalization

Raw response times are not directly comparable across skills — division naturally takes longer than basic addition. Normalization solves this:

```
t_capped = min(averageTime, parTime)
normalizedResponseTime = t_capped / parTime
```

**Capping** at `parTime` prevents outlier responses from inflating the score. If a user takes 30 seconds on a skill with par time 5 seconds, the normalized value is 1.0, not 6.0. This keeps the priority score bounded and prevents broken weighting.

**Per-skill par times**:

| Skill | Par Time | Rationale |
|-------|----------|-----------|
| BasicAddition | 3.0s | Simple single/double digit addition |
| CarryAddition | 5.0s | Requires mental carry management |
| BasicSubtraction | 3.0s | Simple single/double digit subtraction |
| BorrowSubtraction | 6.0s | Borrowing adds cognitive load |
| TablesMultiplication | 3.0s | Rote recall of times tables |
| LargerMultiplication | 8.0s | Multi-digit requires partial products |
| ExactDivision | 4.0s | Inverse of multiplication tables |
| LargerDivision | 10.0s | Most cognitively demanding |

### 6.3 Weighted Random Selection

The `AdaptiveSelector` does not always pick the highest-scoring skill — that would cause the user to only practice their worst skill and never maintain stronger ones. Instead, it uses **weighted random sampling**:

1. Collect all unlocked skills that have registered generators
2. Compute `score + ε` for each (ε = 0.05 ensures every skill has some probability)
3. Construct a `std::discrete_distribution` from the adjusted scores
4. Sample one skill using the distribution

This means:
- Weaker skills are selected **more often** (proportional to their score)
- Stronger skills are still selected **sometimes** (preventing skill decay)
- The selection is **non-deterministic**, keeping the experience varied

### 6.4 Difficulty Suggestion

After selecting a skill, the `AdaptiveSelector` determines the appropriate difficulty:

```
if attempts < 3:
    → Easy                          // Insufficient data, start safe
if accuracy ≥ 85% AND avgTime ≤ 1.2 × parTime:
    → Hard                          // Mastered, push to next level
if accuracy ≥ 70% AND avgTime ≤ 1.5 × parTime:
    → Medium                        // Competent, increase challenge
otherwise:
    → Easy                          // Still developing
```

The minimum attempt requirement (3) prevents the system from suggesting Hard difficulty based on 1–2 lucky answers.

---

## 7. SkillManager (Data Ownership Layer)

### 7.1 Responsibility

`SkillManager` is the **single owner** of all skill-related data. No other component directly edits statistics or records — they go through `SkillManager`'s API.

```cpp
class SkillManager {
    std::unordered_map<SkillID, SkillStats> stats_;
    std::map<SkillID, SkillRecord> records_;
    GlobalRecord globalRecord_;
    SkillDAG dag_;

public:
    void recordAttempt(SkillID id, bool wasCorrect, double responseTime);
    SkillStats& getSkillStats(SkillID id);
    SkillRecord& getSkillRecord(SkillID id);
    GlobalRecord& globalRecord();
    double overallAccuracy() const;
    std::vector<std::pair<SkillID, double>> getWeakestSkills(int count) const;
    std::vector<SkillID> getUnlockedSkills() const;
    void reset();
    void startSession();
    const SkillDAG& dag() const;
};
```

### 7.2 Why SkillManager Exists

Without `SkillManager`, the `Session` would directly edit `unordered_map<SkillID, SkillStats>`. This violates SRP because `Session` would then be responsible for both orchestration and data management. By introducing `SkillManager`:

- `Session` only calls `recordAttempt()` — it doesn't know the internal data structure
- `AdaptiveSelector` queries via `getSkillStats()` and `getUnlockedSkills()`
- `Statistics` reads via `allStats()` and `allRecords()`
- `Persistence` serializes the data via the same public API

If the internal data structure changes (e.g., migrating from `unordered_map` to a vector indexed by `SkillID`), only `SkillManager` changes.

### 7.3 Record Keeping

**SkillRecord** tracks personal bests per skill:

```cpp
struct SkillRecord {
    double bestAccuracy = 0.0;    // Best accuracy ratio (0.0–1.0)
    int bestStreak = 0;           // Longest consecutive correct streak
    double fastestAvgTime = 0.0;  // Fastest average response time
};
```

**GlobalRecord** tracks overall milestones:

```cpp
struct GlobalRecord {
    int overallBestStreak = 0;
    double overallBestAccuracy = 0.0;
    double overallFastestAvg = 0.0;
    int totalSessions = 0;
    int totalQuestions = 0;
};
```

Records are updated in `recordAttempt()`, keeping the update logic co-located with the data it modifies.

---

## 8. Statistics (Analytics Layer)

### 8.1 Responsibility

`Statistics` is a **read-only analytics wrapper** over `SkillManager`. It does not modify data — it produces formatted reports for the `TerminalUI`.

```cpp
class Statistics {
    SkillManager& skillManager_;

public:
    std::string skillReport(SkillID id) const;
    std::string fullReport() const;
    std::string sessionSummary(const SessionSummary& summary) const;
    std::string skillComparisonTable() const;
    std::string skillTreeDisplay() const;
};
```

### 8.2 Report Types

**Per-Skill Report** — Shows status, attempts, accuracy (with progress bar), average time vs. par time, priority score, personal records, and prerequisite chain.

**Full Report** — All skills in topological order plus global records.

**Skill Comparison Table** — ASCII table with accuracy, average time, and priority for side-by-side comparison.

**Skill Tree Display** — Indented visualization of the DAG showing `[UNLOCKED]`/`[LOCKED]` status and current accuracy for each skill.

**Session Summary** — Post-session overview: questions answered, accuracy, total time, current and best streak.

### 8.3 Why a Separate Statistics Class?

The formatting logic (progress bars, table borders, percentage formatting) is presentation concern that doesn't belong in `SkillManager`. Separating it allows:
- Changing report formats without touching domain logic
- Adding new report types (e.g., CSV export) without modifying `SkillManager`
- Testing formatting independently

---

## 9. Persistence (Storage Layer)

### 9.1 Design

```cpp
class Persistence {
    std::string dataDir_;

public:
    bool saveStatistics(const SkillManager& manager) const;
    bool loadStatistics(SkillManager& manager) const;
    bool hasSaveData() const;
    bool deleteSaveData() const;
};
```

**Self-documenting API**: `saveStatistics()` and `loadStatistics()` clearly indicate what is persisted, unlike generic `save()`/`load()` names.

### 9.2 JSON Schema

```json
{
  "skillStats": {
    "0": {
      "attempts": 25,
      "correct": 20,
      "totalTime": 62.5,
      "lastPracticed": 1709000000,
      "parTime": 3.0
    }
  },
  "skillRecords": {
    "0": {
      "bestAccuracy": 0.85,
      "bestStreak": 8,
      "fastestAvgTime": 2.1
    }
  },
  "globalRecord": {
    "overallBestStreak": 12,
    "overallBestAccuracy": 0.88,
    "overallFastestAvg": 0.0,
    "totalSessions": 5,
    "totalQuestions": 150
  }
}
```

Keys in `skillStats` and `skillRecords` are the integer values of `SkillID` (e.g., `"0"` for `BasicAddition`). This avoids embedding enum name strings and keeps the schema compact.

### 9.3 JSON Serialization

The project uses nlohmann/json's `to_json()` / `from_json()` free function overloads:

```cpp
// Automatic: json j = stats;  instead of manually writing each field
inline void to_json(nlohmann::json& j, const SkillStats& s) {
    j = nlohmann::json{
        {"attempts", s.attempts},
        {"correct", s.correct},
        {"totalTime", s.totalTime},
        {"lastPracticed", static_cast<std::int64_t>(s.lastPracticed)},
        {"parTime", s.parTime}
    };
}
```

This is the **modern C++ approach** to JSON serialization — the compiler resolves the overload based on type, and the syntax reads naturally.

### 9.4 Dependency Management

`nlohmann/json` is integrated via CMake `FetchContent`:

```cmake
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)
```

This downloads and builds the dependency at configure time, requiring no manual installation or system-level package manager. This is the recommended approach for header-only libraries in modern CMake.

---

## 10. TerminalUI (Presentation Layer)

### 10.1 Design Principle

**Main should never print.** All terminal output goes through `TerminalUI`. This ensures:
- Consistent formatting and color usage
- The ability to swap UI implementations (e.g., a GUI) without touching domain logic
- Testability of domain logic without I/O side effects

### 10.2 API

```cpp
class TerminalUI {
public:
    // Display
    void showWelcome() const;
    void showMainMenu() const;
    void showSessionConfigMenu() const;
    void showQuestion(const Question& q, int num, int streak) const;
    void showCorrect(int streak) const;
    void showIncorrect(const Question& q, int answer) const;
    void showSessionEnd() const;
    void showMessage(const std::string& msg) const;
    void showError(const std::string& msg) const;
    void showProgress(int current, int total, double elapsed, double limit) const;

    // Input
    int getMenuChoice() const;
    SessionConfig getSessionConfig() const;
    std::optional<int> getAnswer() const;
    bool getYesNo(const std::string& prompt) const;
};
```

### 10.3 ANSI Color System

The UI uses a `Color` struct with `static constexpr` ANSI escape codes:

```cpp
struct Color {
    static constexpr const char* Reset   = "\033[0m";
    static constexpr const char* Bold    = "\033[1m";
    static constexpr const char* Green   = "\033[32m";
    // ...
};
```

Color usage follows a semantic scheme:
- **Green**: Correct answers, positive actions
- **Red**: Wrong answers, errors, destructive actions
- **Yellow**: Headers, borders, streaks
- **Cyan**: Informational messages, question display
- **Magenta**: Exceptional achievements (10+ streak)
- **Dim**: Secondary info (question numbers, skill labels)

### 10.4 Streak Feedback

The UI provides escalating feedback for consecutive correct answers:
- 1–4 correct: "CORRECT!"
- 5–9 correct: "CORRECT! Streak xN!"
- 10+ correct: "CORRECT! Streak xN! AMAZING!"

This creates a game-like incentive to maintain accuracy.

---

## 11. Session (Orchestration Layer)

### 11.1 Lifecycle

```
main()
  └── Session()
        ├── Constructs all modules (SkillManager, AdaptiveSelector, Statistics, Persistence, UI)
        ├── Registers 8 generators with AdaptiveSelector
        ├── Loads saved progress from data/progress.json
        └── run()
              ├── Shows welcome banner
              └── Enters main menu loop
                    ├── 1: Practice session
                    ├── 2: View statistics
                    ├── 3: View skill tree
                    ├── 4: View personal records
                    ├── 5: Reset progress (with confirmation)
                    └── 6: Save + exit
```

### 11.2 Practice Session Flow

```
1. Get session configuration (duration / question count / endless)
2. Start session counter in SkillManager
3. Loop:
   a. Check termination condition (time limit / question count)
   b. Show progress bar / timer
   c. AdaptiveSelector.nextQuestion()
      ├── selectSkill() — weighted random by priority score
      ├── suggestDifficulty() — based on current accuracy & speed
      └── generator.generate(difficulty) — constraint-based O(1) construction
   d. Display question
   e. Measure response time
   f. Validate answer
   g. SkillManager.recordAttempt() — updates stats + records
   h. Update streak tracking
   i. Show feedback (correct/incorrect)
4. Show session summary + skill comparison table
5. Auto-save progress to JSON
```

### 11.3 Why Session Owns Everything

`Session` acts as the **composition root** — the single point where all dependencies are wired together. This pattern (from dependency injection theory) ensures that:
- No module creates its own dependencies
- The dependency graph is explicit and visible
- Modules can be tested in isolation with mock dependencies

---

## 12. Design Patterns Used

### 12.1 Strategy Pattern (IQuestionGenerator)

The `IQuestionGenerator` interface allows `AdaptiveSelector` to switch between generation strategies at runtime without knowing the concrete type. This is the Strategy pattern — the algorithm (question generation) varies independently of the client (selector).

### 12.2 Template Method Pattern (Generator Difficulty)

Each generator's `generate(DifficultyLevel)` method dispatches to `generateEasy()`, `generateMedium()`, or `generateHard()`. While not a strict Template Method (no base class implementation), the pattern of delegating to difficulty-specific methods is consistent across all generators.

### 12.3 Facade Pattern (SkillManager)

`SkillManager` provides a simplified interface to a complex subsystem (stats, records, DAG, unlocking). Clients like `Session` and `AdaptiveSelector` interact with `SkillManager` rather than directly manipulating maps and graphs.

### 12.4 Observer-like Pattern (Record Updates)

When `SkillManager::recordAttempt()` is called, it updates both the skill stats and the personal records. The record update is triggered by the same call that updates stats — there is no separate "notify" step. This co-location ensures records are always consistent with stats.

---

## 13. CMake Design

### 13.1 Modern CMake Practices

```cmake
# target_include_directories (not include_directories)
target_include_directories(adaptive_math_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# FetchContent for dependencies (not find_package or manual download)
FetchContent_Declare(json ...)
FetchContent_MakeAvailable(json)

# target_link_libraries with modern targets (not library names)
target_link_libraries(adaptive_math_core
    PUBLIC nlohmann_json::nlohmann_json
)
```

### 13.2 Library + Executable Structure

The project builds a static library (`adaptive_math_core`) containing all domain logic, and a thin executable (`adaptive-math-trainer`) that links against it. This separation enables:
- Future unit testing by linking the test executable against `adaptive_math_core`
- Separate compilation — changes to `main.cpp` don't recompile the library

---

## 14. Roadmap

The architecture is designed for extension without modification:

| Future Feature | What to Change | What Stays the Same |
|---------------|----------------|---------------------|
| New skill type | Add a new `IQuestionGenerator` + `SkillID` enum value | Everything else |
| Glicko-2 adaptive ratings | Replace heuristic priority with Glicko-2 rating and rating deviation | Overall architecture |
| Spaced repetition | Incorporate recency scheduling using `lastPracticed` | Overall architecture |
| GUI frontend | Replace `TerminalUI` with a GUI implementation | All domain classes |
| CSV export | Add method to `Statistics` | Domain classes |
| Unit tests | Link test executable against `adaptive_math_core` | No changes needed |

---

## 15. Non-Goals (Intentionally Excluded from v1)

- GUI framework (Qt, ncurses)
- Networking / multiplayer
- Database storage (SQLite, etc.)
- Cloud deployment
- Machine learning models
- User authentication / accounts
- Animations or cosmetic effects

These are excluded because they add complexity without significantly improving the engineering quality or resume value of the project. The architecture supports adding them later if needed.
