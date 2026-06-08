#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace dusk {
namespace tp_online {

enum class PacketType : uint8_t {
    Login = 0x01,
    Logout = 0x02,
    Position = 0x03,
    Chat = 0x04,
    Sync = 0x05,
    KeepAlive = 0x06,
    Join = 0x07,
    Leave = 0x08,
    EntityUpdate = 0x09,
    Heartbeat = 0x0A
};

struct Packet {
    PacketType type;
    std::vector<uint8_t> payload;

    std::vector<uint8_t> serialize() const;
    static Packet deserialize(const std::vector<uint8_t>& data);
};

struct LoginPacket {
    std::string username;
    uint8_t player_color;
    uint8_t wolf_color;

    std::vector<uint8_t> serialize() const;
    static LoginPacket deserialize(const std::vector<uint8_t>& data);
};

struct PositionPacket {
    uint16_t room;
    float x, y, z;
    float angle;

    std::vector<uint8_t> serialize() const;
    static PositionPacket deserialize(const std::vector<uint8_t>& data);
};

struct ChatPacket {
    std::string message;

    std::vector<uint8_t> serialize() const;
    static ChatPacket deserialize(const std::vector<uint8_t>& data);
};

bool initialize();

} // namespace tp_online
} // namespace dusk
