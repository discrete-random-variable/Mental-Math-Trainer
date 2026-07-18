#pragma once

#include "Question.h"
#include "DifficultyLevel.h"
#include "SkillID.h"
#include <memory>

namespace adaptive_math {

class IQuestionGenerator {
public:
    virtual ~IQuestionGenerator() = default;

    virtual Question generate(DifficultyLevel difficulty) = 0;
    virtual SkillID  getSkillID() const = 0;
};

} // namespace adaptive_math
