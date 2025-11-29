/**
 * @file rtos.hpp
 * @brief PROS RTOS API for Host Mode
 * 
 * This header provides Task, Mutex, and other RTOS primitives
 * that match the PROS API.
 */

#ifndef PROS_RTOS_HPP
#define PROS_RTOS_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace pros {

/**
 * Task state enumeration
 */
typedef enum task_state_e {
    E_TASK_STATE_RUNNING = 0,
    E_TASK_STATE_READY,
    E_TASK_STATE_BLOCKED,
    E_TASK_STATE_SUSPENDED,
    E_TASK_STATE_DELETED,
    E_TASK_STATE_INVALID
} task_state_e_t;

/**
 * Task notification action enumeration
 */
typedef enum task_notify_action_e {
    E_NOTIFY_ACTION_NONE = 0,
    E_NOTIFY_ACTION_BITS,
    E_NOTIFY_ACTION_INCR,
    E_NOTIFY_ACTION_OWRITE,
    E_NOTIFY_ACTION_NO_OWRITE
} notify_action_e_t;

// Forward declaration
class Task;

/**
 * Type alias for task function
 */
using task_fn_t = void (*)(void*);

/**
 * Task priority definitions
 */
#define TASK_PRIORITY_MAX 16
#define TASK_PRIORITY_MIN 1
#define TASK_PRIORITY_DEFAULT 8

/**
 * Task stack size definitions
 */
#define TASK_STACK_DEPTH_DEFAULT 0x2000
#define TASK_STACK_DEPTH_MIN 0x200

/**
 * Task class for managing FreeRTOS-style tasks
 */
class Task {
public:
    /**
     * Creates a new task with the given function.
     *
     * @param function Pointer to the task function
     * @param parameters A pointer to pass to the task function
     * @param priority The task's priority (1-16)
     * @param stack_depth The task's stack depth
     * @param name A name for the task
     */
    Task(task_fn_t function, void* parameters = nullptr,
         uint32_t priority = TASK_PRIORITY_DEFAULT,
         uint16_t stack_depth = TASK_STACK_DEPTH_DEFAULT,
         const char* name = "");

    /**
     * Creates a new task with a std::function.
     *
     * @param function The task function
     * @param priority The task's priority
     * @param stack_depth The task's stack depth
     * @param name A name for the task
     */
    Task(std::function<void()> function,
         uint32_t priority = TASK_PRIORITY_DEFAULT,
         uint16_t stack_depth = TASK_STACK_DEPTH_DEFAULT,
         const char* name = "");

    /**
     * Destructor - removes the task.
     */
    ~Task();

    /**
     * Gets the name of the task.
     *
     * @return The task's name
     */
    const char* get_name();

    /**
     * Gets the priority of the task.
     *
     * @return The task's priority
     */
    uint32_t get_priority();

    /**
     * Gets the state of the task.
     *
     * @return The task's state
     */
    task_state_e_t get_state();

    /**
     * Sends a notification to the task.
     *
     * @return Always returns true
     */
    uint32_t notify();

    /**
     * Sends a notification with a value to the task.
     *
     * @param value The notification value
     * @param action The notification action
     * @return The previous notification value
     */
    uint32_t notify_ext(uint32_t value, notify_action_e_t action);

    /**
     * Clears a notification for the task.
     *
     * @return True if notification was pending
     */
    bool notify_clear();

    /**
     * Sets the task's priority.
     *
     * @param priority The new priority
     */
    void set_priority(uint32_t priority);

    /**
     * Suspends the task.
     */
    void suspend();

    /**
     * Resumes the task.
     */
    void resume();

    /**
     * Removes/deletes the task.
     */
    void remove();

    /**
     * Joins the task (waits for completion).
     */
    void join();

    /**
     * Gets the current task.
     *
     * @return Pointer to the current task (may be null in main thread)
     */
    static Task* current();

    /**
     * Delay the current task for a given number of milliseconds.
     *
     * @param milliseconds The number of milliseconds to wait
     */
    static void delay(uint32_t milliseconds);

    /**
     * Delay the current task until a given time.
     *
     * @param prev_time Pointer to the time the task last woke up
     * @param delta The time to wait
     */
    static void delay_until(uint32_t* prev_time, uint32_t delta);

    /**
     * Gets the count of currently running tasks.
     *
     * @return The number of tasks
     */
    static uint32_t get_count();

private:
    std::thread _thread;
    std::string _name;
    uint32_t _priority;
    task_state_e_t _state;
    bool _running;
    uint32_t _notification_value;
    bool _notification_pending;
    std::mutex _mutex;
    std::condition_variable _cv;
};

/**
 * Mutex class for thread synchronization
 */
class Mutex {
public:
    /**
     * Creates a new mutex.
     */
    Mutex();

    /**
     * Destructor.
     */
    ~Mutex();

    /**
     * Takes the mutex, blocking until available.
     *
     * @param timeout Maximum time to wait in milliseconds (0 = forever)
     * @return True if mutex was taken
     */
    bool take(uint32_t timeout = 0);

    /**
     * Gives/releases the mutex.
     *
     * @return True if mutex was given
     */
    bool give();

    /**
     * Locks the mutex (alias for take).
     */
    void lock();

    /**
     * Unlocks the mutex (alias for give).
     */
    void unlock();

private:
    std::timed_mutex _mutex;
};

/**
 * Clock class for time measurement
 */
class Clock {
public:
    /**
     * Gets the current time since program start in milliseconds.
     *
     * @return Time in milliseconds
     */
    static uint32_t now();

    /**
     * Gets the current time since program start in microseconds.
     *
     * @return Time in microseconds
     */
    static uint64_t now_us();
};

} // namespace pros

// C-style delay function
extern "C" {
    void task_delay(uint32_t milliseconds);
    void task_delay_until(uint32_t* prev_time, uint32_t delta);
}

#endif // PROS_RTOS_HPP
