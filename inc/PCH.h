#pragma once

#define WIN32_LEAN_AND_MEAN

#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <xbyak/xbyak.h>

using namespace std::literals;

#include <Version.h>

#ifdef SKYRIM_AE
  #define OFFSET(se, ae) ae
  #define OFFSET_3(se, ae, vr) ae
#elif SKYRIMVR
  #define OFFSET(se, ae) se
  #define OFFSET_3(se, ae, vr) vr
#else
  #define OFFSET(se, ae) se
  #define OFFSET_3(se, ae, vr) se
#endif

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
      static T aSingleton;
      return std::addressof(aSingleton);
    }
};

namespace stl {
  using namespace SKSE::stl;

  template <class F, class T>
  void write_vfunc() {
    REL::Relocation<std::uintptr_t> lVTABLE{F::VTABLE[T::fIdx]};
    T::fFunc = lVTABLE.write_vfunc(T::fSize, T::Thunk);
  }
}
