#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>
#include <Papyrus.h>

bool Papyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept {
  aVM->RegisterFunction("UpdateLogLvl", "TNG_PapyrusUtil", UpdateLogLvl);
  aVM->RegisterFunction("ShowLogLocation", "TNG_PapyrusUtil", ShowLogLocation);

  aVM->RegisterFunction("GetBoolValue", "TNG_PapyrusUtil", GetBoolValue);
  aVM->RegisterFunction("SetBoolValue", "TNG_PapyrusUtil", SetBoolValue);

  aVM->RegisterFunction("GetRGNames", "TNG_PapyrusUtil", GetRGNames);
  aVM->RegisterFunction("GetRGAddn", "TNG_PapyrusUtil", GetRGAddn);
  aVM->RegisterFunction("GetRGMult", "TNG_PapyrusUtil", GetRGMult);
  aVM->RegisterFunction("SetRGAddn", "TNG_PapyrusUtil", SetRGAddn);
  aVM->RegisterFunction("SetRGMult", "TNG_PapyrusUtil", SetRGMult);

  aVM->RegisterFunction("GetAddonStatus", "TNG_PapyrusUtil", GetAddonStatus);
  aVM->RegisterFunction("SetAddonStatus", "TNG_PapyrusUtil", SetAddonStatus);
  aVM->RegisterFunction("GetAllPossibleAddons", "TNG_PapyrusUtil", GetAllPossibleAddons);
  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("SetActorAddn", "TNG_PapyrusUtil", SetActorAddn);
  aVM->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);

  aVM->RegisterFunction("GetSlot52Mods", "TNG_PapyrusUtil", GetSlot52Mods);
  aVM->RegisterFunction("Slot52ModBehavior", "TNG_PapyrusUtil", Slot52ModBehavior);
  aVM->RegisterFunction("SwapRevealing", "TNG_PapyrusUtil", SwapRevealing);

  aVM->RegisterFunction("UpdateSettings", "TNG_PapyrusUtil", UpdateSettings);
  return true;
}

int Papyrus::UpdateLogLvl(RE::StaticFunctionTag*, int aLogLvl) {
  Inis::SetLogLvl(aLogLvl < 0 ? aLogLvl : aLogLvl + spdlog::level::info);
  return int(Inis::GetLogLvl()) - int(spdlog::level::info);
}

std::string Papyrus::ShowLogLocation(RE::StaticFunctionTag*) {
  auto lMaybeDir{SKSE::log::log_directory};
  std::filesystem::path lDir = lMaybeDir().value_or("$TNG_LDN");
  return lDir.string();
}

bool Papyrus::GetBoolValue(RE::StaticFunctionTag*, int aID) {
  if (Inis::cNoneBoolID < aID && aID < Inis::cBoolIDsCount) return Inis::GetSettingBool(static_cast<Inis::IniBoolIDs>(aID));
  return false;
}

void Papyrus::SetBoolValue(RE::StaticFunctionTag*, int aID, bool aValue) {
  if (Inis::cNoneBoolID < aID && aID < Inis::cBoolIDsCount) Inis::SaveSettingBool(static_cast<Inis::IniBoolIDs>(aID), aValue);
}

std::vector<std::string> Papyrus::GetRGNames(RE::StaticFunctionTag*) { return Base::GetRGNames(); }

int Papyrus::GetRGAddn(RE::StaticFunctionTag*, int aRgId) {
  if (aRgId < 0) return Tng::pgErr;
  return Base::GetRGAddn(static_cast<std::size_t>(aRgId));
}

float Papyrus::GetRGMult(RE::StaticFunctionTag*, int aRgId) {
  if (aRgId < 0) return -1.0f;
  return Base::GetRGMult(static_cast<std::size_t>(aRgId));
}

void Papyrus::SetRGAddn(RE::StaticFunctionTag*, int aRgId, int aGenOption) {
  if (aRgId < 0) return;
  Core::UpdateRGAddn(static_cast<int>(aRgId), aGenOption);
}

void Papyrus::SetRGMult(RE::StaticFunctionTag*, int aRgId, float aGenMult) {
  if (Base::SetRGMult(static_cast<std::size_t>(aRgId), aGenMult)) Inis::SaveRaceMult(static_cast<std::size_t>(aRgId), aGenMult);
}

bool Papyrus::GetAddonStatus(RE::StaticFunctionTag*, bool aIsFemale, int aAddn) {
  if (aAddn < 0 || aAddn >= Base::GetAddonCount(aIsFemale)) return false;
  return Base::GetAddonStatus(aIsFemale, static_cast<std::size_t>(aAddn));
}

void Papyrus::SetAddonStatus(RE::StaticFunctionTag*, bool aIsFemale, int aAddn, bool aStatus) {
  if (aAddn < 0 || aAddn >= Base::GetAddonCount(aIsFemale)) return;
  Inis::SaveActiveAddon(aIsFemale, aAddn, aStatus);
  Base::SetAddonStatus(aIsFemale, aAddn, aStatus);
}

std::vector<std::string> Papyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale) {
  auto lNames = Base::GetRGAddonNames(aIsFemale);
  return std::vector<std::string>{lNames.begin(), lNames.end()};
}

int Papyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor) { return Core::CanModifyActor(aActor); }

int Papyrus::SetActorAddn(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption) {
  const auto npc = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !npc) return Tng::npcErr;
  if (aActor->IsPlayerRef()) Events::SetPlayerInfo(aActor, aGenOption);
  return Core::SetNPCSkin(npc, aGenOption);
}

int Papyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize) {
  const auto npc = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !npc) return Tng::npcErr;
  return Core::SetCharSize(aActor, npc, aGenSize);
}

std::vector<std::string> Papyrus::GetSlot52Mods(RE::StaticFunctionTag*) {
  auto lMods = Core::GetSlot52Mods();
  return std::vector<std::string>(lMods.begin(), lMods.end());
}

bool Papyrus::Slot52ModBehavior(RE::StaticFunctionTag*, std::string aModName, int aBehavior) { return Inis::Slot52ModBehavior(aModName, aBehavior); }

bool Papyrus::SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor) {
  if (!aArmor) return false;
  return Core::SwapRevealing(aArmor);
}

void Papyrus::UpdateSettings(RE::StaticFunctionTag*) {
  Inis::SaveGlobals();
  Core::RevisitRevealingArmor();
}