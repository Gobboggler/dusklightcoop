#pragma once

#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>
#include <cstdint>

namespace dusk {
namespace network {

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Error
};

using PacketHandler = std::function<void(const std::vector<uint8_t>&)>;

class TcpClient {
public:
    TcpClient();
    ~TcpClient();

    void connect(const std::string& host, uint16_t port);
    void disconnect();
    void send(const std::vector<uint8_t>& data);
    bool is_connected() const;
    ConnectionState state() const;

    void set_packet_handler(PacketHandler handler);

private:
    void reader_thread();

    std::string m_host;
    uint16_t m_port;
    int m_sock;
    std::atomic<ConnectionState> m_state;
    std::thread m_reader;
    std::mutex m_write_mutex;
    PacketHandler m_packet_handler;
};

} // namespace network
} // namespace dusk
