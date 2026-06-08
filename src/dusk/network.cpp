#include "dusk/network.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using SOCKET_TYPE = SOCKET;
constexpr SOCKET_TYPE INVALID_SOCK = INVALID_SOCKET;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
using SOCKET_TYPE = int;
constexpr SOCKET_TYPE INVALID_SOCK = -1;
#endif

#include <cstring>
#include <array>
#include <cstdio>

namespace dusk {
namespace network {

TcpClient::TcpClient()
    : m_sock(static_cast<int>(INVALID_SOCK))
    , m_state(ConnectionState::Disconnected)
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

TcpClient::~TcpClient()
{
    disconnect();
#ifdef _WIN32
    WSACleanup();
#endif
}

void TcpClient::connect(const std::string& host, uint16_t port)
{
    if (m_state != ConnectionState::Disconnected) {
        disconnect();
    }

    m_state = ConnectionState::Connecting;
    m_host = host;
    m_port = port;

    SOCKET_TYPE s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCK) {
        m_state = ConnectionState::Error;
        return;
    }
    m_sock = static_cast<int>(s);

    struct addrinfo hints, *res = nullptr;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    std::string port_str = std::to_string(port);

    int ret = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (ret != 0 || !res) {
#ifdef _WIN32
        closesocket(static_cast<SOCKET>(m_sock));
#else
        ::close(m_sock);
#endif
        m_sock = static_cast<int>(INVALID_SOCK);
        m_state = ConnectionState::Error;
        return;
    }

    if (::connect(static_cast<SOCKET>(m_sock), res->ai_addr, static_cast<int>(res->ai_addrlen)) < 0) {
#ifdef _WIN32
        closesocket(static_cast<SOCKET>(m_sock));
#else
        ::close(m_sock);
#endif
        freeaddrinfo(res);
        m_sock = static_cast<int>(INVALID_SOCK);
        m_state = ConnectionState::Error;
        return;
    }

    freeaddrinfo(res);
    m_state = ConnectionState::Connected;

    if (m_reader.joinable()) {
        m_reader.join();
    }
    m_reader = std::thread(&TcpClient::reader_thread, this);
}

void TcpClient::disconnect()
{
    m_state = ConnectionState::Disconnected;
    if (m_sock != static_cast<int>(INVALID_SOCK)) {
#ifdef _WIN32
        closesocket(static_cast<SOCKET>(m_sock));
#else
        ::close(m_sock);
#endif
        m_sock = static_cast<int>(INVALID_SOCK);
    }
    if (m_reader.joinable()) {
        m_reader.join();
    }
}

void TcpClient::send(const std::vector<uint8_t>& data)
{
    if (m_state != ConnectionState::Connected) return;
    std::lock_guard<std::mutex> lock(m_write_mutex);
#ifdef _WIN32
    ::send(static_cast<SOCKET>(m_sock),
        reinterpret_cast<const char*>(data.data()),
        static_cast<int>(data.size()), 0);
#else
    ::write(m_sock, data.data(), data.size());
#endif
}

bool TcpClient::is_connected() const
{
    return m_state == ConnectionState::Connected;
}

ConnectionState TcpClient::state() const
{
    return m_state;
}

void TcpClient::set_packet_handler(PacketHandler handler)
{
    m_packet_handler = handler;
}

void TcpClient::reader_thread()
{
    std::array<uint8_t, 4096> buf;
    while (m_state == ConnectionState::Connected) {
#ifdef _WIN32
        int n = recv(static_cast<SOCKET>(m_sock),
            reinterpret_cast<char*>(buf.data()),
            static_cast<int>(buf.size()), 0);
#else
        int n = static_cast<int>(::read(m_sock, buf.data(), buf.size()));
#endif
        if (n <= 0) {
            m_state = ConnectionState::Disconnected;
            break;
        }
        std::vector<uint8_t> packet(buf.begin(), buf.begin() + n);
        if (m_packet_handler) {
            m_packet_handler(packet);
        }
    }
}

} // namespace network
} // namespace dusk
