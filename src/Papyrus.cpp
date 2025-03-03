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
  vm->RegisterFunction("GetActorAddon", "TNG_PapyrusUtil", GetActorAddon);
  vm->RegisterFunction("SetActorAddon", "TNG_PapyrusUtil", SetActorAddon);
  vm->RegisterFunction("GetActorSize", "TNG_PapyrusUtil", GetActorSize);
  vm->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);
  vm->RegisterFunction("ActorItemsInfo", "TNG_PapyrusUtil", ActorItemsInfo);
  vm->RegisterFunction("SwapRevealing", "TNG_PapyrusUtil", SwapRevealing);
  vm->RegisterFunction("CheckActors", "TNG_PapyrusUtil", CheckActors);

  vm->RegisterFunction("GetSlot52Mods", "TNG_PapyrusUtil", GetSlot52Mods);
  vm->RegisterFunction("Slot52ModBehavior", "TNG_PapyrusUtil", Slot52ModBehavior);

  vm->RegisterFunction("UpdateSettings", "TNG_PapyrusUtil", UpdateSettings);

  vm->RegisterFunction("UpdateLogLvl", "TNG_PapyrusUtil", UpdateLogLvl);
  vm->RegisterFunction("ShowLogLocation", "TNG_PapyrusUtil", ShowLogLocation);
  vm->RegisterFunction("GetErrDscr", "TNG_PapyrusUtil", GetErrDscr);

  vm->RegisterFunction("WhyProblem", "TNG_PapyrusUtil", WhyProblem);

  return true;
}

bool Papyrus::GetBoolValue(RE::StaticFunctionTag*, int settingID) {
  if (0 <= settingID && settingID < Tng::BoolSettingCount) return Base::GetBoolSetting(static_cast<Tng::BoolSetting>(settingID));
  return false;
}

void Papyrus::SetBoolValue(RE::StaticFunctionTag*, int settingID, bool value) {
  if (0 <= settingID && settingID < Tng::BoolSettingCount) Core::SetBoolSetting(static_cast<Tng::BoolSetting>(settingID), value);
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
  Core::SetAddonStatus(isFemale, addonIdx, status);
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
  if (choice < Tng::cDef) return;
  auto list = Base::GetRgAddonList(static_cast<size_t>(rgIdx), false, true, false);
  int addnIdx = choice < 0 ? choice : static_cast<int>(list[choice]);
  Core::SetRgAddon(static_cast<size_t>(rgIdx), addnIdx, true);
}

float Papyrus::GetRgMult(RE::StaticFunctionTag*, int rgIdx) {
  if (rgIdx < 0) return -1.0f;
  return Base::GetRgMult(static_cast<size_t>(rgIdx), true);
}

void Papyrus::SetRgMult(RE::StaticFunctionTag*, int rgIdx, float mult) { Core::SetRgMult(static_cast<size_t>(rgIdx), mult, true); }

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
  switch (addnPair.first) {
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
      return Base::AddonByIdx(npc->IsFemale(), static_cast<size_t>(addnPair.first), false);
  }
}

int Papyrus::SetActorAddon(RE::StaticFunctionTag*, RE::Actor* actor, int choice) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Tng::npcErr;
  if (!npc->race) return Tng::raceErr;
  auto list = Base::GetRgAddonList(npc->race, npc->IsFemale(), false);
  int addnIdx = choice < 0 ? choice : static_cast<int>(list[choice]);
  if (npc->race->HasKeyword(Tng::Key(Tng::kyPreProcessed)) && !Base::ReevaluateRace(npc->race, actor)) return Tng::raceErr;
  if (actor->IsPlayerRef()) Base::SetPlayerInfo(actor, choice);
  auto res = Core::SetNPCAddon(npc, addnIdx, true);
  if (res >= 0) Events::DoChecks(actor);
  return res;
}

int Papyrus::GetActorSize(RE::StaticFunctionTag*, RE::Actor* actor) {
  int sizeCat = Tng::cNA;
  return Base::GetActorSizeCat(actor, sizeCat) < 0 ? Tng::cNul : sizeCat;
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

std::vector<RE::Actor*> Papyrus::CheckActors(RE::StaticFunctionTag*) {
  SKSE::log::debug("Started checking actors");
  std::vector<RE::Actor*> res{};
  size_t tot = 0;
  RE::TES::GetSingleton()->ForEachReference([&](RE::TESObjectREFR* ref) {
    if (ref && ref->Is3DLoaded() && ref->GetBaseObject()) {
      if (ref->GetBaseObject()->formType != RE::FormType::NPC && ref->GetBaseObject()->formType != RE::FormType::LeveledNPC) return RE::BSContainer::ForEachResult::kContinue;
      tot++;
      auto actor = ref->As<RE::Actor>();
      auto npc = actor ? actor->GetActorBase() : nullptr;
      if (!npc || Core::CanModifyNPC(npc) != Tng::resOkRaceP) return RE::BSContainer::ForEachResult::kContinue;
      if (npc->HasKeyword(Tng::Key(Tng::kyProcessed))) return RE::BSContainer::ForEachResult::kContinue;
      auto addnPair = Events::GetNPCAutoAddon(npc);
      Core::SetNPCAddon(npc, addnPair.first, addnPair.second, false);
      Events::DoChecks(actor);
      if (actor->GetSkin()->HasKeyword(Tng::Key(Tng::kyTngSkin))) {
        SKSE::log::debug("\tFixed [0x{:x}:{}] with addon [{}].", actor->GetFormID(), npc->GetName(), addnPair.first);
        res.push_back(actor);
      }
    }
    return RE::BSContainer::ForEachResult::kContinue;
  });
  SKSE::log::debug("Finished checking actors. {} out of {} actors needed update!", res.size(), tot);
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
  auto logDir{SKSE::log::log_directory};
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

std::string Papyrus::WhyProblem(RE::StaticFunctionTag* tag, RE::Actor* actor, int issueID) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return "$TNG_PD9";
  auto down = actor->GetWornArmor(Tng::cSlotGenital);
  auto cover = Events::GetCoveringItem(actor, nullptr);
  switch (issueID) {
    case iidCanSee:
      if (!cover) return "$TNG_PA1";
      if (down) return FormToLocView(down) == Tng::cCover ? "$TNG_PD0" : "$TNG_PA2";
      return WhyProblem(tag, actor, iidCanSeeRep);
    case iidCanSeeRep:
      Events::DoChecks(actor);
      return actor->GetWornArmor(Tng::cSlotGenital) ? "$TNG_PD1" : "$TNG_PD2";
    case iidCannotSee:
      auto skin = npc->skin ? npc->skin : npc->race->skin;
      if (cover) return "$TNG_PA6";
      if (down && FormToLocView(down) != Tng::cCover) return "$TNG_PA2";
      if (!down) {
        auto res = Core::CanModifyNPC(npc);
        switch (res) {
          case Tng::resOkRaceP:
            break;
          case Tng::resOkRaceR:
            return GetErrDscr(tag, Tng::raceErr).c_str();
          case Tng::resOkRacePP:
            if (!Base::ReevaluateRace(npc->race, actor)) {
              return GetErrDscr(tag, Tng::raceErr).c_str();
            }
          default:
            return GetErrDscr(tag, res).c_str();
        }
        if (skin->HasKeyword(Tng::Key(Tng::kyTngSkin))) return "$TNG_PD0";
        if (npc->HasKeyword(Tng::Key(Tng::kyExcluded))) return "$TNG_PA3";
        if (npc->IsFemale() && static_cast<size_t>(std::floor(Tng::WRndGlb()->value + 0.1)) < 100) return "$TNG_PA4";
        if (Base::GetRgAddon(npc->race) == Tng::cNul) return "$TNG_PA5";
      }
      Events::DoChecks(actor);
      return !actor->GetWornArmor(Tng::cSlotGenital) && skin->HasKeyword(Tng::Key(Tng::kyTngSkin)) ? "$TNG_PD1" : "$TNG_PD2";
  }
  return "";
}
