#include <Core.h>

Core* core = Core::GetSingleton();

void Core::Process() {
  LoadTngInis();
  LoadMainIni();
  LoadAddons();
  ProcessRaces();
  ProcessNPCs();
  CheckArmorPieces();
}

Common::RaceGroupInfo* Core::Rg(const RgKey& ky) {
  if (ky.race) {
    for (auto& rg : rgInfoList)
      if (std::find(rg.races.begin(), rg.races.end(), ky.race) != rg.races.end()) {
        return &rg;
      }
    return nullptr;
  } else if (ky.index >= 0) {
    if (ky.onlyMCM && !boolSettings.Get(Common::bsShowAllRaces)) {
      int curr = -1;
      for (auto& rg : rgInfoList) {
        if (!rg.noMCM) curr++;
        if (curr == ky.index) return &rg;
      }
    } else {
      if (ky.index < rgInfoList.size()) return &rgInfoList[static_cast<size_t>(ky.index)];
    }
  }
  return nullptr;
}

const Common::RaceGroupInfo* Core::Rg(const RgKey& ky) const {
  if (ky.race) {
    for (auto& rg : rgInfoList)
      if (std::find(rg.races.begin(), rg.races.end(), ky.race) != rg.races.end()) {
        return &rg;
      }
    return nullptr;
  } else if (ky.index >= 0) {
    if (ky.onlyMCM && !boolSettings.Get(Common::bsShowAllRaces)) {
      int curr = -1;
      for (auto& rg : rgInfoList) {
        if (!rg.noMCM) curr++;
        if (curr == ky.index) return &rg;
      }
    } else {
      if (ky.index < rgInfoList.size()) return &rgInfoList[static_cast<size_t>(ky.index)];
    }
  }
  return nullptr;
}

int Core::GetRgAddon(RgKey rgChoice) const {
  auto rg = Rg(rgChoice);
  if (!rg) return Common::err40;
  return rg->addonIdx;
}

void Core::SetRgAddon(RgKey rgChoice, const int addonIdx) {
  auto rg = Rg(rgChoice);
  if (!rg || addonIdx < Common::def || (addonIdx >= 0 && rg->malAddons.find(addonIdx) == rg->malAddons.end())) return;
  rg->addonIdx = (addonIdx == Common::def) ? rg->defAddonIdx : addonIdx;
  auto skin = (rg->addonIdx == Common::nul) ? rg->ogSkin : GetSkinWithAddonForRg(rg, rg->ogSkin, rg->addonIdx, false);
  if (!skin) {
    SKSE::log::critical("The race group [{}]'s original skin was not found!", rg->name);
    return;
  }
  for (auto& race : rg->races) race->skin = skin;
  auto addon = addonIdx < 0 ? nullptr : malAddons[rg->addonIdx].first;
  Inis::SetRgAddon(rg->races[0], addon, addonIdx);
}

float Core::GetRgMult(RgKey rgChoice) const {
  auto rg = Rg(rgChoice);
  return rg ? rg->mult : Common::fErr;
}

void Core::SetRgMult(RgKey rgChoice, const float mult) {
  auto rg = Rg(rgChoice);
  if (!rg || mult < 0.1f || mult >= 10.0f) {
    SKSE::log::critical("\tFailure in setting a race mult!");
    return;
  }
  rg->mult = mult;
  Inis::SetRgMult(rg->races[0], mult);
}

const std::string Core::GetRgInfo(RgKey rgChoice) const {
  std::string res{""};
  auto rg = Rg(rgChoice);
  if (!rg) return res;
  if (rg == &rgInfoList[0]) return "Internal group for TNG to handle main race groups with same skin.";
  auto& list = rg->races;
  res = "Main race: [" + rg->file + " : " + rg->name + "]; ";
  res = res + "Armor race: [" + rg->armorRace->GetFormEditorID() + "]; ";
  res = res + "Additional races: [";
  for (auto race : list) {
    if (race == list.front()) continue;
    res = res + std::string(race->GetFile(0) ? race->GetFile(0)->GetFilename() : "(Unknown)") + " : " + race->GetFormEditorID();
    if (race != list.back()) {
      res = res + ", ";
    }
  }
  res = res + "]";
  return res;
}

std::vector<std::pair<size_t, bool>> Core::GetRgAddons(RgKey rgChoice) const {
  std::vector<std::pair<size_t, bool>> res{};
  auto rg = Rg(rgChoice);
  if (!rg) return res;
  for (auto& addonPair : rg->malAddons) res.push_back({addonPair.first, addonPair.second.first});
  return res;
}

bool Core::ReevaluateRace(RE::TESRace* const race, RE::Actor* const actor) {
  if (!actor || !race || !Rg(RgKey(race))) return false;
  SKSE::log::debug("Re-evaluating race [0x{:x}:{}] ...", race->GetFormID(), race->GetFormEditorID());
  if (!actor->Is3DLoaded()) return false;
  auto rg = Rg(RgKey(race));
  bool isValid = true;
  for (auto& boneName : Common::genBoneNames)
    if (!actor->GetNodeByName(boneName)) {
      isValid = false;
      SKSE::log::info("\tTNG would neglect the race [0x{:x}:{}] since its skeleton is missing the bone [{}]", race->GetFormID(), race->GetFormEditorID(), boneName);
    }
  if (isValid) {
    if (race->skeletonModels[0].model.empty() || race->skeletonModels[1].model.empty()) {
      SKSE::log::info("\tTNG would neglect the race [0x{:x}:{}] since its skeleton could not be recognized.", race->GetFormID(), race->GetFormEditorID());
      isValid = false;
    }
  }
  if (isValid) {
    SetValidSkeleton(race->skeletonModels[0].model.data());
    SetValidSkeleton(race->skeletonModels[1].model.data());
    SKSE::log::debug("\tThe race [0x{:x}:{}] was evaluated to be supported by TNG!", race->GetFormID(), race->GetFormEditorID());
    race->RemoveKeyword(ut->Key(Common::kyPreProcessed));
    race->AddKeyword(ut->Key(Common::kyProcessed));
  } else {
    race->skin = rg->ogSkin;
    race->RemoveKeyword(ut->Key(Common::kyPreProcessed));
    race->AddKeyword(ut->Key(Common::kyIgnored));
    race->RemoveSlotFromMask(Common::genitalSlot);
    auto it = std::find(rg->races.begin(), rg->races.end(), race);
    rg->races.erase(it);
    auto rgIt = std::ranges::find_if(rgInfoList, [&](const auto& item) { return &item == rg; });
    if (rgIt != rgInfoList.end() && rg->races.empty()) rgInfoList.erase(rgIt);
  }
  return isValid;
}

Common::eRes Core::CanModifyActor(RE::Actor* const actor) const {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Common::errNPC;
  if (!npc->race) return Common::errRace;
  if (IsNPCExcluded(npc)) return Common::errNPC;
  if (auto skin = npc->skin; skin && skin->HasPartOf(Common::genitalSlot) && !npc->skin->HasKeyword(ut->Key(Common::kyTngSkin))) {
    for (auto& aa : skin->armorAddons)
      if (aa && aa->HasPartOf(Common::genitalSlot)) return Common::resOkRaceR;
    return Common::errSkin;
  }
  if (npc->race->HasKeyword(ut->Key(Common::kyReady))) return Common::resOkRaceR;
  if (auto rg = Rg(RgKey(npc->race)); !rg || rg->malAddons.size() == 0) return Common::errRace;
  if (npc->race->HasKeyword(ut->Key(Common::kyProcessed))) return Common::resOkRaceP;
  if (npc->race->HasKeyword(ut->Key(Common::kyPreProcessed))) return Common::resOkRacePP;
  return Common::errRace;
}

void Core::UpdateActor(RE::Actor* const actor, RE::TESObjectARMO* const armor, const bool isEquipped) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || actor->IsDisabled() || !npc->race || !npc->race->skin) return;
  auto canModify = CanModifyActor(actor);
  auto skin = npc->skin;
  if (canModify == Common::resOkRacePP) ReevaluateRace(actor->GetRace(), actor);
  if (canModify < 0 || canModify == Common::resOkRacePP) {
    if (skin && skin->HasKeyword(ut->Key(Common::kyTngSkin))) npc->skin = nullptr;
    return;
  }
  if (skin && skin->HasKeyword(ut->Key(Common::kyTngSkin)) &&
      std::ranges::find_if(skin->armorAddons, [&](const auto& aa) { return aa->IsValidRace(npc->race); }) == skin->armorAddons.end())
    npc->skin = nullptr;
  if (canModify == Common::resOkRaceP || canModify == Common::resOkRaceR) {
    if (actor->IsPlayerRef()) {
      UpdatePlayer(actor, canModify == Common::resOkRaceR);
    } else {
      UpdateAddon(actor, canModify == Common::resOkRaceR);
    }
  }
  UpdateBlock(actor, armor, isEquipped);
  if (npc->skin != skin) ut->QueueNiNodeUpdate(actor);
}

std::vector<std::pair<size_t, bool>> Core::GetActorAddons(RE::Actor* const actor, const bool onlyActive) const {
  std::vector<std::pair<size_t, bool>> res{};
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race) return res;
  if (auto rg = Rg(RgKey(npc->race)); rg) {
    auto& list = npc->IsFemale() ? rg->femAddons : rg->malAddons;
    auto& master = npc->IsFemale() ? femAddons : malAddons;
    for (auto& addonPair : list) {
      if (!onlyActive || master[addonPair.first].second) res.push_back({addonPair.first, addonPair.second.first});
    }
  }
  return res;
}

Common::eRes Core::GetActorAddon(RE::Actor* actor, int& addonIdx, bool& isAuto) const {
  isAuto = true;
  addonIdx = Common::nul;
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Common::errNPC;
  if (IsNPCExcluded(npc) || npc->HasKeyword(ut->Key(Common::kyExcluded))) {
    isAuto = false;
    if (npc->skin && npc->skin->HasKeyword(ut->Key(Common::kyTngSkin))) return Common::err40;
    return Common::resOkNoAddon;
  }
  auto rg = Rg(RgKey(npc->race));
  if (!rg) return Common::errRace;
  if (!npc->IsFemale() && rg->addonIdx != Common::nul) addonIdx = rg->addonIdx;
  if (!npc->skin) return addonIdx > Common::nul ? Common::resOkHasAddon : Common::resOkNoAddon;
  if (!npc->skin->HasKeyword(ut->Key(Common::kyTngSkin))) {
    if (!npc->IsFemale() && rg->addonIdx != Common::nul) return Common::err40;
    return Common::resOkNoAddon;
  }
  npc->ForEachKeyword([&](RE::BGSKeyword* kw) {
    if (!kw || kw->GetFormEditorID() == NULL) return RE::BSContainer::ForEachResult::kContinue;
    const std::string kwStr(kw->GetFormEditorID());
    std::string str = "";
    if (kwStr.starts_with(Common::cNPCAutoAddon)) {
      addonIdx = std::stoi(kwStr.substr(strlen(Common::cNPCAutoAddon), 2));
    } else if (kwStr.starts_with(Common::cNPCUserAddon)) {
      isAuto = false;
      addonIdx = std::stoi(kwStr.substr(strlen(Common::cNPCUserAddon), 2));
    }
    return addonIdx >= 0 ? RE::BSContainer::ForEachResult::kStop : RE::BSContainer::ForEachResult::kContinue;
  });
  return addonIdx >= 0 ? Common::resOkHasAddon : Common::resOkNoAddon;
}

Common::eRes Core::SetActorAddon(RE::Actor* const actor, const int choice, const bool isUser, const bool shouldSave) {
  if (shouldSave) {
    if (CanModifyActor(actor) == Common::resOkRacePP) ReevaluateRace(actor->GetRace(), actor);
    if (auto res = CanModifyActor(actor); res != Common::resOkRaceP) {
      if (res >= 0) res = Common::errRace;
      return res;
    }
  }
  if (actor->IsPlayerRef() && choice == Common::defPlayer) return UpdatePlayer(actor, false);
  const auto npc = actor->GetActorBase();
  auto list = GetActorAddons(actor, !isUser);
  if (shouldSave) {
    if (choice >= static_cast<int>(list.size())) return Common::errAddon;
    SKSE::log::debug("Setting addon [{}] for actor [0x{:x}:{}].", choice, actor->GetFormID(), npc->GetName());
  }
  auto addonIdx = choice < 0 ? choice : shouldSave ? static_cast<int>(list[choice].first) : choice;
  auto oldSkin = npc->skin;
  auto res = SetNPCAddon(npc, addonIdx, isUser);
  if (res < 0) return res;
  auto addon = addonIdx < 0 ? nullptr : (npc->IsFemale() ? femAddons[addonIdx].first : malAddons[addonIdx].first);
  if (actor->IsPlayerRef() && shouldSave) SetPlayerInfo(actor, addon, addonIdx);
  if (!npc->IsPlayer() && shouldSave) Inis::SetActorAddon(actor, npc, addon, addonIdx);
  if (shouldSave || (!isUser && npc->skin != oldSkin)) {
    UpdateFormLists(actor);
    UpdateBlock(actor, nullptr, false);
  }
  return res;
}

Common::eRes Core::GetActorSize(RE::Actor* const actor, int& sizeCat) const {
  sizeCat = Common::nan;
  if (auto res = CanModifyActor(actor); res < 0) return res;
  const auto npc = actor->GetActorBase();
  if (npc->IsPlayer() && boolSettings.Get(Common::bsExcludePlayerSize)) return Common::errPlayer;
  if (sizeCat = Inis::GetActorSize(actor, npc); sizeCat >= 0) return Common::resOkSizable;
  sizeCat = ut->HasKeywordInList(npc, ut->SizeKeys());
  if (sizeCat < 0) sizeCat = npc->formID % Common::sizeCatCount;
  return Common::resOkSizable;
}

Common::eRes Core::SetActorSize(RE::Actor* const actor, int sizeCat, const bool shouldSave) {
  if (shouldSave) {
    if (CanModifyActor(actor) == Common::resOkRacePP) ReevaluateRace(actor->GetRace(), actor);
    if (auto res = CanModifyActor(actor); res < 0) return res;
  }
  const auto npc = actor->GetActorBase();
  if (!actor->IsPlayerRef() && npc->IsPlayer()) return Common::resOkFixed;  // Don't change the size for copies of player actor
  int currCat = Common::nan;
  if (sizeCat == Common::def) npc->RemoveKeywords(ut->SizeKeys());
  auto res = GetActorSize(actor, currCat);
  if (res != Common::resOkSizable) return res;
  auto cat = (sizeCat < 0) ? currCat : sizeCat;
  if (cat != currCat) {
    npc->RemoveKeywords(ut->SizeKeys());
    npc->AddKeyword(ut->SizeKey(cat));
  }
  if (auto rg = Rg(RgKey(npc->race)); rg) {
    auto mult = rg->mult;
    if (mult < 0.0f) return Common::errRg;
    auto scale = mult * floatSettings.Get(static_cast<Common::eFloatSetting>(cat));
    if (scale < 0.1) scale = 1;
    const auto isPlayer = actor->IsPlayerRef();
    const std::string failMessage = fmt::format("Failed to scale actor [0x{:x}] genitalia to [{}] since they are not loaded after standard delay.", actor->GetFormID(), scale);
    ut->DoDelayed(
        [actor, isPlayer, scale, shouldSave, sizeCat]() {
          auto ac = isPlayer ? RE::PlayerCharacter::GetSingleton() : actor;
          if (!ac) return;
          RE::NiAVObject* baseNode = ac->GetNodeByName(Common::genBoneNames[Common::egbBase]);
          RE::NiAVObject* scrotNode = ac->GetNodeByName(Common::genBoneNames[Common::egbScrot]);
          if (baseNode && scrotNode) {
            if (baseNode->local.scale == scale) return;
            baseNode->local.scale = scale;
            scrotNode->local.scale = 1.0f / sqrt(scale);
            if (sizeCat != Common::nul) SKSE::log::debug("Actor [0x{:x}] genitalia {} to [{}].", ac->GetFormID(), shouldSave ? "scaled" : "restored", scale);
          } else {
            SKSE::log::debug("Failed to scale actor [0x{:x}] genitalia to [{}] since their skeleton does not seem to be compatible.", ac->GetFormID(), scale);
          }
        },
        [actor]() -> bool { return actor && actor->Is3DLoaded() && actor->GetNodeByName(Common::genBoneNames[Common::egbBase]); }, 0, true, failMessage);
  }
  if (actor->IsPlayerRef() && sizeCat != Common::nul && shouldSave) SetPlayerInfo(actor, nullptr, Common::nan, sizeCat);
  if (!actor->IsPlayerRef() && shouldSave) Inis::SetActorSize(actor, actor->GetActorBase(), sizeCat);
  return res;
}

void Core::UpdatePlayerAfterLoad() {
  SKSE::log::debug("TNG would keep an eye on the player skin for a while...");
  if (!core) return;
  auto player = RE::PlayerCharacter::GetSingleton();
  if (!player) return;
  core->UpdateActor(player);
  if (!core->boolSettings.Get(Common::bsForceRechecks) && !core->boolSettings.Get(Common::bsCheckPlayerAddon)) return;
  auto LoadPC = []() {};
  auto LoadCond = []() {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return true;
    auto pc = player->GetActorBase();
    if (!pc || !player->Is3DLoaded()) return false;
    auto skin = pc->skin;
    if (skin && !skin->HasKeyword(ut->Key(Common::kyTngSkin))) {
      core->UpdateActor(player);
    }
    return false;
  };
  ut->DoDelayed(LoadPC, LoadCond, 0, true, "Finished checking the player skin.");
}

bool Core::SwapRevealing(RE::Actor* const actor, RE::TESObjectARMO* const armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (armor->HasKeyword(ut->Key(Common::kyUnderwear))) return false;
  std::vector<RE::BGSKeyword*> rcKeys = {ut->Key(statusKeys[0]), ut->Key(statusKeys[1]), ut->Key(statusKeys[2]), ut->Key(statusKeys[3])};
  auto kb = ut->HasKeywordInList(armor, rcKeys);
  int mask = npc->IsFemale() ? 2 : 1;
  if (kb < 0) {
    armor->RemoveKeyword(ut->Key(Common::kyIgnored));
    armor->AddKeyword(ut->Key(Common::kyCovering));
    SetArmorStatus(armor, Common::kyCovering);
  } else {
    armor->RemoveKeyword(rcKeys[kb]);
    if (armor->HasPartOf(Common::bodySlot)) {
      armor->AddKeyword(rcKeys[kb ^ mask]);
      SetArmorStatus(armor, statusKeys[kb ^ mask]);
    } else {
      SetArmorStatus(armor, Common::keywordsCount);
    }
  }
  UpdateBlock(actor, nullptr, false);
  return true;
}

void Core::RevisitRevealingArmor() const {
  auto s52 = Slot52Mods();
  if (s52.size() == 0) return;
  auto& armorList = ut->SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  std::vector<RE::BGSKeyword*> rc = {ut->Key(Common::kyCovering), ut->Key(Common::kyRevealing)};
  for (const auto& armor : armorList) {
    if (!armor || !armor->HasPartOf(Common::bodySlot) || !armor->HasKeywordInArray(rc, false)) continue;
    auto modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (modName.empty() || std::ranges::find(s52, modName) == s52.end()) continue;
    if (auto b = Slot52ModBehavior(modName); b != armor->HasKeyword(ut->Key(Common::kyRevealing))) {
      armor->RemoveKeywords(rc);
      armor->AddKeyword(ut->Key(b ? Common::kyRevealing : Common::kyCovering));
    }
  }
}

void Core::LoadAddons() {
  SKSE::log::info("Loading the addons...");
  malAddons.clear();
  femAddons.clear();
  const auto& armorList = ut->SEDH()->GetFormArray<RE::TESObjectARMO>();
  for (const auto& armor : armorList) {
    if (armor->HasKeyword(ut->Key(Common::kyAddonM))) malAddons.emplace_back(armor, true);
    if (armor->HasKeyword(ut->Key(Common::kyAddonF))) femAddons.emplace_back(armor, false);
    if (armor->HasKeyword(ut->Key(Common::kyPreSkin))) preSkins.insert(armor);
  }
  for (auto& addonPair : malAddons) {
    if (!addonPair.first->HasKeyword(ut->Key(Common::kyIgnored))) addonPair.first->AddKeyword(ut->Key(Common::kyIgnored));
    auto addonLoc = ut->FormToLoc(addonPair.first);
    if (activeMalAddons.find(addonLoc) != activeMalAddons.end()) addonPair.second = false;
  }
  for (auto& addonPair : femAddons) {
    if (!addonPair.first->HasKeyword(ut->Key(Common::kyIgnored))) addonPair.first->AddKeyword(ut->Key(Common::kyIgnored));
    auto addonLoc = ut->FormToLoc(addonPair.first);
    if (activeFemAddons.find(addonLoc) != activeFemAddons.end()) addonPair.second = true;
  }
  SKSE::log::debug("Loaded all addons with [{}] addons for men and [{}] addons for women.", malAddons.size(), femAddons.size());
}

int Core::AddonIdxByLoc(const bool isFemale, const SEFormLocView addonLoc) const {
  if (addonLoc.second.empty()) return Common::nan;
  if (addonLoc.second == Common::nulStr) return Common::nul;
  if (addonLoc.second == Common::defStr) return Common::def;
  auto& list = isFemale ? femAddons : malAddons;
  for (int i = 0; i < list.size(); i++) {
    if (ut->FormToLoc(list[i].first) == addonLoc) return i;
  }
  return Common::nan;
}

void Core::ProcessRaces() {
  SKSE::log::info("Processing races...");
  auto& rg0 = rgInfoList.emplace_back();
  rg0.name = "TNGRg0";
  rg0.isMain = true;
  rg0.noMCM = true;
  const auto& allRaces = ut->SEDH()->GetFormArray<RE::TESRace>();
  int logInfo[4] = {0, 0, 0, 0};
  std::vector<RE::BGSKeyword*> keywords = {ut->Key(Common::kyIgnored), ut->Key(Common::kyReady), ut->Key(Common::kyProcessed), ut->Key(Common::kyPreProcessed)};
  SetValidSkeleton(ut->Race(Common::raceDefault)->skeletonModels[0].model.data());
  SetValidSkeleton(ut->Race(Common::raceDefault)->skeletonModels[1].model.data());
  SetValidSkeleton(ut->Race(Common::raceDefBeast)->skeletonModels[0].model.data());
  SetValidSkeleton(ut->Race(Common::raceDefBeast)->skeletonModels[1].model.data());
  for (const auto& race : allRaces) {
    if (!race) continue;
    if (!race->HasKeywordInArray(keywords, false)) ProcessRace(race);
    auto k = ut->HasKeywordInList(race, keywords);
    k >= 0 ? logInfo[k]++ : logInfo[0]++;
  }
  for (auto& rg : rgInfoList) {
    if (rg.addonIdx >= 0) {
      auto skin = GetSkinWithAddonForRg(&rg, rg.ogSkin, rg.addonIdx, false);
      if (skin)
        for (auto& race : rg.races) race->skin = skin;
    }
  }
  SKSE::log::info("Processed [{}] races: assigned genitalia to [{}] races, preprocessed [{}] races, found [{}] races to be ready and ignored [{}] races.", allRaces.size(),
                  logInfo[2], logInfo[3], logInfo[1], logInfo[0]);
}

void Core::IgnoreRace(RE::TESRace* const race, const bool ready) {
  if (!race) return;
  if (auto& skin = race->skin; skin) skin->AddKeyword(ut->Key(Common::kyIgnored));
  race->RemoveKeywords(ut->Keys(Common::kyProcessed, Common::kyExcluded));
  race->AddKeyword(ut->Key(ready ? Common::kyReady : Common::kyIgnored));
}

Common::RaceGroupInfo* const Core::ProcessRace(RE::TESRace* const race) {
  if (auto rg = Rg(RgKey(race)); rg) return rg;
  switch (CheckRace(race)) {
    case Common::resOkRacePP:
      return AddRace(race, false);
    case Common::resOkRaceP:
      return AddRace(race, true);
    case Common::resOkRaceR:
      IgnoreRace(race, true);
      break;
    case Common::errRace:
      IgnoreRace(race, false);
      break;
    default:
      break;
  }
  return nullptr;
}

Common::eRes Core::CheckRace(RE::TESRace* const race) const {
  try {
    for (auto raceInfo : hardCodedRaces)
      if (ut->FormToLoc(race) == raceInfo) return Common::resOkRaceP;
    if (race->HasKeyword(ut->Key(Common::kyCreature)) || race->IsChildRace() || !race->HasKeyword(ut->Key(Common::kyManMer))) return Common::errRaceBase;
    if (IsRaceExcluded(race)) {
      SKSE::log::info("\tThe race [{}: xx{:x}: {}] was ignored because an ini excludes it!", race->GetFile(0)->GetFilename(), race->GetLocalFormID(), race->GetFormEditorID());
      return Common::errRaceBase;
    }
    if (!race->skin) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since they do not have a skin! It was last modified by [{}].", race->GetFormID(), race->GetFormEditorID(),
                      race->GetFile() ? race->GetFile()->GetFilename() : "Unrecognized File");
      return Common::errRaceBase;
    }
    bool skinFound = false;
    for (const auto& aa : race->skin->armorAddons) {
      if (aa->HasPartOf(Common::bodySlot) && aa->IsValidRace(race)) {
        skinFound = true;
        break;
      }
    }
    if (!skinFound) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since their skin cannot be recognized!  It was last modified by [{}].", race->GetFormID(),
                      race->GetFormEditorID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unrecognized File");
      return Common::errRace;
    }
    bool isVanilla = race->GetFile(0) && race->GetFile(0)->GetFilename() == Common::skyrimFile;
    if (race->HasPartOf(Common::genitalSlot) && !isVanilla) {
      auto ready = race->skin->HasPartOf(Common::genitalSlot);
      SKSE::log::info("\tThe race [{}] is designed to be {} TNG. It was not modified.", race->GetFormEditorID(), ready ? "ready for" : "ignored by");
      return ready ? Common::resOkRaceR : Common::errRace;
    }
  } catch (const std::exception& er) {
    SKSE::log::warn("\tThe race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(),
                    race->GetFormEditorID(), er.what());
    const char* message = fmt::format("TheNewGentleman: The race [0x{:x}: {}] caused an error [{}]. TNG tries to ignore it but it might cause other problems!", race->GetFormID(),
                                      race->GetFormEditorID(), er.what())
                              .c_str();
    ut->ShowSkyrimMessage(message);
    return Common::errRace;
  }
  bool isValidSk =
      validSkeletons.find(race->skeletonModels[0].model.data()) != validSkeletons.end() && validSkeletons.find(race->skeletonModels[1].model.data()) != validSkeletons.end();
  return isValidSk ? Common::resOkRaceP : Common::resOkRacePP;
}

Common::RaceGroupInfo* Core::AddRace(RE::TESRace* const race, const bool isProcessed) {
  race->AddKeyword(ut->Key(isProcessed ? Common::kyProcessed : Common::kyPreProcessed));
  race->AddSlotToMask(Common::genitalSlot);
  race->skin->AddKeyword(ut->Key(Common::kyIgnored));
  SKSE::log::debug("\tTrying to add race [0x{:x}: {}] ...", race->GetFormID(), race->GetFormEditorID());
  if (race->armorParentRace) {
    for (auto& rg : rgInfoList) {
      if (rg.armorRace == race->armorParentRace && rg.ogSkin == race->skin) {
        rg.races.push_back(race);
        SKSE::log::info("\tThe race [0x{:x}: {}] was recognized as a member of existing group {}.", race->GetFormID(), race->GetFormEditorID(), rg.name);
        return &rg;
      }
    }
    if (race->armorParentRace->skin == race->skin) {
      SKSE::log::debug("\t\t...processing parent race [0x{:x}: {}]", race->armorParentRace->GetFormID(), race->armorParentRace->GetFormEditorID());
      if (auto rgPtr = ProcessRace(race->armorParentRace); rgPtr) {
        rgPtr->races.push_back(race);
        SKSE::log::info("\tThe race [0x{:x}: {}] was recognized as a member of existing group {}.", race->GetFormID(), race->GetFormEditorID(), rgPtr->name);
        return rgPtr;
      }
      SKSE::log::debug("\t\t...parent race [0x{:x}: {}] was not a valid race.", race->armorParentRace->GetFormID(), race->armorParentRace->GetFormEditorID());
    }
  }
  auto pRace = race;
  while (pRace->armorParentRace) {
    pRace = pRace->armorParentRace;
  };
  auto filename = race->GetFile(0) ? race->GetFile(0)->GetFilename() : "Unknown";
  rgInfoList.push_back({});
  auto& rg = rgInfoList.back();
  rg.name = race->GetFormEditorID();
  rg.file = filename;
  rg.armorRace = pRace;
  rg.ogSkin = race->skin;
  rg.isMain = pRace == race;
  rg.races.push_back(race);
  rg.noMCM = !race->GetPlayable() && !race->HasKeyword(ut->Key(Common::kyVampire));
  rg.mult = 1.0f;
  rg.defAddonIdx = GetRgDefAddon(rg);
  ProcessRgAddons(rg, malAddons, false);
  ProcessRgAddons(rg, femAddons, true);
  if (ut->Block() && !ut->Block()->armorAddons[0]->IsValidRace(race)) ut->Block()->armorAddons[0]->additionalRaces.push_back(rg.armorRace);
  SKSE::log::info("\tThe race [0x{:x}: {}] was recognized as a new {} group [{}].", race->GetFormID(), race->GetFormEditorID(), rg.isMain ? "primary" : "secondary", rg.name);
  ApplyUserSettings(rg);
  return &rg;
}

int Core::GetRgDefAddon(Common::RaceGroupInfo& rg) {
  if (rg.defAddonIdx != Common::nan) return rg.defAddonIdx;
  SEFormLoc defAddon;
  bool defAddonSet = false;
  auto raceStr = std::string(rg.armorRace->GetFormEditorID()) + std::string(rg.armorRace->GetName());
  std::transform(raceStr.begin(), raceStr.end(), raceStr.begin(), [](uint8_t c) { return static_cast<char>(std::tolower(c)); });
  auto rgIsBeast = rg.armorRace->HasKeyword(ut->Key(Common::kyBeast));
  std::ranges::for_each(Common::cVanillaDefaults, [&](const auto& tup) {
    auto [s1, s2, isBeast, addon] = tup;
    if ((raceStr.contains(s1) || raceStr.contains(s2)) && (rgIsBeast == isBeast)) {
      defAddon = std::make_pair(addon, Common::mainFile);
      defAddonSet = true;
    };
  });
  if (!defAddonSet) defAddon = std::make_pair(std::get<3>(Common::cVanillaDefaults[rgIsBeast ? 9 : 0]), Common::mainFile);
  if (auto idx = AddonIdxByLoc(false, defAddon); idx >= 0) {
    return idx;
  }
  SKSE::log::critical("TNG faced an error getting the default addon for a race group.");
  return Common::nul;
}

void Core::ProcessRgAddons(Common::RaceGroupInfo& rg, const std::vector<std::pair<RE::TESObjectARMO*, bool>>& addons, const bool isFemale) {
  auto& rgAddons = isFemale ? rg.femAddons : rg.malAddons;
  const char* gender = isFemale ? "women" : "men";
  for (size_t i = 0; i < addons.size(); i++) {
    auto& addon = addons[i].first;
    bool supports = false;
    for (const auto& aa : addon->armorAddons) {
      if (aa && aa->IsValidRace(rg.armorRace)) {
        std::set<RE::TESRace*> aaRaces{aa->race};
        std::set<RE::TESRace*> toEraseRaces{};
        if (rg.isMain) {
          if (aa->additionalRaces.size() > 0) aaRaces.insert(aa->additionalRaces.begin(), aa->additionalRaces.end());
          for (auto& race : aaRaces) {
            if (auto r = Rg(RgKey(race)); r && !r->isMain) toEraseRaces.insert(r->armorRace);
          }
          for (auto& race : toEraseRaces) aaRaces.erase(race);
          if (aaRaces.find(rg.armorRace) == aaRaces.end()) continue;
        }
        supports = true;
        auto isDedicated = aaRaces.find(rg.races[0]) != aaRaces.end();
        rgAddons.insert_or_assign(i, std::make_pair(isDedicated, aa));
        if (isDedicated) break;
      }
    }
    if (!supports) {
      if (i == rg.defAddonIdx) rg.defAddonIdx = rg.malAddons.empty() ? Common::nul : static_cast<int>(rg.malAddons.begin()->first);
      if (rg.addonIdx < 0 || rg.addonIdx == i) rg.addonIdx = rg.defAddonIdx;
      continue;
    }
    if (rg.defAddonIdx < 0) rg.defAddonIdx = static_cast<int>(i);
    if (i == rg.defAddonIdx && rg.addonIdx < 0) rg.addonIdx = static_cast<int>(i);
    if (!rg.isMain && rgAddons[i].first && (rg.addonIdx < 0 || !rgAddons[rg.addonIdx].first)) {
      rg.defAddonIdx = static_cast<int>(i);
      rg.addonIdx = static_cast<int>(i);
    }
    SKSE::log::debug("\t\tThe addon [0x{:x}] from file [{}] {} {} in the race group [{}]!", addon->GetFormID(), addon->GetFile(0)->GetFilename(),
                     rgAddons[i].first ? "fully supports" : "can be used for", gender, rg.name);
  }
}

void Core::ApplyUserSettings(Common::RaceGroupInfo& rg) {
  if (auto rg0Loc = ut->FormToLoc(rg.races[0]); !rg0Loc.second.empty()) {
    if (racialAddons.find(rg0Loc) != racialAddons.end()) {
      auto& addonLoc = racialAddons[rg0Loc];
      auto index = AddonIdxByLoc(false, addonLoc);
      if (index >= 0 && rg.malAddons.find(static_cast<size_t>(index)) != rg.malAddons.end()) {
        rg.addonIdx = index;
        SKSE::log::debug("\tRestored group [{}] addon to [xx{:x}] from file [{}]!", rg.name, addonLoc.first, addonLoc.second);
      } else if (index == Common::nul) {
        rg.addonIdx = Common::nul;
        SKSE::log::debug("\tRestored group [{}] addon to be empty!", rg.name);
      } else {
        SKSE::log::debug("\tThe addon [xx{:x}] from file [{}] could not be used for group [{}]!", addonLoc.first, addonLoc.second, rg.name);
      }
    }
    if (racialSizes.find(rg0Loc) != racialSizes.end()) {
      auto size = racialSizes[rg0Loc];
      if (size > 0.01f && size < 10.0f) {
        rg.mult = size;
        SKSE::log::debug("\tRestored group [{}] size to [{}]!", rg.name, size);
      } else {
        SKSE::log::debug("\tThe size [{}] is out of range for group [{}]!", size, rg.name);
      }
    }
  }
}

RE::TESObjectARMO* Core::GetSkinWithAddonForRg(Common::RaceGroupInfo* const rg, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale) {
  auto& r = rg->isMain ? rgInfoList[0] : *rg;
  auto& skinMap = isFemale ? r.femSkins : r.malSkins;
  auto ogSkin = GetOgSkin(skin);
  if (!ogSkin->HasKeyword(ut->Key(Common::kyTngSkin)) && !ogSkin->HasKeyword(ut->Key(Common::kyIgnored))) ogSkin->AddKeyword(ut->Key(Common::kyIgnored));
  if (skinMap.find(ogSkin) == skinMap.end()) skinMap.insert({ogSkin, std::map<size_t, RE::TESObjectARMO*>{}});
  RE::TESObjectARMO* resSkin = nullptr;
  if (skinMap[ogSkin].find(addonIdx) == skinMap[ogSkin].end()) {
    if (preSkins.size() > 0) {
      resSkin = *preSkins.begin();
      preSkins.erase(preSkins.begin());
    } else {
      resSkin = ogSkin->CreateDuplicateForm(true, (void*)resSkin)->As<RE::TESObjectARMO>();
    }
    if (!resSkin) {
      SKSE::log::critical("Failed to create a new skin");
      return nullptr;
    }
    resSkin->Copy(ogSkin);
    resSkin->SetFullName(ogSkin->GetFullName());
    resSkin->AddKeyword(ut->Key(Common::kyTngSkin));
    if (isFemale && femAddons[addonIdx].first->HasKeyword(ut->Key(Common::kySkinWP))) resSkin->AddKeyword(ut->Key(Common::kySkinWP));
    resSkin->AddSlotToMask(Common::genitalSlot);
    ogSkinMap.insert({resSkin, ogSkin});
    skinMap[ogSkin].insert({addonIdx, resSkin});
  } else {
    resSkin = skinMap[ogSkin][addonIdx];
  }
  auto& reqAA = isFemale ? rg->femAddons[addonIdx].second : rg->malAddons[addonIdx].second;
  if (std::find(resSkin->armorAddons.begin(), resSkin->armorAddons.end(), reqAA) == resSkin->armorAddons.end()) resSkin->armorAddons.push_back(reqAA);
  return resSkin;
}

RE::TESObjectARMO* Core::GetOgSkin(RE::TESObjectARMO* skin) const {
  if (!skin) return nullptr;
  auto res = skin->HasKeyword(ut->Key(Common::kyTngSkin)) && ogSkinMap.find(skin) != ogSkinMap.end() ? ogSkinMap.at(skin) : skin;
  auto problem = 0;
  if (!res) {
    problem = 1;
  } else {
    if (res->HasKeyword(ut->Key(Common::kyTngSkin))) problem = 2;
  }
  if (problem) {
    SKSE::log::critical("TNG routine to find the original skin failed with flag [{}]! Please report this issue.", problem);
  }
  return res;
}

void Core::ProcessNPCs() {
  SKSE::log::info("Checking NPCs for custom skins...");
  std::map<std::pair<RE::TESObjectARMO*, RE::TESRace*>, std::set<RE::TESNPC*>> skinsToPatch{};
  std::map<std::string_view, size_t> customSkinMods{};
  std::map<RE::TESRace*, size_t> raceNPCCount;
  auto& allNPCs = ut->SEDH()->GetFormArray<RE::TESNPC>();
  size_t sizeCount[Common::sizeCatCount]{0};
  for (const auto& npc : allNPCs) {
    if (!npc) continue;
    if (IsNPCExcluded(npc)) {
      npc->AddKeyword(ut->Key(Common::kyExcluded));
      continue;
    }
    const auto race = npc->race;
    if (!race) {
      SKSE::log::warn("\tThe NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", npc->GetFormID(), npc->GetFormEditorID());
      continue;
    }
    if (!race->HasKeyword(ut->Key(Common::kyProcessed)) && !race->HasKeyword(ut->Key(Common::kyPreProcessed))) continue;
    raceNPCCount[npc->race]++;
    const auto skin = npc->skin;
    if (npc->IsFemale()) continue;
    sizeCount[npc->formID % Common::sizeCatCount]++;
    if (!skin) continue;
    if (skin->HasKeyword(ut->Key(Common::kyTngSkin))) continue;
    skinsToPatch[{skin, race}].insert(npc);
  }
  for (auto& racePair : raceNPCCount) {
    if (!racePair.first || racePair.second < 5) continue;
    if (auto rg = Rg(RgKey(racePair.first)); rg && rg->malAddons.size() > 0) rg->noMCM = false;
  }
  for (auto& skinPair : skinsToPatch) {
    auto& oldSkin = skinPair.first.first;
    auto& race = skinPair.first.second;
    auto newSkin = FixSkin(oldSkin, race, nullptr);
    if (!newSkin) continue;
    auto modName = oldSkin->GetFile(0) ? oldSkin->GetFile(0)->GetFilename() : "(no mod name)";
    customSkinMods[modName]++;
    for (auto& npc : skinPair.second) npc->skin = newSkin;
  }
  if (skinsToPatch.size() > 0) {
    auto tot = skinsToPatch.size();
    SKSE::log::debug("\tHandled {} custom skin-race combinations from following mod(s):", skinsToPatch.size());
    for (const auto& entry : customSkinMods) {
      SKSE::log::debug("\t\t[{}] skins from {}", entry.second, entry.first);
      tot -= entry.second;
    }
    if (tot > 0) SKSE::log::debug("\t\t[{}] skins were not patched.", tot);
  }
  SKSE::log::info("Finished checking NPCs for custom skins.");
  SKSE::log::debug("TNG distributed the sizes (from smallest to largest) to [{}] NPCs!", fmt::join(sizeCount, ", "));
  SKSE::log::info("\nTNG would not show the following race groups in the MCM since there are either no genital available to them or there are very few NPCs which use them:");
  for (size_t i = 1; i < rgInfoList.size(); i++)
    if (rgInfoList[i].noMCM) SKSE::log::info("\tRace group [{}] with the following information: {}.", rgInfoList[i].name, GetRgInfo(RgKey(i, false)));
}

RE::TESObjectARMO* Core::FixSkin(RE::TESObjectARMO* const skin, RE::TESRace* const race, const char* const name) {
  skin->RemoveKeywords(ut->Keys(Common::kyRevealingF, Common::kyRevealing));
  skin->AddKeyword(ut->Key(Common::kyIgnored));
  if (!skin->HasPartOf(Common::bodySlot)) {
    SKSE::log::info("\t\tThe skin [0x{:x}] used does not have a body part. TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->HasPartOf(Common::genitalSlot)) {
    SKSE::log::warn("\t\tThe skin [0x{:x}] cannot have a TNG addon since it already has slot 52.", skin->GetFormID());
    return nullptr;
  }
  if (!skin->race) {
    SKSE::log::warn("\t\tThe skin [0x{:x}] does not have a race! TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->armorAddons.size() == 0) {
    SKSE::log::warn("\t\tThe skin [0x{:x}] does not have any arma! TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (auto rg = Rg(RgKey(race)); !rg) {
    SKSE::log::critical("\t\tSkin [xx{:x}] from file [{}] together with race [xx{:x}] from file [{}] caused a critical error!", skin->GetLocalFormID(),
                        skin->GetFile() ? skin->GetFile()->GetFilename() : "Unknown", race->GetLocalFormID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unknown");
    return nullptr;
  } else if (rg->addonIdx == Common::nul || rg->malAddons.size() == 0) {
    return skin;
  } else {
    if (name) SKSE::log::info("\t\tThe skin [0x{:x}: {}] added as extra skin.", skin->GetFormID(), name);
    return GetSkinWithAddonForRg(rg, skin, rg->addonIdx, false);
  }
}

std::pair<int, bool> Core::GetApplicableAddon(RE::Actor* const actor) const {
  int addonIdx{Common::def};
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return {addonIdx, false};
  auto savedAddon = Inis::GetActorAddon(actor, npc);
  auto list = GetActorAddons(actor, true);
  if (!savedAddon.second.empty()) {
    addonIdx = savedAddon.second == Common::nulStr ? Common::nul : AddonIdxByLoc(npc->IsFemale(), savedAddon);
    if (addonIdx >= 0 && std::ranges::find_if(list, [&](const auto& pair) { return pair.first == static_cast<size_t>(addonIdx); }) != list.end()) {
      SKSE::log::debug("The addon [0x{:x}] from file [{}] was restored for actor [0x{:x}: {}]", savedAddon.first, savedAddon.second, actor->GetFormID(), npc->GetName());
      return {addonIdx, true};
    } else if (addonIdx == Common::nul) {
      SKSE::log::debug("The actor [0x{:x}: {}] was restored to have no addon", actor->GetFormID(), npc->GetName());
      return {Common::nul, true};
    } else {
      SKSE::log::error("The addon [0x{:x}] from file [{}] could not be reused for actor [0x{:x}: {}]", savedAddon.first, savedAddon.second, actor->GetFormID(), npc->GetName());
    }
  }
  const auto count = list.size();
  const auto malChance = static_cast<size_t>(std::floor(boolSettings.Get(Common::bsRandomizeMaleAddon) * floatSettings.Get(Common::fsMalRndChance) + 0.1f));
  const auto femChance = static_cast<size_t>(std::floor(floatSettings.Get(Common::fsFemRndChance) + 0.1f));
  const size_t chance = static_cast<size_t>(npc->IsFemale() ? femChance : malChance);
  if (count == 0 || chance == 0) return {addonIdx, false};
  if (npc->GetFormID() % 100 < chance) addonIdx = static_cast<int>(list[npc->GetFormID() % count].first);
  return {addonIdx, false};
}

Common::eRes Core::SetNPCAddon(RE::TESNPC* const npc, const int addonIdx, const bool isUser) {
  if (addonIdx < Common::def) return Common::errAddon;
  if (!npc || !npc->race || !npc->race->skin) {
    SKSE::log::critical("Failure in setting a NPC genital!");
    return Common::errNPC;
  }
  auto res = (npc->IsPlayer() && boolSettings.Get(Common::bsExcludePlayerSize)) || addonIdx == Common::nul ? Common::resOkFixed : Common::resOkSizable;
  npc->RemoveKeyword(ut->Key(Common::kyExcluded));
  if (addonIdx == Common::def && !npc->skin) return !npc->IsFemale() ? res : Common::resOkFixed;
  auto& npcSkin = npc->skin;
  auto& raceSkin = npc->race->skin;
  auto& activeSkin = npcSkin ? npcSkin : raceSkin;
  bool skinHasRace = false;
  for (auto& aa : activeSkin->armorAddons) {
    if (aa->IsValidRace(npc->race) && aa->HasPartOf(Common::bodySlot)) {
      skinHasRace = true;
      break;
    }
  }
  if (!skinHasRace) activeSkin = npc->race->skin;
  auto activeOgSkin = GetOgSkin(activeSkin);
  if (activeOgSkin && activeOgSkin->HasKeyword(ut->Key(Common::kyCovering))) {
    activeOgSkin->RemoveKeyword(ut->Key(Common::kyCovering));
    activeOgSkin->AddKeyword(ut->Key(Common::kyIgnored));
    SKSE::log::info("The skin [0x{:x}] was updated accordingly", activeOgSkin->formID);
  }
  auto rg = Rg(RgKey(npc->race));
  if (!rg) return Common::errRg;
  auto& list = npc->IsFemale() ? rg->femAddons : rg->malAddons;
  if (addonIdx >= 0 && list.find(addonIdx) == list.end()) return Common::errAddon;
  if (addonIdx == Common::def && npc->IsFemale()) {
    OrganizeNPCKeywords(npc, addonIdx, false);
    if (npcSkin && npcSkin->HasKeyword(ut->Key(Common::kyTngSkin))) npc->skin = activeOgSkin == raceSkin ? nullptr : activeOgSkin;
    return Common::resOkFixed;
  }
  auto addonChoice = addonIdx == Common::def ? rg->addonIdx : addonIdx;
  OrganizeNPCKeywords(npc, addonIdx, isUser);
  auto resSkin = addonChoice == Common::nul ? activeOgSkin : GetSkinWithAddonForRg(rg, activeOgSkin, addonChoice, npc->IsFemale());
  if (resSkin != npcSkin) {
    npc->skin = resSkin == npc->race->skin ? nullptr : resSkin;
  }
  return !npc->IsFemale() || npc->HasKeyword(ut->Key(Common::kyGentlewoman)) ? res : Common::resOkFixed;
}

void Core::OrganizeNPCKeywords(RE::TESNPC* const npc, int addonIdx, const bool isUser) const {
  npc->ForEachKeyword([&](RE::BGSKeyword* lKw) {
    if (lKw->formEditorID.contains("TNG_ActorAddn")) {
      npc->RemoveKeyword(lKw);
      return RE::BSContainer::ForEachResult::kStop;
    }
    return RE::BSContainer::ForEachResult::kContinue;
  });
  if (addonIdx == Common::nul) {
    npc->AddKeyword(ut->Key(Common::kyExcluded));
  } else if (addonIdx >= 0) {
    const std::string reqKw = (isUser ? Common::cNPCUserAddon : Common::cNPCAutoAddon) + (addonIdx < 10 ? "0" + std::to_string(addonIdx) : std::to_string(addonIdx));
    auto kw = ut->ProduceOrGetKw(reqKw);
    if (!kw) SKSE::log::critical("Keyword generation routine failed with keyword {}", reqKw);
    npc->AddKeyword(kw);
  }
  if (npc->IsFemale()) {
    if (addonIdx >= 0 && femAddons[addonIdx].first->HasKeyword(ut->Key(Common::kySkinWP))) {
      npc->AddKeyword(ut->Key(Common::kyGentlewoman));
    } else {
      npc->RemoveKeyword(ut->Key(Common::kyGentlewoman));
    }
  }
}

void Core::UpdateAddon(RE::Actor* const actor, const bool isRRace) {
  SetActorSize(actor, Common::nul, false);
  if (isRRace) return;
  int currIdx;
  bool isAuto = true;
  auto addonRes = GetActorAddon(actor, currIdx, isAuto);
  if (addonRes < 0 && addonRes != Common::err40) return;
  if (!isAuto) {
    if (addonRes == Common::err40) SetActorAddon(actor, currIdx, true, false);
    return;
  }
  auto [autoIdx, isSaved] = GetApplicableAddon(actor);
  if (autoIdx == Common::def) {
    if (addonRes == Common::err40) SetActorAddon(actor, currIdx, false, false);
    return;
  }
  if (addonRes == Common::err40 || currIdx != autoIdx) SetActorAddon(actor, autoIdx, isSaved, false);
}

Common::eRes Core::UpdatePlayer(RE::Actor* const actor, const bool isRRace) {
  if (auto pcInfo = GetPlayerInfo(actor, false); pcInfo) {
    SetActorSize(actor, pcInfo->sizeCat, false);
    if (isRRace) return Common::resOkFixed;
    auto addonIdx = AddonIdxByLoc(pcInfo->isFemale, pcInfo->addon);
    return SetActorAddon(actor, addonIdx, true, false);
  } else {
    return SetActorAddon(actor, Common::def, false, false);
  }
}

void Core::UpdateFormLists(RE::Actor* const actor) const {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return;
  auto list = ut->FormList(npc->IsFemale() ? Common::flmGentleWomen : Common::flmNonGentleMen);
  auto key = ut->Key(npc->IsFemale() ? Common::kyGentlewoman : Common::kyExcluded);
  if (!list || !key) {
    SKSE::log::critical("TNG faced an error when trying to update the form lists. Keyword available: {}. Form list available: {}.", key != nullptr, list != nullptr);
    return;
  }
  ut->UpdateFormList(list, actor, npc->HasKeyword(key));
}

void Core::UpdateBlock(RE::Actor* const actor, RE::TESObjectARMO* const armor, const bool isEquipped) const {
  if (!actor) return;
  static bool showErrMessage = true;
  auto down = armor && isEquipped && armor->HasPartOf(Common::genitalSlot) ? armor : actor->GetWornArmor(Common::genitalSlot);
  if (down && down == armor && !isEquipped) down = nullptr;
  auto hasCover = armor && isEquipped && !armor->HasPartOf(Common::genitalSlot) ? true : ut->HasCovering(actor, isEquipped ? nullptr : armor);
  if (!NeedsBlock(actor) || (down && (!ut->IsBlock(down) || !hasCover))) {
    actor->RemoveItem(ut->Block(), 10, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
    return;
  }
  if ((hasCover && down) || (!hasCover && !down)) return;
  auto tngBlock = ut->Block();
  if (!tngBlock) {
    if (showErrMessage) {
      showErrMessage = false;
      ut->ShowSkyrimMessage("TNG faced an error when trying to cover genitalia. The New Gentleman won't function properly!");
    }
    return;
  }
  actor->AddObjectToContainer(tngBlock, nullptr, 1, nullptr);
  RE::ActorEquipManager::GetSingleton()->EquipObject(actor, tngBlock);
}

bool Core::NeedsBlock(RE::Actor* const actor) const {
  if (CanModifyActor(actor) < 0) return false;
  auto npc = actor->GetActorBase();
  if (!npc) return false;
  auto skin = actor->GetSkin();
  return skin && skin->HasPartOf(Common::genitalSlot) && (!npc->IsFemale() || npc->HasKeyword(ut->Key(Common::kyGentlewoman)));
}

void Core::CheckArmorPieces() {
  SKSE::log::info("Checking ARMO records...");
  auto& armorList = ut->SEDH()->GetFormArray<RE::TESObjectARMO>();
  auto defRace = ut->Race(Common::raceDefault);
  int logInfo[7]{0};
  std::map<RE::TESObjectARMO*, std::string> potentialArmor = {};
  std::set<std::string> potentialMods{};
  std::set<std::string> potentialSlot52Mods{};
  std::vector<RE::BGSKeyword*> armorKeys = ut->Keys(Common::kyRevealingF, Common::kyRevealing);
  armorKeys.push_back(ut->Key(Common::kyIgnored));
  std::vector<RE::BGSKeyword*> relRaceKeys{ut->Key(Common::kyProcessed), ut->Key(Common::kyPreProcessed), ut->Key(Common::kyReady)};
  for (auto covRec : hardCodedCovering) {
    auto armor = ut->SEDH()->LookupForm<RE::TESObjectARMO>(covRec.first, covRec.second);
    if (!armor) continue;
    armor->RemoveKeywords(armorKeys);
    armor->AddKeyword(ut->Key(Common::kyCovering));
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
      armor->AddKeyword(ut->Key(Common::kyIgnored));
      logInfo[6]++;
      continue;
    }
    const auto armorID = (std::string(armor->GetName()).empty()) ? armor->GetFormEditorID() : armor->GetName();
    std::string modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (!armor->race) {
      armor->AddKeyword(ut->Key(Common::kyIgnored));
      logInfo[6]++;
      continue;
    }
    if (defRace && !armor->race->HasKeywordInArray(relRaceKeys, false) && armor->race != defRace) {
      logInfo[6]++;
      continue;
    }
    if (IsSkin(armor, modName)) {
      SKSE::log::info("\t\tThe record [0x{:x}: {}] was marked as a skin.", armor->GetFormID(), armorID);
      armor->AddKeyword(ut->Key(Common::kyIgnored));
      logInfo[6]++;
      continue;
    }
    bool has52 = armor->HasPartOf(Common::genitalSlot) ||
                 std::any_of(armor->armorAddons.begin(), armor->armorAddons.end(), [](const auto& aa) { return aa && aa->HasPartOf(Common::genitalSlot); });
    if (!has52) {
      if (auto status = HasStatus(armor); status < Common::keywordsCount) {
        if (!ut->Key(status)) {
          SKSE::log::error("TNG cannot load its own keywords! Make sure the esp is active in the load order.");
          logInfo[0]++;
          continue;
        }
        armor->AddKeyword(ut->Key(status));
        SKSE::log::info("\t\tThe armor [0x{:x}: {}] was marked with [{}] keyword.", armor->GetFormID(), armorID, ut->Key(status)->GetFormEditorID());
        (status == Common::kyCovering) ? logInfo[2]++ : logInfo[3]++;
        continue;
      }
    } else {
      armor->RemoveKeywords(armorKeys);
      armor->AddKeyword(ut->Key(Common::kyIgnored));
      logInfo[2]++;
      if (armor->HasPartOf(Common::bodySlot)) continue;
      if (modName != "") potentialSlot52Mods.insert(modName);
      SKSE::log::info("\t\tThe armor [0x{:x}] would cover genitals!", armor->GetFormID());
      continue;
    }
    if (armor->HasPartOf(Common::bodySlot)) {
      if (modName != "") potentialMods.insert(std::string{modName});
      potentialArmor.insert({armor, modName});
    } else {
      logInfo[6]++;
    }
  }
  for (auto entry = potentialSlot52Mods.begin(); entry != potentialSlot52Mods.end(); entry++) {
    if (potentialMods.find(*entry) != potentialMods.end()) Slot52ModBehavior(*entry, boolSettings.Get(Common::bsRevealSlot52Mods));
  }
  for (auto& armorPair : potentialArmor) {
    auto b = Slot52ModBehavior(armorPair.second);
    armorPair.first->AddKeyword(ut->Key(b ? Common::kyRevealing : Common::kyCovering));
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
