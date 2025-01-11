#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>
#include <Papyrus.h>

bool Papyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) {
  aVM->RegisterFunction("UpdateLogLvl", "TNG_PapyrusUtil", UpdateLogLvl);

  aVM->RegisterFunction("GetBoolValue", "TNG_PapyrusUtil", GetBoolValue);
  aVM->RegisterFunction("SetBoolValue", "TNG_PapyrusUtil", SetBoolValue);

  aVM->RegisterFunction("GetAllPossibleAddons", "TNG_PapyrusUtil", GetAllPossibleAddons);
  aVM->RegisterFunction("GetAddonStatus", "TNG_PapyrusUtil", GetAddonStatus);
  aVM->RegisterFunction("SetAddonStatus", "TNG_PapyrusUtil", SetAddonStatus);

  aVM->RegisterFunction("GetRgNames", "TNG_PapyrusUtil", GetRgNames);
  aVM->RegisterFunction("GetRgAddons", "TNG_PapyrusUtil", GetRgAddons);
  aVM->RegisterFunction("GetRgAddn", "TNG_PapyrusUtil", GetRgAddn);
  aVM->RegisterFunction("GetRgMult", "TNG_PapyrusUtil", GetRgMult);
  aVM->RegisterFunction("SetRgAddn", "TNG_PapyrusUtil", SetRgAddn);
  aVM->RegisterFunction("SetRgMult", "TNG_PapyrusUtil", SetRgMult);

  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("GetActorAddons", "TNG_PapyrusUtil", GetActorAddons);
  aVM->RegisterFunction("SetActorAddn", "TNG_PapyrusUtil", SetActorAddn);
  aVM->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);

  aVM->RegisterFunction("GetSlot52Mods", "TNG_PapyrusUtil", GetSlot52Mods);
  aVM->RegisterFunction("Slot52ModBehavior", "TNG_PapyrusUtil", Slot52ModBehavior);
  aVM->RegisterFunction("SwapRevealing", "TNG_PapyrusUtil", SwapRevealing);

  aVM->RegisterFunction("UpdateSettings", "TNG_PapyrusUtil", UpdateSettings);

  aVM->RegisterFunction("ShowLogLocation", "TNG_PapyrusUtil", ShowLogLocation);
  aVM->RegisterFunction("GetErrDscr", "TNG_PapyrusUtil", GetErrDscr);
  return true;
}

int Papyrus::UpdateLogLvl(RE::StaticFunctionTag*, int aLogLvl) {
  Inis::SetLogLvl(aLogLvl < 0 ? aLogLvl : aLogLvl + spdlog::level::info);
  return int(Inis::GetLogLvl()) - int(spdlog::level::info);
}

bool Papyrus::GetBoolValue(RE::StaticFunctionTag*, int settingID) {
  if (0 <= settingID && settingID < Tng::BoolSettingCount) return Inis::GetSettingBool(static_cast<Tng::BoolSetting>(settingID));
  return false;
}

void Papyrus::SetBoolValue(RE::StaticFunctionTag*, int settingID, bool value) {
  if (0 <= settingID && settingID < Tng::BoolSettingCount) Inis::SaveSettingBool(static_cast<Tng::BoolSetting>(settingID), value);
}

std::vector<std::string> Papyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool isFemale) {
  std::vector<std::string> res{};
  auto count = Base::GetAddonCount(isFemale, false);
  for (auto i = 0; i < count; i++) res.push_back(Base::AddonByIdx(isFemale, i, false)->GetName());
  return res;
}

bool Papyrus::GetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int aAddn) {
  if (aAddn < 0 || aAddn >= Base::GetAddonCount(isFemale, false)) return false;
  return Base::GetAddonStatus(isFemale, static_cast<size_t>(aAddn));
}

void Papyrus::SetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int aAddn, bool status) {
  if (aAddn < 0 || aAddn >= Base::GetAddonCount(isFemale, false)) return;
  Inis::SaveAddonStatus(isFemale, aAddn, status);
  Base::SetAddonStatus(isFemale, aAddn, status);
}

std::vector<std::string> Papyrus::GetRgNames(RE::StaticFunctionTag*) { return Base::GetRgNames(true); }

std::vector<std::string> Papyrus::GetRgAddons(RE::StaticFunctionTag*, int rgIdx) {
  std::vector<std::string> res{};
  if (rgIdx < 0) return res;
  auto list = Base::GetRgAddonList(static_cast<size_t>(rgIdx), false, false, true);
  res.push_back("$TNG_TRS");
  res.push_back("$TNG_TNT");
  for (auto i : list) res.push_back(Base::AddonByIdx(false, i, false)->GetName());
  return res;
}

int Papyrus::GetRgAddn(RE::StaticFunctionTag*, int rgIdx) {
  if (rgIdx < 0) return Tng::pgErr;
  auto rgAddn = Base::GetRgAddn(static_cast<size_t>(rgIdx), true);
  if (rgAddn < 0) return rgAddn == Tng::cDef ? 0 : rgAddn;
  auto list = Base::GetRgAddonList(static_cast<size_t>(rgIdx), false, false, true);
  auto it = std::find(list.begin(), list.end(), rgAddn);
  return static_cast<int>(it != list.end() ? std::distance(list.begin(), it) : Tng::rgErr);
}

void Papyrus::SetRgAddn(RE::StaticFunctionTag*, int rgIdx, int choice) {
  if (rgIdx < 0) return;
  if (choice < 0) return;
  auto list = Base::GetRgAddonList(static_cast<size_t>(rgIdx), false, false, true);
  int addnIdx = choice < 0 ? choice : static_cast<int>(list[choice]);
  Core::SetRgAddn(static_cast<size_t>(rgIdx), addnIdx);
}

float Papyrus::GetRgMult(RE::StaticFunctionTag*, int rgIdx) {
  if (rgIdx < 0) return -1.0f;
  return Base::GetRgMult(static_cast<size_t>(rgIdx), true);
}

void Papyrus::SetRgMult(RE::StaticFunctionTag*, int rgIdx, float mult) {
  if (Base::SetRgMult(static_cast<size_t>(rgIdx), mult)) Inis::SaveRgMult(static_cast<size_t>(rgIdx), mult);
}

int Papyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* actor) { return Core::CanModifyActor(actor); }

std::vector<std::string> Papyrus::GetActorAddons(RE::StaticFunctionTag*, RE::Actor* actor) {
  std::vector<std::string> res{};
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race) return res;
  auto list = Base::GetRgAddonList(npc->race, npc->IsFemale(), false);
  res.push_back("$TNG_TRS");
  res.push_back("$TNG_TNT");
  for (auto i : list) res.push_back(Base::AddonByIdx(false, i, false)->GetName());
  return res;
}

int Papyrus::SetActorAddn(RE::StaticFunctionTag*, RE::Actor* actor, int choice) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Tng::npcErr;
  if (!npc->race) return Tng::raceErr;
  auto list = Base::GetRgAddonList(npc->race, npc->IsFemale(), false);
  int addnIdx = choice < 0 ? choice : static_cast<int>(list[choice]);
  if (actor->IsPlayerRef()) Events::SetPlayerInfo(actor, addnIdx);
  return Core::SetNPCAddn(npc, addnIdx, true);
}

int Papyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* actor, int genSize) { return Core::SetActorSize(actor, genSize); }

std::vector<std::string> Papyrus::GetSlot52Mods(RE::StaticFunctionTag*) { return Inis::Slot52Mods(); }

bool Papyrus::Slot52ModBehavior(RE::StaticFunctionTag*, std::string modName, int behavior) { return Inis::Slot52ModBehavior(modName, behavior); }

bool Papyrus::SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* armor) {
  if (!armor) return false;
  return Core::SwapRevealing(armor);
}

void Papyrus::UpdateSettings(RE::StaticFunctionTag*) {
  Inis::SaveGlobals();
  Core::RevisitRevealingArmor();
}

std::string Papyrus::ShowLogLocation(RE::StaticFunctionTag*) {
  auto lMaybeDir{Tng::logger::log_directory};
  std::filesystem::path lDir = lMaybeDir().value_or("$TNG_LDN");
  return lDir.string();
}

std::string Papyrus::GetErrDscr(RE::StaticFunctionTag*, int errCode) {
  switch (errCode) {
    case Tng::pgErr:
      return "$TNG_WN9";
    case Tng::rgErr:
      return "$TNG_WN8";
    case Tng::skeletonErr:
      return "$TNG_WN7";
    case Tng::playerErr:
      return "$TNG_WN6";
    case Tng::skinErr:
      return "$TNG_WN5";
    case Tng::armoErr:
      return "$TNG_WN4";
    case Tng::addonErr:
      return "$TNG_WN3";
    case Tng::npcErr:
      return "$TNG_WN2";
    case Tng::raceErr:
      return "$TNG_WN1";
    default:
      return "$TNG_WN9";
  }
}
