#pragma once

namespace Hooks {
    void Install() noexcept;

    class Load3D : public Singleton<Load3D> {
    public:
        static RE::NiAVObject* Thunk(RE::Character* a_this, bool a_arg1) noexcept;
        inline static REL::Relocation<decltype(Thunk)> func;
        static inline constexpr std::size_t index{0};
        static inline constexpr std::size_t size{0x6A};
    };
}