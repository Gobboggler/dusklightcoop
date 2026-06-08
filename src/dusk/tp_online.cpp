#include "dusk/tp_online.hpp"
#include <cstring>
#include <cstdio>

namespace dusk {
namespace tp_online {

std::vector<uint8_t> Packet::serialize() const
{
    std::vector<uint8_t> data;
    data.push_back(static_cast<uint8_t>(type));
    uint32_t len = static_cast<uint32_t>(payload.size());
    data.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(len & 0xFF));
    data.insert(data.end(), payload.begin(), payload.end());
    return data;
}

Packet Packet::deserialize(const std::vector<uint8_t>& data)
{
    Packet pkt;
    if (data.size() < 3) return pkt;
    pkt.type = static_cast<PacketType>(data[0]);
    uint32_t len = (static_cast<uint32_t>(data[1]) << 8) | data[2];
    if (data.size() < 3 + len) return pkt;
    pkt.payload.assign(data.begin() + 3, data.begin() + 3 + len);
    return pkt;
}

std::vector<uint8_t> LoginPacket::serialize() const
{
    std::vector<uint8_t> data;
    uint16_t name_len = static_cast<uint16_t>(username.size());
    data.push_back(static_cast<uint8_t>((name_len >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(name_len & 0xFF));
    data.insert(data.end(), username.begin(), username.end());
    data.push_back(player_color);
    data.push_back(wolf_color);
    return data;
}

LoginPacket LoginPacket::deserialize(const std::vector<uint8_t>& data)
{
    LoginPacket pkt{};
    if (data.size() < 4) return pkt;
    uint16_t name_len = (static_cast<uint16_t>(data[0]) << 8) | data[1];
    if (data.size() < 4 + name_len) return pkt;
    pkt.username.assign(data.begin() + 2, data.begin() + 2 + name_len);
    pkt.player_color = data[2 + name_len];
    pkt.wolf_color = data[3 + name_len];
    return pkt;
}

std::vector<uint8_t> PositionPacket::serialize() const
{
    std::vector<uint8_t> data;
    data.push_back(static_cast<uint8_t>((room >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(room & 0xFF));
    auto append_f32 = [&](float v) {
        uint32_t iv;
        std::memcpy(&iv, &v, sizeof(iv));
        data.push_back(static_cast<uint8_t>((iv >> 24) & 0xFF));
        data.push_back(static_cast<uint8_t>((iv >> 16) & 0xFF));
        data.push_back(static_cast<uint8_t>((iv >> 8) & 0xFF));
        data.push_back(static_cast<uint8_t>(iv & 0xFF));
    };
    append_f32(x);
    append_f32(y);
    append_f32(z);
    append_f32(angle);
    return data;
}

PositionPacket PositionPacket::deserialize(const std::vector<uint8_t>& data)
{
    PositionPacket pkt{};
    if (data.size() < 18) return pkt;
    pkt.room = (static_cast<uint16_t>(data[0]) << 8) | data[1];
    auto read_f32 = [&](size_t off) {
        uint32_t iv = (static_cast<uint32_t>(data[off]) << 24) |
                      (static_cast<uint32_t>(data[off + 1]) << 16) |
                      (static_cast<uint32_t>(data[off + 2]) << 8) |
                      data[off + 3];
        float v;
        std::memcpy(&v, &iv, sizeof(v));
        return v;
    };
    pkt.x = read_f32(2);
    pkt.y = read_f32(6);
    pkt.z = read_f32(10);
    pkt.angle = read_f32(14);
    return pkt;
}

std::vector<uint8_t> ChatPacket::serialize() const
{
    std::vector<uint8_t> data;
    uint16_t msg_len = static_cast<uint16_t>(message.size());
    data.push_back(static_cast<uint8_t>((msg_len >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(msg_len & 0xFF));
    data.insert(data.end(), message.begin(), message.end());
    return data;
}

ChatPacket ChatPacket::deserialize(const std::vector<uint8_t>& data)
{
    ChatPacket pkt{};
    if (data.size() < 2) return pkt;
    uint16_t msg_len = (static_cast<uint16_t>(data[0]) << 8) | data[1];
    if (data.size() < 2 + msg_len) return pkt;
    pkt.message.assign(data.begin() + 2, data.begin() + 2 + msg_len);
    return pkt;
}

bool initialize()
{
    return true;
}

} // namespace tp_online
} // namespace dusk
