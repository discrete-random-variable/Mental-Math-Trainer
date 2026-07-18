#pragma once

#include "Operation.h"
#include "SkillID.h"
#include "DifficultyLevel.h"
#include <string>

namespace adaptive_math {

struct Question {
    int operand1;
    int operand2;
    Operation operation;
    int correctAnswer;
    SkillID skillId;
    DifficultyLevel difficulty;

    std::string display() const {
        return std::to_string(operand1) + " " +
               operationToString(operation) + " " +
               std::to_string(operand2) + " = ?";
    }

    bool checkAnswer(int answer) const {
        return answer == correctAnswer;
    }
};

} // namespace adaptive_math
