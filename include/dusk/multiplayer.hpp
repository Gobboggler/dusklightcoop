#pragma once

#include <string>
#include <cstdint>

namespace dusk {
namespace multiplayer {

enum class JoinType : uint8_t {
    Host = 0,
    Join = 1
};

bool initialize();
void shutdown();

bool has_pending_connection();
void set_pending_connection(const std::string& ip, uint16_t port,
                            const std::string& username,
                            JoinType join_type);

bool connect_online();
void disconnect_online();
bool is_connected();

void launch_server(const std::string& jar_path);
void shutdown_server();

} // namespace multiplayer
} // namespace dusk
