#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <random>
#include <sstream>

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

// determinista con variables compartidas
void speculative_worker(std::vector<speculative_data>& shared_vars, std::atomic<int>& successes, int thread_id, unsigned seed) {
    std::mt19937 rng(seed);  // Generador determinista por hilo
    std::uniform_int_distribution<int> var_dist(0, shared_vars.size() - 1);// Para elegir la variable
    std::uniform_int_distribution<int> delay_dist(10, 29);// Para simular retardo

    for (int i = 0; i < 5; ++i) {
        thread_context ctx;
        int var_index = var_dist(rng);//Selección determinista de la variable que se va acompartir
        speculative_data& data = shared_vars[var_index];

        while (!ctx.committed && ctx.retries_left >= 0) {
            switch(ctx.current_state) {
                case state::READ:
                    ctx.start_version = data.version.load(std::memory_order_acquire);
                    ctx.local_value = data.value;
                    ctx.current_state = state::EXECUTE;
                break;

                case state::EXECUTE:
                    ctx.local_value += 1;
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(rng)));
                    ctx.current_state = state::VALIDATE;
                break;

                case state::VALIDATE:
                    if (data.version.compare_exchange_weak(ctx.start_version, ctx.start_version + 1, std::memory_order_release, std::memory_order_relaxed)){
                        ctx.current_state = state::COMMIT;
                    } else {
                        ctx.current_state = state::RETRY;
                    }
                break;

                case state::COMMIT:
                    data.value = ctx.local_value;
                    successes.fetch_add(1, std::memory_order_relaxed);
                    ctx.committed = true;
                break;

                case state::RETRY:
                    ctx.retries_left--;
                    if (ctx.retries_left >= 0) {
                        ctx.current_state = state::READ;
                    } else {
                        std::cout << "Thread ID: " << thread_id << " retry limit exceeded.\n";
                    }
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    int thread_count = 4;
    int shared_var_count = 2;

    //Lee el número de threads si se pasa como argumento
    if (argc >= 2) {
        std::istringstream ss(argv[1]);
        ss >> thread_count;
    }

    // Lee el número de variables que se quieren compartir esto desde la ejecucion
    if (argc >= 3) {
        std::istringstream ss(argv[2]);
        ss >> shared_var_count;
    }

    std::vector<speculative_data> shared_data(shared_var_count);
    std::atomic<int> total_successes{0};

    std::vector<std::thread> workers;
    for (int i = 0; i < thread_count; ++i) {
        workers.emplace_back(speculative_worker, std::ref(shared_data), std::ref(total_successes), i, /*seed=*/ 42 + i);
    }

    for (auto& t : workers) {
        t.join();
    }

    for (int i = 0; i < shared_var_count; ++i) {
        std::cout << "Final value of shared_data[" << i << "]: " << shared_data[i].value << "\n";
    }

    std::cout << "Successful Commits: " << total_successes << "\n";
    std::cout << "Theoretical maximum: " << thread_count * 5 << "\n";
    std::cout<<argv[0]<<std::endl;
    return 0;
}
