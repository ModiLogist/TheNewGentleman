#include <TngSizeShape.h>

bool TngSizeShape::Init() noexcept {
  fDH = RE::TESDataHandler::GetSingleton();
  fDefRace = fDH->LookupForm<RE::TESRace>(Tng::cDefRaceID, Tng::cSkyrim);
  fBstKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cBstKeywID, Tng::cSkyrim);
  fFemAddKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cFemAddKeyID, Tng::cName);
  fMalAddKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cMalAddKeyID, Tng::cName);
  fPRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  fRRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  fIAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  fSkinWithPenisKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  fGWKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cGentleWomanKeyID, Tng::cName);
  fExKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cExcludeKeyID, Tng::cName);
  fGentified = fDH->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  if (!(fFemAddKey && fMalAddKey && fPRaceKey && fRRaceKey && fIAKey && fSkinWithPenisKey && fGWKey && fExKey && fGentified)) {
    Tng::gLogger::critical("Could not find the base information required for shape variations!");
    return false;
  }
  for (int i = 0; i < Tng::cVanillaRaceTypes; i++) {
    fBaseRaces[i] = fDH->LookupForm<RE::TESRace>(cBaseRaceIDs[i].first, cBaseRaceIDs[i].second);
    if (!fBaseRaces[i]) return false;
    fBaseRaces[i]->AddKeyword(fPRaceKey);
    auto lIdx = GetRaceGrp(fBaseRaces[i]);
    fRacesInfo[lIdx].raceName = cRaceNames[lIdx].first[0];
    fRacesInfo[lIdx].races.push_back(fBaseRaces[i]);
    fRacesInfo[lIdx].armorRaces.insert(fBaseRaces[i]->armorParentRace ? fBaseRaces[i]->armorParentRace : fBaseRaces[i]);
    fRacesInfo[lIdx].originalSkin = fBaseRaces[i]->skin;
    fRacesInfo[lIdx].raceDefAddon = cVanillaRaceDefaults[i];
    fRacesInfo[lIdx].raceAddn = cVanillaRaceDefaults[i];
  }
  for (int i = 0; i < Tng::cEqRaceTypes; i++) {
    fEqRaces[i] = fDH->LookupForm<RE::TESRace>(cEquiRaceIDs[i].first.first, cEquiRaceIDs[i].first.second);
    if (!fEqRaces[i]) return false;
    fEqRaces[i]->AddKeyword(fPRaceKey);
    GetRaceGrp(fEqRaces[i]);
  }
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    fSizeKws[i] = fDH->LookupForm<RE::BGSKeyword>(cSizeKeyWIDs[i], Tng::cName);
    fSizeGlbs[i] = fDH->LookupForm<RE::TESGlobal>(cSizeGlobIDs[i], Tng::cName);
    if (!fSizeKws[i] || !fSizeGlbs[i]) {
      Tng::gLogger::error("Could not find the information required to load saved size information for NPCs!");
      return false;
    }
  }
  return true;
}

void TngSizeShape::LoadAddons() noexcept {
  fFemAddons.clear();
  fMalAddons.clear();
  auto &lAllArmor = fDH->GetFormArray<RE::TESObjectARMO>();
  for (const auto &lArmor : lAllArmor) {
    if (lArmor->HasKeyword(fFemAddKey)) {
      fFemAddons.push_back(lArmor);
      lArmor->AddKeyword(fIAKey);
    }
    if (lArmor->HasKeyword(fMalAddKey)) {
      fMalAddons.push_back(lArmor);
      lArmor->AddKeyword(fIAKey);
    }
  }
  for (auto &lInfo : fRacesInfo)
    for (auto lRace : lInfo.armorRaces) UpdateAddons(lRace);
  CategorizeAddons();
}

std::size_t TngSizeShape::GetAddonCount(bool aIsFemale) noexcept { return aIsFemale ? fFemAddons.size() : fMalAddons.size(); }

RE::TESObjectARMO *TngSizeShape::GetAddonAt(bool aIsFemale, int aChoice) noexcept { return aIsFemale ? fFemAddons[aChoice] : fMalAddons[aChoice]; };

std::vector<std::string> TngSizeShape::GetAddonNames(bool aIsFemale) noexcept {
  std::vector<std::string> lRes{};
  auto &lList = aIsFemale ? fFemAddons : fMalAddons;
  for (int i = 0; i < lList.size(); i++) lRes.push_back(lList[i]->GetName());
  return lRes;
}

std::vector<std::string> TngSizeShape::GetAllPossibleAddons(RE::Actor *aActor) noexcept {
  std::vector<std::string> lRes{};
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return lRes;
  lRes.push_back("$TNG_AST");
  lRes.push_back(aActor->GetName());
  lRes.push_back("$TNG_ASR");
  for (auto lSkin : lNPC->IsFemale() ? fFemAddons : fMalAddons) {
    lRes.push_back(lSkin->GetName());
  }
  return lRes;
}

std::size_t TngSizeShape::GetRaceGrp(RE::TESRace *aRace) noexcept {
  for (std::size_t i = 0; i < fRacesInfo.size(); i++) {
    auto lIt = std::find(fRacesInfo[i].races.begin(), fRacesInfo[i].races.end(), aRace);
    if (lIt != fRacesInfo[i].races.end()) return i;
    if (strcmp(fRacesInfo[i].races[0]->GetName(), aRace->GetName()) == 0 && fRacesInfo[i].originalSkin == aRace->skin) {
      fRacesInfo[i].races.push_back(aRace);
      fRacesInfo[i].armorRaces.insert(aRace->armorParentRace ? aRace->armorParentRace : aRace);
      return i;
    }
    if (aRace->armorParentRace && fRacesInfo[i].originalSkin == aRace->skin && fRacesInfo[i].armorRaces.find(aRace->armorParentRace) != fRacesInfo[i].armorRaces.end()) {
      fRacesInfo[i].races.push_back(aRace);
      return i;
    }
  }
  if (aRace->armorParentRace && aRace->armorParentRace != fDefRace && aRace->armorParentRace->skin == aRace->skin) {
    fRacesInfo[GetRaceGrp(aRace->armorParentRace)].races.push_back(aRace);
    return GetRaceGrp(aRace->armorParentRace);
  } else {
    fRacesInfo.push_back(RaceInfo{});
    fRacesInfo.back().raceName = aRace->GetName();
    fRacesInfo.back().races.push_back(aRace);
    fRacesInfo.back().armorRaces.insert(aRace->armorParentRace ? aRace->armorParentRace : aRace);
    fRacesInfo.back().originalSkin = aRace->skin;
    fRacesInfo.back().raceDefAddon = 0;
    fRacesInfo.back().raceAddn = 0;
    return static_cast<std::size_t>(fRacesInfo.size() - 1);
  }
}

bool TngSizeShape::LoadRaceMult(const std::string aRaceRecord, const float aSize) noexcept {
  auto lRace = LoadForm<RE::TESRace>(aRaceRecord);
  if (!lRace) {
    Tng::gLogger::error("A previously saved race cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  fRacesInfo[GetRaceGrp(lRace)].raceMult = aSize;
  return true;
}

bool TngSizeShape::LoadRaceAddn(const std::string aRaceRecord, const std::string aAddonRecord) noexcept {
  auto lRace = LoadForm<RE::TESRace>(aRaceRecord);
  if (!lRace) {
    Tng::gLogger::error("A previously saved race cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  auto lAddon = LoadForm<RE::TESObjectARMO>(aAddonRecord);
  if (!lAddon) {
    Tng::gLogger::error("A previously saved addon cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  int lIdx = -1;
  for (int i = 0; i < fMalAddons.size(); i++)
    if (fMalAddons[i] == lAddon) {
      lIdx = i;
      break;
    }
  if (lIdx < 0) {
    Tng::gLogger::error("A previously installed addon {} cannot be loaded anymore! Please report this issue. {} would use original skin.", aAddonRecord, lRace->GetName());
    return true;
  }
  fRacesInfo[GetRaceGrp(lRace)].raceAddn = lIdx;
  return true;
}

float TngSizeShape::GetRaceGrpMult(RE::TESRace *aRace) noexcept {
  if (!aRace) return -1.0f;
  return fRacesInfo[GetRaceGrp(aRace)].raceMult;
}

float TngSizeShape::GetRaceGrpMult(const std::size_t aRaceIdx) noexcept {
  if (fRacesInfo.size() <= aRaceIdx) return -1.0f;
  return fRacesInfo[aRaceIdx].raceMult;
}

bool TngSizeShape::SetRaceGrpMult(RE::TESRace *aRace, const float aMult) noexcept {
  if (!aRace || aMult < 0.1f || aMult >= 10.0f) {
    Tng::gLogger::critical("Failure in setting a race mult!");
    return false;
  }
  fRacesInfo[GetRaceGrp(aRace)].raceMult = aMult;
  return true;
}

bool TngSizeShape::SetRaceGrpMult(const std::size_t aRaceIdx, const float aMult) noexcept {
  if (fRacesInfo.size() <= aRaceIdx || aMult < 0.1f || aMult >= 10.0f) {
    Tng::gLogger::critical("Failure in setting a race mult!");
    return false;
  }
  fRacesInfo[aRaceIdx].raceMult = aMult;
  return true;
}

int TngSizeShape::GetRaceGrpDefAddn(RE::TESRace *aRace) noexcept {
  if (!aRace) return Tng::pgErr;
  return fRacesInfo[GetRaceGrp(aRace)].raceDefAddon;
}

int TngSizeShape::GetRaceGrpDefAddn(const std::size_t aRaceIdx) noexcept {
  if (fRacesInfo.size() <= aRaceIdx) return Tng::pgErr;
  return fRacesInfo[aRaceIdx].raceDefAddon;
}

bool TngSizeShape::SetRaceGrpDefAddn(RE::TESRace *aRace, int aChoice) noexcept {
  if (!aRace || aChoice >= GetAddonCount(false) || aChoice < 0) {
    Tng::gLogger::critical("Failure in setting a race addon!");
    return false;
  }
  fRacesInfo[GetRaceGrp(aRace)].raceDefAddon = aChoice;
  return true;
}

bool TngSizeShape::SetRaceGrpDefAddn(const std::size_t aRaceIdx, int aChoice) noexcept {
  if (fRacesInfo.size() <= aRaceIdx || aChoice >= GetAddonCount(false) || aChoice < 0) {
    Tng::gLogger::critical("Failure in setting a race addon!");
    return false;
  }
  fRacesInfo[aRaceIdx].raceDefAddon = aChoice;
  return true;
}

int TngSizeShape::GetRaceGrpAddn(RE::TESRace *aRace) noexcept {
  if (!aRace) return Tng::pgErr;
  return fRacesInfo[GetRaceGrp(aRace)].raceAddn;
}

int TngSizeShape::GetRaceGrpAddn(const std::size_t aRaceIdx) noexcept {
  if (fRacesInfo.size() <= aRaceIdx) return Tng::pgErr;
  return fRacesInfo[aRaceIdx].raceAddn;
}

void TngSizeShape::UpdateRaceGrpAddn(const std::size_t aRaceIdx, const int aAddon) noexcept {
  fRacesInfo[aRaceIdx].raceAddn = aAddon;
  GentifySkin(fRacesInfo[aRaceIdx].originalSkin);
  for (auto lRace : fRacesInfo[aRaceIdx].races) lRace->skin = fRacesInfo[aRaceIdx].originalSkin;
}

std::size_t TngSizeShape::GroupCount() noexcept { return fRacesInfo.size(); }

RE::TESRace *TngSizeShape::GetRaceByIdx(const std::size_t aRaceIdx) noexcept { return fRacesInfo[aRaceIdx].races[0]; }

std::string TngSizeShape::GetRaceName(const std::size_t aRaceIdx) noexcept { return fRacesInfo[aRaceIdx].raceName; }

std::vector<std::string> TngSizeShape::GetRaceGrpNames() noexcept {
  std::vector<std::string> lRes{};
  for (const auto &lEntry : fRacesInfo) lRes.push_back(lEntry.raceName);
  return lRes;
}

RE::TESObjectARMO *TngSizeShape::GetRaceGrpSkin(int aRaceIdx) noexcept { return fRacesInfo[aRaceIdx].originalSkin; }

bool TngSizeShape::LoadNPCSize(const std::string aNPCRecord, const int aSize) noexcept {
  auto lNPC = LoadForm<RE::TESNPC>(aNPCRecord);
  if (!lNPC) return false;
  for (int i = 0; i < Tng::cSizeCategories; i++)
    if (lNPC->HasKeyword(fSizeKws[i])) lNPC->RemoveKeyword(fSizeKws[i]);
  lNPC->AddKeyword(fSizeKws[aSize]);
  return true;
}

bool TngSizeShape::LoadNPCAddn(const std::string aNPCRecord, const std::string aAddonRecord) noexcept {
  auto lNPC = LoadForm<RE::TESNPC>(aNPCRecord);
  if (!lNPC) return false;
  auto lAddon = LoadForm<RE::TESObjectARMO>(aAddonRecord);
  if (!lAddon) {
    Tng::gLogger::error("The addon {} saved for NPC {} cannot be found anymore!", aAddonRecord, aNPCRecord);
    return false;
  }
  int lIdx = -1;
  const auto &lAddons = lNPC->IsFemale() ? fFemAddons : fMalAddons;
  for (int i = 0; i < lAddons.size(); i++)
    if (lAddons[i] == lAddon) {
      lIdx = i;
      break;
    }
  if (lIdx < 0) {
    Tng::gLogger::error("A previously installed addon {} cannot be loaded anymore! Please report this issue. {} would use original skin.", aAddonRecord, lNPC->GetName());
    return false;
  }
  if (lIdx == GetRaceGrpAddn(lNPC->race) && !lNPC->IsFemale()) return true;
  return SetNPCAddn(lNPC, lIdx);
}

void TngSizeShape::ExcludeNPC(const std::string aNPCRecord) noexcept {
  auto lNPC = LoadForm<RE::TESNPC>(aNPCRecord);
  if (!lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->skin) return;
  lNPC->AddKeyword(fExKey);
}

int TngSizeShape::GetNPCAddn(RE::TESNPC *aNPC) noexcept {
  if (!aNPC) {
    Tng::gLogger::critical("Failure in getting a NPC shape!");
    return Tng::pgErr;
  }
  for (auto lKw : aNPC->GetKeywords()) {
    const std::string lKwStr(lKw->formEditorID);
    if (lKwStr.starts_with(cNPCAddn)) {
      return std::strtol(lKwStr.substr(strlen(cNPCAddn), 2).data(), nullptr, 0);
    }
  }
  return Tng::resOkNoAddon;
}

bool TngSizeShape::SetNPCAddn(RE::TESNPC *aNPC, int aAddon) noexcept {
  if (!aNPC) {
    Tng::gLogger::critical("Failure in setting a NPC shape!");
    return false;
  }
  auto &lList = aNPC->IsFemale() ? fFemAddons : fMalAddons;
  if (aAddon == -2) {
    if (aNPC->HasKeyword(fGWKey)) aNPC->RemoveKeyword(fGWKey);
    for (const auto &lExistingKw : aNPC->GetKeywords()) {
      if (lExistingKw->formEditorID.contains(cNPCAddn)) aNPC->RemoveKeyword(lExistingKw);
    }
    if (aNPC->IsFemale()) aNPC->AddKeyword(fExKey);
    return true;
  }
  if (lList.size() <= aAddon || aAddon < 0) {
    Tng::gLogger::critical("Cannot set the NPC {} to use addon {}! There are only {} addons.", aNPC->GetFormEditorID(), aAddon + 1, lList.size());
    return false;
  }
  if (aNPC->HasKeyword(fExKey)) aNPC->RemoveKeyword(fExKey);
  auto lChoice = static_cast<std::size_t>(aAddon);
  auto &lAllKws = fDH->GetFormArray<RE::BGSKeyword>();
  std::string lReqKw = cNPCAddn + (lChoice < 10 ? "0" + std::to_string(lChoice) : std::to_string(lChoice));
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
      Tng::gLogger::critical("Couldn't create keyword [{}]!", lReqKw);
      return true;
    }
  }
  for (const auto &lExistingKw : aNPC->GetKeywords()) {
    if (lExistingKw->formEditorID.contains(cNPCAddn)) aNPC->RemoveKeyword(lExistingKw);
  }
  aNPC->AddKeyword(lKw);
  if (lList[aAddon]->HasKeyword(fSkinWithPenisKey)) {
    if (!fGentified->HasForm(aNPC)) fGentified->AddForm(aNPC);
    if (!aNPC->HasKeyword(fGWKey)) aNPC->AddKeyword(fGWKey);
  } else {
    if (aNPC->HasKeyword(fGWKey)) aNPC->RemoveKeyword(fGWKey);
  }
  return true;
}

Tng::TNGRes TngSizeShape::CanModifyActor(RE::Actor *aActor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  if (!lNPC->race) return Tng::raceErr;
  if (lNPC->race->HasKeyword(fPRaceKey)) return Tng::resOkRaceP;
  if (lNPC->race->HasKeyword(fRRaceKey)) return Tng::resOkRaceR;
  return Tng::raceErr;
}

float TngSizeShape::GetGlobalSize(int aIdx) noexcept {
  if (aIdx < 0 || aIdx >= Tng::cSizeCategories) return 1.0f;
  return fSizeGlbs[aIdx]->value;
}

void TngSizeShape::SetGlobalSize(int aIdx, float aSize) noexcept {
  if (aIdx < 0 || aIdx >= Tng::cSizeCategories) return;
  fSizeGlbs[aIdx]->value = aSize;
}

void TngSizeShape::RandomizeScale(RE::Actor *aActor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!(lNPC->race->HasKeyword(fPRaceKey) || lNPC->race->HasKeyword(fRRaceKey))) return;
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (lNPC->HasKeyword(fSizeKws[i])) {
      ScaleGenital(aActor, fSizeGlbs[i]);
      return;
    }
  }
  const int lDefSize = lNPC->formID % 5;
  lNPC->AddKeyword(fSizeKws[lDefSize]);
  ScaleGenital(aActor, fSizeGlbs[lDefSize]);
}

Tng::TNGRes TngSizeShape::SetActorSize(RE::Actor *aActor, int aGenSize) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  if (!lNPC->race) return Tng::raceErr;
  if (!(lNPC->race->HasKeyword(fPRaceKey) || lNPC->race->HasKeyword(fRRaceKey))) return Tng::raceErr;
  for (int i = 0; i < Tng::cSizeCategories; i++)
    if (lNPC->HasKeyword(fSizeKws[i])) lNPC->RemoveKeyword(fSizeKws[i]);
  lNPC->AddKeyword(fSizeKws[aGenSize]);
  ScaleGenital(aActor, fSizeGlbs[aGenSize]);
  return Tng::resOkGen;
}

void TngSizeShape::ScaleGenital(RE::Actor *aActor, RE::TESGlobal *aGlobal) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  auto lScale = GetRaceGrpMult(lNPC->race) * aGlobal->value;
  RE::NiAVObject *aBaseNode = aActor->GetNodeByName(cBaseBone);
  RE::NiAVObject *aScrtNode = aActor->GetNodeByName(cScrtBone);
  if (!aBaseNode || !aScrtNode) return;
  aBaseNode->local.scale = lScale;
  aScrtNode->local.scale = 1.0f / lScale;
}

std::set<RE::TESObjectARMA *> TngSizeShape::GetAddonAAs(TNGRaceTypes aRaceType, int aAddonIdx, bool aIsFemale) {
  return aIsFemale ? fFemAddonAAs[aRaceType][aAddonIdx] : fMalAddonAAs[aRaceType][aAddonIdx];
}

void TngSizeShape::UpdateAddons(RE::TESRace *aRace) noexcept {
  auto lBaseRace = FindEqVanilla(aRace);
  std::set<RE::TESObjectARMO *> lAllAddons{fMalAddons.begin(), fMalAddons.end()};
  lAllAddons.insert(fFemAddons.begin(), fFemAddons.end());
  for (auto &lAddon : lAllAddons) {
    bool lSupports = false;
    for (auto &lAA : lAddon->armorAddons) {
      if (lAA->race == aRace || std::find(lAA->additionalRaces.begin(), lAA->additionalRaces.end(), aRace) != lAA->additionalRaces.end()) {
        lSupports = true;
        break;
      }
      auto lIt = std::find(lAA->additionalRaces.begin(), lAA->additionalRaces.end(), lBaseRace);
      if (lAA->race == lBaseRace || lIt != lAA->additionalRaces.end()) {
        lAA->additionalRaces.emplace_back(aRace);
        lSupports = true;
        break;
      }
    }
    if (!lSupports)
      Tng::gLogger::error("The addon [0x{:x}] from file [{}] cannot support the race [{:x}: {}]!", lAddon->GetLocalFormID(), lAddon->GetFile(0)->GetFilename(),
                          aRace->GetLocalFormID(), aRace->GetFormEditorID());
  }
};

TngSizeShape::TNGRaceTypes TngSizeShape::GetSkinType(RE::TESObjectARMO *aSkin) noexcept {
  if (aSkin->race == fDefRace) return raceManMer;
  if (aSkin->race->HasKeyword(fBstKey)) return raceBeast;
  if (aSkin->race == fBaseRaces[10]) return raceDremora;
  if (aSkin->race == fBaseRaces[11]) return raceElder;
  if (aSkin->race == fBaseRaces[12]) return raceAfflicted;
  if (aSkin->race == fBaseRaces[13]) return raceSnowElf;
  return raceManMer;
}

void TngSizeShape::CategorizeAddons() noexcept {
  auto lEmptySet = std::set<RE::TESObjectARMA *>{};
  for (auto &lCAs : fMalAddonAAs) lCAs = std::vector<std::set<RE::TESObjectARMA *>>(fMalAddons.size(), lEmptySet);
  for (auto &lCAs : fFemAddonAAs) lCAs = std::vector<std::set<RE::TESObjectARMA *>>(fFemAddons.size(), lEmptySet);
  for (int i = 0; i < fMalAddons.size(); i++) CategorizeAddon(fMalAddons[i], i, false);
  for (int i = 0; i < fFemAddons.size(); i++) CategorizeAddon(fFemAddons[i], i, true);
}

void TngSizeShape::CategorizeAddon(RE::TESObjectARMO *aAddon, const int aIdx, bool aIsFemale) noexcept {
  auto &lAddonAAs = aIsFemale ? fFemAddonAAs : fMalAddonAAs;
  for (const auto &lAA : aAddon->armorAddons) {
    if (!lAA->HasPartOf(Tng::cSlotGenital)) continue;
    std::set<RE::TESRace *> lAARaces{lAA->race};
    if (lAA->race != fBaseRaces[12]) lAARaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
    for (const auto &lRace : lAARaces) {
      if (lRace->HasKeyword(fBstKey)) {
        lAddonAAs[raceBeast][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[10] || lRace == fEqRaces[10]) {
        lAddonAAs[raceDremora][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[11] || lRace == fEqRaces[11]) {
        lAddonAAs[raceElder][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[12]) {
        lAddonAAs[raceAfflicted][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[13]) {
        lAddonAAs[raceSnowElf][aIdx].insert(lAA);
        continue;
      }
      lAddonAAs[raceManMer][aIdx].insert(lAA);
    }
  }
}

RE::TESRace *TngSizeShape::FindEqVanilla(RE::TESRace *aRace) noexcept {
  if (std::find(std::begin(fBaseRaces), std::end(fBaseRaces), aRace) != std::end(fBaseRaces)) return aRace;
  if (std::find(std::begin(fEqRaces), std::end(fEqRaces), aRace) != std::end(fEqRaces)) return aRace;
  const auto lRaceDesc = std::string(aRace->GetFormEditorID()) + std::string(aRace->GetName());
  for (int i = 0; i < Tng::cVanillaRaceTypes; i++) {
    if (lRaceDesc.contains(cRaceNames[i].first[0]) || lRaceDesc.contains(cRaceNames[i].first[1]) && (aRace->HasKeyword(fBstKey) == cRaceNames[i].second)) return fBaseRaces[i];
  }
  return aRace->HasKeyword(fBstKey) ? fBaseRaces[9] : fBaseRaces[0];
}

std::set<RE::TESObjectARMA *> TngSizeShape::GentifySkin(RE::TESObjectARMO *aSkin, int aAddon, bool aIsFemale) noexcept {
  auto lRes = std::set<RE::TESObjectARMA *>{};
  if (!aSkin->HasPartOf(Tng::cSlotGenital)) aSkin->AddSlotToMask(Tng::cSlotGenital);
  for (auto &lAA : aSkin->armorAddons)
    if (lAA->HasPartOf(Tng::cSlotBody) && (lAA->race == fDefRace) || lAA->race->HasKeyword(fPRaceKey)) lRes.insert(lAA);
  std::set<RE::TESRace *> lReqRaces{};
  bool lHasAddons = false;
  for (const auto &lAA : aSkin->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotGenital)) lHasAddons = true;
    if (!lAA->HasPartOf(Tng::cSlotBody)) continue;
    if (lAA->race->HasKeyword(fPRaceKey)) lReqRaces.insert(fRacesInfo[GetRaceGrp(lAA->race)].armorRaces.begin(), fRacesInfo[GetRaceGrp(lAA->race)].armorRaces.end());
    for (const auto &lAAAddRace : lAA->additionalRaces)
      if (lAAAddRace->HasKeyword(fPRaceKey)) lReqRaces.insert(fRacesInfo[GetRaceGrp(lAAAddRace)].armorRaces.begin(), fRacesInfo[GetRaceGrp(lAAAddRace)].armorRaces.end());
  }
  while (lHasAddons) {
    auto lAA = aSkin->armorAddons.back();
    lHasAddons = lAA->HasPartOf(Tng::cSlotGenital);
    if (lHasAddons) aSkin->armorAddons.pop_back();
  }
  auto lType = GetSkinType(aSkin);
  auto lAddonsToAdd = aAddon >= 0 ? GetAddonAAs(lType, aAddon, aIsFemale) : GetCombinedAddons(lType, lReqRaces);
  for (const auto &lAA : lAddonsToAdd) aSkin->armorAddons.emplace_back(lAA);
  return lRes;
}

std::set<RE::TESObjectARMA *> TngSizeShape::GentifySkin(int aRaceGrp) noexcept { return GentifySkin(fRacesInfo[aRaceGrp].originalSkin); }

std::set<RE::TESObjectARMA *> TngSizeShape::GetCombinedAddons(TNGRaceTypes aRaceType, std::set<RE::TESRace *> aReqRaces) noexcept {
  std::set<RE::TESObjectARMA *> lRes{};
  for (const auto &lRace : aReqRaces) {
    for (const auto &lAA : fMalAddonAAs[aRaceType][GetRaceGrpAddn(lRace)]) {
      if ((lAA->race == lRace) || (std::find(lAA->additionalRaces.begin(), lAA->additionalRaces.end(), lRace) != lAA->additionalRaces.end())) {
        lRes.insert(lAA);
        break;
      }
    }
  }
  return lRes;
}
