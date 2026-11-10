#include "stack.h"
#include <stdexcept>

std::unique_ptr<Stack> create_stack(StackType type) {
    switch (type) {
        case StackType::MUTEX:
            return std::make_unique<MutexStack>();
        case StackType::TREIBER:
            return std::make_unique<TreiberStack>();
        case StackType::ELIMINATION:
            return std::make_unique<EliminationStack>();
        default:
            throw std::invalid_argument("Unknown stack implementation type.");
    }
}

StackType parse_stack_type(const std::string& impl_str) {
    if (impl_str == "mutex" || impl_str == "baseline") {
        return StackType::MUTEX;
    } else if (impl_str == "treiber") {
        return StackType::TREIBER;
    } else if (impl_str == "elimination") {
        return StackType::ELIMINATION;
    }
    throw std::invalid_argument("Invalid --impl argument: " + impl_str);
}
