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
namespace Tng {
  namespace gLogger = SKSE::log;
  inline static constexpr std::string_view cName = "TheNewGentleman.esp";
  inline static constexpr std::string_view cSkyrim = "Skyrim.esm";
  inline static constexpr int cRaceTypes{14};

  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};
  inline static constexpr RE::FormID cNPCKeywID{0x13794};
  inline static constexpr RE::FormID cCrtKeywID{0x13795};
  inline static constexpr RE::FormID cBstKeywID{0xD61D1};
  inline static constexpr RE::FormID cRevealingKeyID{0xFFF};
  inline static constexpr RE::FormID cUnderwearKeyID{0xFFE};
  inline static constexpr RE::FormID cAutoCoverKeyID{0xFFC};
  inline static constexpr RE::FormID cCoveringKeyID{0xFFD};  
  inline static constexpr RE::FormID cCoverID{0xA00};
  inline static constexpr int cSizeCategories{5};
  inline static constexpr RE::FormID cSizeKeyWIDs[cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};
  inline static constexpr RE::FormID cSizeGlobIDs[cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};
  inline static constexpr RE::FormID cSizeListIDs[cSizeCategories]{0xCF1, 0xCF2, 0xCF3, 0xCF4, 0xCF5};
}

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
