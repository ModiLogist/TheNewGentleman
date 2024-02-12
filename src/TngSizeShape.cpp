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
  for (std::size_t i = 0; i < Tng::cSizeCategories; i++) {
    fSizeKws.push_back(fDH->LookupForm<RE::BGSKeyword>(cSizeKeyWIDs[i], Tng::cName));
    fSizeGlbs.push_back(fDH->LookupForm<RE::TESGlobal>(cSizeGlobIDs[i], Tng::cName));
    if (!fSizeKws[i] || !fSizeGlbs[i]) {
      Tng::gLogger::error("Could not find the information required to load saved size information for NPCs!");
      return false;
    }
  }
  return true;
}

void TngSizeShape::LoadAddons() noexcept {
  fMalAddons.clear();
  fFemAddons.clear();
  fActiveFemAddons.clear();
  auto &lAllArmor = fDH->GetFormArray<RE::TESObjectARMO>();
  for (const auto &lArmor : lAllArmor) {
    if (lArmor->HasKeyword(fFemAddKey)) {
      fFemAddons.push_back(lArmor);
      fActiveFemAddons.push_back(false);
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

std::size_t TngSizeShape::GetActiveFAddnCount() noexcept {
  std::size_t lRes = 0;
  for (const auto lActive : fActiveFemAddons)
    if (lActive) lRes++;
  return lRes;
}

int TngSizeShape::GetActualAddon(int aActiveAddon) noexcept {
  int lRes = -1;
  int lActives = -1;
  while (lActives != aActiveAddon) {
    lRes++;
    if (fActiveFemAddons[lRes]) lActives++;
  }
  return lRes;
}

RE::TESObjectARMO *TngSizeShape::GetAddonAt(bool aIsFemale, std::size_t aChoice) noexcept { return aIsFemale ? fFemAddons[aChoice] : fMalAddons[aChoice]; }

bool TngSizeShape::GetAddonStatus(std::size_t aFemaleAddon) noexcept { return fActiveFemAddons[aFemaleAddon]; }

void TngSizeShape::SetAddonStatus(std::size_t aFemaleAddon, bool aIsActive) noexcept { fActiveFemAddons[aFemaleAddon] = aIsActive; }

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
  GentifyMalSkin(fRacesInfo[aRaceIdx].originalSkin);
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
  lNPC->RemoveKeywords(fSizeKws);
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
  return SetNPCAddn(lNPC, lIdx, true);
}

void TngSizeShape::ExcludeNPC(const std::string aNPCRecord) noexcept {
  auto lNPC = LoadForm<RE::TESNPC>(aNPCRecord);
  if (!lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->skin) return;
  lNPC->AddKeyword(fExKey);
}

std::pair<bool, int> TngSizeShape::GetNPCAddn(RE::TESNPC *aNPC) noexcept {
  if (!aNPC) {
    Tng::gLogger::critical("Failure in getting a NPC shape!");
    return std::make_pair(false, Tng::pgErr);
  }
  for (auto lKw : aNPC->GetKeywords()) {
    const std::string lKwStr(lKw->formEditorID);
    if (lKwStr.starts_with(cNPCAutoAddn)) {
      return std::make_pair(false, std::strtol(lKwStr.substr(strlen(cNPCAutoAddn), 2).data(), nullptr, 0));
    }
    if (lKwStr.starts_with(cNPCUserAddn)) {
      return std::make_pair(true, std::strtol(lKwStr.substr(strlen(cNPCUserAddn), 2).data(), nullptr, 0));
    }
  }
  return std::make_pair(false, -1);
}

bool TngSizeShape::SetNPCAddn(RE::TESNPC *aNPC, int aAddon, bool aIsUser) noexcept {
  if (!aNPC) {
    Tng::gLogger::critical("Failure in setting a NPC shape!");
    return false;
  }
  auto &lList = aNPC->IsFemale() ? fFemAddons : fMalAddons;
  if (aNPC->HasKeyword(fGWKey)) aNPC->RemoveKeyword(fGWKey);
  if (aNPC->HasKeyword(fExKey)) aNPC->RemoveKeyword(fExKey);
  for (const auto &lExistingKw : aNPC->GetKeywords()) {
    if (lExistingKw->formEditorID.contains("TNG_ActorAddn")) aNPC->RemoveKeyword(lExistingKw);
  }
  if (aAddon == -2) {
    if (aNPC->IsFemale()) aNPC->AddKeyword(fExKey);
    for (auto lIt = fGentified->forms.begin(); lIt != fGentified->forms.end(); lIt++)
      if ((*lIt)->As<RE::TESNPC>() == aNPC) fGentified->forms.erase(lIt);
    return true;
  }
  if (lList.size() <= aAddon || aAddon < 0) {
    Tng::gLogger::critical("Cannot set the NPC {} to use addon {}! There are only {} addons.", aNPC->GetFormEditorID(), aAddon + 1, lList.size());
    return false;
  }
  auto lChoice = static_cast<std::size_t>(aAddon);
  auto &lAllKws = fDH->GetFormArray<RE::BGSKeyword>();
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
      Tng::gLogger::critical("Couldn't create keyword [{}]!", lReqKw);
      return true;
    }
  }
  aNPC->AddKeyword(lKw);
  if (lList[aAddon]->HasKeyword(fSkinWithPenisKey)) {
    fGentified->AddForm(aNPC);
    aNPC->AddKeyword(fGWKey);
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

float TngSizeShape::GetGlobalSize(std::size_t aIdx) noexcept {
  if (aIdx < 0 || aIdx >= Tng::cSizeCategories) return 1.0f;
  return fSizeGlbs[aIdx]->value;
}

void TngSizeShape::SetGlobalSize(std::size_t aIdx, float aSize) noexcept {
  if (aIdx < 0 || aIdx >= Tng::cSizeCategories) return;
  fSizeGlbs[aIdx]->value = aSize;
}

Tng::TNGRes TngSizeShape::SetActorSize(RE::Actor *aActor, int aGenSize) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  if (!lNPC->race) return Tng::raceErr;
  if (!(lNPC->race->HasKeyword(fPRaceKey) || lNPC->race->HasKeyword(fRRaceKey))) return Tng::raceErr;
  auto lCurSize = GetScale(lNPC);
  if (lCurSize == aGenSize || aGenSize == -1) {
    ScaleGenital(aActor, fSizeGlbs[lCurSize]);
    return Tng::resOkGen;
  }
  lNPC->RemoveKeywords(fSizeKws);
  if (aActor->IsPlayerRef() && aGenSize == -2) return Tng::resOkNoGen;
  lNPC->AddKeyword(fSizeKws[aGenSize]);
  ScaleGenital(aActor, fSizeGlbs[aGenSize]);
  return Tng::resOkGen;
}

std::set<RE::TESObjectARMA *> TngSizeShape::GentifyGrpSkin(int aRaceGrp) noexcept { return GentifyMalSkin(fRacesInfo[aRaceGrp].originalSkin); }

int TngSizeShape::GetScale(RE::TESNPC *aNPC) noexcept {
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (aNPC->HasKeyword(fSizeKws[i])) return i;
  }
  return (aNPC->formID % 5);
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
  aScrtNode->local.scale = 1.0f / sqrt(lScale);
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

Tng::RaceType TngSizeShape::GetSkinType(RE::TESObjectARMO *aSkin) noexcept { return GetRaceType(aSkin->race); }

Tng::RaceType TngSizeShape::GetRaceType(RE::TESRace *aRace) noexcept {
  if (!aRace) return Tng::raceManMer;
  if (aRace == fDefRace) return Tng::raceManMer;
  if (aRace->HasKeyword(fBstKey)) return Tng::raceBeast;
  if (aRace == fBaseRaces[10]) return Tng::raceDremora;
  if (aRace == fBaseRaces[11]) return Tng::raceElder;
  if (aRace == fBaseRaces[12]) return Tng::raceAfflicted;
  if (aRace == fBaseRaces[13]) return Tng::raceSnowElf;
  return Tng::raceManMer;
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
    if (!aIsFemale) fAllMalAAs.insert(lAA);
    std::set<RE::TESRace *> lAARaces{lAA->race};
    if (lAA->race != fBaseRaces[12]) lAARaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
    for (const auto &lRace : lAARaces) {
      if (lRace->HasKeyword(fBstKey)) {
        lAddonAAs[Tng::raceBeast][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[10] || lRace == fEqRaces[10]) {
        lAddonAAs[Tng::raceDremora][aIdx].insert(lAA);
        lAddonAAs[Tng::raceManMer][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[6] || lRace == fEqRaces[6]) {
        lAddonAAs[Tng::raceDremora][aIdx].insert(lAA);
        lAddonAAs[Tng::raceManMer][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[11] || lRace == fEqRaces[11]) {
        lAddonAAs[Tng::raceElder][aIdx].insert(lAA);
        lAddonAAs[Tng::raceManMer][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[12]) {
        lAddonAAs[Tng::raceAfflicted][aIdx].insert(lAA);
        continue;
      }
      if (lRace == fBaseRaces[13]) {
        lAddonAAs[Tng::raceSnowElf][aIdx].insert(lAA);
        continue;
      }
      lAddonAAs[Tng::raceManMer][aIdx].insert(lAA);
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

std::set<RE::TESObjectARMA *> TngSizeShape::GentifyMalSkin(RE::TESObjectARMO *aSkin, int aAddon) noexcept {
  auto lRes = std::set<RE::TESObjectARMA *>{};
  if (!aSkin->HasPartOf(Tng::cSlotGenital)) aSkin->AddSlotToMask(Tng::cSlotGenital);
  bool lHasAddons{false};
  for (auto &lAA : aSkin->armorAddons) {
    if (fAllMalAAs.find(lAA) != fAllMalAAs.end()) lHasAddons = true;
    if (lAA->HasPartOf(Tng::cSlotBody) && lAA->race && ((lAA->race == fDefRace) || lAA->race->HasKeyword(fPRaceKey))) lRes.insert(lAA);
  }
  auto lType = GetSkinType(aSkin);
  auto lAddons = aAddon >= 0 ? GetAddonAAs(lType, aAddon, false) : GetCombinedAddons(lType, aSkin);
  if (lHasAddons) {
    for (std::uint32_t i = 0; i < aSkin->armorAddons.size(); i++)
      if (fAllMalAAs.find(aSkin->armorAddons[i]) != fAllMalAAs.end()) aSkin->armorAddons[i] = lAddons.at(aSkin->armorAddons[i]->race);
  } else {
    for (auto &lAA : lAddons) aSkin->armorAddons.emplace_back(lAA.second);
  }
  return lRes;
}

std::set<RE::TESObjectARMA *> TngSizeShape::GentifyFemSkin(RE::TESObjectARMO *aSkin, int aAddon) noexcept {
  auto lRes = std::set<RE::TESObjectARMA *>{};
  if (aAddon < 0) return lRes;
  if (!aSkin->HasPartOf(Tng::cSlotGenital)) aSkin->AddSlotToMask(Tng::cSlotGenital);
  if (aSkin)
    for (auto &lAA : aSkin->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody) && lAA->race && ((lAA->race == fDefRace) || lAA->race->HasKeyword(fPRaceKey))) lRes.insert(lAA);
  auto lType = GetSkinType(aSkin);
  auto lAddonsToAdd = GetAddonAAs(lType, aAddon, true);
  bool lHasMalAddons{false};
  for (const auto &lAA : aSkin->armorAddons) {
    if (fAllMalAAs.find(lAA) != fAllMalAAs.end()) {
      lHasMalAddons = true;
      break;
    }
  }
  while (lHasMalAddons) {
    aSkin->armorAddons.pop_back();
    lHasMalAddons = (aSkin->armorAddons.size() > 0) && (fAllMalAAs.find(aSkin->armorAddons.back()) != fAllMalAAs.end());
  }
  for (const auto &lAA : lAddonsToAdd) aSkin->armorAddons.emplace_back(lAA.second);
  return lRes;
}

std::map<RE::TESRace *, RE::TESObjectARMA *> TngSizeShape::GetCombinedAddons(Tng::RaceType aRaceType, RE::TESObjectARMO *aSkin) noexcept {
  std::map<RE::TESRace *, RE::TESObjectARMA *> lRes{};
  std::set<RE::TESRace *> lReqRaces{};
  for (const auto &lAA : aSkin->armorAddons) {
    if (!lAA->HasPartOf(Tng::cSlotBody) || !lAA->race) continue;
    if (lAA->race->HasKeyword(fPRaceKey)) lReqRaces.insert(fRacesInfo[GetRaceGrp(lAA->race)].armorRaces.begin(), fRacesInfo[GetRaceGrp(lAA->race)].armorRaces.end());
    for (const auto &lAAAddRace : lAA->additionalRaces)
      if (lAAAddRace->HasKeyword(fPRaceKey)) lReqRaces.insert(fRacesInfo[GetRaceGrp(lAAAddRace)].armorRaces.begin(), fRacesInfo[GetRaceGrp(lAAAddRace)].armorRaces.end());
  }
  for (const auto &lRace : lReqRaces) {
    for (const auto &lAA : fMalAddonAAs[aRaceType][GetRaceGrpAddn(lRace)]) {
      if ((lAA->race == lRace) || (std::find(lAA->additionalRaces.begin(), lAA->additionalRaces.end(), lRace) != lAA->additionalRaces.end())) {
        lRes.insert_or_assign(lAA->race, lAA);
        break;
      }
    }
  }
  return lRes;
}

std::map<RE::TESRace *, RE::TESObjectARMA *> TngSizeShape::GetAddonAAs(Tng::RaceType aRaceType, int aAddonIdx, bool aIsFemale) {
  std::map<RE::TESRace *, RE::TESObjectARMA *> lRes{};
  auto &lList = aIsFemale ? fFemAddonAAs[aRaceType][aAddonIdx] : fMalAddonAAs[aRaceType][aAddonIdx];
  for (auto lAA : lList) lRes.insert({lAA->race, lAA});
  return lRes;
}
