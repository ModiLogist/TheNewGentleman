#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>
#include <Papyrus.h>

bool Papyrus::BindPapyrus(RE::BSScript::IVirtualMachine* vm) {
  vm->RegisterFunction("GetBoolValue", "TNG_PapyrusUtil", GetBoolValue);
  vm->RegisterFunction("SetBoolValue", "TNG_PapyrusUtil", SetBoolValue);

  vm->RegisterFunction("GetAllPossibleAddons", "TNG_PapyrusUtil", GetAllPossibleAddons);
  vm->RegisterFunction("GetAddonStatus", "TNG_PapyrusUtil", GetAddonStatus);
  vm->RegisterFunction("SetAddonStatus", "TNG_PapyrusUtil", SetAddonStatus);

  vm->RegisterFunction("GetRgNames", "TNG_PapyrusUtil", GetRgNames);
  vm->RegisterFunction("GetRgInfo", "TNG_PapyrusUtil", GetRgInfo);
  vm->RegisterFunction("GetRgAddons", "TNG_PapyrusUtil", GetRgAddons);
  vm->RegisterFunction("GetRgAddon", "TNG_PapyrusUtil", GetRgAddon);
  vm->RegisterFunction("GetRgMult", "TNG_PapyrusUtil", GetRgMult);
  vm->RegisterFunction("SetRgAddon", "TNG_PapyrusUtil", SetRgAddon);
  vm->RegisterFunction("SetRgMult", "TNG_PapyrusUtil", SetRgMult);

  vm->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  vm->RegisterFunction("GetActorAddons", "TNG_PapyrusUtil", GetActorAddons);
  vm->RegisterFunction("SetActorAddon", "TNG_PapyrusUtil", SetActorAddon);
  vm->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);
  vm->RegisterFunction("ActorItemsInfo", "TNG_PapyrusUtil", ActorItemsInfo);
  vm->RegisterFunction("SwapRevealing", "TNG_PapyrusUtil", SwapRevealing);
  vm->RegisterFunction("GetActorAddon", "TNG_PapyrusUtil", GetActorAddon);
  vm->RegisterFunction("GetActorSize", "TNG_PapyrusUtil", GetActorSize);

  vm->RegisterFunction("GetSlot52Mods", "TNG_PapyrusUtil", GetSlot52Mods);
  vm->RegisterFunction("Slot52ModBehavior", "TNG_PapyrusUtil", Slot52ModBehavior);

  vm->RegisterFunction("UpdateSettings", "TNG_PapyrusUtil", UpdateSettings);

  vm->RegisterFunction("UpdateLogLvl", "TNG_PapyrusUtil", UpdateLogLvl);
  vm->RegisterFunction("ShowLogLocation", "TNG_PapyrusUtil", ShowLogLocation);
  vm->RegisterFunction("GetErrDscr", "TNG_PapyrusUtil", GetErrDscr);
  return true;
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

bool Papyrus::GetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int addonIdx) {
  if (addonIdx < 0 || addonIdx >= Base::GetAddonCount(isFemale, false)) return false;
  return Base::GetAddonStatus(isFemale, static_cast<size_t>(addonIdx));
}

void Papyrus::SetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int addonIdx, bool status) {
  if (addonIdx < 0 || addonIdx >= Base::GetAddonCount(isFemale, false)) return;
  Inis::SaveAddonStatus(isFemale, addonIdx, status);
  Base::SetAddonStatus(isFemale, addonIdx, status);
}

std::vector<std::string> Papyrus::GetRgNames(RE::StaticFunctionTag*) { return Base::GetRgNames(true); }

std::string Papyrus::GetRgInfo(RE::StaticFunctionTag*, int rgIdx) { return Base::GetRgInfo(rgIdx, true); };

std::vector<std::string> Papyrus::GetRgAddons(RE::StaticFunctionTag*, int rgIdx) {
  std::vector<std::string> res{};
  if (rgIdx < 0) return res;
  auto list = Base::GetRgAddonList(static_cast<size_t>(rgIdx), false, true, false);
  res.push_back("$TNG_TRS");
  res.push_back("$TNG_TNT");
  for (auto i : list) {
    auto isDed = Base::IsAddonDedicatedToRg(rgIdx, false, true, i);
    std::string name = Base::AddonByIdx(false, i, false)->GetName();
    switch (isDed) {
      case Tng::resOkMain:
        break;
      case Tng::resOkDedicated:
        name = name + " (d)";
        break;
      case Tng::resOkSupported:
        name = name + " (s)";
        break;
      default:
        continue;
        break;
    }
    res.push_back(name);
  }
  return res;
}

int Papyrus::GetRgAddon(RE::StaticFunctionTag*, int rgIdx) {
  if (rgIdx < 0) return Tng::pgErr;
  auto rgAddon = Base::GetRgAddon(static_cast<size_t>(rgIdx), true);
  if (rgAddon < 0) return rgAddon == Tng::cDef ? 0 : rgAddon;
  auto list = Base::GetRgAddonList(static_cast<size_t>(rgIdx), false, true, false);
  auto it = std::find(list.begin(), list.end(), rgAddon);
  return static_cast<int>(it != list.end() ? std::distance(list.begin(), it) : Tng::rgErr);
}

void Papyrus::SetRgAddon(RE::StaticFunctionTag*, int rgIdx, int choice) {
  if (rgIdx < 0) return;
  if (choice < 0) return;
  auto list = Base::GetRgAddonList(static_cast<size_t>(rgIdx), false, true, false);
  int addnIdx = choice < 0 ? choice : static_cast<int>(list[choice]);
  Core::SetRgAddon(static_cast<size_t>(rgIdx), addnIdx);
}

float Papyrus::GetRgMult(RE::StaticFunctionTag*, int rgIdx) {
  if (rgIdx < 0) return -1.0f;
  return Base::GetRgMult(static_cast<size_t>(rgIdx), true);
}

void Papyrus::SetRgMult(RE::StaticFunctionTag*, int rgIdx, float mult) {
  if (Base::SetRgMult(static_cast<size_t>(rgIdx), mult, true)) Inis::SaveRgMult(static_cast<size_t>(rgIdx), mult);
}

int Papyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* actor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  auto res = Core::CanModifyNPC(npc);
  switch (res) {
    case Tng::resOkRaceP:
      return Tng::resOkSizable;
    case Tng::resOkRacePP:
      return Tng::resOkSizable;
    case Tng::resOkRaceR:
      return Tng::resOkFixed;
    default:
      return res;
  }
}

std::vector<std::string> Papyrus::GetActorAddons(RE::StaticFunctionTag*, RE::Actor* actor) {
  std::vector<std::string> res{};
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race) return res;
  auto list = Base::GetRgAddonList(npc->race, npc->IsFemale(), false);
  auto rgIdx = Base::GetRaceRgIdx(npc->race);
  res.push_back("$TNG_TRS");
  res.push_back("$TNG_TNT");
  for (auto i : list) {
    auto isDed = Base::IsAddonDedicatedToRg(rgIdx, npc->IsFemale(), false, i);
    std::string name = Base::AddonByIdx(npc->IsFemale(), i, false)->GetName();
    switch (isDed) {
      case Tng::resOkMain:
        break;
      case Tng::resOkDedicated:
        name = name + " (d)";
        break;
      case Tng::resOkSupported:
        name = name + " (s)";
        break;
      default:
        continue;
        break;
    }
    res.push_back(name);
  }
  return res;
}

RE::TESObjectARMO* Papyrus::GetActorAddon(RE::StaticFunctionTag*, RE::Actor* actor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return nullptr;
  auto addnPair = Base::GetNPCAddon(npc);
  switch (addnPair.second) {
    case Tng::pgErr:
      return nullptr;
    case Tng::cDef:
      if (npc->IsFemale()) {
        return nullptr;
      } else {
        auto rgAddon = Base::GetRgAddon(npc->race);
        switch (rgAddon) {
          case Tng::pgErr:
            return nullptr;
          case Tng::cNul:
            return nullptr;
          case Tng::cDef:
            return nullptr;
          default:
            return Base::AddonByIdx(false, static_cast<size_t>(rgAddon), false);
        }
      }
    case Tng::cNul:
      return nullptr;
    default:
      return Base::AddonByIdx(npc->IsFemale(), static_cast<size_t>(addnPair.second), false);
  }
}

int Papyrus::SetActorAddon(RE::StaticFunctionTag*, RE::Actor* actor, int choice) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Tng::npcErr;
  if (!npc->race) return Tng::raceErr;
  auto list = Base::GetRgAddonList(npc->race, npc->IsFemale(), false);
  int addnIdx = choice < 0 ? choice : static_cast<int>(list[choice]);
  if (actor->IsPlayerRef()) Base::SetPlayerInfo(actor, addnIdx);
  if (npc->race->HasKeyword(Tng::RaceKey(Tng::rkeyPreprocessed)) && !Base::ReevaluateRace(npc->race, actor)) return Tng::raceErr;
  return Core::SetNPCAddon(npc, addnIdx, true);
}

int Papyrus::GetActorSize(RE::StaticFunctionTag*, RE::Actor* actor) {
  int sizeCat = -1;
  return Base::GetActorSizeCat(actor, sizeCat) < 0 ? -1 : sizeCat;
}

int Papyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* actor, int genSize) { return Core::SetActorSize(actor, genSize); }

std::vector<std::string> Papyrus::ActorItemsInfo(RE::StaticFunctionTag*, RE::Actor* actor) {
  std::vector<std::string> res{};
  auto wornArmor = Core::GetActorWornArmor(actor);
  for (auto& armor : wornArmor) res.push_back(armor->GetName());
  return res;
}

bool Papyrus::SwapRevealing(RE::StaticFunctionTag*, RE::Actor* actor, int choice) {
  auto wornArmor = Core::GetActorWornArmor(actor);
  if (choice < 0 || choice > wornArmor.size()) return false;
  auto res = Core::SwapRevealing(actor, wornArmor[choice]);
  Events::DoChecks(actor);
  return res;
}

std::vector<std::string> Papyrus::GetSlot52Mods(RE::StaticFunctionTag*) { return Inis::Slot52Mods(); }

bool Papyrus::Slot52ModBehavior(RE::StaticFunctionTag*, std::string modName, int behavior) { return Inis::Slot52ModBehavior(modName, behavior); }

void Papyrus::UpdateSettings(RE::StaticFunctionTag*) {
  Inis::SaveGlobals();
  Core::RevisitRevealingArmor();
}

int Papyrus::UpdateLogLvl(RE::StaticFunctionTag*, int logLevel) {
  Inis::SetLogLvl(logLevel < 0 ? logLevel : logLevel + spdlog::level::debug);
  return int(Inis::GetLogLvl()) - int(spdlog::level::debug);
}

std::string Papyrus::ShowLogLocation(RE::StaticFunctionTag*) {
  auto logDir{Tng::logger::log_directory};
  std::filesystem::path path = logDir().value_or("$TNG_LDN");
  return path.string();
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
