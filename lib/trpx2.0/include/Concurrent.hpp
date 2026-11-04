#ifndef Concurrent_h
#define Concurrent_h

#include <algorithm>
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <vector>
#include <atomic>
#include <list>
#include <optional>
#include <tuple>
#include <unordered_map>

namespace jpa {
/**
 * @brief Class to control and represent the degree of parallelism.
 *
 * The `Deg_of_parallelism` class allows you to control the degree of parallelism
 * for multi-threaded applications. It dynamically determines how many
 * cores should be available for execution and ensures that parallelism is within
 * a defined range.
 *
 * This class is used both in concurrent programming (managing different tasks simultaneously)
 * and in parallel processing (dividing a task into subtasks processed concurrently).
 */
class Deg_of_parallelism {
public:
    /**
     * @brief Default constructor initializes the degree of parallelism to 0.0 (sequential execution).
     */
    constexpr Deg_of_parallelism() : d_dop(0) {}

    /**
     * @brief Constructor that initializes the degree of parallelism to a given value.
     *
     * @param val A value between 0 and 1, where 0 implies sequential execution and
     * 1 implies full use of available cores. Values outside this range will be clamped.
     */
    constexpr Deg_of_parallelism(double const val) : d_dop(std::clamp(val, 0.0, 1.0)) {}
    
    /**
     * @brief Cast operator to double, allowing implicit conversion to a degree of parallelism value.
     *
     * @return The degree of parallelism as a double between 0 and 1.
     */
    constexpr operator double() const { return d_dop; }

    /**
     * @brief Set the degree of parallelism to a new value.
     *
     * @param val A value between 0 and 1, where 0 implies sequential execution and
     * 1 implies full use of available cores. Values outside this range will be clamped.
     *
     * @return A reference to the updated `Deg_of_parallelism` object.
     */
    constexpr Deg_of_parallelism& operator=(double const val) {
        d_dop = std::clamp(val, 0.0, 1.0);
        return *this;
    }

    /**
     * @brief Set the degree of parallelism based on a given number of cores.
     *
     * @param val The desired number of cores. It will be clamped to the range of 1 to the total number of available cores.
     *
     * @return A reference to the updated `Deg_of_parallelism` object.
     */
    Deg_of_parallelism& cores(unsigned const val) {
        d_dop = std::clamp(val, 1u, d_cores) / double(d_cores);
        return *this;
    }

    /**
     * @brief Get the number of cores currently assigned based on the degree of parallelism.
     *
     * @return The number of cores available for concurrent processing, based on the current degree of parallelism.
     */
    unsigned cores() const {
        return std::clamp(unsigned(d_dop * d_cores), 1u, d_cores);
    }

private:
    double d_dop; ///< Degree of parallelism, a value between 0 (sequential) and 1 (full parallelism).
    static inline unsigned const d_cores = std::max(1u, std::thread::hardware_concurrency()); ///< Total number of available cores on the system.
};

/**
 * @brief Default degree of parallelism is sequential execution.
 */
static Deg_of_parallelism degree_of_parallelism_default = Deg_of_parallelism();

/**
 * @brief A concurrent task manager for dispatching and executing different tasks using a thread_local thread pool.
 *
 * The `Concurrent` class is designed for managing the concurrent execution of multiple, potentially different tasks.
 * It allows you to run tasks asynchronously, facilitating concurrent programming where tasks can run simultaneously.
 * This is distinct from parallel processing of a single task divided into subtasks (handled by separate algorithms).
 *
 * The class manages a pool of threads shared by all instances of `Concurrent` to optimize resource use.
 * You can run background tasks using `std::future` for standard use cases.
 * It also accepts **functors** for advanced control, such as monitoring, pausing, or stopping tasks while they run.
 *
 * ### Example Usage:
 *
 * **Standard usage:**
 * @code
 * jpa::Concurrent tasks;
 * auto result1 = tasks.background(my_function1, arg1, arg2);
 * auto result2 = tasks.background(my_function2, arg3, arg4);
 * // Do other work here...
 * int final_result = result1.get() + result2.get();  // Blocks execution until the tasks are finished
 * @endcode
 * This approach allows for **fire-and-forget** or **fire-and-expect** behavior, where you dispatch tasks and retrieve
 * their results later.
 *
 * ### Functor-based Task Control:
 *
 * You can pass a **functor** to the `background()` function, where the functor has state variables
 * like `progress`, `kill`, etc. This allows you to monitor or control the task as it runs.
 *
 * **Example:**
 * @code
 * // Define a functor that tracks progress and can be killed
 * struct TaskControl {
 *     int progress = 0;
 *     bool kill = false;
 *
 *     // Define the task (operator()) to be run in the background
 *     void operator()(int duration) {
 *         for (int i = 0; i <= 100; ++i) {
 *             if (kill) {
 *                 std::cout << "Task killed at " << progress << "%!" << std::endl;
 *                 return;
 *             }
 *             progress = i;
 *             std::this_thread::sleep_for(std::chrono::milliseconds(duration));
 *         }
 *         std::cout << "Task completed!" << std::endl;
 *     }
 * };
 *
 * // Instantiate the functor and start the task in the background
 * jpa::Concurrent tasks;
 * TaskControl task;
 * auto result = tasks.background(task, 50);
 *
 * // Monitor the task's progress and kill it when necessary
 * while (task.progress < 100) {
 *     std::cout << "Progress: " << task.progress << "%" << std::endl;
 *     std::this_thread::sleep_for(std::chrono::milliseconds(500));
 *
 *     if (task.progress > 50) {
 *         task.kill = true;  // Stop the task
 *     }
 * }
 *
 * result.get();  // Blocks until the task completes or is killed
 * @endcode
 *
 * ### Note on Concurrency vs. Parallelism:
 *
 * The `Concurrent` class focuses on concurrent programming—running different tasks simultaneously.
 * For parallel processing of a single task (like processing elements of a container in parallel),
 * refer to the algorithms provided in `Parallel.hpp`.
 *
 * ### Note on Nested Concurrent Calls:
 *
 * The Concurrent class is designed to handle nested calls to ensure that concurrency occurs at the highest
 * abstraction level while avoiding deadlocks. If multiple instances of Concurrent are created (for example,
 * within different functions), only the first instance created will manage all its tasks asynchronously via the thread
 * pool. All subsequent Concurrent instances that are created during the lifetime of the first instance will
 * only run asynchronously if there are free threads in the thread pool. If there are no free threads, they will
 * execute their tasks sequentially in the current thread. This ensures that tasks within tasks are executed, even
 * if there are top-level tasks in the queue. Otherwise a deadlock could result, when top level tasks are queued
 * with a higher priority than sub-tasks, yet requiring the sub-tasks to finish first.
 *
 * This design allows for efficient task management, where lower-level tasks run sequentially when dispatched
 * by higher-level concurrent operations, thus preventing potential deadlocks caused by excessive nesting and
 * ensuring that concurrency is handled where it’s most needed.
 */
class Concurrent {
public:
    /**
     * @brief Constructs the task manager, specifying the degree of parallelism (dop).
     *
     * @param dop The degree of parallelism, representing the proportion of available cores to use.
     * It is a number between 0 and 1, where 0 implies sequential execution and
     * 1 implies full use of available cores.
     */
    Concurrent(Deg_of_parallelism dop) : d_dop(dop) {
        if (d_dop > 0)
            d_unique_id = d_thread_pool().f_get_unique_id();
    }

    /**
     * @brief Backgrounds a task for asynchronous execution.
     *
     * This function creates a packaged task from the function and arguments, pushes it into the task queue,
     * and manages the number of threads allocated for this task manager.
     *
     * @tparam Func The function type.
     * @tparam Args The argument types for the function.
     * @param func The function to be executed in the background.
     * @param args The arguments to be passed to the function.
     * @return A future object representing the result of the task.
     */
    template <typename Func, typename... Args>
    auto background(Func&& func, Args&&... args) -> std::future<typename std::invoke_result_t<Func, Args...>> {
        using return_type = typename std::invoke_result_t<Func, Args...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>
        ([captured_func = std::forward<Func>(func), ... captured_args = std::forward<Args>(args)]() mutable {
            if constexpr (std::is_void_v<return_type>)
                captured_func(std::forward<Args>(captured_args)...);
            else
                return captured_func(std::forward<Args>(captured_args)...);
        });
        std::future<return_type> result = task->get_future();
        if (d_dop == 0.0 || !d_thread_pool().f_add_task(*this, [task]() { (*task)(); }))
            (*task)();  // Execute the task immediately
        return result;
    }
    
    /**
     * @brief Waits until all backgrounded tasks have been completed.
     *
     * This method blocks the caller until the task queue is empty and no active threads are running.
     */
    void finish() {
        while (running_tasks() > 0)
            std::this_thread::yield();  // Yield control to other threads
    }
    
    /**
     * @brief Returns the number of tasks submitted by this instance of `Concurrent` that are either in the queue or being processed.
     *
     * @return The number of running tasks.
     */
    unsigned running_tasks() const {
        unsigned task_count = 0;
        {
            std::lock_guard<std::mutex> lock(d_thread_pool().d_mutex);
            for (const auto& task : d_thread_pool().d_task_queue)
                if (std::get<2>(task) == d_unique_id)
                    ++task_count;
            if (d_thread_pool().d_running_tasks.count(d_unique_id))
                task_count += d_thread_pool().d_running_tasks.at(d_unique_id);
        }
        return task_count;
    }
    
    /**
     * @brief Returns the degree of partiality of this instance of `Concurrent`.
     *
     * @return The Deg_of_parallelism: 0 for sequential processing, 1 for full paralllel processing.
     */
    constexpr Deg_of_parallelism dop() const noexcept { return d_dop; }

    
private:
    Deg_of_parallelism d_dop;
    std::size_t d_unique_id = 0;

    class c_Global_thread_pool {
        friend class Concurrent;

        unsigned const d_max_threads = std::max(1u, std::thread::hardware_concurrency() - 1);
        std::atomic<std::size_t> d_unique_id_generator = 0;
        std::mutex d_mutex;
        std::list<std::tuple<std::function<void()>, Deg_of_parallelism, std::size_t>> d_task_queue;
        std::unordered_map<std::size_t, unsigned> d_running_tasks;
        std::vector<std::thread> d_workers;
        std::condition_variable d_condition;
        std::atomic<bool> d_stop {false};
 
        c_Global_thread_pool() {
            d_workers.reserve(d_max_threads);
            for (unsigned i = 0; i < d_max_threads; ++i)
                d_workers.emplace_back(&c_Global_thread_pool::f_worker_thread, this);
        }

        ~c_Global_thread_pool() {
            d_stop = true;
            d_condition.notify_all();
            for (auto& worker : d_workers)
                if (worker.joinable()) {
                    try { worker.join(); }
                    catch (const std::system_error& e) {  std::cerr << "Error joining thread: " << e.what() << std::endl; }
                    catch (...) { std::cerr << "Unknown error occurred while joining thread." << std::endl; }
                }
        }
        
        bool f_add_task(Concurrent const& concurrent_instance, std::function<void()> const& task) {
            if (d_max_threads == 0)
                return false;
            auto top_level_id = f_get_top_level_id();
            if (top_level_id.has_value() &&
                concurrent_instance.d_unique_id != *top_level_id &&
                d_running_tasks.size() == d_max_threads)
                return false;
            {
                std::lock_guard<std::mutex> lock(d_mutex);
                d_task_queue.emplace_back(std::move(task), concurrent_instance.d_dop, concurrent_instance.d_unique_id);
            }
            d_condition.notify_one();
            return true;
        }

        std::size_t f_get_unique_id() { return d_unique_id_generator++; }

        std::optional<std::size_t> f_get_top_level_id() {
            std::lock_guard<std::mutex> lock(d_mutex);
            std::optional<std::size_t> top_level_id;
            for (const auto& [id, count] : d_running_tasks)
                if (!top_level_id || id < *top_level_id)
                    top_level_id = id;
            for (const auto& [task, dop, id] : d_task_queue)
                if (!top_level_id || id < *top_level_id)
                    top_level_id = id;
            return top_level_id;
        }
        
        void f_worker_thread() {
            while (true) {
                std::function<void()> task;
                std::size_t concurrent_id;
                {
                    std::unique_lock<std::mutex> lock(d_mutex);
                    d_condition.wait(lock, [this]() { return d_stop || !d_task_queue.empty(); });
                    if (d_task_queue.empty() && d_stop)
                        return;
                    auto it = d_task_queue.begin();
                    while (it != d_task_queue.end()) {
                        Deg_of_parallelism const& dop = get<1>(*it);
                        concurrent_id = get<2>(*it);
                        if (d_running_tasks[concurrent_id] < dop.cores()) {
                            task = std::move(get<0>(*it));
                            d_task_queue.erase(it);
                            ++d_running_tasks[concurrent_id];
                            break;
                        }
                        ++it;
                    }
                    if (!task)
                        continue;
                }
                try { task(); }
                catch (const std::exception& e) { std::cerr << "Exception in worker thread: " << e.what() << std::endl; }
                catch (...) { std::cerr << "Unknown exception in worker thread." << std::endl; }
                {
                    std::lock_guard<std::mutex> lock(d_mutex);
                    if (d_running_tasks[concurrent_id] == 1)
                        d_running_tasks.erase(concurrent_id);
                    else
                        --d_running_tasks[concurrent_id];
                    d_condition.notify_all();
                }
            }
        }
    };

    // The global thread pool is created on first use instead of at program
    // start-up, so that merely including this header does not spawn threads.
    static c_Global_thread_pool& d_thread_pool() {
        static c_Global_thread_pool pool;
        return pool;
    }
};

/**
 * @brief Backgrounds a task for asynchronous execution, returning a future representing the result.
 *
 * This function provides a simple and intuitive interface for executing tasks in the background.
 * It works similarly to `std::async()`, allowing you to run tasks asynchronously in a **global thread pool**,
 * ensuring efficient resource management without excessive thread creation.
 *
 * **Example:**
 * @code
 * auto result = jpa::background(my_function, arg1, arg2);
 * // Do other stuff
 * int final_result = result.get();  // Blocks until my_function completes
 * @endcode
 *
 * ### When to Use `background()` vs `std::async`:
 *
 * - **Use `background()`** for tasks that are **CPU-bound** and need efficient concurrent execution, especially when
 *   you need to execute multiple different tasks concurrently using a centralized thread pool. It prevents the creation of
 *   excessive threads and manages resources effectively.
 *
 * - **Use `std::async`** for tasks that are expected to spend most of their time waiting, such as:
 *   - Network requests (waiting for responses).
 *   - File I/O (waiting for disk read/write operations).
 *   - Database queries (waiting for results).
 *
 *   Also, use `std::async` if you need explicit control over task execution policies, such as choosing between launching
 *   a new thread or using deferred execution. `std::async` is appropriate for lightweight or occasional tasks where
 *   thread management is not a primary concern.
 *
 * ### Note on Concurrency vs. Parallelism:
 *
 * The `background()` function and the `Concurrent` class are designed for concurrent programming—running multiple
 * different tasks simultaneously. For parallel processing of a single task (e.g., operating on elements of a container concurrently),
 * refer to the parallel algorithms provided in a separate header (e.g., `Parallel.hpp`).
 *
 * @tparam Func The function type.
 * @tparam Args The argument types for the function.
 * @param func The function to be executed in the background.
 * @param args The arguments to be passed to the function.
 * @return A future object representing the result of the background task.
 */
template <typename Func, typename... Args>
auto background(Func&& func, Args&&... args) -> std::future<typename std::invoke_result_t<Func, Args...>> {
    Concurrent task(1);
    return task.background(std::forward<Func>(func), std::forward<Args>(args)...);
}
}  // namespace jpa

#endif /* Concurrent_h */
