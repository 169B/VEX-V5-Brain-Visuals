/**
 * @file ipc.hpp
 * @brief WebSocket IPC Client for Host Mode
 * 
 * This header provides the IPC client that communicates with the
 * Node.js UI server via WebSocket.
 */

#ifndef HOST_IPC_HPP
#define HOST_IPC_HPP

#include <cstdint>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <vector>

namespace host {

/**
 * IPC message types
 */
enum class IPCMessageType {
    // Host -> UI
    SCREEN,          // Screen update
    MOTOR,           // Motor telemetry
    LOG,             // Log message
    AUTONS,          // Autonomous list
    LCD,             // LCD text update
    MODE,            // Current mode
    
    // UI -> Host
    TOUCH,           // Touch input
    CONTROLLER,      // Controller input
    SET_MODE,        // Set robot mode
    SELECT_AUTO      // Select autonomous
};

/**
 * Screen update data
 */
struct ScreenUpdate {
    int32_t x1, y1, x2, y2;
    std::vector<uint16_t> pixels;  // RGB565 data
};

/**
 * Touch input data
 */
struct TouchInput {
    int16_t x;
    int16_t y;
    bool pressed;
};

/**
 * Controller input data
 */
struct ControllerInput {
    int32_t lx, ly, rx, ry;
    uint32_t buttons;  // Bitmask of pressed buttons
};

/**
 * IPC Client for WebSocket communication
 */
class IPCClient {
public:
    /**
     * Gets the singleton instance.
     *
     * @return Reference to the IPC client instance
     */
    static IPCClient& instance();

    // Delete copy/move constructors
    IPCClient(const IPCClient&) = delete;
    IPCClient& operator=(const IPCClient&) = delete;
    IPCClient(IPCClient&&) = delete;
    IPCClient& operator=(IPCClient&&) = delete;

    /**
     * Connects to the WebSocket server.
     *
     * @param host The server hostname
     * @param port The server port
     * @return True if connection was successful
     */
    bool connect(const std::string& host = "localhost", uint16_t port = 9000);

    /**
     * Disconnects from the server.
     */
    void disconnect();

    /**
     * Checks if connected to the server.
     *
     * @return True if connected
     */
    bool is_connected();

    /**
     * Sends a screen update to the UI.
     *
     * @param update The screen update data
     */
    void send_screen_update(const ScreenUpdate& update);

    /**
     * Sends a full screen update to the UI.
     *
     * @param pixels The pixel data (480x272 RGB565)
     */
    void send_full_screen(const uint16_t* pixels);

    /**
     * Sends motor telemetry to the UI.
     *
     * @param port The motor port (1-21)
     * @param voltage The motor voltage
     * @param velocity The motor velocity
     * @param position The motor position
     */
    void send_motor_telemetry(uint8_t port, int32_t voltage, double velocity, double position);

    /**
     * Sends a log message to the UI.
     *
     * @param level The log level (info, warn, error)
     * @param message The log message
     */
    void send_log(const std::string& level, const std::string& message);

    /**
     * Sends the autonomous list to the UI.
     *
     * @param match_autos Match autonomous names
     * @param skills_autos Skills autonomous names
     */
    void send_auton_list(const std::vector<std::string>& match_autos,
                         const std::vector<std::string>& skills_autos);

    /**
     * Sends LCD text to the UI.
     *
     * @param lines The LCD lines (0-7)
     */
    void send_lcd_update(const std::vector<std::string>& lines);

    /**
     * Sends the current robot mode to the UI.
     *
     * @param mode The mode string ("disabled", "autonomous", "opcontrol")
     */
    void send_mode(const std::string& mode);

    /**
     * Processes incoming messages.
     */
    void process_messages();

    // Callbacks
    using TouchCallback = std::function<void(const TouchInput&)>;
    using ControllerCallback = std::function<void(const ControllerInput&)>;
    using ModeCallback = std::function<void(const std::string&)>;
    using AutoSelectCallback = std::function<void(const std::string&, int)>;

    void set_touch_callback(TouchCallback callback);
    void set_controller_callback(ControllerCallback callback);
    void set_mode_callback(ModeCallback callback);
    void set_auto_select_callback(AutoSelectCallback callback);

private:
    IPCClient();
    ~IPCClient();

    void receive_thread();
    void send_message(const std::string& json);
    void parse_message(const std::string& json);

    std::atomic<bool> _connected;
    std::atomic<bool> _running;
    
    int _socket_fd;
    std::thread _receive_thread;
    
    std::mutex _send_mutex;
    std::mutex _callback_mutex;
    
    std::queue<std::string> _send_queue;
    
    TouchCallback _touch_callback;
    ControllerCallback _controller_callback;
    ModeCallback _mode_callback;
    AutoSelectCallback _auto_select_callback;
};

} // namespace host

#endif // HOST_IPC_HPP
