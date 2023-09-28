#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <REL/Relocation.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

using namespace std::literals;
using namespace REL::literals;

namespace logger = SKSE::log;

template <typename T>
class Singleton {
protected:
    constexpr Singleton() noexcept = default;
    constexpr ~Singleton() noexcept = default;

public:
    constexpr Singleton(const Singleton&) = delete;
    constexpr Singleton(Singleton&&) = delete;
    constexpr auto operator=(const Singleton&) = delete;
    constexpr auto operator=(Singleton&&) = delete;

    [[nodiscard]] static constexpr T* GetSingleton() noexcept {
        static T singleton;
        return std::addressof(singleton);
    }
};

namespace stl {
    using namespace SKSE::stl;

    template <typename T>
    constexpr auto write_thunk_call() noexcept {
        SKSE::AllocTrampoline(14);
        auto& trampoline{SKSE::GetTrampoline()};
        T::func = trampoline.write_call<5>(T::address, T::Thunk);
    }

    template <typename TDest, typename TSource>
    constexpr auto write_vfunc() noexcept {
        REL::Relocation<std::uintptr_t> vtbl{TDest::VTABLE[TSource::index]};
        TSource::func = vtbl.write_vfunc(TSource::size, TSource::Thunk);
    }

    template <typename T>
    constexpr auto write_vfunc(const REL::VariantID variant_id) noexcept {
        REL::Relocation<std::uintptr_t> vtbl{variant_id};
        T::func = vtbl.write_vfunc(T::size, T::Thunk);
    }
}  // namespace stl
