#include "stack.h"
#include <iostream>
#include <omp.h>

// ==========================================
// Part A0: MutexStack (Skeleton Architecture)
// ==========================================
MutexStack::MutexStack() : head(nullptr) {
	omp_init_lock(&lock);
	
}
MutexStack::~MutexStack() {
	
	Node* curr = head;
	while (curr != NULL) {
	
		Node* next = curr->next;
		delete curr;
		curr = next;
		
	}
	omp_destroy_lock(&lock);
}

void MutexStack::push(int value) {
	Node* new_node = new Node(value);
	omp_set_lock(&lock);
	new_node->next = head;
	head = new_node;
	omp_unset_lock(&lock);
}

int MutexStack::pop(int *value) {
	omp_set_lock(&lock);

	if (head == NULL) {
		omp_unset_lock(&lock);
		return 0;
	}
	
	Node* old_head = head;
	head = head->next;
	*value = old_head->val;
	omp_unset_lock(&lock);

    return 1;
}

// ==========================================
// Part A1: TreiberStack (Skeleton Architecture)
// ==========================================
TreiberStack::TreiberStack() : head(nullptr) {}
TreiberStack::~TreiberStack() {
	Node* curr = head;
	while (curr != NULL) {
	
		Node* next = curr->next;
		delete curr;
		curr = next;
		
	}

}

bool TreiberStack::try_push(int value) {
	Node* new_node = new Node(value);
	new_node->next = head.load();
	if (head.compare_exchange_weak(new_node->next, new_node)) {
		return true;
	}
	delete new_node;
	return false;
}

int TreiberStack::try_pop(int* value) {
	Node* old_head = head.load();
	if (old_head == NULL) {
		return 0;
	}
	if (head.compare_exchange_weak(old_head, old_head->next)) {
		*value = old_head->val;
		return 1;
	}
	return -1;
}
void TreiberStack::push(int value) {
	while (!try_push(value)) {}
}

int TreiberStack::pop(int* value) {
	while (true) {
		int res = try_pop(value);
		if (res == 1 || res == 0) {
			return res;
		}
		// res == -1, repeat loop
	}

}

// ==========================================
// Part A2: EliminationStack (Skeleton Architecture)
// ==========================================
EliminationStack::EliminationStack() {}
EliminationStack::~EliminationStack() {}

int EliminationStack::ExchangerSlot::push_update_status(int val) {
	Status expected = EMPTY;
	value = val;
	if (!status.compare_exchange_strong(expected, WAITING)) {
		return 0;
	}


	for (int spin = 0; spin < 100; spin++) {
		//if status is busy, a pop found it
		if (status.load(std::memory_order_acquire) == BUSY) {
			status.store(EMPTY, std::memory_order_release);
			return 1;
		}
	}

	expected = WAITING;
	if (!status.compare_exchange_strong(expected, EMPTY)) {
		return 0;
	}

	
	status.store(EMPTY, std::memory_order_release);
	return 1;
}


int EliminationStack::ExchangerSlot::pop_update_status(int *val) {
	Status expected = WAITING;
	if (status.compare_exchange_strong(expected, BUSY)) {
		*val = value;
		return 1;
	}
	return 0;
}


int EliminationStack::pick_random_elimination_slot() {
	static std::atomic<unsigned int> thread_counter{1};

	thread_local unsigned int seed = thread_counter.fetch_add(1) * 2654435761u;
	
	seed ^= seed << 13;
	seed ^= seed >> 17;
	seed ^= seed << 5;
	
	return seed & 7;
}

void EliminationStack::push(int value) {
    // High-level architecture: Try central stack, back off to elimination array on contention
	int success = 0;
	while (!success) {
		if (stack.try_push(value)) {
			return;
		}	

		int slot_ind = pick_random_elimination_slot();
		
		success = eliminationArray[slot_ind].push_update_status(value);
	}
}

int EliminationStack::pop(int *value) {
	int success = 0;
	while (!success) {
		int res = stack.try_pop(value);
		if (res == 1 || res == 0) {
			return res;
		}
		int slot_ind = pick_random_elimination_slot();
		
		success = eliminationArray[slot_ind].pop_update_status(value);

	}
	return success;
    // High-level architecture: Try central stack, back off to elimination array on contention
}
