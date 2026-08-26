#include <Core.h>
using namespace TNG;

Core* core = Core::GetSingleton();

void Core::Process() {
  LoadTngInis();
  LoadMainIni();
  LoadAddons();
  ProcessRaces();
  ProcessNPCs();
  CheckArmorPieces();
}

RaceGroupInfo* const Core::Rg(const RgKey& ky) {
  if (ky.race) {
    for (auto& rg : rgInfoList)
      if (std::find(rg.races.begin(), rg.races.end(), ky.race) != rg.races.end()) {
        return &rg;
      }
    return nullptr;
  } else if (ky.index >= 0 && ky.index < rgInfoList.size()) {
    if (ky.onlyMCM && !boolSettings.Get(bsShowAllRaces)) {
      int curr = -1;
      for (auto& rg : rgInfoList) {
        if (!rg.noMCM) curr++;
        if (curr == ky.index) return &rg;
      }
    } else {
      return &rgInfoList[static_cast<size_t>(ky.index)];
    }
  }
  return nullptr;
}

const RaceGroupInfo* const Core::Rg(const RgKey& ky) const {
  if (ky.race) {
    for (auto& rg : rgInfoList)
      if (std::find(rg.races.begin(), rg.races.end(), ky.race) != rg.races.end()) {
        return &rg;
      }
    return nullptr;
  } else if (ky.index >= 0 && ky.index < rgInfoList.size()) {
    if (ky.onlyMCM && !boolSettings.Get(bsShowAllRaces)) {
      int curr = -1;
      for (auto& rg : rgInfoList) {
        if (!rg.noMCM) curr++;
        if (curr == ky.index) return &rg;
      }
    } else {
      return &rgInfoList[static_cast<size_t>(ky.index)];
    }
  }
  return nullptr;
}

const bool TNG::Core::RgIsMain(RgKey rgChoice) const { return Rg(rgChoice) && Rg(rgChoice)->isMain; }

const int TNG::Core::GetRgAddon(RgKey rgChoice) const { return Rg(rgChoice) ? Rg(rgChoice)->addonIdx : err40; }

void Core::SetRgAddon(RgKey rgChoice, const int addonIdx) {
  auto rg = Rg(rgChoice);
  if (!rg || addonIdx < def || (addonIdx >= 0 && rg->malAddons.find(addonIdx) == rg->malAddons.end())) return;
  rg->addonIdx = (addonIdx == def) ? rg->defAddonIdx : addonIdx;
  auto addon = addonIdx < 0 ? nullptr : rg->malAddons[static_cast<size_t>(addonIdx)];
  StoreRgAddon(rg->races[0], addon, addonIdx);
}

const float Core::GetRgMult(RgKey rgChoice) const { return Rg(rgChoice) ? Rg(rgChoice)->mult : errFlt; }

void Core::SetRgMult(RgKey rgChoice, const float mult) {
  auto rg = Rg(rgChoice);
  if (!rg || mult < 0.1f || mult >= 10.0f) {
    SKSE::log::critical("\tFailure in setting a race mult!");
    return;
  }
  rg->mult = mult;
  StoreRgMult(rg->races[0], mult);
}

const std::string Core::GetRgInfo(RgKey rgChoice) const {
  std::string res{""};
  auto rg = Rg(rgChoice);
  if (!rg) return res;
  auto& list = rg->races;
  res = "Main race: [" + rg->file + " : " + rg->name + "]; ";
  res = res + "Armor race: [" + EI(rg->armorRace) + "]; ";
  res = res + "Additional races: [";
  for (auto race : list) {
    if (race == list.front()) continue;
    res = res + F0(race) + " : " + EI(race);
    if (race != list.back()) {
      res = res + ", ";
    }
  }
  res = res + "]";
  return res;
}

const std::vector<size_t> Core::GetRgAddons(RgKey rgChoice) const {
  std::vector<size_t> res{};
  auto rg = Rg(rgChoice);
  if (!rg) return res;
  for (auto& addonPair : rg->malAddons) res.push_back(addonPair.first);
  return res;
}

const eRes Core::ReevaluateRace(RE::TESRace* const race, RE::Actor* const actor) {
  if (!actor || !actor->Is3DLoaded() || !race || !Rg(RgKey(race))) return resOkRacePP;
  auto rg = Rg(RgKey(race));
  bool isValid = true;
  for (auto& boneName : genBoneNames)
    if (!actor->GetNodeByName(boneName)) {
      isValid = false;
      SKSE::log::info("\tTNG would neglect the race [0x{:x}:{}] since its skeleton is missing the bone [{}]", IG(race), EI(race), boneName);
      break;
    }
  if (isValid) {
    if (race->skeletonModels[0].model.empty() || race->skeletonModels[1].model.empty()) {
      SKSE::log::info("\tTNG would neglect the race [0x{:x}:{}] since its skeleton could not be recognized.", IG(race), EI(race));
      isValid = false;
    }
  }
  if (isValid) {
    StoreSkeleton(race->skeletonModels[0].model.data());
    StoreSkeleton(race->skeletonModels[1].model.data());
    SKSE::log::debug("\tThe race [0x{:x}:{}] can have TNG addons!", IG(race), EI(race));
    race->RemoveKeyword(ut->Key(kyPreProcessed));
    race->AddKeyword(ut->Key(kyProcessed));
  } else {
    race->RemoveKeyword(ut->Key(kyPreProcessed));
    race->AddKeyword(ut->Key(kyIgnored));
    race->RemoveSlotFromMask(genitalSlot);
    auto it = std::find(rg->races.begin(), rg->races.end(), race);
    rg->races.erase(it);
    auto rgIt = std::ranges::find_if(rgInfoList, [&](const auto& item) { return &item == rg; });
    if (rgIt != rgInfoList.end() && rg->races.empty()) rgInfoList.erase(rgIt);
  }
  return isValid ? resOkRaceP : errRace;
}

const eRes Core::CanModifyActor(RE::Actor* const actor) const {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return errNPC;
  if (npc->HasKeyword(ut->Key(kyExcluded))) return errNPC;
  if (!npc->race) return errRace;
  if (auto skin = npc->skin; skin && skin->HasPartOf(genitalSlot)) {
    for (auto& aa : skin->armorAddons)
      if (aa && aa->HasPartOf(genitalSlot)) return resOkRaceR;
    return errSkin;
  }
  if (npc->race->HasKeyword(ut->Key(kyReady))) return resOkRaceR;
  if (auto rg = Rg(RgKey(npc->race)); !rg || (npc->IsFemale() ? rg->femAddons.size() == 0 : rg->malAddons.size() == 0)) return errRace;
  if (npc->race->HasKeyword(ut->Key(kyProcessed))) return resOkRaceP;
  if (npc->race->HasKeyword(ut->Key(kyPreProcessed))) return resOkRacePP;
  return errRace;
}

void Core::UpdateActor(RE::Actor* const actor, RE::TESObjectARMO* const armor, const bool isEquipped) {
  std::unique_lock<std::mutex> lock(actorLock[actor]);
  if (actorProcessing.find(actor) == actorProcessing.end()) actorProcessing[actor] = false;
  actorCv.wait(lock, [this, actor]() { return !actorProcessing[actor]; });
  actorProcessing[actor] = true;
  try {
    DoUpdateActor(actor, armor, isEquipped);
  } catch (...) {
    SKSE::log::error("Exception caught in Core::UpdateActor for actor [0x{:x}:{}]!", IG(actor), EI(actor));
  }
  actorProcessing[actor] = false;
  actorCv.notify_all();
}

const std::vector<size_t> Core::GetActorAddons(RE::Actor* const actor, const bool onlyActive) const {
  std::vector<size_t> res{};
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race) return res;
  if (auto rg = Rg(RgKey(npc->race)); rg) {
    auto& list = npc->IsFemale() ? rg->femAddons : rg->malAddons;
    auto& master = npc->IsFemale() ? allFemAddons : allMalAddons;
    for (auto& addonPair : list) {
      if (!onlyActive || master[addonPair.first].isActive) res.push_back(addonPair.first);
    }
  }
  return res;
}

const eRes Core::GetActorAddon(RE::Actor* actor, int& addonIdx, bool& isAuto) const {
  isAuto = true;
  addonIdx = nul;
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return errNPC;
  if (IsNPCExcluded(npc) || npc->HasKeyword(ut->Key(kyExcluded)) || npc->HasKeyword(ut->Key(kyIgnored))) {
    isAuto = false;
    return resOkNoAddon;
  }
  auto rg = Rg(RgKey(npc->race));
  if (!rg) return errRace;
  npc->ForEachKeyword([&](RE::BGSKeyword* kw) {
    if (!kw || kw->GetFormEditorID() == NULL) return RE::BSContainer::ForEachResult::kContinue;
    const std::string kwStr{EI(kw)};
    std::string str = "";
    if (kwStr.starts_with(cNPCUserAddon)) {
      isAuto = false;
      addonIdx = std::stoi(kwStr.substr(size(cNPCUserAddon), 2));
      return RE::BSContainer::ForEachResult::kStop;
    }
    return RE::BSContainer::ForEachResult::kContinue;
  });
  if (!isAuto) return resOkHasAddon;
  auto savedAddon = ActorSavedAddon(actor, npc);
  auto list = GetActorAddons(actor, true);
  if (!savedAddon.second.empty()) {
    addonIdx = savedAddon.second == nulStr ? nul : AddonIdxByLoc(npc->IsFemale(), savedAddon);
    if (addonIdx >= 0 && std::find(list.begin(), list.end(), static_cast<size_t>(addonIdx)) != list.end()) {
      isAuto = false;
      return resOkHasAddon;
    } else if (addonIdx == nul) {
      isAuto = false;
      return resOkNoAddon;
    }
  }
  addonIdx = npc->IsFemale() ? nul : rg->defAddonIdx;
  const auto count = list.size();
  const auto malChance = static_cast<size_t>(std::floor(boolSettings.Get(bsRandomizeMaleAddon) * floatSettings.Get(fsMalRndChance) + 0.1f));
  const auto femChance = static_cast<size_t>(std::floor(floatSettings.Get(fsFemRndChance) + 0.1f));
  const size_t chance = static_cast<size_t>(npc->IsFemale() ? femChance : malChance);
  if (count > 0 && chance > 0 && (IG(npc) % 100 < chance)) addonIdx = static_cast<int>(list[IG(npc) % count]);
  return (addonIdx >= 0) ? resOkHasAddon : resOkNoAddon;
}

const eRes Core::SetActorAddon(RE::Actor* const actor, const int choice, const bool isUser, const bool shouldSave) {
  std::lock_guard<std::mutex> lock(actorLock[actor]);
  // TODO: Revise
  //  if (shouldSave) {
  //    if (CanModifyActor(actor) == resOkRacePP) ReevaluateRace(actor->GetRace(), actor);
  //    if (auto res = CanModifyActor(actor); res != resOkRaceP) {
  //      if (res >= 0) res = errRace;
  //      return res;
  //    }
  //  }
  //  const auto npc = actor->GetActorBase();
  //  auto list = GetActorAddons(actor, !isUser);
  //  if (shouldSave) {
  //    if (choice >= static_cast<int>(list.size())) return errAddon;
  //    SKSE::log::debug("Setting addon [{}] for actor [0x{:x}:{}].", choice, IG(actor), npc->GetName());
  //  }
  //  auto addonIdx = choice < 0 ? choice : shouldSave ? static_cast<int>(list[choice].first) : choice;
  //  auto oldSkin = npc->skin;
  //  auto res = SetNPCAddon(npc, addonIdx, isUser);
  //  if (res < 0) return res;
  //  auto addon = addonIdx < 0 ? nullptr : (npc->IsFemale() ? femAddons[addonIdx].first : malAddons[addonIdx].first);
  //  if (actor->IsPlayerRef() && shouldSave)
  //    StorePlayerInfo(actor, addon, addonIdx);
  //  else if (!npc->IsPlayer() && shouldSave)
  //    Inis::StoreActorAddon(actor, npc, addon, addonIdx);
  //  if (shouldSave || (!isUser && npc->skin != oldSkin)) {
  //    UpdateFormLists(actor);
  //  }
  //  return res;
}

const eRes Core::GetActorSize(RE::Actor* const actor, int& sizeCat) const {
  sizeCat = errInt;
  if (auto res = CanModifyActor(actor); res < 0) return res;
  const auto npc = actor->GetActorBase();
  if (npc->IsPlayer()) {
    if (boolSettings.Get(bsExcludePlayerSize)) return errPlayer;
    auto pcInfo = PlayerInfo(actor);
    if (pcInfo) {
      sizeCat = pcInfo->sizeCat;
      return resOkSizable;
    } else {
      sizeCat = nul;
      return resOkSizable;
    }
  }
  if (sizeCat = Inis::ActorSavedSize(actor, npc); sizeCat >= 0) return resOkSizable;
  sizeCat = HasKeywordInList(npc, ut->SizeKeys());
  if (sizeCat < 0) sizeCat = npc->formID % sizeCatCount;
  return resOkSizable;
}

const eRes Core::SetActorSize(RE::Actor* const actor, int sizeCat, const bool shouldSave) {
  std::lock_guard<std::mutex> lock(actorLock[actor]);
  if (shouldSave) {
    if (CanModifyActor(actor) == resOkRacePP) ReevaluateRace(actor->GetRace(), actor);
    if (auto res = CanModifyActor(actor); res < 0) return res;
  }
  const auto npc = actor->GetActorBase();
  if (!actor->IsPlayerRef() && npc->IsPlayer()) return resOkFixed;  // Don't change the size for copies of player actor
  if (sizeCat == def) npc->RemoveKeywords(ut->SizeKeys());
  int currCat = errInt;
  auto res = GetActorSize(actor, currCat);
  if (res != resOkSizable || currCat < 0) return res;
  auto cat = (sizeCat < 0) ? currCat : sizeCat;
  if (cat != currCat) {
    npc->RemoveKeywords(ut->SizeKeys());
    npc->AddKeyword(ut->SizeKey(cat));
  }
  auto rg = Rg(RgKey(npc->race));
  if (!rg) return errRg;
  auto mult = rg->mult;
  if (mult < 0.0f) return errRg;
  auto scale = mult * floatSettings.Get(static_cast<eFloatSetting>(cat));
  if (scale < 0.1) scale = 1;
  const auto isPlayer = actor->IsPlayerRef();
  const std::string failMessage = fmt::format("Failed to scale actor [0x{:x}] genitalia to [{}].", IG(actor), scale);
  DoDelayed(
      [actor, isPlayer, scale, shouldSave, sizeCat, failMessage]() {
        auto ac = isPlayer ? RE::PlayerCharacter::GetSingleton() : actor;
        if (!ac) return;
        RE::NiAVObject* baseNode = ac->GetNodeByName(genBoneNames[egbBase]);
        RE::NiAVObject* scrotNode = ac->GetNodeByName(genBoneNames[egbScrot]);
        if (baseNode && scrotNode) {
          if (baseNode->local.scale == scale) return;
          baseNode->local.scale = scale;
          scrotNode->local.scale = 1.0f / sqrt(scale);
          if (sizeCat != nul) SKSE::log::debug("Actor [0x{:x}] genitalia {} to [{}].", IG(ac), shouldSave ? "scaled" : "restored", scale);
        } else {
          SKSE::log::debug(failMessage);
        }
      },
      [actor]() -> bool { return actor && actor->Is3DLoaded() && actor->GetNodeByName(genBoneNames[egbBase]); }, 0, true, failMessage);
  if (shouldSave && sizeCat != nul) actor->IsPlayerRef() ? StorePlayerInfo(actor, nullptr, errInt, sizeCat) : StoreActorSize(actor, npc, sizeCat);
  return res;
}

const bool Core::SwapRevealing(RE::Actor* const actor, RE::TESObjectARMO* const armor) {
  std::lock_guard<std::mutex> lock(armorLock[armor]);
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (armor->HasKeyword(ut->Key(kyUnderwear))) return false;
  std::vector<RE::BGSKeyword*> rcKeys = {ut->Key(statusKeys[0]), ut->Key(statusKeys[1]), ut->Key(statusKeys[2]), ut->Key(statusKeys[3])};
  auto kb = HasKeywordInList(armor, rcKeys);
  int mask = npc->IsFemale() ? 2 : 1;
  if (kb < 0) {
    armor->RemoveKeyword(ut->Key(kyIgnored));
    armor->AddKeyword(ut->Key(kyCovering));
    StoreArmorStatus(armor, kyCovering);
  } else {
    armor->RemoveKeyword(rcKeys[kb]);
    if (armor->HasPartOf(bodySlot)) {
      armor->AddKeyword(rcKeys[kb ^ mask]);
      StoreArmorStatus(armor, statusKeys[kb ^ mask]);
    } else {
      StoreArmorStatus(armor, keywordsCount);
    }
  }
  return true;
}

void Core::RevisitRevealingArmor() const {
  auto s52 = Slot52Mods();
  if (s52.size() == 0) return;
  auto& armorList = SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  std::vector<RE::BGSKeyword*> rc = {ut->Key(kyCovering), ut->Key(kyRevealing)};
  for (const auto& armor : armorList) {
    if (!armor || !armor->HasPartOf(bodySlot) || !armor->HasKeywordInArray(rc, false)) continue;
    auto modName = F0(armor, false);
    if (modName.empty() || std::ranges::find(s52, modName) == s52.end()) continue;
    if (auto b = Slot52ModBehavior(modName); b != armor->HasKeyword(ut->Key(kyRevealing))) {
      armor->RemoveKeywords(rc);
      armor->AddKeyword(ut->Key(b ? kyRevealing : kyCovering));
    }
  }
}

void Core::LoadAddons() {
  SKSE::log::info("Loading the addons...");
  allMalAddons.clear();
  allFemAddons.clear();
  const auto& armorList = SEDH()->GetFormArray<RE::TESObjectARMO>();
  for (const auto& armor : armorList) {
    if (!armor || !armor->GetName()) continue;
    if (armor->HasKeyword(ut->Key(kyAddonM))) {
      if (!armor->HasKeyword(ut->Key(kyIgnored))) armor->AddKeyword(ut->Key(kyIgnored));
      auto addonLoc = FormToLoc(armor);
      if (auto it = std::ranges::find_if(allMalAddons, [&](const auto& item) { return item.name == armor->GetName(); }); it == allMalAddons.end()) {
        allMalAddons.push_back({armor->GetName(), {armor}, userMalAddons.find(addonLoc) == userMalAddons.end(), armor->HasKeyword(ut->Key(kyGenderChanger))});
      } else {
        it->records.push_back(armor);
        it->isActive = it->isActive && (userMalAddons.find(addonLoc) == userMalAddons.end());
        if (armor->HasKeyword(ut->Key(kyGenderChanger)) != it->isGenderChanger)
          SKSE::log::error("Inconsistent keywords were found in addon [{}] between [0x{:x}] and [0x{:x}] regarding gender specification. This would cause problems in animations!",
                           it->name, IG(it->records[0]), IG(armor));
      }
    }
    if (armor->HasKeyword(ut->Key(kyAddonF))) {
      if (!armor->HasKeyword(ut->Key(kyIgnored))) armor->AddKeyword(ut->Key(kyIgnored));
      auto addonLoc = FormToLoc(armor);
      if (auto it = std::ranges::find_if(allFemAddons, [&](const auto& item) { return item.name == armor->GetName(); }); it == allFemAddons.end()) {
        allFemAddons.push_back({armor->GetName(), {armor}, userFemAddons.find(addonLoc) != userFemAddons.end(), armor->HasKeyword(ut->Key(kyGenderChanger))});
      } else {
        it->records.push_back(armor);
        it->isActive = it->isActive && (userFemAddons.find(addonLoc) != userFemAddons.end());
        if (armor->HasKeyword(ut->Key(kyGenderChanger)) != it->isGenderChanger)
          SKSE::log::error("Inconsistent keywords were found in addon [{}] between [0x{:x}] and [0x{:x}] regarding gender specification. This would cause problems in animations!",
                           it->name, IG(it->records[0]), IG(armor));
      }
    }
  }
  SKSE::log::debug("Loaded all addons. Found [{}] addons for men and [{}] addons for women.", allMalAddons.size(), allFemAddons.size());
}

int Core::AddonIdxByLoc(const bool isFemale, const SEFormLocView addonLoc) const {
  if (addonLoc.second.empty()) return errInt;
  if (addonLoc.second == nulStr) return nul;
  if (addonLoc.second == defStr) return def;
  auto& list = isFemale ? allFemAddons : allMalAddons;
  for (int i = 0; i < list.size(); i++) {
    for (auto record : list[i].records) {
      if (FormToLoc(record) == addonLoc) return i;
    }
  }
  return errInt;
}

void Core::ProcessRaces() {
  SKSE::log::info("Processing races...");
  const auto& allRaces = SEDH()->GetFormArray<RE::TESRace>();
  int logInfo[4] = {0, 0, 0, 0};
  std::vector<RE::BGSKeyword*> keywords = {ut->Key(kyIgnored), ut->Key(kyReady), ut->Key(kyProcessed), ut->Key(kyPreProcessed)};
  StoreSkeleton(ut->Race(raceDefault)->skeletonModels[0].model.data());
  StoreSkeleton(ut->Race(raceDefault)->skeletonModels[1].model.data());
  StoreSkeleton(ut->Race(raceDefBeast)->skeletonModels[0].model.data());
  StoreSkeleton(ut->Race(raceDefBeast)->skeletonModels[1].model.data());
  for (const auto& race : allRaces) {
    if (!race) continue;
    if (!race->HasKeywordInArray(keywords, false)) ProcessRace(race);
    auto k = HasKeywordInList(race, keywords);
    k >= 0 ? logInfo[k]++ : logInfo[0]++;
  }
  for (auto& rg : rgInfoList) {
    ProcessRgAddons(rg, false);
    ProcessRgAddons(rg, true);
    ApplyUserSettings(rg);
  }
  SKSE::log::info("Processed [{}] races: assigned genitalia to [{}] races, preprocessed [{}] races, found [{}] races to be ready and ignored [{}] races.", allRaces.size(),
                  logInfo[2], logInfo[3], logInfo[1], logInfo[0]);
}

RaceGroupInfo* const Core::ProcessRace(RE::TESRace* const race) {
  if (auto rg = Rg(RgKey(race)); rg) return rg;
  RaceGroupInfo* res = nullptr;
  SKSE::log::debug("\tProcessing race [0x{:x}: {}] ...", IG(race), EI(race));
  switch (CheckRace(race)) {
    case resOkRacePP:
      res = AddRace(race, false);
      break;
    case resOkRaceP:
      res = AddRace(race, true);
      break;
    case resOkRaceR:
      IgnoreRace(race, true);
      break;
    case errRace:
      IgnoreRace(race, false);
      break;
    default:
      break;
  }
  if (res) SKSE::log::info("\tRace [0x{:x}: {}]: member of {} group {}.", IG(race), EI(race), res->isMain ? "primary" : "secondary", res->name);
  return res;
}

eRes Core::CheckRace(RE::TESRace* const race) const {
  try {
    for (auto raceInfo : hardCodedRaces)
      if (FormToLoc(race) == raceInfo) return resOkRaceP;
    if (race->HasKeyword(ut->Key(kyCreature)) || race->IsChildRace() || !race->HasKeyword(ut->Key(kyManMer))) return errRace;
    auto raceID = EI(race);
    std::transform(raceID.begin(), raceID.end(), raceID.begin(), ::tolower);
    if (raceID.contains("child")) {
      SKSE::log::info("\tThe race [{}: xx{:x}: {}] was ignored because its editorID contains 'child'!", F0(race), I0(race), EI(race));
      return errRace;
    }
    if (IsRaceExcluded(race)) {
      SKSE::log::info("\tThe race [{}: xx{:x}: {}] was ignored because an ini excludes it!", F0(race), I0(race), EI(race));
      return errRace;
    }
    if (!race->skin) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since they do not have a skin! It was last modified by [{}].", IG(race), EI(race), FL(race));
      return errRace;
    }
    bool skinFound = false;
    for (const auto& aa : race->skin->armorAddons) {
      if (aa->HasPartOf(bodySlot) && aa->IsValidRace(race)) {
        skinFound = true;
        break;
      }
    }
    if (!skinFound) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since their skin cannot be recognized!  It was last modified by [{}].", IG(race), EI(race), FL(race));
      return errRace;
    }
    bool isVanilla = F0(race) == skyrimFile;
    if (race->HasPartOf(genitalSlot) && !isVanilla) {
      auto ready = race->skin->HasPartOf(genitalSlot);
      SKSE::log::info("\tThe race [{}] is designed to be {} TNG. It was not modified.", EI(race), ready ? "ready for" : "ignored by");
      return ready ? resOkRaceR : errRace;
    }
  } catch (const std::exception& er) {
    auto msg = fmt::format("The race [0x{:x}: {}] caused an error [{}]. TNG tries to ignore it but it might cause other problems!", IG(race), EI(race), er.what());
    SKSE::log::warn("\t{}", msg);
    ShowSkyrimMessage(msg.c_str());
    return errRace;
  }
  bool isValidSk =
      validSkeletons.find(race->skeletonModels[0].model.data()) != validSkeletons.end() && validSkeletons.find(race->skeletonModels[1].model.data()) != validSkeletons.end();
  return isValidSk ? resOkRaceP : resOkRacePP;
}

RaceGroupInfo* Core::AddRace(RE::TESRace* const race, const bool isValidSkeleton) {
  race->AddKeyword(ut->Key(isValidSkeleton ? kyProcessed : kyPreProcessed));
  race->AddSlotToMask(genitalSlot);
  race->skin->AddKeyword(ut->Key(kyIgnored));
  auto pRace = race;
  while (pRace->armorParentRace && pRace->armorParentRace != ut->Race(raceDefault)) {
    pRace = pRace->armorParentRace;
  };
  if (pRace != race) {
    for (auto& rg : rgInfoList) {
      if (rg.armorRace == pRace && rg.races[0]->skin == race->skin) {
        rg.races.push_back(race);
        return &rg;
      }
    }
    if (pRace->skin == race->skin) {
      SKSE::log::debug("\t\t...jumping to parent race...");
      if (auto parentRg = ProcessRace(pRace); parentRg) {
        parentRg->races.push_back(race);
        return parentRg;
      }
      SKSE::log::debug("\t\t...parent race [0x{:x}: {}] is not a valid race.", IG(pRace), EI(pRace));
    }
  }
  rgInfoList.push_back({});
  auto& rg = rgInfoList.back();
  rg.name = EI(race);
  rg.file = F0(race);
  rg.armorRace = pRace;
  rg.isMain = pRace == race;
  rg.races.push_back(race);
  rg.noMCM = !race->GetPlayable() && !race->HasKeyword(ut->Key(kyVampire));
  rg.mult = 1.0f;
  rg.defAddonIdx = GetRgDefAddon(rg);
  return &rg;
}

void Core::IgnoreRace(RE::TESRace* const race, const bool ready) {
  if (!race) return;
  race->RemoveKeywords(ut->Keys(kyProcessed, kyExcluded));
  race->AddKeyword(ut->Key(ready ? kyReady : kyIgnored));
}

int Core::GetRgDefAddon(RaceGroupInfo& rg) {
  if (rg.defAddonIdx != errInt) return rg.defAddonIdx;
  SEFormLoc defAddon;
  bool defAddonSet = false;
  auto raceStr = EI(rg.armorRace) + std::string(rg.armorRace->GetName());
  std::transform(raceStr.begin(), raceStr.end(), raceStr.begin(), [](uint8_t c) { return static_cast<char>(std::tolower(c)); });
  auto rgIsBeast = rg.armorRace->HasKeyword(ut->Key(kyBeast));
  std::ranges::for_each(cVanillaDefaults, [&](const auto& tup) {
    auto [s1, s2, isBeast, addon] = tup;
    if ((raceStr.contains(s1) || raceStr.contains(s2)) && (rgIsBeast == isBeast)) {
      defAddon = std::make_pair(addon, mainFile);
      defAddonSet = true;
    };
  });
  if (!defAddonSet) defAddon = std::make_pair(std::get<3>(cVanillaDefaults[rgIsBeast ? 9 : 0]), mainFile);
  if (auto idx = AddonIdxByLoc(false, defAddon); idx >= 0) {
    return idx;
  }
  SKSE::log::critical("TNG faced an error getting the default addon for a race group.");
  return nul;
}

void Core::ProcessRgAddons(RaceGroupInfo& rg, const bool isFemale) {
  auto& rgAddons = isFemale ? rg.femAddons : rg.malAddons;
  auto& addons = isFemale ? allFemAddons : allMalAddons;
  bool fullySupports = false;
  for (size_t i = 0; i < addons.size(); i++) {
    for (const auto& record : addons[i].records) {
      if (rgAddons.contains(i) && fullySupports) break;
      if (rg.isMain && record->HasKeyword(ut->Key(kyAddonSec))) continue;
      if (rg.isMain) {
        for (const auto& aa : record->armorAddons) {
          if (aa && aa->IsValidRace(rg.armorRace)) {
            rgAddons.insert_or_assign(i, record);
            fullySupports = true;
            break;
          }
        }
        continue;
      }
      if (record->HasKeyword(ut->Key(kyAddonSec))) {
        std::set<RE::TESRace*> addonRaces;
        for (const auto& aa : record->armorAddons) {
          addonRaces.insert(aa->race);
          addonRaces.insert(aa->additionalRaces.begin(), aa->additionalRaces.end());
        }
        for (const auto& race : rg.races) {
          if (addonRaces.contains(race)) {
            rgAddons.insert_or_assign(i, record);
            fullySupports = true;
            break;
          }
        }
        continue;
      }
      for (const auto& aa : record->armorAddons) {
        if (aa && aa->IsValidRace(rg.armorRace)) {
          rgAddons.insert_or_assign(i, record);
          break;
        }
      }
    }
  }
  if (isFemale) return;
  if (rg.defAddonIdx < 0 || !rgAddons.contains(rg.defAddonIdx)) rg.defAddonIdx = rgAddons.empty() ? nul : static_cast<int>(rgAddons.begin()->first);
}

void Core::ApplyUserSettings(RaceGroupInfo& rg) {
  if (auto rgLoc = FormToLoc(rg.races[0]); !rgLoc.second.empty()) {
    if (userRacialAddons.find(rgLoc) != userRacialAddons.end()) {
      auto& addonLoc = userRacialAddons[rgLoc];
      auto index = AddonIdxByLoc(false, addonLoc);
      if (index >= 0 && rg.malAddons.find(static_cast<size_t>(index)) != rg.malAddons.end()) {
        rg.addonIdx = index;
        SKSE::log::debug("\tRestored group [{}] addon to [xx{:x}] from file [{}]!", rg.name, addonLoc.first, addonLoc.second);
      } else if (index == nul) {
        rg.addonIdx = nul;
        SKSE::log::debug("\tRestored group [{}] addon to be empty!", rg.name);
      } else {
        SKSE::log::debug("\tThe addon [xx{:x}] from file [{}] could not be used for group [{}]!", addonLoc.first, addonLoc.second, rg.name);
      }
    }
    if (userRacialSizes.find(rgLoc) != userRacialSizes.end()) {
      auto size = userRacialSizes[rgLoc];
      if (size > 0.01f && size < 10.0f) {
        rg.mult = size;
        SKSE::log::debug("\tRestored group [{}] size to [{}]!", rg.name, size);
      } else {
        SKSE::log::debug("\tThe size [{}] is out of range for group [{}]!", size, rg.name);
      }
    }
  }
}

void Core::ProcessNPCs() {
  // TODO: Re-implement
  //  SKSE::log::info("Checking NPCs for custom skins...");
  //  std::map<std::pair<RE::TESObjectARMO*, RE::TESRace*>, std::set<RE::TESNPC*>> skinsToPatch{};
  //  std::map<std::string_view, size_t> customSkinMods{};
  //  std::map<RE::TESRace*, size_t> raceNPCCount;
  //  auto& allNPCs = SEDH()->GetFormArray<RE::TESNPC>();
  //  size_t sizeCount[sizeCatCount]{0};
  //  for (const auto& npc : allNPCs) {
  //    if (!npc) continue;
  //    if (IsNPCExcluded(npc)) {
  //      ExcludeNPC(npc);
  //      continue;
  //    }
  //    const auto race = npc->race;
  //    if (!race) {
  //      SKSE::log::warn("\tThe NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", IG(npc), EI(npc));
  //      continue;
  //    }
  //    if (!race->HasKeyword(ut->Key(kyProcessed)) && !race->HasKeyword(ut->Key(kyPreProcessed))) continue;
  //    raceNPCCount[npc->race]++;
  //    const auto skin = npc->skin;
  //    if (npc->IsFemale()) continue;
  //    sizeCount[npc->formID % sizeCatCount]++;
  //    if (!skin) continue;
  //    if (skin->HasKeyword(ut->Key(kyTngSkin))) continue;
  //    skinsToPatch[{skin, race}].insert(npc);
  //  }
  //  for (auto& racePair : raceNPCCount) {
  //    if (!racePair.first || racePair.second < 5) continue;
  //    if (auto rg = Rg(RgKey(racePair.first)); rg && rg->malAddons.size() > 0) rg->noMCM = false;
  //  }
  //  for (auto& skinPair : skinsToPatch) {
  //    auto& oldSkin = skinPair.first.first;
  //    auto& race = skinPair.first.second;
  //    auto newSkin = FixSkin(oldSkin, race, nullptr);
  //    if (!newSkin) continue;
  //    customSkinMods[F0(oldSkin)]++;
  //    for (auto& npc : skinPair.second) npc->skin = newSkin;
  //  }
  //  if (skinsToPatch.size() > 0) {
  //    auto tot = skinsToPatch.size();
  //    SKSE::log::debug("\tHandled {} custom skin-race combinations from following mod(s):", skinsToPatch.size());
  //    for (const auto& entry : customSkinMods) {
  //      SKSE::log::debug("\t\t[{}] skins from {}", entry.second, entry.first);
  //      tot -= entry.second;
  //    }
  //    if (tot > 0) SKSE::log::debug("\t\t[{}] skins were not patched.", tot);
  //  }
  //  SKSE::log::info("Finished checking NPCs for custom skins.");
  //  SKSE::log::debug("TNG distributed the sizes (from smallest to largest) to [{}] NPCs!", fmt::join(sizeCount, ", "));
  //  SKSE::log::info("\nTNG would not show the following race groups in the MCM since there are either no genital available to them or there are very few NPCs which use them:");
  //  for (size_t i = 1; i < rgInfoList.size(); i++)
  //    if (rgInfoList[i].noMCM) SKSE::log::info("\tRace group [{}] with the following information: {}.", rgInfoList[i].name, GetRgInfo(RgKey(i, false)));
}

void TNG::Core::ExcludeNPC(RE::TESNPC* const npc) { npc->AddKeyword(ut->Key(kyExcluded)); }

eRes Core::SetNPCAddon(RE::TESNPC* const npc, const int addonIdx, const bool isUser) {
  // TODO: Re-implement
  //   if (addonIdx < def) return errAddon;
  //   if (!npc || !npc->race || !npc->race->skin) {
  //     SKSE::log::critical("Failure in setting a NPC genital!");
  //     return errNPC;
  //   }
  //   auto res = (npc->IsPlayer() && boolSettings.Get(bsExcludePlayerSize)) || addonIdx == nul ? resOkFixed : resOkSizable;
  //   npc->RemoveKeyword(ut->Key(kyIgnored));
  //   if (addonIdx == def && !npc->skin) return !npc->IsFemale() ? res : resOkFixed;
  //   auto& npcSkin = npc->skin;
  //   auto& raceSkin = npc->race->skin;
  //   auto& activeSkin = npcSkin ? npcSkin : raceSkin;
  //   bool skinHasRace = false;
  //   for (auto& aa : activeSkin->armorAddons) {
  //     if (aa->IsValidRace(npc->race) && aa->HasPartOf(bodySlot)) {
  //       skinHasRace = true;
  //       break;
  //     }
  //   }
  //   if (!skinHasRace) activeSkin = npc->race->skin;
  //   auto activeOgSkin = GetOgSkin(activeSkin);
  //   if (activeOgSkin && activeOgSkin->HasKeyword(ut->Key(kyCovering))) {
  //     activeOgSkin->RemoveKeyword(ut->Key(kyCovering));
  //     activeOgSkin->AddKeyword(ut->Key(kyIgnored));
  //     SKSE::log::info("The skin [0x{:x}] was updated accordingly", activeOgSkin->formID);
  //   }
  //   auto rg = Rg(RgKey(npc->race));
  //   if (!rg) return errRg;
  //   auto& list = npc->IsFemale() ? rg->femAddons : rg->malAddons;
  //   if (addonIdx >= 0 && list.find(addonIdx) == list.end()) return errAddon;
  //   if (addonIdx == def && npc->IsFemale()) {
  //     OrganizeNPCKeywords(npc, addonIdx, false);
  //     if (npcSkin && npcSkin->HasKeyword(ut->Key(kyTngSkin))) npc->skin = activeOgSkin == raceSkin ? nullptr : activeOgSkin;
  //     return resOkFixed;
  //   }
  //   auto addonChoice = (addonIdx == def) ? rg->addonIdx : addonIdx;
  //   OrganizeNPCKeywords(npc, addonIdx, isUser);
  //   auto resSkin = (addonChoice == nul) ? activeOgSkin : GetSkinWithAddonForRg(rg, activeOgSkin, addonChoice, npc->IsFemale());
  //   if (resSkin != npcSkin) {
  //     npc->skin = (resSkin == npc->race->skin) ? nullptr : resSkin;
  //   }
  //   return !npc->IsFemale() || npc->HasKeyword(ut->Key(kyGentlewoman)) ? res : resOkFixed;
}

void Core::OrganizeNPCKeywords(RE::TESNPC* const npc, int addonIdx, const bool isUser) const {
  npc->ForEachKeyword([&](RE::BGSKeyword* lKw) {
    if (lKw->formEditorID.contains("TNG_ActorAddn")) {
      npc->RemoveKeyword(lKw);
      return RE::BSContainer::ForEachResult::kStop;
    }
    return RE::BSContainer::ForEachResult::kContinue;
  });
  if (addonIdx == nul) {
    npc->AddKeyword(ut->Key(kyIgnored));
  } else if (addonIdx >= 0 && isUser) {
    const std::string reqKw = std::string(cNPCUserAddon) + fmt::format("{:02}", addonIdx);
    auto kw = ProduceOrGetKw(reqKw);
    if (!kw) SKSE::log::critical("Keyword generation routine failed with keyword {}", reqKw);
    npc->AddKeyword(kw);
  }
  if ((npc->IsFemale() ? allFemAddons : allMalAddons)[addonIdx].isGenderChanger) {
    npc->AddKeyword(ut->Key(kyGenderSwapped));
  } else {
    npc->RemoveKeyword(ut->Key(kyGenderSwapped));
  }
}

void TNG::Core::DoUpdateActor(RE::Actor* const actor, RE::TESObjectARMO* const armor, const bool isEquipped) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || actor->IsDisabled() || !npc->race || !npc->race->skin) return;
  auto canModify = CanModifyActor(actor);
  if (canModify == resOkRacePP) canModify = ReevaluateRace(npc->race, actor);
  if (canModify < resOkRaceR) {
    if (canModify < resOkRacePP) ExcludeNPC(npc);
    UnequipActorAddon(actor);
    return;
  }
  SetActorSize(actor, nul, false);
  if (canModify == resOkRaceR) return;
  if (actor->IsPlayerRef()) {
    if (pcInfo) {
      auto addonIdx = AddonIdxByLoc(pcInfo->isFemale, pcInfo->addon);
      SetActorAddon(actor, addonIdx, true, false);
    } else {
      SetActorAddon(actor, def, false, false);
    }
  } else {
    int currIdx;
    bool isAuto = true;
    auto addonRes = GetActorAddon(actor, currIdx, isAuto);
    if (addonRes < 0 || !isAuto) return;
  }
}

void Core::UpdateFormLists(RE::Actor* const actor) const {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return;
  auto key = ut->Key(kyGenderSwapped);
  if (!key) {
    SKSE::log::critical("TNG faced an error when trying to update the form lists. The keyword is not available.");
    return;
  }
  UpdateFormList(ut->FormList(npc->IsFemale() ? flmGentleWomen : flmGentlerMen), actor, npc->HasKeyword(key));
}

void Core::CheckArmorPieces() {
  SKSE::log::info("Checking ARMO records...");
  auto& armorList = SEDH()->GetFormArray<RE::TESObjectARMO>();
  auto defRace = ut->Race(raceDefault);
  int logInfo[7]{0};
  std::map<RE::TESObjectARMO*, std::string> potentialArmor = {};
  std::set<std::string> potentialMods{};
  std::set<std::string> potentialSlot52Mods{};
  std::vector<RE::BGSKeyword*> armorKeys = ut->Keys(kyRevealingF, kyRevealing);
  armorKeys.push_back(ut->Key(kyIgnored));
  std::vector<RE::BGSKeyword*> relRaceKeys{ut->Key(kyProcessed), ut->Key(kyPreProcessed), ut->Key(kyReady)};
  for (auto covRec : hardCodedCovering) {
    auto armor = SEDH()->LookupForm<RE::TESObjectARMO>(covRec.first, covRec.second);
    if (!armor) continue;
    armor->RemoveKeywords(armorKeys);
    armor->AddKeyword(ut->Key(kyCovering));
  }
  for (const auto& armor : armorList) {
    if (!armor) {
      logInfo[0]++;
      continue;
    }
    if (armor->HasKeywordInArray(armorKeys, false)) {
      logInfo[1]++;
      continue;
    }
    if (armor->armorAddons.size() == 0) {
      armor->AddKeyword(ut->Key(kyIgnored));
      logInfo[6]++;
      continue;
    }
    const auto armorID = (std::string(armor->GetName()).empty()) ? EI(armor) : armor->GetName();
    std::string modName = F0(armor, false);
    if (!armor->race) {
      armor->AddKeyword(ut->Key(kyIgnored));
      logInfo[6]++;
      continue;
    }
    if (defRace && !armor->race->HasKeywordInArray(relRaceKeys, false) && armor->race != defRace) {
      logInfo[6]++;
      continue;
    }
    if (IsSkin(I0(armor), modName)) {
      SKSE::log::info("\t\tThe record [0x{:x}: {}] was marked as a skin.", IG(armor), armorID);
      armor->AddKeyword(ut->Key(kyIgnored));
      logInfo[6]++;
      continue;
    }
    bool has52 =
        armor->HasPartOf(genitalSlot) || std::any_of(armor->armorAddons.begin(), armor->armorAddons.end(), [](const auto& aa) { return aa && aa->HasPartOf(genitalSlot); });
    if (!has52) {
      if (auto status = HasStatus(armor); status < keywordsCount) {
        if (!ut->Key(status)) {
          SKSE::log::error("TNG cannot load its own keywords! Make sure the esp is active in the load order.");
          logInfo[0]++;
          continue;
        }
        armor->AddKeyword(ut->Key(status));
        SKSE::log::info("\t\tThe armor [0x{:x}: {}] was marked with [{}] keyword.", IG(armor), armorID, EI(ut->Key(status)));
        (status == kyCovering) ? logInfo[2]++ : logInfo[3]++;
        continue;
      }
    } else {
      armor->RemoveKeywords(armorKeys);
      armor->AddKeyword(ut->Key(kyIgnored));
      logInfo[2]++;
      if (armor->HasPartOf(bodySlot)) continue;
      if (!modName.empty()) potentialSlot52Mods.insert(modName);
      SKSE::log::info("\t\tThe armor [0x{:x}] would cover genitals!", IG(armor));
      continue;
    }
    if (armor->HasPartOf(bodySlot)) {
      if (!modName.empty()) potentialMods.insert(std::string{modName});
      potentialArmor.insert({armor, modName});
    } else {
      logInfo[6]++;
    }
  }
  for (auto entry = potentialSlot52Mods.begin(); entry != potentialSlot52Mods.end(); entry++) {
    if (potentialMods.find(*entry) != potentialMods.end()) Slot52ModBehavior(*entry, boolSettings.Get(bsRevealSlot52Mods));
  }
  for (auto& armorPair : potentialArmor) {
    auto b = Slot52ModBehavior(armorPair.second);
    armorPair.first->AddKeyword(ut->Key(b ? kyRevealing : kyCovering));
    logInfo[b ? 5 : 4]++;
  }
  SKSE::log::info("\tProcessed [{}] armor pieces:", armorList.size());
  if (logInfo[0] > 0) SKSE::log::warn("\t\t[{}]: were problematic!", logInfo[0]);
  if (logInfo[1] > 0) SKSE::log::info("\t\t[{}]: were already marked with TNG keywords.", logInfo[1]);
  if (logInfo[2] > 0) SKSE::log::info("\t\t[{}]: are covering due to ini-files, having slot 52, or previous user input.", logInfo[2]);
  if (logInfo[3] > 0) SKSE::log::info("\t\t[{}]: are revealing, due to ini-files, having SOS_Revealing, or previous user input.", logInfo[3]);
  if (logInfo[4] > 0) SKSE::log::info("\t\t[{}]: were recognized to be covering.", logInfo[4]);
  if (logInfo[5] > 0) SKSE::log::info("\t\t[{}]: were recognized to be revealing.", logInfo[5]);
  if (logInfo[6] > 0) SKSE::log::info("\t\tThe rest [{}] are not relevant and are ignored!", logInfo[6]);
  SKSE::log::info("Finished checking ARMO records.");

  ClearInis();
}
