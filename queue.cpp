#include "queue.h"
#include <omp.h>
#include <climits>

// ==========================================
// Part B0: MutexQueue (Skeleton Architecture)
// ==========================================
MutexQueue::MutexQueue(): head(nullptr), tail(nullptr) {
	omp_init_lock(&lock);
}
MutexQueue::~MutexQueue() {
	Node* curr = head;
	while (curr != NULL) {
	
		Node* next = curr->next;
		delete curr;
		curr = next;
	}
	omp_destroy_lock(&lock);
}

void MutexQueue::enqueue(int value) {
    // High-level architecture: Lock mutex, append node to tail, unlock mutex
	Node* new_node = new Node(value);
	omp_set_lock(&lock);
	if (tail == NULL) {
		tail = new_node;
		head = new_node;
	}
	else {
		tail->next = new_node;
		tail = new_node;
	}
	omp_unset_lock(&lock);
}

int MutexQueue::dequeue(int *value) {
    // High-level architecture: Lock mutex, check if empty, detach head node, unlock mutex
	omp_set_lock(&lock);
	if (head == NULL) {
		omp_unset_lock(&lock);
		return 0;
	}
    
	Node* old_head = head;
	head = head->next;
	if (head == NULL) {
		tail = nullptr;
	}
	*value = old_head->value;
	omp_unset_lock(&lock);

	return 1;
}

// ==========================================
// Part B1: MSQueue (Michael-Scott Skeleton Architecture)
// ==========================================
MSQueue::MSQueue() {
	Node* dummy = new Node(INT_MIN);
	head = dummy;
	tail = dummy;
}
MSQueue::~MSQueue() {
	Node* curr = head;
	while (curr != NULL) {
	
		Node* next = curr->next;
		delete curr;
		curr = next;
	}
}

void MSQueue::enqueue(int value) {
    // High-level architecture: Allocate node, atomic CAS to swing tail->next, then swing tail
	int success = 0;
	Node* new_node = new Node(value);
	while (!success) {
		Node* old_tail = tail.load();
		Node* tail_next = old_tail->next.load();
		if (tail_next != NULL) {
			tail.compare_exchange_weak(old_tail, tail_next);
			continue;
		}
		else if (tail_next == NULL) {
			Node* expected = NULL;
			if(old_tail->next.compare_exchange_weak(expected, new_node)) {
				tail.compare_exchange_weak(old_tail, new_node);
				success = 1;
				return;
			}
		}
	}

}

int MSQueue::dequeue(int *value) {
    // High-level architecture: Read head & tail, atomic CAS to advance head, extract value
	while (true) {
		Node* old_tail = tail.load();
		Node* old_head = head.load();
		Node* next = old_head->next.load(); // head is never null
		if (old_head == head.load()) {
			if (old_head == old_tail) {
				if (next == NULL) {
					return 0;
				}
				else if (old_tail->next != NULL) {
					Node* tail_next = old_tail->next.load();
					tail.compare_exchange_weak(old_tail, tail_next);
					continue;
				}
			}
			else {
				if (next == NULL) {
					continue;
				}
				if (head.compare_exchange_weak(old_head, old_head->next)) {
					*value = next->value;
					return 1;
				}

			}

		}
	}
}

