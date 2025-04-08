#include <Core.h>

Core* core = Core::GetSingleton();

void Core::Init() {
  rgInfoList.emplace_back();
  auto& rg0 = rgInfoList[0];
  rg0.name = "TNGRg0";
  rg0.isMain = true;
  rg0.noMCM = true;
  LoadAddons();
}

void Core::LoadAddons() {
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
}

size_t Core::GetAddonCount(const bool isFemale, const bool onlyActive) const {
  auto& list = isFemale ? femAddons : malAddons;
  if (onlyActive) {
    size_t res = 0;
    for (const auto& addonPair : list)
      if (addonPair.second) res++;
    return res;
  } else {
    return list.size();
  }
}

bool Core::GetAddonStatus(const bool isFemale, const size_t addonIdx) const {
  auto& list = isFemale ? femAddons : malAddons;
  if (addonIdx >= list.size()) return false;
  return list[addonIdx].second;
}

void Core::SetAddonStatus(const bool isFemale, const size_t addonIdx, const bool status) {
  auto& list = isFemale ? femAddons : malAddons;
  if (addonIdx >= list.size()) return;
  list[addonIdx].second = status;
  Inis::SetAddonStatus(isFemale, list[addonIdx].first, status);
}

const std::string Core::GetAddonName(const bool isFemale, const size_t addonIdx) const {
  const auto& list = isFemale ? femAddons : malAddons;
  return addonIdx < list.size() && list[addonIdx].first ? list[addonIdx].first->GetName() : "";
}

RE::TESObjectARMO* const Core::GetAddonForActor(RE::Actor* const actor, const int addonIdx) const {
  if (addonIdx < 0) return nullptr;
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return nullptr;
  return npc->IsFemale() ? femAddons[static_cast<size_t>(addonIdx)].first : malAddons[static_cast<size_t>(addonIdx)].first;
}

int Core::GetAddonIdxByLoc(const bool isFemale, const SEFormLocView addonLoc) const {
  auto& list = isFemale ? femAddons : malAddons;
  for (int i = 0; i < list.size(); i++) {
    if (ut->FormToLoc(list[i].first) == addonLoc) return i;
  }
  return Common::nan;
}

void Core::ProcessRaces() {
  SKSE::log::info("Finding the genitals for relevant races...");
  const auto& allRaces = ut->SEDH()->GetFormArray<RE::TESRace>();
  int logInfo[4] = {0, 0, 0, 0};
  std::vector<RE::BGSKeyword*> keywords = {ut->Key(Common::kyIgnored), ut->Key(Common::kyReady), ut->Key(Common::kyProcessed), ut->Key(Common::kyPreProcessed)};
  validSkeletons.emplace(ut->Race(Common::raceDefault)->skeletonModels[0].model.data());
  validSkeletons.emplace(ut->Race(Common::raceDefault)->skeletonModels[1].model.data());
  validSkeletons.emplace(ut->Race(Common::raceDefBeast)->skeletonModels[0].model.data());
  validSkeletons.emplace(ut->Race(Common::raceDefBeast)->skeletonModels[1].model.data());
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
  SKSE::log::info("\tProcessed [{}] races: assigned genitalia to [{}] races, preprocessed [{}] races, found [{}] races to be ready and ignored [{}] races.", allRaces.size(),
                  logInfo[2], logInfo[3], logInfo[1], logInfo[0]);
}

std::vector<std::string> Core::GetRgNames() const {
  std::vector<std::string> res{};
  for (auto& rg : rgInfoList) {
    if (boolSettings.Get(Common::bsShowAllRaces) || !rg.noMCM) res.push_back(rg.name);
  }
  return res;
}

bool Core::RgIsMain(RgKey rgChoice) const {
  auto rg = rgChoice.Get();
  if (!rg) return false;
  return rg->isMain;
}

int Core::GetRgAddon(RgKey rgChoice) const {
  auto rg = rgChoice.Get();
  if (!rg) return Common::err40;
  return rg->addonIdx;
}

void Core::SetRgAddon(RgKey rgChoice, const int addonIdx) {
  auto rg = rgChoice.Get();
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
  auto rg = rgChoice.Get();
  return rg ? rg->mult : Common::fErr;
}

void Core::SetRgMult(RgKey rgChoice, const float mult) {
  auto rg = rgChoice.Get();
  if (!rg || mult < 0.1f || mult >= 10.0f) {
    SKSE::log::critical("\tFailure in setting a race mult!");
    SKSE::log::debug("\t\tThe race index was {} and the mult was {}", rgChoice, mult);
    return;
  }
  rg->mult = mult;
  Inis::SetRgMult(rg->races[0], mult);
}

const std::string Core::GetRgInfo(RgKey rgChoice) const {
  std::string res{""};
  auto rg = rgChoice.Get();
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
  auto rg = rgChoice.Get();
  if (!rg) return res;
  for (auto& addonPair : rg->malAddons) res.push_back({addonPair.first, addonPair.second.first});
  return res;
}

bool Core::ReevaluateRace(RE::TESRace* const race, RE::Actor* const actor) {
  if (!actor || !race || raceRgs.find(race) == raceRgs.end()) return false;
  SKSE::log::debug("Re-evaluating race [0x{:x}:{}] ...", race->GetFormID(), race->GetFormEditorID());
  if (!actor->Is3DLoaded()) return false;
  auto rg = raceRgs[race];
  bool isInvalid = false;
  for (auto& boneName : Common::genBoneNames)
    if (!actor->GetNodeByName(boneName)) {
      isInvalid = true;
      SKSE::log::info("\tTNG would neglect the race [0x{:x}:{}] since its skeleton is missing the bone [{}]", race->GetFormID(), race->GetFormEditorID(), boneName);
    }
  if (!isInvalid) {
    if (race->skeletonModels[0].model.empty() || race->skeletonModels[1].model.empty()) {
      SKSE::log::info("\tTNG would neglect the race [0x{:x}:{}] since its skeleton could not be recognized.", race->GetFormID(), race->GetFormEditorID());
      isInvalid = true;
    }
  }
  if (!isInvalid) {
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
    raceRgs.erase(race);
    rg->races.erase(it);
    auto rgIt = std::find(rgInfoList.begin(), rgInfoList.end(), *rg);
    if (rgIt != rgInfoList.end() && rg->races.empty()) rgInfoList.erase(rgIt);
  }
  return true;
}

void Core::IgnoreRace(RE::TESRace* const race, bool ready) {
  if (!race) return;
  if (auto& skin = race->skin; skin) skin->AddKeyword(ut->Key(Common::kyIgnored));
  race->RemoveKeywords(ut->Keys(Common::kyProcessed, Common::kyExcluded));
  race->AddKeyword(ut->Key(ready ? Common::kyReady : Common::kyIgnored));
}

Common::RaceGroupInfo* const Core::ProcessRace(RE::TESRace* const race) {
  if (raceRgs.find(race) != raceRgs.end()) return raceRgs[race];
  switch (CheckRace(race)) {
    case Common::resOkRacePP:
      return AddRace(race, false);
    case Common::resOkRaceP:
      return AddRace(race, true);
    case Common::resOkRaceR:
      break;
    case Common::errRace:
      break;
    default:
      break;
  }
  return nullptr;
}

Common::eRes Core::CheckRace(RE::TESRace* const race) {
  // TODO: Can probably become const
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
      IgnoreRace(race, false);
      return Common::errRace;
    }
    bool isVanilla = race->GetFile(0) && race->GetFile(0)->GetFilename() == Common::skyrimFile;
    if (race->HasPartOf(Common::genitalSlot) && !isVanilla) {
      auto ready = race->skin->HasPartOf(Common::genitalSlot);
      SKSE::log::info("\tThe race [{}] is designed to be {} TNG. It was not modified.", race->GetFormEditorID(), ready ? "ready for" : "ignored by");
      IgnoreRace(race, ready);
      return ready ? Common::resOkRaceR : Common::errRace;
    }
  } catch (const std::exception& er) {
    SKSE::log::warn("\tThe race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(),
                    race->GetFormEditorID(), er.what());
    const char* message = fmt::format("TheNewGentleman: The race [0x{:x}: {}] caused an error [{}]. TNG tries to ignore it but it might cause other problems!", race->GetFormID(),
                                      race->GetFormEditorID(), er.what())
                              .c_str();
    ut->ShowSkyrimMessage(message);
    IgnoreRace(race, false);
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
        raceRgs.insert({race, &rg});
        SKSE::log::info("\tThe race [0x{:x}: {}] was recognized as a member of existing group {}.", race->GetFormID(), race->GetFormEditorID(), rg.name);
        return &rg;
      }
    }
    if (race->armorParentRace->skin == race->skin) {
      SKSE::log::debug("\t\t...processing parent race [0x{:x}: {}]", race->armorParentRace->GetFormID(), race->armorParentRace->GetFormEditorID());
      if (auto rgPtr = ProcessRace(race->armorParentRace); rgPtr) {
        rgPtr->races.push_back(race);
        raceRgs.insert({race, rgPtr});
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
  raceRgs.insert({race, &rg});
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
  if (auto idx = GetAddonIdxByLoc(false, defAddon); idx >= 0) {
    return idx;
  }
  SKSE::log::critical("TNG faced an error getting the default addon for a race group.");
  return Common::nul;
}

void Core::ProcessRgAddons(Common::RaceGroupInfo& rg, const std::vector<std::pair<RE::TESObjectARMO* const, bool>>& addons, const bool isFemale) {
  auto& rgAddons = isFemale ? rg.femAddons : rg.malAddons;
  const char* gender = isFemale ? "women" : "men";
  for (size_t i = 0; i < addons.size(); i++) {
    auto& addon = addons[i].first;
    bool supports = false;
    for (const auto& aa : addon->armorAddons) {
      if (aa->IsValidRace(rg.armorRace)) {
        std::set<RE::TESRace*> aaRaces{aa->race};
        if (rg.isMain) {
          if (aa->additionalRaces.size() > 0) aaRaces.insert(aa->additionalRaces.begin(), aa->additionalRaces.end());
          for (auto& race : aaRaces) {
            if (raceRgs.find(race) != raceRgs.end() && !raceRgs[race]->isMain) aaRaces.erase(raceRgs[race]->armorRace);
          }
          if (aaRaces.find(rg.armorRace) == aaRaces.end()) continue;
        }
        supports = true;
        auto isDedicated = aaRaces.find(rg.races[0]) != aaRaces.end();
        rgAddons.insert_or_assign(i, std::make_pair(isDedicated, aa));
        if (isDedicated) break;
      }
    }
    if (!supports) {
      if (i == rg.defAddonIdx) rg.defAddonIdx = rg.malAddons.size() == 0 ? Common::nul : static_cast<int>(rg.malAddons.begin()->first);
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
      auto index = GetAddonIdxByLoc(false, addonLoc);
      if (index >= 0 && rg.malAddons.find(static_cast<size_t>(index)) != rg.malAddons.end()) {
        rg.addonIdx = index;
        SKSE::log::debug("\tRestored group [{}] addon to [0x{:x}] from file [{}]!", rg.name, addonLoc.first, addonLoc.second);
      } else {
        SKSE::log::debug("\tThe addon [0x{:x}] from file [{}] could not be used for group [{}]!", addonLoc.first, addonLoc.second, rg.name);
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
    ApplyUserSettings(npc);
    if (npc->IsFemale()) continue;
    sizeCount[npc->formID % Common::sizeCatCount]++;
    if (!skin) continue;
    if (skin->HasKeyword(ut->Key(Common::kyTngSkin))) continue;
    skinsToPatch[{skin, race}].insert(npc);
  }
  for (auto& racePair : raceNPCCount) {
    if (racePair.second < 5) continue;
    if (!racePair.first || raceRgs.find(racePair.first) == raceRgs.end()) continue;
    if (auto rg = raceRgs[racePair.first]; rg && rg->malAddons.size() > 0) rg->noMCM = false;
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

Common::eRes Core::CanModifyActor(RE::Actor* const actor) const {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Common::errNPC;
  if (!npc->race) return Common::errRace;
  if (IsNPCExcluded(npc)) return Common::errNPC;
  if (npc->skin && npc->skin->HasPartOf(Common::genitalSlot) && !npc->skin->HasKeyword(ut->Key(Common::kyTngSkin))) return Common::errSkin;
  if (npc->race->HasKeyword(ut->Key(Common::kyReady))) return Common::resOkRaceR;
  if (raceRgs.find(npc->race) == raceRgs.end() || raceRgs.at(npc->race)->malAddons.size() == 0) return Common::errRace;
  if (npc->race->HasKeyword(ut->Key(Common::kyProcessed))) return Common::resOkRaceP;
  if (npc->race->HasKeyword(ut->Key(Common::kyPreProcessed))) return Common::resOkRacePP;
  return Common::errRace;
}

void Core::UpdateActor(RE::Actor* const actor, RE::TESObjectARMO* const armor, const bool isEquipped) {
  UpdateAddon(actor);
  UpdateCover(actor, armor, isEquipped);
}

std::vector<std::pair<size_t, bool>> Core::GetActorAddons(RE::Actor* const actor, const bool onlyActive) const {
  std::vector<std::pair<size_t, bool>> res{};
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return res;
  if (auto rg = RgKey(npc->race).Get(); rg) {
    auto& list = npc->IsFemale() ? rg->femAddons : rg->malAddons;
    auto& master = npc->IsFemale() ? femAddons : malAddons;
    for (auto& addonPair : list)
      if (!onlyActive || master[addonPair.first].second) res.push_back({addonPair.first, addonPair.second.first});
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
  auto rg = RgKey(npc->race).Get();
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
  if (actor && actor->IsPlayerRef() && choice == Common::defPlayer) return UpdatePlayer(actor);
  const auto npc = actor->GetActorBase();
  auto list = GetActorAddons(actor, !isUser);
  if (shouldSave) {
    if (choice >= static_cast<int>(list.size())) return Common::errAddon;
    SKSE::log::debug("Setting addon [{}] for actor [0x{:x}:{}].", choice, actor->GetFormID(), npc->GetName());
  } else {
    if (choice >= 0 && std::ranges::find(list, std::make_pair(static_cast<size_t>(choice), !isUser)) == list.end()) return Common::errAddon;
  }
  auto addonIdx = choice < 0 ? choice : shouldSave ? static_cast<int>(list[choice].first) : choice;
  auto res = SetNPCAddon(npc, addonIdx, isUser);
  if (res < 0) return res;
  auto addon = addonIdx < 0 ? nullptr : (npc->IsFemale() ? femAddons[addonIdx].first : malAddons[addonIdx].first);
  if (actor->IsPlayerRef()) SetPlayerInfo(actor, addon, addonIdx);
  if (!npc->IsPlayer() && shouldSave) {
    auto saved = Inis::SetNPCAddon(npc, addon, addonIdx);
    if (!saved) Inis::SetActorAddon(actor, addon, addonIdx);
  }
  return res;
}

Common::eRes Core::GetActorSize(RE::Actor* const actor, int& sizeCat) const {
  sizeCat = Common::nan;
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (auto res = CanModifyActor(actor); res < 0) return res;
  if (npc->IsPlayer() && boolSettings.Get(Common::bsExcludePlayerSize)) return Common::errPlayer;
  if (sizeCat = Inis::GetActorSize(actor); sizeCat >= 0) return Common::resOkSizable;
  for (size_t i = 0; i < Common::sizeCatCount; i++) {
    if (npc->HasKeyword(ut->SizeKey(i))) sizeCat = static_cast<int>(i);
  }
  if (sizeCat < 0) sizeCat = npc->formID % Common::sizeCatCount;
  return Common::resOkSizable;
}

Common::eRes Core::SetActorSize(RE::Actor* const actor, int sizeCat, bool shouldSave) {
  if (shouldSave) {
    if (CanModifyActor(actor) == Common::resOkRacePP) ReevaluateRace(actor->GetRace(), actor);
    if (auto res = CanModifyActor(actor); res < 0) return res;
  }
  const auto npc = actor->GetActorBase();
  if (npc->IsPlayer()) return Common::resOkFixed;  // Don't change the size for copies of player actor
  int currCat = Common::nan;
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Common::errNPC;
  if (npc->IsPlayer() && !actor->IsPlayerRef()) return Common::resOkFixed;  // Don't change the size for copies of player actor
  if (sizeCat == Common::def) npc->RemoveKeywords(ut->SizeKeys());
  auto res = GetActorSize(actor, currCat);
  if (res != Common::resOkSizable) return res;
  auto cat = (sizeCat < 0) ? currCat : sizeCat;
  if (cat != currCat) {
    npc->RemoveKeywords(ut->SizeKeys());
    npc->AddKeyword(ut->SizeKey(cat));
  }
  if (auto race = actor->GetRace(); race, raceRgs.find(race) != raceRgs.end()) {
    auto mult = raceRgs[race]->mult;
    if (mult < 0.0f) return Common::errRg;
    auto scale = mult * floatSettings.Get(static_cast<Common::eFloatSetting>(cat));
    if (scale < 0.1) scale = 1;
    std::thread([actor, scale]() {
      int ms = 500;
      int count = 0;
      while (!actor->Is3DLoaded() && count < 10) {
        count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      }
      RE::NiAVObject* baseNode = actor->GetNodeByName(Common::genBoneNames[Common::egbBase]);
      RE::NiAVObject* scrotNode = actor->GetNodeByName(Common::genBoneNames[Common::egbScrot]);
      if (baseNode && scrotNode) {
        baseNode->local.scale = scale;
        scrotNode->local.scale = 1.0f / sqrt(scale);
      } else {
        SKSE::log::error("Failed to scale actor [0x{:x}] genitalia to [{}] since their skeleton was not loaded after {} seconds.", actor->GetFormID(), scale, count * ms / 1000);
      }
    }).detach();
    if (sizeCat >= 0) SKSE::log::debug("Reloaded actor [0x{:x}] genitalia scale to [{}] to be a size category [{}].", actor->GetFormID(), scale, sizeCat);
  }
  if (!actor->GetActorBase()->IsPlayer() && shouldSave) {
    auto saved = Inis::SetNPCSize(actor->GetActorBase(), sizeCat);
    if (!saved) Inis::SetActorSize(actor, sizeCat);
  }
  return res;
}

void Core::UpdateFormLists(RE::Actor* const actor, RE::TESNPC* const npc) const {
  if (npc->IsFemale()) {
    if (npc->HasKeyword(ut->Key(Common::kyGentlewoman)) && !ut->FormList(Common::flmGentleWomen)->HasForm(actor)) {
      ut->FormList(Common::flmGentleWomen)->AddForm(actor);
    } else if (!npc->HasKeyword(ut->Key(Common::kyGentlewoman)) && ut->FormList(Common::flmGentleWomen)->HasForm(actor)) {
      for (RE::BSTArray<RE::TESForm*>::const_iterator it = ut->FormList(Common::flmGentleWomen)->forms.begin(); it < ut->FormList(Common::flmGentleWomen)->forms.end(); it++) {
        if ((*it)->As<RE::Actor>() == actor) ut->FormList(Common::flmGentleWomen)->forms.erase(it);
      }
    }
  }
  if (!npc->IsFemale()) {
    if (npc->HasKeyword(ut->Key(Common::kyExcluded)) && !ut->FormList(Common::flmNonGentleMen)->HasForm(actor)) {
      ut->FormList(Common::flmNonGentleMen)->AddForm(actor);
    } else if (!npc->HasKeyword(ut->Key(Common::kyExcluded)) && ut->FormList(Common::flmNonGentleMen)->HasForm(actor)) {
      for (RE::BSTArray<RE::TESForm*>::const_iterator it = ut->FormList(Common::flmNonGentleMen)->forms.begin(); it < ut->FormList(Common::flmNonGentleMen)->forms.end(); it++) {
        if ((*it)->As<RE::Actor>() == actor) ut->FormList(Common::flmNonGentleMen)->forms.erase(it);
      }
    }
  }
}

RE::TESObjectARMO* Core::FixSkin(RE::TESObjectARMO* const skin, RE::TESRace* const race, const char* const name) {
  skin->RemoveKeywords(ut->Keys(Common::kyRevealingF, Common::kyRevealing));
  skin->AddKeyword(ut->Key(Common::kyIgnored));
  if (!skin->HasPartOf(Common::bodySlot)) {
    SKSE::log::info("\t\tThe skin [0x{:x}] used does not have a body part. TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->HasPartOf(Common::genitalSlot)) {
    SKSE::log::warn("\t\tThe skin [0x{:x}] cannot have a male genital.", skin->GetFormID());
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
  if (raceRgs.find(race) == raceRgs.end()) {
    SKSE::log::critical("\t\tSkin [xx{:x}] from file [{}] together with race [xx{:x}] from file [{}] caused a critical error!", skin->GetLocalFormID(),
                        skin->GetFile() ? skin->GetFile()->GetFilename() : "Unknown", race->GetLocalFormID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unknown");
    return nullptr;
  }
  if (raceRgs[race]->addonIdx == Common::nul) return skin;
  if (name) SKSE::log::info("\t\tThe skin [0x{:x}: {}] added as extra skin.", skin->GetFormID(), name);
  return GetSkinWithAddonForRg(raceRgs[race], skin, raceRgs[race]->addonIdx, false);
}

void Core::ApplyUserSettings(RE::TESNPC* npc) {
  if (auto npcLoc = ut->FormToLoc(npc); !npcLoc.second.empty()) {
    if (npcAddons.find(npcLoc) != npcAddons.end()) {
      auto& addonLoc = npcAddons[npcLoc];
      auto index = GetAddonIdxByLoc(npc->IsFemale(), addonLoc);
      if (SetNPCAddon(npc, index, true) >= 0) {
        SKSE::log::debug("\tRestored the addon of npc [xx{:x}] from file [{}] to addon [xx{:x}] from file [{}]", npcLoc.first, npcLoc.second, addonLoc.first, addonLoc.second);
      } else {
        SKSE::log::debug("\tThe addon [xx{:x}] from file [{}] could not be used for npc [xx{:x}] from file [{}]", addonLoc.first, addonLoc.second, npcLoc.first, npcLoc.second);
      }
    }
    if (npcSizeCats.find(npcLoc) != npcSizeCats.end()) {
      auto sizeCat = npcSizeCats[npcLoc];
      if (sizeCat >= 0 && sizeCat <= Common::sizeCatCount) {
        npc->AddKeyword(ut->SizeKey(sizeCat));
        SKSE::log::debug("\tRestored the size category of npc [xx{:x}] from file [{}] to [{}]", npcLoc.first, npcLoc.second, sizeCat);
      } else {
        SKSE::log::debug("\tThe size category [{}] is out of range for npc [xx{:x}] from file [{}]", sizeCat, npcLoc.first, npcLoc.second);
      }
    }
  }
}

void Core::UpdateAddon(RE::Actor* const actor) {
  if (CanModifyActor(actor) == Common::resOkRacePP) ReevaluateRace(actor->GetRace(), actor);
  if (CanModifyActor(actor) != Common::resOkRaceP) return;
  if (actor->IsPlayerRef()) {
    UpdatePlayer(actor);
    return;
  }
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

void Core::UpdateCover(RE::Actor* const actor, RE::TESObjectARMO* const armor, const bool isEquipped) const {
  if (!actor) return;
  auto down = armor && isEquipped && armor->HasPartOf(Common::genitalSlot) ? armor : actor->GetWornArmor(Common::genitalSlot);
  if (down && down == armor && !isEquipped) down = nullptr;
  auto cover = armor && isEquipped && !armor->HasPartOf(Common::genitalSlot) ? armor : GetCoveringItem(actor, isEquipped ? nullptr : armor);
  bool needsCover = NeedsCover(actor);
  if (!needsCover || (down && (!ut->IsBlock(down) || !cover))) {
    actor->RemoveItem(ut->Block(), 10, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
    return;
  }
  if ((cover && down) || (!cover && !down)) return;
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

std::pair<int, bool> Core::GetApplicableAddon(RE::Actor* const actor) const {
  int addonIdx{Common::def};
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return {addonIdx, false};
  auto savedAddon = Inis::GetActorAddon(actor);
  auto list = core->GetActorAddons(actor, true);
  if (!savedAddon.second.empty()) {
    addonIdx = savedAddon.second == Common::nulStr ? Common::nul : GetAddonIdxByLoc(npc->IsFemale(), savedAddon);
    if (addonIdx >= 0 && std::ranges::find_if(list, [&](const auto& pair) { return pair.first == static_cast<size_t>(addonIdx); }) != list.end()) {
      SKSE::log::debug("The addon [0x{:x}] from file [{}] was restored for actor [0x{:x}: {}]", savedAddon.first, savedAddon.second, actor->GetFormID(), npc->GetName());
      return {addonIdx, true};
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
  if (!npc) {
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
  auto ogSkin = GetOgSkin(npc->skin);
  auto raceOgSkin = GetOgSkin(npc->race->skin);
  auto& activeOgSkin = !ogSkin ? raceOgSkin : ogSkin;
  if (ogSkin && ogSkin->HasKeyword(ut->Key(Common::kyCovering))) {
    ogSkin->RemoveKeyword(ut->Key(Common::kyCovering));
    ogSkin->AddKeyword(ut->Key(Common::kyIgnored));
    SKSE::log::info("The skin [0x{:x}] was updated accordingly", ogSkin->formID);
  }
  if (raceRgs.find(npc->race) == raceRgs.end()) return Common::errRg;
  auto& rg = raceRgs[npc->race];
  auto& list = npc->IsFemale() ? rg->femAddons : rg->malAddons;
  if (addonIdx >= 0 && list.find(addonIdx) == list.end()) return Common::errAddon;
  if (addonIdx == Common::def && npc->IsFemale()) {
    OrganizeNPCAddonKeywords(npc, addonIdx, false);
    if (npcSkin && npcSkin->HasKeyword(ut->Key(Common::kyTngSkin))) npc->skin = ogSkin == raceOgSkin ? nullptr : ogSkin;
    return Common::resOkFixed;
  }
  auto addonChoice = addonIdx == Common::def ? rg->addonIdx : addonIdx;
  OrganizeNPCAddonKeywords(npc, addonIdx, isUser);
  auto resSkin = addonChoice == Common::nul ? (npc->IsFemale() ? raceSkin : activeOgSkin) : GetSkinWithAddonForRg(rg, activeOgSkin, addonChoice, npc->IsFemale());
  if (resSkin != npcSkin && !(!npcSkin && resSkin == raceSkin)) {
    npc->skin = resSkin == npc->race->skin ? nullptr : resSkin;
  }
  return !npc->IsFemale() || npc->HasKeyword(ut->Key(Common::kyGentlewoman)) ? res : Common::resOkFixed;
}

void Core::OrganizeNPCAddonKeywords(RE::TESNPC* const npc, int addonIdx, bool isUser) const {
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
  if (npc->IsFemale() && addonIdx >= 0) {
    if (femAddons[addonIdx].first->HasKeyword(ut->Key(Common::kySkinWP))) {
      npc->AddKeyword(ut->Key(Common::kyGentlewoman));
    } else {
      npc->RemoveKeyword(ut->Key(Common::kyGentlewoman));
    }
  }
}

Common::eRes Core::UpdatePlayer(RE::Actor* const actor) {
  auto player = actor ? actor->GetActorBase() : nullptr;
  if (!player || !actor->IsPlayerRef()) return Common::errNPC;
  // TODO: Update player size and addon
  // if (!core->GetBoolSetting(Common::bsExcludePlayerSize)) core->SetActorSize(actor, Common::nul);
}

void Core::CheckArmorPieces() {
  SKSE::log::info("Checking ARMO records...");
  auto& armorList = ut->SEDH()->GetFormArray<RE::TESObjectARMO>();
  int logInfo[7]{0};
  std::map<std::string, RE::TESObjectARMO*> potentialArmor = {};
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
    if (!armor->race->HasKeywordInArray(relRaceKeys, false) && armor->race != ut->Race(Common::raceDefault)) continue;
    if (IsSkin(armor, modName)) {
      SKSE::log::info("\t\tThe record [0x{:x}: {}] was marked as a skin.", armor->GetFormID(), armorID);
      armor->AddKeyword(ut->Key(Common::kyIgnored));
      logInfo[6]++;
      continue;
    }
    bool has52 = armor->HasPartOf(Common::genitalSlot);
    size_t i = 0;
    while (!has52 && i < armor->armorAddons.size()) {
      auto aa = armor->armorAddons[i];
      has52 = aa && aa->HasPartOf(Common::genitalSlot);
      i++;
    }
    if (!has52) {
      if (auto status = HasStatus(armor); status < Common::keywordsCount) {
        if (!ut->Key(status)) {
          SKSE::log::error("TNG cannot load its own keywords! Make sure the esp is active in the load order.");
          continue;
        }
        armor->AddKeyword(ut->Key(status));
        SKSE::log::info("\t\tThe armor [0x{:x}: {}] was marked with [{}] keyword.", armor->GetFormID(), armorID, ut->Key(status)->GetFormEditorID());
        (status == Common::kyCovering) ? logInfo[2]++ : logInfo[3]++;
      }
      if (IsExtraRevealing(modName)) {
        armor->AddKeyword(ut->Key(Common::kyRevealing));
        SKSE::log::info("\t\tArmor [0x{:x}: {}] was marked revealing since it is in a mod with slot 52 items.", armor->GetFormID(), armorID);
        logInfo[5]++;
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
      potentialArmor.insert({modName, armor});
    } else {
      logInfo[6]++;
    }
  }
  for (auto entry = potentialSlot52Mods.begin(); entry != potentialSlot52Mods.end();) {
    if (potentialMods.find(*entry) == potentialMods.end()) {
      entry = potentialSlot52Mods.erase(entry);
    } else {
      Process52(*entry);
      ++entry;
    }
  }
  for (auto& modName : potentialSlot52Mods)
    for (auto& armorPair : potentialArmor) {
      if (IsExtraRevealing(armorPair.first)) {
        armorPair.second->AddKeyword(ut->Key(Common::kyRevealing));
        logInfo[5]++;
      } else {
        armorPair.second->AddKeyword(ut->Key(Common::kyCovering));
        logInfo[4]++;
      }
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

bool Core::SwapRevealing(RE::Actor* const actor, RE::TESObjectARMO* const armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (armor->HasKeyword(ut->Key(Common::kyUnderwear))) return false;
  std::vector<RE::BGSKeyword*> rcKeys = {ut->Key(Common::kyCovering), ut->Key(Common::kyRevealingM), ut->Key(Common::kyRevealingF), ut->Key(Common::kyRevealing)};
  auto kb = ut->HasKeywordInList(armor, rcKeys);
  int mask = npc->IsFemale() ? 2 : 1;
  if (kb < 0) {
    armor->RemoveKeyword(ut->Key(Common::kyIgnored));
    armor->AddKeyword(ut->Key(Common::kyCovering));
    SetArmorStatus(armor, Common::kyCovering);
  } else {
    armor->RemoveKeyword(rcKeys[kb]);
    if (armor->HasPartOf(Common::bodySlot)) armor->AddKeyword(rcKeys[kb ^ mask]);
  }
  return true;
}

void Core::RevisitRevealingArmor() const {
  std::set<std::string> potentialMods = {Slot52Mods().begin(), Slot52Mods().end()};
  if (potentialMods.size() == 0) return;
  auto& armorList = ut->SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  std::vector<RE::BGSKeyword*> rc = {ut->Key(Common::kyCovering), ut->Key(Common::kyRevealing)};
  for (const auto& armor : armorList) {
    if (!armor || !armor->HasPartOf(Common::bodySlot) || !armor->HasKeywordInArray(rc, false)) continue;
    auto modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (modName.empty() || potentialMods.find(modName) == potentialMods.end()) continue;
    if (IsExtraRevealing(modName) == armor->HasKeyword(ut->Key(Common::kyRevealing))) continue;
    armor->RemoveKeywords(rc);
    armor->AddKeyword(ut->Key(IsExtraRevealing(modName) ? Common::kyRevealing : Common::kyCovering));
  }
}

// RgKey
Common::RaceGroupInfo* Core::RgKey::Get() const {
  if (race) {
    auto it = core->raceRgs.find(race);
    return (it != core->raceRgs.end()) ? it->second : nullptr;
  } else if (index >= 0) {
    if (onlyMCM && !core->boolSettings.Get(Common::bsShowAllRaces)) {
      int curr = -1;
      for (auto& rg : core->rgInfoList) {
        if (!rg.noMCM) curr++;
        if (curr == index) return &rg;
      }
    } else {
      if (index < core->rgInfoList.size()) return &core->rgInfoList[static_cast<size_t>(index)];
    }
  }
  return nullptr;
}
