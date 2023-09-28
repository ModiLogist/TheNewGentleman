#pragma once

namespace Hooks {
    void Install() noexcept;    

    class AddWornItem : public Singleton<AddWornItem> {
    public:
        static bool Thunk(RE::Character* a_this, RE::TESBoundObject* a_item, std::int32_t a_count, bool a_forceEquip, std::uint32_t a_arg4, std::uint32_t a_arg5) noexcept;
        inline static REL::Relocation<decltype(Thunk)> func;
        static inline constexpr std::size_t index{0};
        static inline constexpr std::size_t size{0x57};
    };

    class Load3D : public Singleton<Load3D> {
    public:
        static RE::NiAVObject* Thunk(RE::Character* a_this, bool a_arg1) noexcept;
        inline static REL::Relocation<decltype(Thunk)> func;
        static inline constexpr std::size_t index{0};
        static inline constexpr std::size_t size{0x6A};
    };
}