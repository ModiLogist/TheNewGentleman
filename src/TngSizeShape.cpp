#include <TngSizeShape.h>

bool TngSizeShape::Init() noexcept {
  fDH = RE::TESDataHandler::GetSingleton();
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
}

std::size_t TngSizeShape::GetAddonCount(bool aIsFemale) noexcept { return aIsFemale ? fFemAddons.size() : fMalAddons.size(); }

RE::TESObjectARMO *TngSizeShape::GetAddonAt(bool aIsFemale, int aChoice) noexcept { return aIsFemale ? fFemAddons[aChoice] : fMalAddons[aChoice]; };

std::set<std::string> TngSizeShape::GetAddonNames(bool aIsFemale) noexcept {
  std::set<std::string> lRes{};
  auto &lList = aIsFemale ? fFemAddons : fMalAddons;
  for (int i = 0; i < lList.size(); i++) lRes.insert(lList[i]->GetName());
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

bool TngSizeShape::LoadRaceMult(const std::string aRaceRecord, const int aSize100) noexcept {
  auto lRace = LoadForm<RE::TESRace>(aRaceRecord);
  if (!lRace) {
    Tng::gLogger::error("A previously saved race cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  SetRaceMult(lRace, static_cast<float>(aSize100 / 100.0f));
  auto lRaceEntry = std::find_if(fRaceInfo.begin(), fRaceInfo.end(), [&lRace](const std::pair<std::string, std::set<RE::TESRace *>> &p) { return p.first == lRace->GetName(); });
  if (lRaceEntry == fRaceInfo.end())
    fRaceInfo.push_back(std::make_pair<std::string, std::set<RE::TESRace *>>(lRace->GetName(), std::set<RE::TESRace *>{lRace}));
  else
    lRaceEntry->second.insert(lRace);
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
  SetRaceAddn(lRace, lIdx);
  return true;
}

float TngSizeShape::GetRaceMult(RE::TESRace *aRace) noexcept {
  if (!aRace) {
    Tng::gLogger::critical("Failure in getting a race mult!");
    return 1.0f;
  }
  for (auto lKw : aRace->GetKeywords()) {
    const std::string lKwStr(lKw->formEditorID);
    if (lKwStr.starts_with(cRaceMult)) {
      const int lMult100 = std::strtol(lKwStr.substr(strlen(cRaceMult), 3).data(), nullptr, 0);
      return static_cast<float>(lMult100) / 100.0f;
    }
  }
  return 1.0f;
}

bool TngSizeShape::SetRaceMult(RE::TESRace *aRace, const float aMult) noexcept {
  if (!aRace) {
    Tng::gLogger::critical("Failure in setting a race mult!");
    return false;
  }
  auto &lAllKws = fDH->GetFormArray<RE::BGSKeyword>();
  int lRaceMult100 = static_cast<int>(aMult * 100);
  if (lRaceMult100 > 1000) {
    Tng::gLogger::warn("Cannot set the race multiplier to a value equal or greater than 10! It was set 9.99");
    lRaceMult100 = 999;
  }
  if (lRaceMult100 < 10) {
    Tng::gLogger::warn("Cannot set the race multiplier to a value smaller than 0.1! It was set 0.1");
    lRaceMult100 = 10;
  }
  std::string lReqKw = cRaceMult + (lRaceMult100 < 100 ? "0" + std::to_string(lRaceMult100) : std::to_string(lRaceMult100));
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
      Tng::gLogger::info("Couldn't create keyword [{}]!", lReqKw);
      return false;
    }
  }
  for (const auto &lExistingKw : aRace->GetKeywords()) {
    if (lExistingKw->formEditorID.contains(cRaceMult)) aRace->RemoveKeyword(lExistingKw);
  }
  aRace->AddKeyword(lKw);
  return true;
}

int TngSizeShape::GetRaceAddn(RE::TESRace *aRace) noexcept {
  if (!aRace) {
    Tng::gLogger::critical("Failure in getting a race shape!");
    return Tng::pgErr;
  }
  for (auto lKw : aRace->GetKeywords()) {
    const std::string lKwStr(lKw->formEditorID);
    if (lKwStr.starts_with(cRaceAddn)) {
      return std::strtol(lKwStr.substr(strlen(cRaceAddn), 2).data(), nullptr, 0);
    }
  }
  return Tng::pgErr;
}

void TngSizeShape::SetRaceAddn(RE::TESRace *aRace, int aChoice) noexcept {
  if (!aRace) {
    Tng::gLogger::critical("Failure in setting a race shape!");
    return;
  }
  auto &lAllKws = fDH->GetFormArray<RE::BGSKeyword>();
  auto lAddon = static_cast<std::size_t>(aChoice);
  if ((fMalAddons.size() <= lAddon) || (lAddon < 0)) {
    Tng::gLogger::critical("Cannot set the race {} to use addon {}! There are only {} addons.", aRace->GetFormEditorID(), lAddon + 1, fMalAddons.size());
    lAddon = 0;
  }
  std::string lReqKw = cRaceAddn + (lAddon < 10 ? "0" + std::to_string(lAddon) : std::to_string(lAddon));
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
      Tng::gLogger::info("Couldn't create keyword [{}]!", lReqKw);
      return;
    }
  }
  for (const auto &lExistingKw : aRace->GetKeywords()) {
    if (lExistingKw->formEditorID.contains(cRaceAddn)) aRace->RemoveKeyword(lExistingKw);
  }
  aRace->AddKeyword(lKw);
  auto lRaceEntry =
      std::find_if(fRaceInfo.begin(), fRaceInfo.end(), [&aRace](const std::pair<std::string_view, std::set<RE::TESRace *>> &p) { return p.first == aRace->GetName(); });
  if (lRaceEntry == fRaceInfo.end())
    fRaceInfo.push_back(std::make_pair<std::string, std::set<RE::TESRace *>>(aRace->GetName(), std::set<RE::TESRace *>{aRace}));
  else
    lRaceEntry->second.insert(aRace);
}

float TngSizeShape::GetRaceMult(const std::size_t aRaceIdx) noexcept {
  if (fRaceInfo.size() <= aRaceIdx) return -1.0f;
  return GetRaceMult(*fRaceInfo[aRaceIdx].second.begin());
}

bool TngSizeShape::SetRaceMult(const std::size_t aRaceIdx, const float aMult) noexcept {
  if (fRaceInfo.size() <= aRaceIdx) return false;
  if (aMult < 0.1f || aMult >= 10.0f) return false;
  for (const auto &lRace : fRaceInfo[aRaceIdx].second) SetRaceMult(lRace, aMult);
  return true;
}

int TngSizeShape::GetRaceAddn(const std::size_t aRaceIdx) noexcept {
  if (fRaceInfo.size() <= aRaceIdx) return -1;
  return GetRaceAddn(*fRaceInfo[aRaceIdx].second.begin());
}

bool TngSizeShape::SetRaceAddn(const std::size_t aRaceIdx, int aChoice) noexcept {
  if (fRaceInfo.size() <= aRaceIdx) return false;
  if (aChoice < GetAddonCount(false) + 1) return false;
  for (const auto &lRace : fRaceInfo[aRaceIdx].second) SetRaceAddn(lRace, aChoice);
}

std::set<RE::TESRace *> TngSizeShape::GetRacesByIdx(const std::size_t aRaceIdx) noexcept {
  std::set<RE::TESRace *> lRes{};
  if (fRaceInfo.size() < aRaceIdx + 1) return lRes;
  return fRaceInfo[aRaceIdx].second;
}

std::vector<std::string> TngSizeShape::GetRaceNames() noexcept {
  std::vector<std::string> lRes{};
  for (const auto &lEntry : fRaceInfo) lRes.push_back(lEntry.first);
  return lRes;
}

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
  if (!lNPC->race) return false;
  if (!lNPC->race->skin) return false;
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
  if (lIdx == GetRaceAddn(lNPC->race)) return true;
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
  RE::BGSKeyword *lSizeKws[Tng::cSizeCategories]{};
  RE::TESGlobal *lSizeGlbs[Tng::cSizeCategories]{};
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return;

  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (lNPC->HasKeyword(lSizeKws[i])) {
      ScaleGenital(aActor, lSizeGlbs[i]);
      return;
    }
  }
  const int lDefSize = lNPC->formID % 5;
  lNPC->AddKeyword(lSizeKws[lDefSize]);
  ScaleGenital(aActor, lSizeGlbs[lDefSize]);
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
  auto lScale = GetRaceMult(lNPC->race) * aGlobal->value;
  RE::NiAVObject *aBaseNode = aActor->GetNodeByName(cBaseBone);
  RE::NiAVObject *aScrtNode = aActor->GetNodeByName(cScrtBone);
  if (!aBaseNode || !aScrtNode) return;
  aBaseNode->local.scale = lScale;
  aScrtNode->local.scale = 1.0f / lScale;
}
