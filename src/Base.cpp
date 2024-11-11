#include <Base.h>

// Addons loading and handling
void Base::LoadAddons() noexcept {
  fMalAddons.clear();
  fFemAddons.clear();
  auto &armorlist = Tng::SEDH()->GetFormArray<RE::TESObjectARMO>();
  for (const auto &armor : armorlist) {
    if (FormHasKW(armor, Tng::cMalAddKeyID)) fMalAddons.push_back({armor, true});
    if (FormHasKW(armor, Tng::cFemAddKeyID)) fFemAddons.push_back({armor, false});
  }
}

RE::TESObjectARMO *Base::GetAddon(bool aIsFemale, std::size_t aChoice, bool aOnlyActive) noexcept {
  auto &list = aIsFemale ? fFemAddons : fMalAddons;
  if (aOnlyActive) {
    int activeCount = 0;
    for (const auto &element : list) {
      if (element.second) {
        if (activeCount == aChoice) {
          return element.first;
        }
        activeCount++;
      }
    }
  } else {
    return list[aChoice].first;
  }
}

std::size_t Base::GetAddonCount(bool aIsFemale, bool aOnlyActive) noexcept {
  auto &list = aIsFemale ? fFemAddons : fMalAddons;
  if (aOnlyActive) {
    std::size_t res = 0;
    for (const auto &activeCount : list)
      if (activeCount.second) res++;
    return res;
  } else {
    return list.size();
  }
}

bool Base::GetAddonStatus(const bool aIsFemale, const std::size_t aAddon) noexcept { return aIsFemale ? fFemAddons[aAddon].second : fMalAddons[aAddon].second; }

void Base::SetAddonStatus(const bool aIsFemale, const std::size_t aAddon, const bool aIsActive) noexcept {
  auto &list = aIsFemale ? fFemAddons : fMalAddons;
  list[aAddon].second = aIsActive;
}

// Sizes

float Base::GetGlobalSize(std::size_t aIdx) noexcept {
  if (aIdx < 0 || aIdx >= Tng::cSizeCategories) return 1.0f;
  return Tng::SizeGlb(aIdx)->value;
}

void Base::SetGlobalSize(std::size_t aIdx, float aSize) noexcept {
  if (aIdx < 0 || aIdx >= Tng::cSizeCategories) return;
  Tng::SizeGlb(aIdx)->value = aSize;
}

// Race handling and info
void Base::AddRace(RE::TESRace *aRace) noexcept {
  int rgCount = fRaceGroupInfoList.size();
  auto &rg = GetRG(aRace);
  if (fRaceGroupInfoList.size() > rgCount)
    SKSE::log::info("The race [0x{:x}: {}] was recognized as a new group {}.", aRace->GetFormID(), aRace->GetFormEditorID(), rg.name);
  else
    SKSE::log::info("The race [0x{:x}: {}] was recognized as a member of existing group {}.", aRace->GetFormID(), aRace->GetFormEditorID(), rg.name);
}

std::vector<std::string> Base::GetRGNames() noexcept {
  std::vector<std::string> res{};
  for (auto &rgId : fRgIdList) res.push_back(fRaceGroupInfoList[rgId].name);
  return res;
}

std::string Base::GetRGRaceNames(std::size_t aRgId) noexcept {
  std::string res{""};
  auto &list = fRaceGroupInfoList[fRgIdList[aRgId]].races;
  for (auto race : list) {
    res = res + std::string(race->GetFile(0) ? race->GetFile(0)->GetFilename() : "Unknown") + " : " + race->GetFormEditorID();
    if (race != list.back()) {
      res = res + Tng::cDelimChar;
    }
    return res;
  };
}

std::vector<std::string> Base::GetRGAddonNames(std::size_t aRgId, bool aIfFemale, bool aOnlyDedicated) noexcept {
  std::vector<std::string> res{};
  auto &rg = fRaceGroupInfoList[fRgIdList[aRgId]];
  auto &list = aIfFemale ? rg.femAddons : rg.malAddons;
  for (auto &addonpair : list) res.push_back(addonpair.first->GetName());
  return res;
}

int Base::GetAddn(RE::TESRace *aRace) noexcept {
  if (!aRace) return Tng::pgErr;
  auto &rg = GetRG(aRace);
  if (!rg.addon) return -1;
  auto it = std::find(rg.malAddonIdList.begin(), rg.malAddonIdList.end(), rg.addon);
  if (it == rg.malAddonIdList.end()) return Tng::pgErr;
  return std::distance(rg.malAddonIdList.begin(), it);
}

int Base::GetAddn(const std::size_t aRgId) noexcept {
  if (aRgId >= fRgIdList.size()) return Tng::pgErr;
  auto &rg = fRaceGroupInfoList[fRgIdList[aRgId]];
  if (!rg.addon) return -1;
  auto it = std::find(rg.malAddonIdList.begin(), rg.malAddonIdList.end(), rg.addon);
  if (it == rg.malAddonIdList.end()) return Tng::pgErr;
  return std::distance(rg.malAddonIdList.begin(), it);
}

bool Base::SetAddn(const std::size_t aRgId, const std::size_t aAddnId) noexcept {
  if (aRgId >= fRgIdList.size()) return false;
  auto &rg = fRaceGroupInfoList[fRgIdList[aRgId]];
  if (aAddnId >= rg.malAddonIdList.size()) return false;
  auto rgOldAddn = rg.addon;
  rg.addon = rg.malAddonIdList[aAddnId];
  UpdateRgSkin(rg, rgOldAddn);
}

float Base::GetMult(RE::TESRace *aRace) noexcept {
  if (!aRace) return -1.0f;
  return GetRG(aRace).mult;
}

float Base::GetMult(const std::size_t aRgId) noexcept {
  if (fRaceGroupInfoList.size() <= aRgId) return -1.0f;
  return fRaceGroupInfoList[fRgIdList[aRgId]].mult;
}

bool Base::SetMult(const std::size_t aRgId, const float aMult) noexcept {
  if (fRaceGroupInfoList.size() <= aRgId || aMult < 0.1f || aMult >= 10.0f) {
    SKSE::log::critical("Failure in setting a race mult!");
    return false;
  }
  fRaceGroupInfoList[fRgIdList[aRgId]].mult = aMult;
  return true;
}

Base::RaceGroupInfo &Base::GetRG(RE::TESRace *aRace) noexcept {
  for (auto &pair : fRaceGroupInfoList) {
    auto it = std::find(pair.second.races.begin(), pair.second.races.end(), aRace);
    if (it != pair.second.races.end()) return pair.second;
    if (pair.second.armorRace == aRace && pair.second.skin == aRace->skin) {
      pair.second.races.push_back(aRace);
      return pair.second;
    }
    if (aRace->armorParentRace && pair.second.armorRace == aRace->armorParentRace && pair.second.skin == aRace->skin) {
      pair.second.races.push_back(aRace);
      return pair.second;
    }
  }
  if (aRace->armorParentRace && aRace->armorParentRace->skin == aRace->skin) {
    GetRG(aRace->armorParentRace).races.push_back(aRace);
  } else {
    auto race = aRace;
    while (race->armorParentRace) {
      race = race->armorParentRace;
    };
    fRaceGroupInfoList.insert({{race, aRace->skin}, Base::RaceGroupInfo{}});
    fRgIdList.push_back({race, aRace->skin});
    auto filename = aRace->GetFile(0) ? aRace->GetFile(0)->GetFilename() : "Unknown";
    auto lName = std::string(filename) + " : " + aRace->GetFormEditorID();
    auto &rg = fRaceGroupInfoList[{race, aRace->skin}];
    rg.name = lName;
    rg.armorRace = race;
    rg.skin = aRace->skin;
    rg.isMain = (aRace == race);
    rg.races.push_back(aRace);
    rg.mult = 1.0f;
    rg.defAddon = GetRGDefAddn(aRace == race ? race : aRace);
    UpdateRGAddons(rg);
    if (rg.addon) UpdateRgSkin(rg);
  }
}

SEFormLoc Base::GetRGDefAddn(RE::TESRace *aIDRace) noexcept {
  const auto lRaceDesc = std::string(aIDRace->GetFormEditorID()) + std::string(aIDRace->GetName());
  for (std::uint32_t i = 0; i < Tng::cVanillaRaceTypes; i++) {
    if (lRaceDesc.contains(cRaceNames[i].first[0]) || lRaceDesc.contains(cRaceNames[i].first[1]) && (FormHasKW(aIDRace, Tng::cBstRaceID) == cRaceNames[i].second)) return {cRaceDefaults[i], Tng::cName};
  }
  return FormHasKW(aIDRace, Tng::cBstRaceID) ? SEFormLoc({cRaceDefaults[9], Tng::cName}) : SEFormLoc({cRaceDefaults[0], Tng::cName});
}

void Base::UpdateRGAddons(Base::RaceGroupInfo &aRG) noexcept {
  auto &def = aRG.defAddon;
  aRG.malAddons.clear();
  aRG.malAddonIdList.clear();
  aRG.femAddons.clear();
  aRG.femAddonIdList.clear();
  for (auto &addonpair : fMalAddons) {
    auto &addon = addonpair.first;
    bool lSupports = false;
    for (auto &aa : addon->armorAddons) {
      if (aa->IsValidRace(aRG.armorRace)) {
        aRG.malAddons.insert_or_assign(addon, std::pair<bool, RE::TESObjectARMA *>({aRG.isMain || AddonHasRace(aa, aRG.races[0]), aa}));
        lSupports = true;
        if (aRG.malAddons[addon].first) break;
      }
    }
    if (!lSupports) continue;
    if (FormToLoc(addon) == def && !aRG.addon) aRG.addon = addon;
    if (!aRG.isMain && aRG.malAddons[addon].first && (!aRG.addon || !aRG.malAddons[aRG.addon].first)) {
      aRG.defAddon = FormToLoc(addon);
      aRG.addon = addon;
    }
    if (aRG.malAddons[addon].first)
      SKSE::log::info("The addon [0x{:x}] from file [{}] fully supports men in the race group [{}]!", addon->GetLocalFormID(), addon->GetFile(0)->GetFilename(), aRG.name);
    else
      SKSE::log::warn("The addon [0x{:x}] from file [{}] can be used for men in the race group [{}]!", addon->GetLocalFormID(), addon->GetFile(0)->GetFilename(), aRG.name);
  }
  for (auto &addonpair : aRG.malAddons) aRG.malAddonIdList.push_back(addonpair.first);
  for (auto &addonpair : fFemAddons) {
    auto &addon = addonpair.first;
    bool lSupports = false;
    for (auto &aa : addon->armorAddons) {
      if (aa->IsValidRace(aRG.armorRace)) {
        aRG.femAddons.insert_or_assign(addon, std::pair<bool, RE::TESObjectARMA *>({aRG.isMain || AddonHasRace(aa, aRG.races[0]), aa}));
        lSupports = true;
        if (aRG.femAddons[addon].first) break;
      }
    }
    if (!lSupports) continue;
    if (aRG.femAddons[addon].first)
      SKSE::log::info("The addon [0x{:x}] from file [{}] fully supports women in the race group [{}]!", addon->GetLocalFormID(), addon->GetFile(0)->GetFilename(), aRG.name);
    else
      SKSE::log::warn("The addon [0x{:x}] from file [{}] can be used for women in the race group [{}]!", addon->GetLocalFormID(), addon->GetFile(0)->GetFilename(), aRG.name);
  }
  for (auto &addonpair : aRG.femAddons) aRG.femAddonIdList.push_back(addonpair.first);
}

void Base::UpdateRgSkin(RaceGroupInfo &aRG, RE::TESObjectARMO *aOldAddon) noexcept {
  auto &skin = aRG.skin;
  if (aOldAddon) {
    RE::BSTArray<RE::TESObjectARMA *> skinAddons{};
    for (auto aa : skin->armorAddons) {
      if (aa != aRG.malAddons[aOldAddon].second) skinAddons.emplace_back(aa);
    }
    skin->armorAddons = skinAddons;
  }
  skin->armorAddons.emplace_back(aRG.malAddons[aRG.addon].second);
  if (!skin->HasKeyword(Tng::AiaKey())) skin->AddKeyword(Tng::AiaKey());
  for (auto &race : aRG.races) race->skin = skin;
}

RE::TESRace *Base::RgIdRace(const std::size_t aRgId) noexcept { return fRaceGroupInfoList[fRgIdList[aRgId]].races[0]; }

//  NPC loading, handling and info
void Base::ExcludeNPC(const std::string aNPCRecord) noexcept {
  auto npc = LoadForm<RE::TESNPC>(aNPCRecord);
  if (!npc) return;
  if (!npc->race) return;
  if (!npc->race->skin) return;
  npc->AddKeyword(Tng::NexKey());
}

std::pair<bool, int> Base::GetNPCAddn(RE::TESNPC *aNPC) noexcept {
  if (!aNPC) {
    SKSE::log::critical("Failure in getting a NPC genital!");
    return std::make_pair(false, Tng::pgErr);
  }
  if (aNPC->keywords) {
    for (std::uint32_t idx = 0; idx < aNPC->numKeywords; ++idx) {
      if (!aNPC->keywords[idx] || aNPC->keywords[idx]->GetFormEditorID() == NULL) continue;
      const std::string lKwStr(aNPC->keywords[idx]->GetFormEditorID());
      if (lKwStr.starts_with(cNPCAutoAddn)) {
        return std::make_pair(false, std::strtol(lKwStr.substr(strlen(cNPCAutoAddn), 2).data(), nullptr, 0));
      }
      if (lKwStr.starts_with(cNPCUserAddn)) {
        return std::make_pair(true, std::strtol(lKwStr.substr(strlen(cNPCUserAddn), 2).data(), nullptr, 0));
      }
    }
  }
  return std::make_pair(false, -1);
}

bool Base::SetNPCAddn(RE::TESNPC *aNPC, int aAddon, bool aIsUser) noexcept {
  if (!aNPC) {
    SKSE::log::critical("Failure in setting a NPC genital!");
    return false;
  }
  auto &list = aNPC->IsFemale() ? fFemAddons : fMalAddons;
  if (aNPC->HasKeyword(Tng::NgwKey())) aNPC->RemoveKeyword(Tng::NgwKey());
  if (aNPC->HasKeyword(Tng::NexKey())) aNPC->RemoveKeyword(Tng::NexKey());
  aNPC->ForEachKeyword([&](RE::BGSKeyword *lKw) {
    if (lKw->formEditorID.contains("TNG_ActorAddn")) {
      aNPC->RemoveKeyword(lKw);
      return RE::BSContainer::ForEachResult::kStop;
    }
    return RE::BSContainer::ForEachResult::kContinue;
  });
  if (aAddon == -2) {
    if (aNPC->IsFemale()) aNPC->AddKeyword(Tng::NexKey());
    for (RE::BSTArray<RE::TESForm *>::const_iterator it = fGentified->forms.begin(); it < fGentified->forms.end(); it++) {
      if ((*it)->As<RE::TESNPC>() == aNPC) fGentified->forms.erase(it);
    }
    return true;
  }
  if (list.size() <= aAddon || aAddon < 0) {
    SKSE::log::critical("Cannot set the NPC {} to use addon {}! There are only {} addons.", aNPC->GetFormEditorID(), aAddon + 1, list.size());
    return false;
  }
  auto lChoice = static_cast<std::size_t>(aAddon);
  auto &lAllKws = Tng::SEDH()->GetFormArray<RE::BGSKeyword>();
  std::string lReqKw = (aIsUser ? cNPCUserAddn : cNPCAutoAddn) + (lChoice < 10 ? "0" + std::to_string(lChoice) : std::to_string(lChoice));
  auto lKwIt = std::find_if(lAllKws.begin(), lAllKws.end(), [&](const auto &kw) { return kw && kw->formEditorID == lReqKw.c_str(); });
  RE::BGSKeyword *lKw{nullptr};
  if (lKwIt != lAllKws.end()) {
    lKw = *lKwIt;
  } else {
    const auto lFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
    if (lKw = lFactory ? lFactory->Create() : nullptr; lKw) {
      lKw->formEditorID = lReqKw;
      lAllKws.push_back(lKw);
    } else {
      SKSE::log::critical("Couldn't create keyword [{}]!", lReqKw);
      return true;
    }
  }
  aNPC->AddKeyword(lKw);
  if (list[aAddon].first->HasKeyword(Tng::GwpKey())) {
    Tng::GentFml()->AddForm(aNPC);
    aNPC->AddKeyword(Tng::NgwKey());
  }
  return true;
}

Tng::TNGRes Base::CanModifyActor(RE::Actor *aActor) noexcept {
  const auto npc = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !npc) return Tng::npcErr;
  if (!npc->race) return Tng::raceErr;
  if (npc->race->HasKeyword(fPRaceKey)) return Tng::resOkRaceP;
  if (npc->race->HasKeyword(fRRaceKey)) return Tng::resOkRaceR;
  return Tng::raceErr;
}

Tng::TNGRes Base::SetCharSize(RE::Actor *aActor, RE::TESNPC *aNPC, int aGenSize) noexcept {
  auto lCurSize = GetScale(aNPC);
  if (lCurSize == aGenSize || aGenSize == -1) {
    ScaleGenital(aActor, fSizeGlbs[lCurSize]);
    return Tng::resOkGen;
  }
  aNPC->RemoveKeywords(fSizeKws);
  if (aNPC->IsPlayer() && aGenSize == -2) return Tng::resOkNoGen;
  aNPC->AddKeyword(fSizeKws[aGenSize]);
  ScaleGenital(aActor, fSizeGlbs[aGenSize]);
  return Tng::resOkGen;
}

std::set<RE::TESObjectARMA *> Base::GentifyGrpSkin(int aRG) noexcept { return GentifyMalSkin(fRaceGroupInfoList[aRG].originalSkin); }

int Base::GetScale(RE::TESNPC *aNPC) noexcept {
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (aNPC->HasKeyword(fSizeKws[i])) return i;
  }
  return (aNPC->formID % 5);
}

void Base::ScaleGenital(RE::Actor *aActor, RE::TESGlobal *aGlobal) noexcept {
  const auto npc = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !npc) return;
  if (!npc->race) return;
  auto lScale = GetRGMult(npc->race) * aGlobal->value;
  RE::NiAVObject *aBaseNode = aActor->GetNodeByName(cBaseBone);
  RE::NiAVObject *aScrtNode = aActor->GetNodeByName(cScrtBone);
  if (!aBaseNode || !aScrtNode) return;
  aBaseNode->local.scale = lScale;
  aScrtNode->local.scale = 1.0f / sqrt(lScale);
}

Tng::RaceType Base::GetSkinType(RE::TESObjectARMO *aSkin) noexcept { return GetRaceType(aSkin->race); }

Tng::RaceType Base::GetRaceType(RE::TESRace *aRace) noexcept {
  if (!aRace) return Tng::raceManMer;
  if (aRace == fDefRace) return Tng::raceManMer;
  if (aRace->HasKeyword(fBstKey)) return Tng::raceBeast;
  if (aRace == fBaseRaces[10]) return Tng::raceDremora;
  if (aRace == fBaseRaces[11]) return Tng::raceElder;
  if (aRace == fBaseRaces[12]) return Tng::raceAfflicted;
  if (aRace == fBaseRaces[13]) return Tng::raceSnowElf;
  return Tng::raceManMer;
}

std::set<RE::TESObjectARMA *> Base::GentifyMalSkin(RE::TESObjectARMO *aSkin, int aAddon) noexcept {
  auto res = std::set<RE::TESObjectARMA *>{};
  if (!aSkin->HasPartOf(Tng::cSlotGenital)) aSkin->AddSlotToMask(Tng::cSlotGenital);
  bool lHasAddons{false};
  for (auto &aa : aSkin->armorAddons) {
    if (fAllMalAAs.find(aa) != fAllMalAAs.end()) lHasAddons = true;
    if (aa->HasPartOf(Tng::cSlotBody) && aa->race && ((aa->race == fDefRace) || aa->race->HasKeyword(fPRaceKey))) res.insert(aa);
  }
  auto lType = GetSkinType(aSkin);
  auto lAddons = aAddon >= 0 ? GetAddonAAs(lType, aAddon, false) : GetCombinedAddons(aSkin);
  if (lHasAddons) {
    for (std::uint32_t i = 0; i < aSkin->armorAddons.size(); i++)
      if (fAllMalAAs.find(aSkin->armorAddons[i]) != fAllMalAAs.end()) aSkin->armorAddons[i] = lAddons.at(aSkin->armorAddons[i]->race);
  } else {
    for (auto &aa : lAddons) aSkin->armorAddons.emplace_back(aa.second);
  }
  return res;
}

std::set<RE::TESObjectARMA *> Base::GentifyFemSkin(RE::TESObjectARMO *aSkin, int aAddon) noexcept {
  auto res = std::set<RE::TESObjectARMA *>{};
  if (aAddon < 0) return res;
  if (!aSkin->HasPartOf(Tng::cSlotGenital)) aSkin->AddSlotToMask(Tng::cSlotGenital);
  if (aSkin)
    for (auto &aa : aSkin->armorAddons)
      if (aa->HasPartOf(Tng::cSlotBody) && aa->race && ((aa->race == fDefRace) || aa->race->HasKeyword(fPRaceKey))) res.insert(aa);
  auto lType = GetSkinType(aSkin);
  auto lAddonsToAdd = GetAddonAAs(lType, aAddon, true);
  bool lHasMalAddons{false};
  for (const auto &aa : aSkin->armorAddons) {
    if (fAllMalAAs.find(aa) != fAllMalAAs.end()) {
      lHasMalAddons = true;
      break;
    }
  }
  while (lHasMalAddons) {
    aSkin->armorAddons.pop_back();
    lHasMalAddons = (aSkin->armorAddons.size() > 0) && (fAllMalAAs.find(aSkin->armorAddons.back()) != fAllMalAAs.end());
  }
  for (const auto &aa : lAddonsToAdd) aSkin->armorAddons.emplace_back(aa.second);
  return res;
}

std::map<RE::TESRace *, RE::TESObjectARMA *> Base::GetCombinedAddons(RE::TESObjectARMO *aSkin) noexcept {
  std::map<RE::TESRace *, RE::TESObjectARMA *> res{};
  std::set<RE::TESRace *> lReqRaces{};
  for (const auto &aa : aSkin->armorAddons) {
    if (!aa->HasPartOf(Tng::cSlotBody) || !aa->race) continue;
    if (aa->race->HasKeyword(fPRaceKey) && (!aa->race->armorParentRace || lReqRaces.find(aa->race->armorParentRace) == lReqRaces.end()))
      lReqRaces.insert(fRaceGroupInfoList[GetRG(aa->race)].armorRaces.begin(), fRaceGroupInfoList[GetRG(aa->race)].armorRaces.end());
    for (const auto &lAAAddRace : aa->additionalRaces)
      if (lAAAddRace->HasKeyword(fPRaceKey) && (!lAAAddRace->armorParentRace || lReqRaces.find(lAAAddRace->armorParentRace) == lReqRaces.end()))
        lReqRaces.insert(fRaceGroupInfoList[GetRG(lAAAddRace)].armorRaces.begin(), fRaceGroupInfoList[GetRG(lAAAddRace)].armorRaces.end());
  }
  for (const auto &race : lReqRaces) {
    for (const auto &aa : fMalAddonAAs[GetRaceType(race)][GetRGAddn(race)]) {
      if ((aa->race == race) || (std::find(aa->additionalRaces.begin(), aa->additionalRaces.end(), race) != aa->additionalRaces.end())) {
        res.insert_or_assign(aa->race, aa);
        break;
      }
    }
  }
  return res;
}

std::map<RE::TESRace *, RE::TESObjectARMA *> Base::GetAddonAAs(Tng::RaceType aRaceType, int aAddonIdx, bool aIsFemale) {
  std::map<RE::TESRace *, RE::TESObjectARMA *> res{};
  auto &list = aIsFemale ? fFemAddonAAs[aRaceType][aAddonIdx] : fMalAddonAAs[aRaceType][aAddonIdx];
  for (auto aa : list) res.insert({aa->race, aa});
  auto lAddType = Tng::cRaceTypeCount;
  if (aRaceType == Tng::raceManMer) lAddType = Tng::raceBeast;
  if (aRaceType == Tng::raceBeast) lAddType = Tng::raceManMer;
  if (lAddType < Tng::cRaceTypeCount) {
    auto &lAddList = aIsFemale ? fFemAddonAAs[lAddType][aAddonIdx] : fMalAddonAAs[lAddType][aAddonIdx];
    for (auto aa : lAddList) res.insert({aa->race, aa});
  }
  return res;
}

bool Base::LoadRGMult(const std::string aRGParent, const std::string aRGSkin, const float aSize) noexcept {
  auto race = LoadForm<RE::TESRace>(aRGParent);
  auto skin = LoadForm<RE::TESObjectARMO>(aRGSkin);
  if (!race || !skin) {
    SKSE::log::error("A previously saved race cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  fRaceGroupInfoList[{race, skin}].raceMult = aSize;
  return true;
}

bool Base::LoadRGAddn(const std::string aRGParent, const std::string aRGSkin, const std::string aAddonRecord) noexcept {
  auto race = LoadForm<RE::TESRace>(aRGParent);
  auto skin = LoadForm<RE::TESObjectARMO>(aRGSkin);
  if (!race || !skin) {
    SKSE::log::error("A previously saved race cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  auto addon = LoadForm<RE::TESObjectARMA>(aAddonRecord);
  if (!addon) {
    SKSE::log::error("A previously saved addon cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  fRaceGroupInfoList[{race, skin}].addon = addon;
  return true;
}

bool Base::LoadNPCSize(const std::string aNPCRecord, const int aSize) noexcept {
  auto npc = LoadForm<RE::TESNPC>(aNPCRecord);
  if (!npc) return false;
  for (std::size_t i = 0; i < Tng::cSizeCategories; i++)
    if (npc->HasKeyword(Tng::SizeKey(i))) npc->RemoveKeyword(Tng::SizeKey(i));
  npc->AddKeyword(Tng::SizeKey(aSize));
  return true;
}

bool Base::LoadNPCAddn(const std::string aNPCRecord, const std::string aAddonRecord) noexcept {
  auto npc = LoadForm<RE::TESNPC>(aNPCRecord);
  if (!npc) return false;
  auto addon = LoadForm<RE::TESObjectARMO>(aAddonRecord);
  if (!addon) {
    SKSE::log::error("The addon {} saved for NPC {} cannot be found anymore!", aAddonRecord, aNPCRecord);
    return false;
  }
  int lIdx = -1;
  const auto &lAddons = npc->IsFemale() ? fFemAddons : fMalAddons;
  for (int i = 0; i < lAddons.size(); i++)
    if (lAddons[i] == addon) {
      lIdx = i;
      break;
    }
  if (lIdx < 0) {
    SKSE::log::error("A previously installed addon {} cannot be loaded anymore! Please report this issue. {} would use original skin.", aAddonRecord, npc->GetName());
    return false;
  }
  if (lIdx == GetRGAddn(npc->race) && !npc->IsFemale()) return true;
  return SetNPCAddn(npc, lIdx, true);
}