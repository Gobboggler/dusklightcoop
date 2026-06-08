#pragma once

#include <cstdint>
#include <tuple>

namespace dusk {

enum class SplitscreenLayout : uint8_t {
    Horizontal = 0,
    Vertical = 1,
    Quad = 2
};

std::tuple<float, float, float, float> getSplitscreenViewport(
    int playerIndex, int playerCount, SplitscreenLayout layout);

} // namespace dusk
