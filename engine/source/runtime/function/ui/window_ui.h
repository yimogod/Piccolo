#pragma once

#include <memory>

namespace Piccolo
{
    class WindowSystem;
    class URenderSystem;

    struct WindowUIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
        std::shared_ptr<URenderSystem> render_system;
    };

    class WindowUI
    {
    public:
        virtual void initialize(WindowUIInitInfo init_info) = 0;
        virtual void preRender() = 0;
    };
} // namespace Piccolo
