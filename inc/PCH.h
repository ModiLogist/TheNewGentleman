#pragma once

#define WIN32_LEAN_AND_MEAN

#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>
#include <spdlog/sinks/basic_file_sink.h>

using namespace std::literals;
using namespace REL::literals;

#include <Version.h>

typedef std::pair<RE::FormID, std::string_view> SEFormLoc;

static void ShowSkyrimMessage(const char* aMessage) noexcept { RE::DebugMessageBox(aMessage); }

static std::pair<std::string, RE::FormID> StrToRecord(const std::string aRecord) noexcept {
  const size_t lSepLoc = aRecord.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aRecord.substr(lSepLoc + 1);
  return std::make_pair(lModName, lFormID);
}

static std::string RecordToStr(RE::TESForm* aForm) noexcept {
  if (!aForm->GetFile(0)) return "";
  std::ostringstream oss;
  oss << std::hex << aForm->GetLocalFormID();
  return "0x" + oss.str() + Tng::cDelimChar + std::string(aForm->GetFile(0)->GetFilename());
}

static SEFormLoc FormToLoc(RE::TESForm* aForm) noexcept {
  auto filename = aForm->GetFile(0) ? aForm->GetFile(0)->GetFilename() : "NoFile";
  return {aForm->GetFormID(), filename};
}

static bool FormHasKW(const RE::BGSKeywordForm* aForm, const SEFormLoc aID) {
  auto lRawID = Tng::SEDH()->LookupFormID(aID.first, aID.second);
  return aForm->HasKeywordID(lRawID);
}

static bool AddonHasRace(const RE::TESObjectARMA* aAddon, const RE::TESRace* aRace) {
  if (aRace = aAddon->race) return true;
  for (auto& race : aAddon->additionalRaces)
    if (aRace = race) return true;
  return false;
}

template <typename FormType>
static constexpr FormType* LoadForm(std::string aFormRecord) {
  auto lRecod = StrToRecord(aFormRecord);
  return RE::TESDataHandler::GetSingleton()->LookupForm<FormType>(lRecod.second, lRecod.first);
};

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
