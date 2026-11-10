#ifndef STACK_H
#define STACK_H

#include <memory>
#include <string>
#include <omp.h>
#include <atomic>
#include <array>

// Enum representing the stack implementations supported via runtime flags
enum class StackType {
    MUTEX,          // Lock-based baseline (Part A0)
    TREIBER,        // Lock-free Treiber stack (Part A1)
    ELIMINATION     // Elimination-backoff stack (Part A2)
};

// Abstract base class defining the uniform stack interface
class Stack {
public:
    virtual ~Stack() = default;

    // Push value onto the stack
    virtual void push(int value) = 0;

    // Pop value from the stack.
    // Returns 1 on success (writing value to *value), 0 if stack is empty.
    virtual int pop(int *value) = 0;
};

// High-level architecture for Part A0: Lock-based stack baseline
class MutexStack : public Stack {
private:
	struct Node {
		int val;
		Node* next;
		Node(int value, Node* nxt) : val(value), next(nxt) {} 
		Node(int value) : val(value), next(nullptr) {}
	};
	Node* head{nullptr};
	omp_lock_t lock;
	
public:
    MutexStack();
    ~MutexStack() override;

    void push(int value) override;
    int pop(int *value) override;
};

// High-level architecture for Part A1: Lock-free Treiber stack
class TreiberStack : public Stack {
private:
	struct Node {
		int val;
		Node* next;
		Node(int value, Node* nxt) : val(value), next(nxt) {} 
		Node(int value) : val(value), next(nullptr) {}
	};
	std::atomic<Node*> head{nullptr};
public:
    TreiberStack();
    ~TreiberStack() override;

	bool try_push(int value);
	int try_pop(int *value);
    void push(int value) override;
    int pop(int *value) override;
};

// High-level architecture for Part A2: Elimination-backoff stack
class EliminationStack : public Stack {
private:
	struct alignas(64) ExchangerSlot {
		enum Status {
			EMPTY,
			WAITING,
			BUSY
		};
		
		int value;
		std::atomic<Status> status;
		ExchangerSlot() : status(EMPTY) {}
		int push_update_status(int val);
		int pop_update_status(int *val);
	};
	std::array<ExchangerSlot, 8> eliminationArray{};
	TreiberStack stack;
public:
    EliminationStack();
    ~EliminationStack() override;

	int pick_random_elimination_slot(); 
    void push(int value) override;
    int pop(int *value) override;
};

// Factory functions
std::unique_ptr<Stack> create_stack(StackType type);
StackType parse_stack_type(const std::string& impl_str);

#endif // STACK_H
