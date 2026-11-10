#ifndef QUEUE_H
#define QUEUE_H

#include <memory>
#include <string>
#include <omp.h>
#include <atomic>

// Enum representing the queue implementations supported via runtime flags
enum class QueueType {
    MUTEX,          // Lock-based baseline (Part B0)
    MS              // Michael-Scott lock-free queue (Part B1)
};

// Abstract base class defining the uniform queue interface
class Queue {
public:
    virtual ~Queue() = default;

    // Enqueue value into the queue
    virtual void enqueue(int value) = 0;

    // Dequeue value from the queue.
    // Returns 1 on success (writing value to *value), 0 if queue is empty.
    virtual int dequeue(int *value) = 0;
};

// High-level architecture for Part B0: Lock-based queue baseline
class MutexQueue : public Queue {
private: 
	struct Node {
		int value;
		Node* next;
		Node(int value, Node* nxt) : value(value), next(nxt) {} 
		Node(int value) : value(value), next(nullptr) {}
	};
	Node* head{nullptr};
	Node* tail{nullptr};
	omp_lock_t lock;
public:
    MutexQueue();
    ~MutexQueue() override;

    void enqueue(int value) override;
    int dequeue(int *value) override;
};

// High-level architecture for Part B1: Michael-Scott lock-free queue
class MSQueue : public Queue {
private:
	struct Node {
		int value;
		std::atomic<Node*> next;
		Node(int value, Node* nxt) : value(value), next(nxt) {} 
		Node(int value) : value(value), next(nullptr) {}
	};
	std::atomic<Node*> head;
	std::atomic<Node*> tail;
	
public:
    MSQueue();
    ~MSQueue() override;

    void enqueue(int value) override;
    int dequeue(int *value) override;
};


// Factory functions
std::unique_ptr<Queue> create_queue(QueueType type);
QueueType parse_queue_type(const std::string& impl_str);

#endif // QUEUE_H
