#include "dusk/splitscreen.hpp"

namespace dusk {

std::tuple<float, float, float, float> getSplitscreenViewport(
    int playerIndex, int playerCount, SplitscreenLayout layout)
{
    float left, top, width, height;

    if (playerCount <= 1) {
        left = 0.0f;
        top = 0.0f;
        width = 1.0f;
        height = 1.0f;
    } else {
        switch (layout) {
        case SplitscreenLayout::Horizontal:
            left = static_cast<float>(playerIndex) / static_cast<float>(playerCount);
            top = 0.0f;
            width = 1.0f / static_cast<float>(playerCount);
            height = 1.0f;
            break;
        case SplitscreenLayout::Vertical:
            left = 0.0f;
            top = static_cast<float>(playerIndex) / static_cast<float>(playerCount);
            width = 1.0f;
            height = 1.0f / static_cast<float>(playerCount);
            break;
        case SplitscreenLayout::Quad:
            left = (playerIndex % 2) * 0.5f;
            top = (playerIndex / 2) * 0.5f;
            width = 0.5f;
            height = 0.5f;
            break;
        }
    }

    return {left, top, width, height};
}

} // namespace dusk
