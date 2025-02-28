#pragma once

#define WIN32_LEAN_AND_MEAN

#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>
#include <spdlog/sinks/basic_file_sink.h>

using namespace std::literals;
using namespace REL::literals;

#include <Version.h>

typedef std::pair<RE::FormID, std::string_view> SEFormLocView;
typedef std::pair<RE::FormID, std::string> SEFormLoc;

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
