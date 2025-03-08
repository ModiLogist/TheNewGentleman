#include <Base.h>

Base *base = Base::GetSingleton();

void Base::Init() {
  rgInfoList.emplace_back();
  auto &rg0 = rgInfoList[0];
  rg0.idx = 0;
  rg0.name = "TNGRg0";
  rg0.isMain = true;
  rg0.noMCM = true;
  LoadAddons();
  playerInfo.isInfoSet = false;
}

// Addons loading and handling
void Base::LoadAddons() {
  malAddons.clear();
  femAddons.clear();
  const auto &armorList = Util::SEDH()->GetFormArray<RE::TESObjectARMO>();
  for (const auto &armor : armorList) {
    if (armor->HasKeyword(Util::Key(Util::kyAddonM))) malAddons.emplace_back(armor, true);
    if (armor->HasKeyword(Util::Key(Util::kyAddonF))) femAddons.emplace_back(armor, false);
  }
  for (auto &armorPair : malAddons)
    if (!armorPair.first->HasKeyword(Util::Key(Util::kyIgnored))) armorPair.first->AddKeyword(Util::Key(Util::kyIgnored));
  for (auto &armorPair : femAddons)
    if (!armorPair.first->HasKeyword(Util::Key(Util::kyIgnored))) armorPair.first->AddKeyword(Util::Key(Util::kyIgnored));
}

int Base::AddonIdxByLoc(bool isFemale, SEFormLocView addonLoc) {
  if (addonLoc.second == Util::cNulStr) return Util::cNul;
  const auto &list = isFemale ? femAddons : malAddons;
  for (int i = 0; i < list.size(); i++)
    if (FormToLocView(list[i].first) == addonLoc) return i;
  return -1;
}

RE::TESObjectARMO *Base::AddonByIdx(bool isFemale, size_t choice, bool onlyActive) {
  const auto &list = isFemale ? femAddons : malAddons;
  RE::TESObjectARMO *res = nullptr;
  if (onlyActive) {
    int activeCount = 0;
    for (const auto &element : list) {
      if (element.second) {
        if (activeCount == choice) {
          res = element.first;
          break;
        }
        activeCount++;
      }
    }
  } else {
    res = choice < list.size() ? list[choice].first : nullptr;
  }
  return res;
}

size_t Base::GetAddonCount(const bool isFemale, const bool onlyActive) {
  auto &list = isFemale ? femAddons : malAddons;
  if (onlyActive) {
    size_t res = 0;
    for (const auto &addonPair : list)
      if (addonPair.second) res++;
    return res;
  } else {
    return list.size();
  }
}

bool Base::GetAddonStatus(const bool isFemale, const size_t addnIdx) {
  auto &list = isFemale ? femAddons : malAddons;
  if (addnIdx >= list.size()) return false;
  return list[addnIdx].second;
}

void Base::SetAddonStatus(const bool isFemale, const size_t addnIdx, const bool aIsActive) {
  auto &list = isFemale ? femAddons : malAddons;
  list[addnIdx].second = aIsActive;
}

// Sizes

float Base::GetGlobalSize(size_t idx) {
  if (idx < 0 || idx >= Util::cSizeCategories) return 1.0f;
  return Util::SizeGlb(idx)->value;
}

void Base::SetGlobalSize(size_t idx, float size) {
  if (idx < 0 || idx >= Util::cSizeCategories) return;
  if (Util::SizeGlb(idx)) {
    Util::SizeGlb(idx)->value = size;
  } else {
    SKSE::log::error("The [{}] record for size #[{}] cannot be loaded!", Util::cName, idx);
  }
}

// Race handling and info
void Base::AddRace(RE::TESRace *race, bool isProcessed) {
  race->AddKeyword(Util::Key(isProcessed ? Util::kyProcessed : Util::kyPreProcessed));
  race->AddSlotToMask(Util::cSlotGenital);
  race->skin->AddKeyword(Util::Key(Util::kyIgnored));
  int rgCount = static_cast<int>(rgInfoList.size());
  SKSE::log::debug("\tTrying to add race [0x{:x}: {}] ...", race->GetFormID(), race->GetFormEditorID());
  auto rg = GetRg(race, true);
  if (rgInfoList.size() > rgCount) {
    SKSE::log::info("\tThe race [0x{:x}: {}] was recognized as a new {} group [{}].", race->GetFormID(), race->GetFormEditorID(), rg->isMain ? "primary" : "secondary", rg->name);
    if (Util::Block() && !Util::Block()->armorAddons[0]->IsValidRace(race)) Util::Block()->armorAddons[0]->additionalRaces.push_back(race);
  } else {
    SKSE::log::info("\tThe race [0x{:x}: {}] was recognized as a member of existing group {}.", race->GetFormID(), race->GetFormEditorID(), rg->name);
  }
}

bool Base::ReevaluateRace(RE::TESRace *race, RE::Actor *actor) {
  if (!actor || !race || raceRgs.find(race) == raceRgs.end()) return false;
  SKSE::log::debug("Re-evaluating race [0x{:x}:{}] ...", race->GetFormID(), race->GetFormEditorID());
  if (!actor->Is3DLoaded()) return false;
  auto rgIdx = raceRgs[race];
  auto &rg = rgInfoList[rgIdx];
  for (auto &boneName : genBoneNames) {
    if (!actor->GetNodeByName(boneName)) {
      race->skin = rg.ogSkin;
      race->RemoveKeyword(Util::Key(Util::kyPreProcessed));
      race->AddKeyword(Util::Key(Util::kyIgnored));
      race->RemoveSlotFromMask(Util::cSlotGenital);
      auto it = std::find(rg.races.begin(), rg.races.end(), race);
      rg.races.erase(it);
      SKSE::log::info("\tTNG would neglect the race [0x{:x}:{}] since its skeleton is missing the bone [{}]", race->GetFormID(), race->GetFormEditorID(), boneName);
      if (rg.races.empty()) rgInfoList.erase(rgInfoList.begin() + rgIdx);
      return false;
    }
  }
  SKSE::log::debug("\tThe race [0x{:x}:{}] was evaluated to be supported by TNG!", race->GetFormID(), race->GetFormEditorID());
  race->RemoveKeyword(Util::Key(Util::kyPreProcessed));
  race->AddKeyword(Util::Key(Util::kyProcessed));
  return true;
}

void Base::TryUnhideRace(RE::TESRace *race) {
  auto rg = GetRg(race, false);
  if (!rg) return;
  if (rg->malAddons.size() == 0) return;
  rg->noMCM = false;
}

int Base::GetRaceRgIdx(RE::TESRace *race) { return race && raceRgs.find(race) != raceRgs.end() ? static_cast<int>(raceRgs[race]) : Util::cNA; }

void Base::UpdateRgSkins() {
  for (auto &rg : rgInfoList) {
    if (rg.addonIdx >= 0) {
      auto skin = GetSkinWithAddonForRg(&rg, rg.ogSkin, rg.addonIdx, false);
      if (skin)
        for (auto &race : rg.races) race->skin = skin;
    }
  }
}

RE::TESRace *Base::GetRgRace0(const size_t rgChoice, const bool onlyMCM) {
  auto rg = GetRg(rgChoice, onlyMCM);
  return rg ? rg->races[0] : nullptr;
}

std::vector<std::string> Base::GetRgNames(const bool onlyMCM) {
  std::vector<std::string> res{};
  for (auto &rg : rgInfoList) {
    if (boolSettings[Util::bsShowAllRaces] || !onlyMCM || !rg.noMCM) res.push_back(rg.name);
  }
  return res;
}

std::string Base::GetRgInfo(size_t rgChoice, bool onlyMCM) {
  std::string res{""};
  auto rg = GetRg(rgChoice, onlyMCM);
  if (!rg) return res;
  auto &list = rg->races;
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

int Base::GetRgAddon(const size_t rgChoice, bool onlyMCM) {
  auto rg = GetRg(rgChoice, onlyMCM);
  if (!rg) return Util::pgErr;
  return rg->addonIdx;
}

int Base::GetRgAddon(RE::TESRace *race) {
  if (!race) return Util::pgErr;
  auto rg = GetRg(race, false);
  if (!rg) return Util::pgErr;
  return rg->addonIdx;
}

bool Base::SetRgAddon(const size_t rgChoice, const int addnChoice, bool onlyMCM) {
  auto rg = GetRg(rgChoice, onlyMCM);
  if (!rg || addnChoice < Util::cDef || (addnChoice >= 0 && rg->malAddons.find(addnChoice) == rg->malAddons.end())) return false;
  rg->addonIdx = (addnChoice == Util::cDef) ? rg->defAddonIdx : addnChoice;
  auto skin = (rg->addonIdx == Util::cNul) ? rg->ogSkin : GetSkinWithAddonForRg(rg, rg->ogSkin, rg->addonIdx, false);
  if (skin) {
    for (auto &race : rg->races) race->skin = skin;
    return true;
  }
  return false;
}

float Base::GetRgMult(const size_t rgChoice, bool onlyMCM) {
  auto rg = GetRg(rgChoice, onlyMCM);
  return rg ? rg->mult : 1.0f;
}

float Base::GetRgMult(RE::TESRace *race) {
  if (!race) return 1.0f;
  auto rg = GetRg(race, false);
  return rg ? rg->mult : 1.0f;
}

bool Base::SetRgMult(const size_t rgChoice, const float mult, bool onlyMCM) {
  auto rg = GetRg(rgChoice, onlyMCM);
  if (!rg || mult < 0.1f || mult >= 10.0f) {
    SKSE::log::critical("\tFailure in setting a race mult!");
    SKSE::log::debug("\t\tThe race index was {} and the mult was {}", rgChoice, mult);
    return false;
  }
  rg->mult = mult;
  return true;
}

std::vector<size_t> Base::GetRgAddonList(size_t rgChoice, bool isFemale, bool onlyMCM, bool onlyActive) {
  std::vector<size_t> res{};
  auto rg = GetRg(rgChoice, onlyMCM);
  if (!rg) return res;
  auto &list = isFemale ? rg->femAddons : rg->malAddons;
  auto &master = isFemale ? femAddons : malAddons;
  for (auto &addonPair : list)
    if (!onlyActive || master[addonPair.first].second) res.push_back(addonPair.first);
  return res;
}

std::vector<size_t> Base::GetRgAddonList(RE::TESRace *race, bool isFemale, bool onlyActive) {
  std::vector<size_t> res{};
  if (!race) return res;
  auto rg = GetRg(race, false);
  if (!rg) return res;
  auto &list = isFemale ? rg->femAddons : rg->malAddons;
  auto &master = isFemale ? femAddons : malAddons;
  for (auto &addonPair : list)
    if (!onlyActive || master[addonPair.first].second) res.push_back(addonPair.first);
  return res;
}

int Base::IsAddonDedicatedToRg(const size_t rgChoice, bool isFemale, bool onlyMCM, size_t addnChoice) {
  auto rg = GetRg(rgChoice, onlyMCM);
  if (!rg) return Util::pgErr;
  if (rg->isMain) return Util::resOkMain;
  auto &list = isFemale ? rg->femAddons : rg->malAddons;
  if (addnChoice >= list.size()) return Util::pgErr;
  return list[addnChoice].first ? Util::resOkDedicated : Util::resOkSupported;
}

RE::TESObjectARMO *Base::GetSkinWithAddonForRg(const size_t rgIdx, RE::TESObjectARMO *skin, const size_t addonIdx, const bool isFemale) {
  return GetSkinWithAddonForRg(&rgInfoList[rgIdx], skin, addonIdx, isFemale);
}

void Base::ReportHiddenRgs() {
  SKSE::log::info("TNG would not show the following race groups in the MCM since there are either no genital available to them or there are very few NPCs which use them:");
  for (size_t i = 1; i < rgInfoList.size(); i++)
    if (rgInfoList[i].noMCM) SKSE::log::info("\tRace group [{}] with the following information: {}.", rgInfoList[i].name, GetRgInfo(i, false));
}

Base::RaceGroupInfo *Base::GetRg(const size_t rgChoice, const bool onlyMCM) {
  int idx = Util::cNA;
  if (onlyMCM && !boolSettings[Util::bsShowAllRaces]) {
    size_t curr = 0;
    for (int i = 0; i < rgInfoList.size(); i++) {
      auto &rg = rgInfoList[i];
      if (!rg.noMCM) {
        if (curr == rgChoice) {
          idx = i;
          break;
        }
        curr++;
      }
    }
  } else {
    idx = rgChoice < rgInfoList.size() ? static_cast<int>(rgChoice) : Util::cNA;
  }
  return idx >= 0 ? &rgInfoList[idx] : nullptr;
}

Base::RaceGroupInfo *Base::GetRg(RE::TESRace *race, const bool allowAdd) {
  if (!race) return nullptr;
  if (raceRgs.find(race) != raceRgs.end()) return &rgInfoList[raceRgs[race]];
  if (!allowAdd) return nullptr;
  for (size_t i = 0; i < rgInfoList.size(); i++) {
    auto &rg = rgInfoList[i];
    auto it = std::find(rg.races.begin(), rg.races.end(), race);
    if (it != rg.races.end()) {
      raceRgs.insert({race, i});
      return &rg;
    }
    if (rg.armorRace == race && rg.ogSkin == race->skin) {
      rg.races.push_back(race);
      raceRgs.insert({race, i});
      return &rg;
    }
    if (race->armorParentRace && rg.armorRace == race->armorParentRace && rg.ogSkin == race->skin) {
      rg.races.push_back(race);
      raceRgs.insert({race, i});
      return &rg;
    }
  }
  if (race->armorParentRace && race->armorParentRace->skin == race->skin) {
    auto rg = GetRg(race->armorParentRace, true);
    rg->races.push_back(race);
    raceRgs.insert({race, rg->idx});
    return rg;
  } else {
    auto pRace = race;
    while (pRace->armorParentRace) {
      pRace = pRace->armorParentRace;
    };
    auto filename = race->GetFile(0) ? race->GetFile(0)->GetFilename() : "Unknown";
    rgInfoList.push_back({});
    auto &rg = rgInfoList.back();
    rg.idx = rgInfoList.size() - 1;
    rg.name = race->GetFormEditorID();
    rg.file = filename;
    rg.armorRace = pRace;
    rg.ogSkin = race->skin;
    rg.isMain = pRace == race;
    rg.races.push_back(race);
    rg.noMCM = !race->GetPlayable() && !race->HasKeyword(Util::Key(Util::kyVampire));
    rg.mult = 1.0f;
    rg.defAddonIdx = GetRgDefAddon(rg);
    raceRgs.insert({race, rg.idx});
    UpdateRgAddons(rg);
    return &rg;
  }
}

int Base::GetRgDefAddon(Base::RaceGroupInfo &rg) {
  if (rg.defAddonIdx != Util::cNA) return rg.defAddonIdx;
  SEFormLocView defAddon;
  bool defAddonSet = false;
  auto raceStr = std::string(rg.armorRace->GetFormEditorID()) + std::string(rg.armorRace->GetName());
  std::transform(raceStr.begin(), raceStr.end(), raceStr.begin(), [](uint8_t c) { return static_cast<char>(std::tolower(c)); });
  for (std::size_t i = 0; i < cVanillaRaceTypes; i++) {
    if (raceStr.contains(cRaceNames[i].first[0]) || raceStr.contains(cRaceNames[i].first[1]) && (rg.armorRace->HasKeyword(Util::Key(Util::kyBeast)) == cRaceNames[i].second)) {
      defAddon = {cRaceDefaults[i], Util::cName};
      defAddonSet = true;
      break;
    }
  }
  if (!defAddonSet) defAddon = rg.armorRace->HasKeyword(Util::Key(Util::kyBeast)) ? SEFormLocView({cRaceDefaults[9], Util::cName}) : SEFormLocView({cRaceDefaults[0], Util::cName});
  for (auto i = 0; i < malAddons.size(); i++) {
    if (FormToLocView(malAddons[i].first) == defAddon) return i;
  }
  SKSE::log::critical("TNG faced an error getting the default addon for a race group.");
  return Util::cNul;
}

void Base::UpdateRgAddons(Base::RaceGroupInfo &rg) {
  rg.malAddons.clear();
  rg.femAddons.clear();

  auto processAddons = [&](auto &addons, auto &rgAddons, const char *gender) {
    for (size_t i = 0; i < addons.size(); i++) {
      auto &addon = addons[i].first;
      bool supports = false;
      for (const auto &aa : addon->armorAddons) {
        if (aa->IsValidRace(rg.armorRace)) {
          auto aaMainRace = aa->race;
          if (aaMainRace && aaMainRace == Util::Race(Util::raceDefault) && !aa->additionalRaces.empty()) {
            aaMainRace = aa->additionalRaces[0];
          }
          if (auto mainRaceRg = GetRg(aaMainRace, false); rg.isMain && mainRaceRg && !mainRaceRg->isMain) continue;
          supports = true;
          rgAddons.insert_or_assign(i, std::pair<bool, RE::TESObjectARMA *>({rg.isMain || AddonHasRace(aa, rg.races[0]), aa}));
          if (rgAddons[i].first) break;
        }
      }
      if (!supports) {
        if (i == rg.defAddonIdx) rg.defAddonIdx = rg.malAddons.size() == 0 ? Util::cNul : static_cast<int>(rg.malAddons.begin()->first);
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
  };

  processAddons(malAddons, rg.malAddons, "men");
  processAddons(femAddons, rg.femAddons, "women");
}

bool Base::RgHasAddon(RaceGroupInfo &rg, bool isFemale, int addonIdx) {
  if (addonIdx < 0) return true;
  const auto &list = isFemale ? rg.femAddons : rg.malAddons;
  return list.find(static_cast<size_t>(addonIdx)) != list.end();
}

bool Base::AddonHasRace(const RE::TESObjectARMA *addnIdx, const RE::TESRace *race) {
  if (race == addnIdx->race) return true;
  for (auto &iRace : addnIdx->additionalRaces)
    if (iRace == race) return true;
  return false;
}

RE::TESObjectARMO *Base::GetSkinWithAddonForRg(RaceGroupInfo *rg, RE::TESObjectARMO *skin, const size_t addonIdx, const bool isFemale) {
  auto &r = rg->isMain ? rgInfoList[0] : *rg;
  auto &skinMap = isFemale ? r.femSkins : r.malSkins;
  auto ogSkin = GetOgSkin(skin);
  if (!ogSkin->HasKeyword(Util::Key(Util::kyTngSkin)) && !ogSkin->HasKeyword(Util::Key(Util::kyIgnored))) ogSkin->AddKeyword(Util::Key(Util::kyIgnored));
  if (skinMap.find(ogSkin) == skinMap.end()) skinMap.insert({ogSkin, std::map<size_t, RE::TESObjectARMO *>{}});
  RE::TESObjectARMO *resSkin = nullptr;
  if (skinMap[ogSkin].find(addonIdx) == skinMap[ogSkin].end()) {
    resSkin = ogSkin->CreateDuplicateForm(true, (void *)resSkin)->As<RE::TESObjectARMO>();
    resSkin->Copy(ogSkin);
    resSkin->AddKeyword(Util::Key(Util::kyTngSkin));
    if (isFemale && femAddons[addonIdx].first->HasKeyword(Util::Key(Util::kySkinWP))) resSkin->AddKeyword(Util::Key(Util::kySkinWP));
    resSkin->AddSlotToMask(Util::cSlotGenital);
    ogSkinMap.insert({resSkin, ogSkin});
    skinMap[ogSkin].insert({addonIdx, resSkin});
  } else {
    resSkin = skinMap[ogSkin][addonIdx];
  }
  auto &reqAA = isFemale ? rg->femAddons[addonIdx].second : rg->malAddons[addonIdx].second;
  if (std::find(resSkin->armorAddons.begin(), resSkin->armorAddons.end(), reqAA) == resSkin->armorAddons.end()) resSkin->armorAddons.push_back(reqAA);
  return resSkin;
}

RE::TESObjectARMO *Base::GetOgSkin(RE::TESObjectARMO *skin) {
  if (!skin) return nullptr;
  auto &res = skin->HasKeyword(Util::Key(Util::kyTngSkin)) ? ogSkinMap[skin] : skin;
  auto problem = 0;
  if (!res) {
    problem = 1;
  } else {
    if (res->HasKeyword(Util::Key(Util::kyTngSkin))) problem = 2;
  }
  if (problem) {
    SKSE::log::critical("TNG routine to find the original skin failed with flag [{}]! Please report this issue.", problem);
  }
  return res;
}

//  NPC handling and info

Util::TNGRes Base::CanModifyNPC(RE::TESNPC *npc) {
  if (!npc) return Util::npcErr;
  if (!npc->race) return Util::raceErr;
  if (npc->race->HasKeyword(Util::Key(Util::kyReady))) return Util::resOkRaceR;
  if (auto r = GetRg(npc->race, false); !r || r->malAddons.size() == 0) return Util::raceErr;
  if (npc->race->HasKeyword(Util::Key(Util::kyProcessed))) return Util::resOkRaceP;
  if (npc->race->HasKeyword(Util::Key(Util::kyPreProcessed))) return Util::resOkRacePP;
  return Util::raceErr;
}

Util::TNGRes Base::GetActorSizeCat(RE::Actor *actor, int &sizeCat) {
  sizeCat = Util::cNA;
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (auto res = CanModifyNPC(npc); res < 0) return res;
  if (npc->IsPlayer() && boolSettings[Util::bsExcludePlayerSize]) return Util::playerErr;
  for (size_t i = 0; i < Util::cSizeCategories; i++) {
    if (npc->HasKeyword(Util::SizeKey(i))) sizeCat = static_cast<int>(i);
  }
  if (sizeCat < 0) sizeCat = npc->formID % Util::cSizeCategories;
  return Util::resOkSizable;
}

Util::TNGRes Base::SetActorSize(RE::Actor *actor, const int sizeCat) {
  int currCat = Util::cNA;
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Util::npcErr;
  if (npc->IsPlayer() && !actor->IsPlayerRef()) return Util::resOkFixed;
  if (sizeCat == Util::cDef) npc->RemoveKeywords(Util::SizeKeys());
  auto res = GetActorSizeCat(actor, currCat);
  if (res != Util::resOkSizable) return res;
  auto cat = (sizeCat < 0) ? currCat : sizeCat;
  if (cat != currCat) {
    npc->RemoveKeywords(Util::SizeKeys());
    npc->AddKeyword(Util::SizeKey(cat));
  }
  auto catGlb = Util::SizeGlb(cat);
  auto mult = GetRgMult(actor->GetRace());
  if (mult < 0.0f) return Util::rgErr;
  auto scale = mult * (catGlb ? catGlb->value : 1.0f);
  if (scale < 0.1) scale = 1;
  RE::NiAVObject *baseNode = actor->GetNodeByName(genBoneNames[egbBase]);
  RE::NiAVObject *scrtNode = actor->GetNodeByName(genBoneNames[egbScrt]);
  if (!baseNode || !scrtNode) return Util::skeletonErr;
  baseNode->local.scale = scale;
  scrtNode->local.scale = 1.0f / sqrt(scale);
  return Util::resOkSizable;
}

void Base::ExcludeNPC(const std::string npcRecordStr) {
  auto record = StrToLoc(npcRecordStr);
  auto npc = Util::SEDH()->LookupForm<RE::TESNPC>(record.first, record.second);
  if (!npc || !npc->race || !npc->race->skin) return;
  npc->AddKeyword(Util::Key(Util::kyExcluded));
}

std::pair<int, bool> Base::GetNPCAddon(RE::TESNPC *npc) {
  if (!npc) {
    SKSE::log::critical("Failure in getting a NPC genital!");
    return std::make_pair(Util::pgErr, false);
  }
  if (npc->IsPlayer()) {
    int addnIdx = Util::PCAddon() ? static_cast<int>(Util::PCAddon()->value) : Util::cDef;
    return std::make_pair(addnIdx, addnIdx > Util::cDef);
  }
  if (npc->HasKeyword(Util::Key(Util::kyExcluded))) return std::make_pair(Util::cNul, true);
  if (npc->keywords) {
    for (std::size_t idx = 0; idx < npc->numKeywords; ++idx) {
      if (!npc->keywords[idx] || npc->keywords[idx]->GetFormEditorID() == NULL) continue;
      const std::string kwStr(npc->keywords[idx]->GetFormEditorID());
      if (kwStr.starts_with(cNPCAutoAddon)) {
        return std::make_pair(std::strtol(kwStr.substr(strlen(cNPCAutoAddon), 2).data(), nullptr, 0), false);
      }
      if (kwStr.starts_with(cNPCUserAddon)) {
        return std::make_pair(std::strtol(kwStr.substr(strlen(cNPCUserAddon), 2).data(), nullptr, 0), true);
      }
    }
  }
  return std::make_pair(Util::cDef, false);
}

Util::TNGRes Base::SetNPCAddon(RE::TESNPC *npc, const int addnIdx, const bool isUser) {
  if (addnIdx < Util::cDef) return Util::addonErr;
  if (!npc) {
    SKSE::log::critical("Failure in setting a NPC genital!");
    return Util::npcErr;
  }
  auto res = (npc->IsPlayer() && boolSettings[Util::bsExcludePlayerSize]) || addnIdx == Util::cNul ? Util::resOkFixed : Util::resOkSizable;
  npc->RemoveKeyword(Util::Key(Util::kyExcluded));
  if (addnIdx == Util::cDef && !npc->skin) return !npc->IsFemale() ? res : Util::resOkFixed;
  auto &npcSkin = npc->skin;
  auto &raceSkin = npc->race->skin;
  auto &activeSkin = npcSkin ? npcSkin : raceSkin;
  bool skinHasRace = false;
  for (auto &aa : activeSkin->armorAddons) {
    if (aa->IsValidRace(npc->race) && !aa->HasPartOf(Util::cSlotGenital)) {
      skinHasRace = true;
      break;
    }
  }
  if (!skinHasRace) activeSkin = npc->race->skin;
  auto ogSkin = GetOgSkin(npc->skin);
  auto raceOgSkin = GetOgSkin(npc->race->skin);
  auto &activeOgSkin = !ogSkin ? raceOgSkin : ogSkin;
  if (ogSkin && ogSkin->HasKeyword(Util::Key(Util::kyCovering))) {
    ogSkin->RemoveKeyword(Util::Key(Util::kyCovering));
    ogSkin->AddKeyword(Util::Key(Util::kyIgnored));
    SKSE::log::info("The skin [0x{:x}] was updated accordingly", ogSkin->formID);
  }
  auto rg = GetRg(npc->race, false);
  if (!rg) return Util::rgErr;
  if (!RgHasAddon(*rg, npc->IsFemale(), addnIdx)) return Util::addonErr;
  if (addnIdx == Util::cDef && npc->IsFemale()) {
    OrganizeNPCAddonKeywords(npc, addnIdx, false);
    if (npcSkin && npcSkin->HasKeyword(Util::Key(Util::kyTngSkin))) npc->skin = ogSkin == raceOgSkin ? nullptr : ogSkin;
    return Util::resOkFixed;
  }
  auto addonChoice = addnIdx == Util::cDef ? rg->addonIdx : addnIdx;
  OrganizeNPCAddonKeywords(npc, addnIdx, isUser);
  auto resSkin = addonChoice == Util::cNul ? (npc->IsFemale() ? raceSkin : activeOgSkin) : GetSkinWithAddonForRg(rg, activeOgSkin, addonChoice, npc->IsFemale());
  if (resSkin != npcSkin && !(!npcSkin && resSkin == raceSkin)) {
    npc->skin = resSkin == npc->race->skin ? nullptr : resSkin;
  }
  return !npc->IsFemale() || npc->HasKeyword(Util::Key(Util::kyGentlewoman)) ? res : Util::resOkFixed;
}

void Base::SetPlayerInfo(RE::Actor *actor, const int addnIdx) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return;
  playerInfo.isFemale = npc->IsFemale();
  playerInfo.race = npc->race;
  playerInfo.isInfoSet = true;
  auto addnFl = static_cast<float>(addnIdx);
  Util::PCAddon()->value = addnFl;
}

void Base::UnsetPlayerInfo() { playerInfo.isInfoSet = false; }

bool Base::HasPlayerChanged(RE::Actor *actor) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return false;
  return playerInfo.isInfoSet && npc->skin && npc->skin != npc->race->skin &&
         (playerInfo.race != npc->race || !Util::PCAddon() || Util::PCAddon()->value >= Base::GetAddonCount(npc->IsFemale(), false));
}

bool Base::GetBoolSetting(const size_t idx) {
  if (idx >= Util::BoolSettingCount) return false;
  return boolSettings[idx];
}

void Base::SetBoolSetting(const size_t idx, const bool value) {
  if (idx < Util::BoolSettingCount) boolSettings[idx] = value;
}

void Base::OrganizeNPCAddonKeywords(RE::TESNPC *npc, int addnIdx, bool isUser) {
  npc->ForEachKeyword([&](RE::BGSKeyword *lKw) {
    if (lKw->formEditorID.contains("TNG_ActorAddn")) {
      npc->RemoveKeyword(lKw);
      return RE::BSContainer::ForEachResult::kStop;
    }
    return RE::BSContainer::ForEachResult::kContinue;
  });
  if (addnIdx == Util::cNul) {
    npc->AddKeyword(Util::Key(Util::kyExcluded));
  } else if (addnIdx >= 0) {
    const std::string reqKw = (isUser ? cNPCUserAddon : cNPCAutoAddon) + (addnIdx < 10 ? "0" + std::to_string(addnIdx) : std::to_string(addnIdx));
    auto kw = Util::ProduceOrGetKw(reqKw);
    if (!kw) SKSE::log::critical("Keyword generation routine failed with keyword {}", reqKw);
    npc->AddKeyword(kw);
  }
  if (npc->IsFemale() && addnIdx >= 0) {
    if (femAddons[addnIdx].first->HasKeyword(Util::Key(Util::kySkinWP))) {
      npc->AddKeyword(Util::Key(Util::kyGentlewoman));
    } else {
      npc->RemoveKeyword(Util::Key(Util::kyGentlewoman));
    }
  }
}
