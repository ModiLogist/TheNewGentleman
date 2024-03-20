#pragma once

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

inline auto MakeHook(REL::ID a_id, std::ptrdiff_t a_offset = 0) { return REL::Relocation<std::uintptr_t>(a_id, a_offset); }

inline auto MakeHook(REL::Offset a_address, std::ptrdiff_t a_offset = 0) { return REL::Relocation<std::uintptr_t>(a_address.address() + a_offset); }

#ifdef SKYRIM_AE
  #ifdef SKYRIM_353
    #define IF_SKYRIMSE(aResAE, aResSE, aOffsetVR, aOffset353) (aOffset353)
  #else
    #define IF_SKYRIMSE(aResAE, aResSE, aOffsetVR, aOffset353) (aResAE)
  #endif
#else
  #ifndef SKYRIMVR
    #define IF_SKYRIMSE(aResAE, aResSE, aOffsetVR, aOffset353) (aResSE)
  #else
    #define IF_SKYRIMSE(aResAE, aResSE, aOffsetVR, aOffset353) (aOffsetVR)
  #endif
#endif

#define MAKE_OFFSET(aIdAE, aIdSE, aOffsetVR, aOffset353) IF_SKYRIMSE(REL::ID(aIdAE), REL::ID(aIdSE), REL::Offset(aOffsetVR), REL::Offset(aOffset353))
