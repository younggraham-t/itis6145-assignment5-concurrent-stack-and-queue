#include "stack.h"
#include "queue.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <stdexcept>

static int BASE_SEED = 12345;

enum StackWorkloadType : int {
	PUSH_HEAVY = 80,
	POP_HEAVY = 20,
	BALANCED = 50
};


StackWorkloadType parse_stack_workload(const std::string& workload_str) {
    if (workload_str == "push_heavy" || workload_str == "push-heavy" || workload_str == "push") {
        return StackWorkloadType::PUSH_HEAVY;
    } else if (workload_str == "pop_heavy" || workload_str == "pop-heavy" || workload_str == "pop") {
        return StackWorkloadType::POP_HEAVY;
    } else if (workload_str == "balanced") {
        return StackWorkloadType::BALANCED;
    }
    throw std::invalid_argument("Invalid --workload argument: " + workload_str + 
                                " (expected 'balanced', 'push_heavy', or 'pop_heavy')");
}

enum QueueWorkloadType : int {
	MP_MC,
	PC_SPLIT,
};
QueueWorkloadType parse_queue_workload(const std::string& workload_str) {
    if (workload_str == "mp_mc"){
        return QueueWorkloadType::MP_MC;
    } else if (workload_str == "pc_split") {
        return QueueWorkloadType::PC_SPLIT;
    }
    throw std::invalid_argument("Invalid --workload argument: " + workload_str + 
                                " (expected 'mp_mc', or 'pc_split')");
}


struct Options {
	int num_threads;
	int num_ops;
	bool debug;
	Options(int num_threads, int num_ops, bool debug): num_threads(num_threads), num_ops(num_ops), debug(debug) {}
};


void run_stack_workload(Stack& s, StackWorkloadType workload, Options opts);
void run_queue_workload(Queue& q, QueueWorkloadType workload, Options opts);

int main(int argc, char* argv[]) {
    std::string type_flag = "";
    std::string impl_flag = "";
    std::string workload_flag = "balanced"; // default stack workload
	int threads = 8;
	int operations = 1000000;
	bool debug = false;

    // Parse runtime flags (e.g. --type queue --impl ms --workload balanced)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--type" || arg == "--struct") && i + 1 < argc) {
            type_flag = argv[++i];
        } else if (arg == "--impl" && i + 1 < argc) {
            impl_flag = argv[++i];
        } else if (arg == "--workload" && i + 1 < argc) {
            workload_flag = argv[++i];
        } else if (arg == "--debug") {
            debug = true;
        } else if (arg == "--threads" && i + 1 < argc) {
			try {
				threads = std::stoi(argv[++i]); // Throws if invalid or out of range
			} catch (const std::invalid_argument& e) {
				std::cerr << "Invalid input: " << e.what() << '\n';
			} catch (const std::out_of_range& e) {
				std::cerr << "Value out of range: " << e.what() << '\n';
			}
        } else if (arg == "--ops" && i + 1 < argc) {
			try {
				operations = std::stoi(argv[++i]); // Throws if invalid or out of range
			} catch (const std::invalid_argument& e) {
				std::cerr << "Invalid input: " << e.what() << '\n';
			} catch (const std::out_of_range& e) {
				std::cerr << "Value out of range: " << e.what() << '\n';
			}
		}
    }

	Options* opts = new Options(threads, operations, debug);

    // Auto-detect type if not explicitly set
    if (type_flag.empty()) {
        if (impl_flag == "ms" || impl_flag == "advanced") {
            type_flag = "queue";
        } else {
            type_flag = "stack";
        }
    }

    // Apply default implementations per data structure
    if (impl_flag.empty()) {
        impl_flag = (type_flag == "queue") ? "ms" : "treiber";
    }

	// Apply default workload type for queue 
	if (type_flag == "queue") {
		if (workload_flag == "balanced") {
			workload_flag = "mp_mc";
		}
	}

    try {
        if (type_flag == "queue") {
            QueueType qtype = parse_queue_type(impl_flag);
            std::unique_ptr<Queue> queue = create_queue(qtype);
			QueueWorkloadType workload = parse_queue_workload(workload_flag);

            std::cout << "Successfully instantiated queue implementation for: " << impl_flag << " on workload: " << workload_flag << "\n";

			run_queue_workload(*queue, workload, *opts);


        } else if (type_flag == "stack") {
            StackType stype = parse_stack_type(impl_flag);
            std::unique_ptr<Stack> stack = create_stack(stype);
            StackWorkloadType workload = parse_stack_workload(workload_flag);

            std::cout << "Successfully instantiated stack implementation for: " << impl_flag << " (workload: " << workload_flag << ")\n";


            run_stack_workload(*stack, workload, *opts);


        } else {
            std::cerr << "Error: Unknown data structure type: " << type_flag << " (use 'stack' or 'queue')\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

void run_stack_workload(Stack& s, StackWorkloadType workload, Options opts) {
	
	
	int push_perc = static_cast<int>(workload);
	omp_set_num_threads(opts.num_threads);
	double before = omp_get_wtime();
	
	#pragma omp parallel 
	{
		unsigned int private_seed = BASE_SEED + omp_get_thread_num();
		#pragma omp for
		for (int i = 0; i < opts.num_ops; i++) {
			int val = 0;
			int op = rand_r(&private_seed) % 100;
			if (op < push_perc) {
				s.push(i);
				
			}
			else {
				int res = s.pop(&val);
				if (opts.debug) {
					if (res) {
						std::cout << "Popped value: " << val << "\n";
					} else {
						std::cout << "Stack is empty\n";
					}
				}
			}
		}
	}
	
	double after = omp_get_wtime();
	
    std::cout << std::right << "Time Elapsed (ms): " << (after - before) * 1000.0 << "\n";
}

void run_queue_workload(Queue& q, QueueWorkloadType workload, Options opts) {
	
	omp_set_num_threads(opts.num_threads);
	double before = omp_get_wtime();

	if (workload == MP_MC) {
		int push_perc = 50;
		
		#pragma omp parallel 
		{
			unsigned int private_seed = BASE_SEED + omp_get_thread_num();
			#pragma omp for
			for (int i = 0; i < opts.num_ops; i++) {
				int val = 0;
				int op = rand_r(&private_seed) % 100;
				if (op < push_perc) {
					q.enqueue(i);
				}
				else {
					int res = q.dequeue(&val);
					
					if (opts.debug) {
						if (res) {
							std::cout << "Dequeued value: " << val << "\n";
						} else {
							std::cout << "Queue is empty (expected for skeleton stubs).\n";
						}

					}
				}
			}
		}

	}
	else if (workload == PC_SPLIT) {
		
		#pragma omp parallel 
		{
			int threadId = omp_get_thread_num();
			#pragma omp for
			for (int i = 0; i < opts.num_ops; i++) {
				int val = 0;
				if (threadId % 2 == 0) {
					q.enqueue(i);
				}
				else {
					int res = q.dequeue(&val);
					
					if (opts.debug) {
						if (res) {
							std::cout << "Dequeued value: " << val << "\n";
						} else {
							std::cout << "Queue is empty (expected for skeleton stubs).\n";
						}
					}
				}
			}
		}

	}

	
	double after = omp_get_wtime();
	
    std::cout << std::right << "Time Elapsed (ms): " << (after - before) * 1000.0 << "\n";
}
