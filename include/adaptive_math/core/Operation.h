#pragma once

#include <string>

namespace adaptive_math {

enum class Operation {
    Add,
    Subtract,
    Multiply,
    Divide
};

inline std::string operationToString(Operation op) {
    switch (op) {
        case Operation::Add:      return "+";
        case Operation::Subtract: return "-";
        case Operation::Multiply: return "x";
        case Operation::Divide:   return "/";
    }
    return "?";
}

inline std::string operationName(Operation op) {
    switch (op) {
        case Operation::Add:      return "Addition";
        case Operation::Subtract: return "Subtraction";
        case Operation::Multiply: return "Multiplication";
        case Operation::Divide:   return "Division";
    }
    return "Unknown";
}

} // namespace adaptive_math
