#pragma once

// clang-format off
#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include <ShlObj_core.h>
#include <Psapi.h>
#include <Windows.h>

// clang-format on
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

using namespace std::literals;
using namespace REL::literals;

namespace gLogger = SKSE::log;

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