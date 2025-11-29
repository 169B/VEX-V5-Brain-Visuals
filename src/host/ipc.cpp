/**
 * @file ipc.cpp
 * @brief WebSocket IPC Client Implementation for Host Mode
 */

#include "host/ipc.hpp"
#include "host/hal.hpp"
#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>

// Platform-specific includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

// Simple Base64 encoding
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const uint8_t* data, size_t len) {
    std::string result;
    result.reserve((len + 2) / 3 * 4);
    
    for (size_t i = 0; i < len; i += 3) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < len) n |= static_cast<uint32_t>(data[i + 1]) << 8;
        if (i + 2 < len) n |= static_cast<uint32_t>(data[i + 2]);
        
        result += base64_chars[(n >> 18) & 0x3F];
        result += base64_chars[(n >> 12) & 0x3F];
        result += (i + 1 < len) ? base64_chars[(n >> 6) & 0x3F] : '=';
        result += (i + 2 < len) ? base64_chars[n & 0x3F] : '=';
    }
    
    return result;
}

// Simple JSON helpers
static std::string json_escape(const std::string& s) {
    std::string result;
    for (char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

namespace host {

// Singleton instance
IPCClient& IPCClient::instance() {
    static IPCClient instance;
    return instance;
}

IPCClient::IPCClient() 
    : _connected(false), _running(false), _socket_fd(INVALID_SOCKET) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

IPCClient::~IPCClient() {
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool IPCClient::connect(const std::string& host, uint16_t port) {
    if (_connected) return true;
    
    // Create socket
    _socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket_fd == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    // Resolve hostname
    struct hostent* he = gethostbyname(host.c_str());
    if (!he) {
        std::cerr << "Failed to resolve hostname: " << host << std::endl;
        closesocket(_socket_fd);
        _socket_fd = INVALID_SOCKET;
        return false;
    }
    
    // Connect
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);
    
    if (::connect(_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        closesocket(_socket_fd);
        _socket_fd = INVALID_SOCKET;
        return false;
    }
    
    _connected = true;
    _running = true;
    
    // Start receive thread
    _receive_thread = std::thread(&IPCClient::receive_thread, this);
    
    std::cout << "Connected to WebSocket server at " << host << ":" << port << std::endl;
    return true;
}

void IPCClient::disconnect() {
    if (!_connected) return;
    
    _running = false;
    _connected = false;
    
    if (_socket_fd != INVALID_SOCKET) {
        closesocket(_socket_fd);
        _socket_fd = INVALID_SOCKET;
    }
    
    if (_receive_thread.joinable()) {
        _receive_thread.join();
    }
}

bool IPCClient::is_connected() {
    return _connected;
}

void IPCClient::receive_thread() {
    char buffer[4096];
    
    while (_running && _connected) {
        int bytes = recv(_socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            if (_running) {
                std::cerr << "Connection lost" << std::endl;
                _connected = false;
            }
            break;
        }
        
        buffer[bytes] = '\0';
        parse_message(buffer);
    }
}

void IPCClient::send_message(const std::string& json) {
    if (!_connected) return;
    
    std::lock_guard<std::mutex> lock(_send_mutex);
    
    // Simple WebSocket frame (text, no mask for server->client)
    std::string frame;
    frame += static_cast<char>(0x81); // Text frame, FIN bit set
    
    if (json.size() <= 125) {
        frame += static_cast<char>(json.size());
    } else if (json.size() <= 65535) {
        frame += static_cast<char>(126);
        frame += static_cast<char>((json.size() >> 8) & 0xFF);
        frame += static_cast<char>(json.size() & 0xFF);
    } else {
        // Extended length - not typically needed
        frame += static_cast<char>(127);
        for (int i = 7; i >= 0; i--) {
            frame += static_cast<char>((json.size() >> (i * 8)) & 0xFF);
        }
    }
    
    frame += json;
    
    send(_socket_fd, frame.c_str(), frame.size(), 0);
}

void IPCClient::parse_message(const std::string& json) {
    // Very simple JSON parsing for expected message formats
    // In production, use a proper JSON library
    
    std::lock_guard<std::mutex> lock(_callback_mutex);
    
    if (json.find("\"type\":\"touch\"") != std::string::npos) {
        if (_touch_callback) {
            TouchInput input;
            // Parse x, y, pressed from JSON
            size_t xpos = json.find("\"x\":");
            size_t ypos = json.find("\"y\":");
            size_t ppos = json.find("\"pressed\":");
            
            if (xpos != std::string::npos) input.x = std::stoi(json.substr(xpos + 4));
            if (ypos != std::string::npos) input.y = std::stoi(json.substr(ypos + 4));
            if (ppos != std::string::npos) input.pressed = (json.substr(ppos + 10, 4) == "true");
            
            _touch_callback(input);
        }
    }
    else if (json.find("\"type\":\"controller\"") != std::string::npos) {
        if (_controller_callback) {
            ControllerInput input = {0, 0, 0, 0, 0};
            // Parse analog and digital values
            // ... simplified parsing
            _controller_callback(input);
        }
    }
    else if (json.find("\"type\":\"mode\"") != std::string::npos) {
        if (_mode_callback) {
            size_t vpos = json.find("\"value\":\"");
            if (vpos != std::string::npos) {
                size_t start = vpos + 9;
                size_t end = json.find("\"", start);
                std::string mode = json.substr(start, end - start);
                _mode_callback(mode);
            }
        }
    }
    else if (json.find("\"type\":\"select_auto\"") != std::string::npos) {
        if (_auto_select_callback) {
            size_t cpos = json.find("\"category\":\"");
            size_t ipos = json.find("\"index\":");
            
            if (cpos != std::string::npos && ipos != std::string::npos) {
                size_t cstart = cpos + 12;
                size_t cend = json.find("\"", cstart);
                std::string category = json.substr(cstart, cend - cstart);
                int index = std::stoi(json.substr(ipos + 8));
                _auto_select_callback(category, index);
            }
        }
    }
}

void IPCClient::send_screen_update(const ScreenUpdate& update) {
    std::ostringstream ss;
    ss << "{\"type\":\"screen\",\"x1\":" << update.x1
       << ",\"y1\":" << update.y1
       << ",\"x2\":" << update.x2
       << ",\"y2\":" << update.y2
       << ",\"data\":\"" << base64_encode(reinterpret_cast<const uint8_t*>(update.pixels.data()),
                                          update.pixels.size() * 2) << "\"}";
    send_message(ss.str());
}

void IPCClient::send_full_screen(const uint16_t* pixels) {
    ScreenUpdate update;
    update.x1 = 0;
    update.y1 = 0;
    update.x2 = 479;
    update.y2 = 271;
    update.pixels.assign(pixels, pixels + (480 * 272));
    send_screen_update(update);
}

void IPCClient::send_motor_telemetry(uint8_t port, int32_t voltage, double velocity, double position) {
    std::ostringstream ss;
    ss << "{\"type\":\"motor\",\"port\":" << static_cast<int>(port)
       << ",\"voltage\":" << voltage
       << ",\"velocity\":" << velocity
       << ",\"position\":" << position << "}";
    send_message(ss.str());
}

void IPCClient::send_log(const std::string& level, const std::string& message) {
    std::ostringstream ss;
    ss << "{\"type\":\"log\",\"level\":\"" << json_escape(level)
       << "\",\"msg\":\"" << json_escape(message) << "\"}";
    send_message(ss.str());
}

void IPCClient::send_auton_list(const std::vector<std::string>& match_autos,
                                 const std::vector<std::string>& skills_autos) {
    std::ostringstream ss;
    ss << "{\"type\":\"autons\",\"match\":[";
    for (size_t i = 0; i < match_autos.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"name\":\"" << json_escape(match_autos[i]) << "\"}";
    }
    ss << "],\"skills\":[";
    for (size_t i = 0; i < skills_autos.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"name\":\"" << json_escape(skills_autos[i]) << "\"}";
    }
    ss << "]}";
    send_message(ss.str());
}

void IPCClient::send_lcd_update(const std::vector<std::string>& lines) {
    std::ostringstream ss;
    ss << "{\"type\":\"lcd\",\"lines\":[";
    for (size_t i = 0; i < lines.size(); i++) {
        if (i > 0) ss << ",";
        ss << "\"" << json_escape(lines[i]) << "\"";
    }
    ss << "]}";
    send_message(ss.str());
}

void IPCClient::send_mode(const std::string& mode) {
    std::ostringstream ss;
    ss << "{\"type\":\"mode\",\"value\":\"" << json_escape(mode) << "\"}";
    send_message(ss.str());
}

void IPCClient::process_messages() {
    // Messages are processed in the receive thread
    // This function can be used for polling-based processing if needed
}

void IPCClient::set_touch_callback(TouchCallback callback) {
    std::lock_guard<std::mutex> lock(_callback_mutex);
    _touch_callback = callback;
}

void IPCClient::set_controller_callback(ControllerCallback callback) {
    std::lock_guard<std::mutex> lock(_callback_mutex);
    _controller_callback = callback;
}

void IPCClient::set_mode_callback(ModeCallback callback) {
    std::lock_guard<std::mutex> lock(_callback_mutex);
    _mode_callback = callback;
}

void IPCClient::set_auto_select_callback(AutoSelectCallback callback) {
    std::lock_guard<std::mutex> lock(_callback_mutex);
    _auto_select_callback = callback;
}

} // namespace host
