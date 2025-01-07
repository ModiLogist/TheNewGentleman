#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>
#include <Papyrus.h>

bool Papyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) {
  aVM->RegisterFunction("UpdateLogLvl", "TNG_PapyrusUtil", UpdateLogLvl);
  aVM->RegisterFunction("ShowLogLocation", "TNG_PapyrusUtil", ShowLogLocation);

  aVM->RegisterFunction("GetBoolValue", "TNG_PapyrusUtil", GetBoolValue);
  aVM->RegisterFunction("SetBoolValue", "TNG_PapyrusUtil", SetBoolValue);

  aVM->RegisterFunction("GetRgNames", "TNG_PapyrusUtil", GetRgNames);
  aVM->RegisterFunction("GetRgAddn", "TNG_PapyrusUtil", GetRgAddn);
  aVM->RegisterFunction("GetRgMult", "TNG_PapyrusUtil", GetRgMult);
  aVM->RegisterFunction("SetRgAddn", "TNG_PapyrusUtil", SetRgAddn);
  aVM->RegisterFunction("SetRgMult", "TNG_PapyrusUtil", SetRgMult);

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
  auto lMaybeDir{Tng::logger::log_directory};
  std::filesystem::path lDir = lMaybeDir().value_or("$TNG_LDN");
  return lDir.string();
}

bool Papyrus::GetBoolValue(RE::StaticFunctionTag*, int settingID) {
  if (0 <= settingID && settingID < Tng::BoolSettingCount) return Inis::GetSettingBool(static_cast<Tng::BoolSetting>(settingID));
  return false;
}

void Papyrus::SetBoolValue(RE::StaticFunctionTag*, int settingID, bool value) {
  if (0 <= settingID && settingID < Tng::BoolSettingCount) Inis::SaveSettingBool(static_cast<Tng::BoolSetting>(settingID), value);
}

std::vector<std::string> Papyrus::GetRgNames(RE::StaticFunctionTag*) { return Base::GetRgNames(true); }

int Papyrus::GetRgAddn(RE::StaticFunctionTag*, int rg) {
  if (rg < 0) return Tng::pgErr;
  return Base::GetRgAddn(static_cast<size_t>(rg), true);
}

float Papyrus::GetRgMult(RE::StaticFunctionTag*, int rg) {
  if (rg < 0) return -1.0f;
  return Base::GetRgMult(static_cast<size_t>(rg),true);
}

void Papyrus::SetRgAddn(RE::StaticFunctionTag*, int rg, int aGenOption) {
  if (rg < 0) return;
  Core::SetRgAddn(static_cast<int>(rg), aGenOption, true);
}

void Papyrus::SetRgMult(RE::StaticFunctionTag*, int rg, float aGenMult) {
  if (Base::SetRgMult(static_cast<size_t>(rg), aGenMult)) Inis::SaveRgMult(static_cast<size_t>(rg), aGenMult);
}

bool Papyrus::GetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int aAddn) {
  if (aAddn < 0 || aAddn >= Base::GetAddonCount(isFemale,false)) return false;
  return Base::GetAddonStatus(isFemale, static_cast<size_t>(aAddn));
}

void Papyrus::SetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int aAddn, bool status) {
  if (aAddn < 0 || aAddn >= Base::GetAddonCount(isFemale,false)) return;
  Inis::SaveActiveAddon(isFemale, aAddn, status);
  Base::SetAddonStatus(isFemale, aAddn, status);
}

std::vector<std::string> Papyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool isFemale) {
  /*auto lNames = Base::Getadd(isFemale);
  return std::vector<std::string>{lNames.begin(), lNames.end()};*/
  return {};
}

int Papyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* actor) { return Core::CanModifyActor(actor); }

int Papyrus::SetActorAddn(RE::StaticFunctionTag*, RE::Actor* actor, int aGenOption) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!actor || !npc) return Tng::npcErr;
  if (actor->IsPlayerRef()) Events::SetPlayerInfo(actor, aGenOption);
  return Core::SetNPCAddn(npc, aGenOption, true);
}

int Papyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* actor, int genSize) { return Core::SetActorSize(actor, genSize); }

std::vector<std::string> Papyrus::GetSlot52Mods(RE::StaticFunctionTag*) {
  /*auto values = Core::GetSlot52Mods();
  return std::vector<std::string>(values.begin(), values.end());*/
  return {};
}

bool Papyrus::Slot52ModBehavior(RE::StaticFunctionTag*, std::string modName, int behavior) { return Inis::Slot52ModBehavior(modName, behavior); }

bool Papyrus::SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* armor) {
  if (!armor) return false;
  return Core::SwapRevealing(armor);
}

void Papyrus::UpdateSettings(RE::StaticFunctionTag*) {
  Inis::SaveGlobals();
  Core::RevisitRevealingArmor();
}