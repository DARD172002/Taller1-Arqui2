// Compile
// $> g++ --std=c++20  -pthread -Wall -Wextra tls.cpp  -o tlp
// Run
// $> ./tls

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>

enum class state {READ, EXECUTE, VALIDATE, COMMIT, RETRY};

struct speculative_data {
    int value {0};
    std::atomic<int> version {0};
};

struct thread_context {
    state current_state {state::READ};
    int start_version {0};
    int local_value {0};
    int retries_left {3};
    bool committed {false};
};

void speculative_worker(speculative_data& data, std::atomic<int>& successes, int thread_id) {
    for (int i = 0; i < 5; ++i) {
        thread_context ctx;
        
        while (!ctx.committed && ctx.retries_left >= 0) {
            switch(ctx.current_state) {
                case state::READ:
                    // Capture initial state
                    ctx.start_version = data.version.load(std::memory_order_acquire);
                    ctx.local_value = data.value;
                    ctx.current_state = state::EXECUTE;
                break;

                case state::EXECUTE:
                    // Speculative computation
                    ctx.local_value += 1;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10 + rand() % 20));
                    ctx.current_state = state::VALIDATE;
                break;

                case state::VALIDATE:
                    // Atomic state validation
                    if (data.version.compare_exchange_weak(ctx.start_version, ctx.start_version + 1, std::memory_order_release, std::memory_order_relaxed)){
                        ctx.current_state = state::COMMIT;
                    }
                    else {
                        ctx.current_state = state::RETRY;
                    }
                break;

                case state::COMMIT:
                    // Finalise successful transaction
                    data.value = ctx.local_value;
                    successes.fetch_add(1, std::memory_order_relaxed);
                    ctx.committed = true;
                break;

                case state::RETRY:
                    // Handle failed transaction
                    ctx.retries_left--; 
                    if (ctx.retries_left >= 0) {
                        ctx.current_state = state::READ;
                    } else {
                        // Logging when retries are exceeded
                        std::cout << "Thread ID: " << thread_id << " retry limit exceeded.\n";
                    }
                break;
            }
        }
    }
}

int main() {
    speculative_data shared_data;
    std::atomic<int> total_successes{0};
    const int thread_count = 4;

    std::vector<std::thread> workers;

    for (int i = 0; i < thread_count; ++i) {
        workers.emplace_back(speculative_worker, std::ref(shared_data), std::ref(total_successes), i);
    }

    for (auto& t : workers) {
        t.join();
    }

    std::cout << "Final value: " << shared_data.value << "\n";
    std::cout << "Successful Commits: " << total_successes << "\n";
    std::cout << "Theoretical maximum: " << thread_count * 5 << "\n";

    return 0;
}
