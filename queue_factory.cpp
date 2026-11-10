#include "queue.h"
#include <stdexcept>

std::unique_ptr<Queue> create_queue(QueueType type) {
    switch (type) {
        case QueueType::MUTEX:
            return std::make_unique<MutexQueue>();
        case QueueType::MS:
            return std::make_unique<MSQueue>();
        default:
            throw std::invalid_argument("Unknown queue implementation type.");
    }
}

QueueType parse_queue_type(const std::string& impl_str) {
    if (impl_str == "mutex" || impl_str == "baseline") {
        return QueueType::MUTEX;
    } else if (impl_str == "ms" || impl_str == "michael_scott") {
        return QueueType::MS;
	}
    throw std::invalid_argument("Invalid queue --impl argument: " + impl_str);
}
