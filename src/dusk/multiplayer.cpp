#include "dusk/multiplayer.hpp"
#include "dusk/tp_online.hpp"
#include "dusk/network.hpp"
#include "dusk/settings.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include <cstdio>
#include <string>

namespace dusk {
namespace multiplayer {

static bool g_initialized = false;
static bool g_pending_connection = false;
static std::string g_pending_ip;
static uint16_t g_pending_port;
static std::string g_pending_username;
static JoinType g_pending_join_type;
static network::TcpClient g_client;

#ifdef _WIN32
static PROCESS_INFORMATION g_server_proc{};
#endif

bool initialize()
{
    if (g_initialized) return true;
    tp_online::initialize();
    g_initialized = true;
    return true;
}

void shutdown()
{
    if (g_client.is_connected()) {
        g_client.disconnect();
    }
    shutdown_server();
    g_initialized = false;
}

bool has_pending_connection()
{
    return g_pending_connection;
}

void set_pending_connection(const std::string& ip, uint16_t port,
                            const std::string& username,
                            JoinType join_type)
{
    g_pending_ip = ip;
    g_pending_port = port;
    g_pending_username = username;
    g_pending_join_type = join_type;
    g_pending_connection = true;
}

bool connect_online()
{
    if (!g_pending_connection) return false;

    bool auto_launch = settings::Get(settings::Setting::Multiplayer_AutoLaunchServer).value_int != 0;
    if (auto_launch && g_pending_join_type == JoinType::Host) {
        std::string jar_path = settings::Get(settings::Setting::Multiplayer_ServerJarPath).value_string;
        if (!jar_path.empty()) {
            launch_server(jar_path);
        }
    }

    g_client.set_packet_handler([](const std::vector<uint8_t>& data) {
        auto pkt = tp_online::Packet::deserialize(data);
        if (pkt.type == tp_online::PacketType::Login) {
            std::printf("[MP] Login response received\n");
        }
    });

    g_client.connect(g_pending_ip, g_pending_port);
    if (!g_client.is_connected()) {
        std::printf("[MP] Failed to connect to %s:%u\n",
                    g_pending_ip.c_str(), g_pending_port);
        g_pending_connection = false;
        return false;
    }

    tp_online::LoginPacket login;
    login.username = g_pending_username;
    login.player_color = static_cast<uint8_t>(
        settings::Get(settings::Setting::Multiplayer_PlayerColor).value_int);
    login.wolf_color = static_cast<uint8_t>(
        settings::Get(settings::Setting::Multiplayer_WolfColor).value_int);

    tp_online::Packet pkt;
    pkt.type = tp_online::PacketType::Login;
    pkt.payload = login.serialize();
    g_client.send(pkt.serialize());

    std::printf("[MP] Connected to %s:%u as %s\n",
                g_pending_ip.c_str(), g_pending_port,
                g_pending_username.c_str());
    g_pending_connection = false;
    return true;
}

void disconnect_online()
{
    if (g_client.is_connected()) {
        tp_online::Packet pkt;
        pkt.type = tp_online::PacketType::Logout;
        g_client.send(pkt.serialize());
        g_client.disconnect();
    }
    shutdown_server();
}

bool is_connected()
{
    return g_client.is_connected();
}

void launch_server(const std::string& jar_path)
{
#ifdef _WIN32
    if (g_server_proc.hProcess) {
        CloseHandle(g_server_proc.hProcess);
        g_server_proc.hProcess = nullptr;
    }
    if (g_server_proc.hThread) {
        CloseHandle(g_server_proc.hThread);
        g_server_proc.hThread = nullptr;
    }

    std::string cmdline = "java -jar \"" + jar_path + "\"";

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    if (!CreateProcessA(nullptr, &cmdline[0], nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &g_server_proc)) {
        std::printf("[MP] Failed to launch server: %lu\n", GetLastError());
        return;
    }
    std::printf("[MP] Server launched (PID: %lu)\n", g_server_proc.dwProcessId);
#else
    pid_t pid = fork();
    if (pid == 0) {
        execlp("java", "java", "-jar", jar_path.c_str(), nullptr);
        _exit(1);
    } else if (pid > 0) {
        std::printf("[MP] Server launched (PID: %d)\n", pid);
    }
#endif
}

void shutdown_server()
{
#ifdef _WIN32
    if (g_server_proc.hProcess) {
        TerminateProcess(g_server_proc.hProcess, 0);
        WaitForSingleObject(g_server_proc.hProcess, 5000);
        CloseHandle(g_server_proc.hProcess);
        CloseHandle(g_server_proc.hThread);
        g_server_proc.hProcess = nullptr;
        g_server_proc.hThread = nullptr;
        std::printf("[MP] Server shut down\n");
    }
#endif
}

} // namespace multiplayer
} // namespace dusk
