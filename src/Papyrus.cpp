#include <Base.h>
#include <Core.h>
#include <Inis.h>
#include <Papyrus.h>
#include <SEEvents.h>
#include <Util.h>

Papyrus* papyrus = Papyrus::GetSingleton();

bool Papyrus::BindPapyrus(RE::BSScript::IVirtualMachine* vm) {
  vm->RegisterFunction("GetBoolValue", "TNG_PapyrusUtil", GetBoolValue);
  vm->RegisterFunction("SetBoolValue", "TNG_PapyrusUtil", SetBoolValue);
  vm->RegisterFunction("GetIntValue", "TNG_PapyrusUtil", GetIntValue);
  vm->RegisterFunction("SetIntValue", "TNG_PapyrusUtil", SetIntValue);
  vm->RegisterFunction("GetFloatValue", "TNG_PapyrusUtil", GetFloatValue);
  vm->RegisterFunction("SetFloatValue", "TNG_PapyrusUtil", SetFloatValue);

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
  if (0 <= settingID && settingID < Core::boolSettingCount) return core->boolSettings.Get(static_cast<Core::eBoolSetting>(settingID));
  return false;
}

void Papyrus::SetBoolValue(RE::StaticFunctionTag*, int settingID, bool value) {
  if (0 <= settingID && settingID < Core::boolSettingCount) core->SetBoolSetting(static_cast<Core::eBoolSetting>(settingID), value);
}

int Papyrus::GetIntValue(RE::StaticFunctionTag*, int settingID) {
  if (0 <= settingID && settingID < Core::intSettingCount) return core->intSettings.Get(static_cast<Core::eIntSetting>(settingID));
  return Common::nan;
}

void Papyrus::SetIntValue(RE::StaticFunctionTag*, int settingID, int value) {
  if (0 <= settingID && settingID < Core::intSettingCount) core->SetIntSetting(static_cast<Core::eIntSetting>(settingID), value);
}

float Papyrus::GetFloatValue(RE::StaticFunctionTag*, int settingID) {
  if (0 <= settingID && settingID < Core::floatSettingCount) return core->floatSettings.Get(static_cast<Core::eFloatSetting>(settingID));
  return 1.0f;
}

void Papyrus::SetFloatValue(RE::StaticFunctionTag*, int settingID, float value) {
  if (0 <= settingID && settingID < Core::floatSettingCount) core->SetFloatSetting(static_cast<Core::eFloatSetting>(settingID), value);
}

std::vector<std::string> Papyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool isFemale) {
  std::vector<std::string> res{};
  auto count = core->GetAddonCount(isFemale, false);
  for (auto i = 0; i < count; i++) res.push_back(core->AddonByIdx(isFemale, i, false)->GetName());
  return res;
}

bool Papyrus::GetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int addonIdx) {
  if (addonIdx < 0 || addonIdx >= core->GetAddonCount(isFemale, false)) return false;
  return core->GetAddonStatus(isFemale, static_cast<size_t>(addonIdx));
}

void Papyrus::SetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int addonIdx, bool status) {
  if (addonIdx < 0) return;
  core->SetAddonStatus(isFemale, static_cast<size_t>(addonIdx), status);
}

std::vector<std::string> Papyrus::GetRgNames(RE::StaticFunctionTag*) { return core->GetRgNames(true); }

std::string Papyrus::GetRgInfo(RE::StaticFunctionTag*, int rgIdx) { return core->GetRgInfo(rgIdx, true); };

std::vector<std::string> Papyrus::GetRgAddons(RE::StaticFunctionTag*, int rgIdx) {
  std::vector<std::string> res{};
  if (rgIdx < 0) return res;
  auto list = core->GetRgAddonList(static_cast<size_t>(rgIdx), false, true, false);
  res.push_back("$TNG_TRS");
  res.push_back("$TNG_TNT");
  for (auto i : list) {
    auto isDed = core->IsAddonDedicatedToRg(rgIdx, false, true, i);
    std::string name = core->AddonByIdx(false, i, false)->GetName();
    switch (isDed) {
      case Common::resOkMain:
        break;
      case Common::resOkDedicated:
        name = name + " (d)";
        break;
      case Common::resOkSupported:
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
  if (rgIdx < 0) return Common::err40;
  auto rgAddon = core->GetRgAddon(static_cast<size_t>(rgIdx), true);
  if (rgAddon < 0) return rgAddon == Common::def ? 0 : rgAddon;
  auto list = core->GetRgAddonList(static_cast<size_t>(rgIdx), false, true, false);
  auto it = std::find(list.begin(), list.end(), rgAddon);
  return static_cast<int>(it != list.end() ? std::distance(list.begin(), it) : Common::errRg);
}

void Papyrus::SetRgAddon(RE::StaticFunctionTag*, int rgIdx, int choice) {
  if (rgIdx < 0) return;
  if (choice < Common::def) return;
  auto list = core->GetRgAddonList(static_cast<size_t>(rgIdx), false, true, false);
  int addonIdx = choice < 0 ? choice : static_cast<int>(list[choice]);
  core->SetRgAddon(static_cast<size_t>(rgIdx), addonIdx, true);
}

float Papyrus::GetRgMult(RE::StaticFunctionTag*, int rgIdx) {
  if (rgIdx < 0) return -1.0f;
  return core->GetRgMult(static_cast<size_t>(rgIdx), true);
}

void Papyrus::SetRgMult(RE::StaticFunctionTag*, int rgIdx, float mult) { core->SetRgMult(static_cast<size_t>(rgIdx), mult, true); }

int Papyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* actor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  auto res = core->CanModifyNPC(npc);
  switch (res) {
    case Common::resOkRaceP:
      return Common::resOkSizable;
    case Common::resOkRacePP:
      return core->ReevaluateRace(actor->GetRace(), actor) ? Common::resOkSizable : Common::errNPC;
    case Common::resOkRaceR:
      return Common::resOkFixed;
    default:
      return res;
  }
}

std::vector<std::string> Papyrus::GetActorAddons(RE::StaticFunctionTag*, RE::Actor* actor) {
  std::vector<std::string> res{};
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race) return res;
  auto list = core->GetRgAddonList(npc->race, npc->IsFemale(), false);
  auto rgIdx = core->GetRaceRgIdx(npc->race);
  res.push_back("$TNG_TRS");
  res.push_back("$TNG_TNT");
  for (auto i : list) {
    auto isDed = core->IsAddonDedicatedToRg(rgIdx, npc->IsFemale(), false, i);
    std::string name = core->AddonByIdx(npc->IsFemale(), i, false)->GetName();
    switch (isDed) {
      case Common::resOkMain:
        break;
      case Common::resOkDedicated:
        name = name + " (d)";
        break;
      case Common::resOkSupported:
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

RE::TESObjectARMO* Papyrus::GetActorAddon(RE::StaticFunctionTag*, RE::Actor* actor) {}

int Papyrus::SetActorAddon(RE::StaticFunctionTag*, RE::Actor* actor, int choice) {
  auto res = core->SetActorAddon(actor, choice, true);
  if (res >= 0) events->DoChecks(actor);
  return res;
}

int Papyrus::GetActorSize(RE::StaticFunctionTag*, RE::Actor* actor) {
  int sizeCat = Common::nan;
  return core->GetActorSize(actor, sizeCat) < 0 ? Common::nul : sizeCat;
}

int Papyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* actor, int genSize) { return core->SetActorSize(actor, genSize); }

std::vector<std::string> Papyrus::ActorItemsInfo(RE::StaticFunctionTag*, RE::Actor* actor) {
  std::vector<std::string> res{};
  auto wornArmor = core->GetActorWornArmor(actor);
  for (auto& armor : wornArmor) res.push_back(armor->GetName());
  return res;
}

bool Papyrus::SwapRevealing(RE::StaticFunctionTag*, RE::Actor* actor, int choice) {
  auto wornArmor = core->GetActorWornArmor(actor);
  if (choice < 0 || choice > wornArmor.size()) return false;
  auto res = core->SwapRevealing(actor, wornArmor[choice]);
  events->DoChecks(actor);
  return res;
}

std::vector<std::string> Papyrus::GetSlot52Mods(RE::StaticFunctionTag*) { return core->Slot52Mods(); }

bool Papyrus::Slot52ModBehavior(RE::StaticFunctionTag*, std::string modName, int behavior) { return core->Slot52ModBehavior(modName, behavior); }

void Papyrus::UpdateSettings(RE::StaticFunctionTag*) { core->SaveGlobals(); }

int Papyrus::UpdateLogLvl(RE::StaticFunctionTag*, int logLevel) {
  core->SetLogLvl(logLevel < 0 ? logLevel : logLevel + spdlog::level::debug);
  return int(core->GetLogLvl()) - int(spdlog::level::debug);
}

std::string Papyrus::ShowLogLocation(RE::StaticFunctionTag*) {
  auto logDir{SKSE::log::log_directory};
  std::filesystem::path path = logDir().value_or("$TNG_LDN");
  return path.string();
}

std::string Papyrus::GetErrDscr(RE::StaticFunctionTag*, int errCode) {
  switch (errCode) {
    case Common::err40:
      return "$TNG_WN9";
    case Common::errRg:
      return "$TNG_WN8";
    case Common::errSkeleton:
      return "$TNG_WN7";
    case Common::errPlayer:
      return "$TNG_WN6";
    case Common::errSkin:
      return "$TNG_WN5";
    case Common::errArmo:
      return "$TNG_WN4";
    case Common::errAddon:
      return "$TNG_WN3";
    case Common::errNPC:
      return "$TNG_WN2";
    case Common::errRace:
      return "$TNG_WN1";
    default:
      return "$TNG_WN9";
  }
}

std::string Papyrus::WhyProblem(RE::StaticFunctionTag* tag, RE::Actor* actor, int issueID) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return "$TNG_PD9";
  auto down = actor->GetWornArmor(Common::genitalSlot);
  auto cover = events->GetCoveringItem(actor, nullptr);
  switch (issueID) {
    case iidCanSee:
      if (!cover) return "$TNG_PA1";
      if (down) return ut->IsBlock(down) ? "$TNG_PD0" : "$TNG_PA2";
      return WhyProblem(tag, actor, iidCanSeeRep);
    case iidCanSeeRep:
      events->DoChecks(actor);
      return actor->GetWornArmor(Common::genitalSlot) ? "$TNG_PD1" : "$TNG_PD2";
    case iidCannotSee:
      auto skin = npc->skin ? npc->skin : npc->race->skin;
      if (cover) return "$TNG_PA6";
      if (down && !ut->IsBlock(down)) return "$TNG_PA2";
      if (!down) {
        auto res = core->CanModifyNPC(npc);
        switch (res) {
          case Common::resOkRaceP:
            break;
          case Common::resOkRaceR:
            return GetErrDscr(tag, Common::errRace).c_str();
          case Common::resOkRacePP:
            if (!core->ReevaluateRace(npc->race, actor)) {
              return GetErrDscr(tag, Common::errRace).c_str();
            }
          default:
            return GetErrDscr(tag, res).c_str();
        }
        if (skin->HasKeyword(ut->Key(Common::kyTngSkin))) return "$TNG_PD0";
        if (npc->HasKeyword(ut->Key(Common::kyExcluded))) return "$TNG_PA3";
        if (npc->IsFemale() && static_cast<size_t>(std::floor(core->GetFloatSetting(Common::fsWomenChance) + 0.1f)) < 100) return "$TNG_PA4";
        if (core->GetRgAddon(npc->race) == Common::nul) return "$TNG_PA5";
      }
      events->DoChecks(actor);
      return !actor->GetWornArmor(Common::genitalSlot) && skin->HasKeyword(ut->Key(Common::kyTngSkin)) ? "$TNG_PD1" : "$TNG_PD2";
  }
  return "";
}
