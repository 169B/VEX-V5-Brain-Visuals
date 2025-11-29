/**
 * @file rtos.cpp
 * @brief PROS RTOS Implementation for Host Mode
 */

#include "pros/rtos.hpp"
#include <chrono>
#include <atomic>

// Static start time for timing functions
static auto program_start = std::chrono::steady_clock::now();

// Task counter
static std::atomic<uint32_t> task_count{1}; // Main task counts as 1

// Thread-local task pointer
static thread_local pros::Task* current_task = nullptr;

namespace pros {

// Task implementation
Task::Task(task_fn_t function, void* parameters,
           uint32_t priority, uint16_t /*stack_depth*/, const char* name)
    : _name(name ? name : ""), _priority(priority),
      _state(E_TASK_STATE_READY), _running(true),
      _notification_value(0), _notification_pending(false) {
    
    task_count++;
    
    _thread = std::thread([this, function, parameters]() {
        current_task = this;
        _state = E_TASK_STATE_RUNNING;
        
        try {
            function(parameters);
        } catch (...) {
            // Task threw an exception
        }
        
        _state = E_TASK_STATE_DELETED;
        _running = false;
        task_count--;
    });
}

Task::Task(std::function<void()> function,
           uint32_t priority, uint16_t /*stack_depth*/, const char* name)
    : _name(name ? name : ""), _priority(priority),
      _state(E_TASK_STATE_READY), _running(true),
      _notification_value(0), _notification_pending(false) {
    
    task_count++;
    
    _thread = std::thread([this, function]() {
        current_task = this;
        _state = E_TASK_STATE_RUNNING;
        
        try {
            function();
        } catch (...) {
            // Task threw an exception
        }
        
        _state = E_TASK_STATE_DELETED;
        _running = false;
        task_count--;
    });
}

Task::~Task() {
    if (_thread.joinable()) {
        _running = false;
        _cv.notify_all();
        _thread.join();
    }
}

const char* Task::get_name() {
    return _name.c_str();
}

uint32_t Task::get_priority() {
    return _priority;
}

task_state_e_t Task::get_state() {
    return _state;
}

uint32_t Task::notify() {
    std::lock_guard<std::mutex> lock(_mutex);
    _notification_pending = true;
    _notification_value++;
    _cv.notify_one();
    return 1;
}

uint32_t Task::notify_ext(uint32_t value, notify_action_e_t action) {
    std::lock_guard<std::mutex> lock(_mutex);
    uint32_t prev = _notification_value;
    
    switch (action) {
        case E_NOTIFY_ACTION_NONE:
            break;
        case E_NOTIFY_ACTION_BITS:
            _notification_value |= value;
            break;
        case E_NOTIFY_ACTION_INCR:
            _notification_value += value;
            break;
        case E_NOTIFY_ACTION_OWRITE:
            _notification_value = value;
            break;
        case E_NOTIFY_ACTION_NO_OWRITE:
            if (!_notification_pending) {
                _notification_value = value;
            }
            break;
    }
    
    _notification_pending = true;
    _cv.notify_one();
    return prev;
}

bool Task::notify_clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    bool was_pending = _notification_pending;
    _notification_pending = false;
    _notification_value = 0;
    return was_pending;
}

void Task::set_priority(uint32_t priority) {
    _priority = priority;
    // Note: std::thread doesn't support priority changes
}

void Task::suspend() {
    std::lock_guard<std::mutex> lock(_mutex);
    _state = E_TASK_STATE_SUSPENDED;
}

void Task::resume() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_state == E_TASK_STATE_SUSPENDED) {
        _state = E_TASK_STATE_READY;
        _cv.notify_one();
    }
}

void Task::remove() {
    _running = false;
    _state = E_TASK_STATE_DELETED;
    _cv.notify_all();
}

void Task::join() {
    if (_thread.joinable()) {
        _thread.join();
    }
}

Task* Task::current() {
    return current_task;
}

void Task::delay(uint32_t milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Task::delay_until(uint32_t* prev_time, uint32_t delta) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - program_start).count();
    
    uint32_t target = *prev_time + delta;
    if (target > elapsed) {
        std::this_thread::sleep_for(std::chrono::milliseconds(target - elapsed));
    }
    
    *prev_time = target;
}

uint32_t Task::get_count() {
    return task_count;
}

// Mutex implementation
Mutex::Mutex() {}

Mutex::~Mutex() {}

bool Mutex::take(uint32_t timeout) {
    if (timeout == 0) {
        _mutex.lock();
        return true;
    } else {
        return _mutex.try_lock_for(std::chrono::milliseconds(timeout));
    }
}

bool Mutex::give() {
    _mutex.unlock();
    return true;
}

void Mutex::lock() {
    _mutex.lock();
}

void Mutex::unlock() {
    _mutex.unlock();
}

// Clock implementation
uint32_t Clock::now() {
    auto now_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now_time - program_start);
    return static_cast<uint32_t>(duration.count());
}

uint64_t Clock::now_us() {
    auto now_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        now_time - program_start);
    return static_cast<uint64_t>(duration.count());
}

} // namespace pros

// C-style functions
extern "C" {

void task_delay(uint32_t milliseconds) {
    pros::Task::delay(milliseconds);
}

void task_delay_until(uint32_t* prev_time, uint32_t delta) {
    pros::Task::delay_until(prev_time, delta);
}

} // extern "C"
