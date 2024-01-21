#include <TngSizeShape.h>

bool TngSizeShape::Init() noexcept {
  fDH = RE::TESDataHandler::GetSingleton();
  fFemAddKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cFemAddKeyID, Tng::cName);
  fMalAddKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cMalAddKeyID, Tng::cName);
  fPRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  fRRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  fIAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  fSkinWithPenisKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  fCustomSkin = fDH->LookupForm<RE::BGSKeyword>(Tng::cCustomSkinID, Tng::cName);
  fFemAddLst = fDH->LookupForm<RE::BGSListForm>(cFemAddLstID, Tng::cName);
  fMalAddLst = fDH->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  fGentified = fDH->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  if (!(fFemAddKey && fMalAddKey && fPRaceKey && fRRaceKey && fIAKey && fSkinWithPenisKey && fFemAddLst && fMalAddLst && fGentified)) {
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
}

void TngSizeShape::LoadAddons() noexcept {
  auto &lAllArmor = fDH->GetFormArray<RE::TESObjectARMO>();
  for (const auto &lArmor : lAllArmor) {
    if (lArmor->HasKeyword(fFemAddKey)) {
      if (!fFemAddLst->HasForm(lArmor)) fFemAddLst->AddForm(lArmor);
      lArmor->AddKeyword(fIAKey);
    }
    if (lArmor->HasKeyword(fMalAddKey)) {
      if (!fMalAddLst->HasForm(lArmor)) fMalAddLst->AddForm(lArmor);
      lArmor->AddKeyword(fIAKey);
    }
  }
}

int TngSizeShape::GetAddonCount(bool aIsFemale) noexcept {
  auto lAddLst = aIsFemale ? fFemAddLst : fMalAddLst;
  if (!lAddLst) return 0;
  return lAddLst->forms.size();
}

std::set<std::string> TngSizeShape::GetAddonNames(bool aIsFemale) noexcept {
  std::set<std::string> lRes{};
  auto lCount = GetAddonCount(aIsFemale);
  for (int i = 0; i < lCount; i++) lRes.insert(GetAddonAt(aIsFemale, i)->GetName());
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

bool TngSizeShape::LoadNPCShape(const std::string aNPCRecord, const std::string aShapeRecord) noexcept {
  auto lNPC = LoadForm<RE::TESNPC>(aNPCRecord);
  if (!lNPC) return false;
  if (!lNPC->race) return false;
  if (!lNPC->race->skin) return false;
  auto lShape = LoadForm<RE::TESObjectARMO>(aShapeRecord);
  if (!lShape) {
    Tng::gLogger::error("The addon {} saved for NPC {} cannot be found anymore!", aShapeRecord, aNPCRecord);
    return false;
  }
  auto lAddLst = lNPC->IsFemale() ? fFemAddLst : fMalAddLst;
  auto lIdx = FindInFormList(lShape, lAddLst);
  if (lIdx < 0) {
    Tng::gLogger::error("A previously installed addon {} cannot be loaded anymore! Please report this issue. {} would use original skin.", aShapeRecord, lNPC->GetName());
    return true;
  }
  if (lIdx == GetRaceShape(lNPC->race)) return true;
  auto &lAllKws = fDH->GetFormArray<RE::BGSKeyword>();
  std::string lReqKw = cActorShape + (lIdx < 10 ? "0" + std::to_string(lIdx) : std::to_string(lIdx));
  auto lKwIt = std::find_if(lAllKws.begin(), lAllKws.end(), [&](const auto &kw) { return kw && kw->formEditorID == lReqKw.c_str(); });
  RE::BGSKeyword *lKw{nullptr};
  if (lKwIt != lAllKws.end()) {
    const auto lKw = *lKwIt;
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
  for (const auto &lExistingKw : lNPC->GetKeywords()) {
    if (lExistingKw->formEditorID.contains(cActorShape)) lNPC->RemoveKeyword(lExistingKw);
  }
  lNPC->AddKeyword(lKw);
  return true;
}

bool TngSizeShape::LoadRaceMult(const std::string aRaceRecord, const int aSize100) noexcept {
  auto lRace = LoadForm<RE::TESRace>(aRaceRecord);
  if (!lRace) {
    Tng::gLogger::error("A previously saved race {} cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  SetRaceMult(lRace, static_cast<float>(aSize100 / 100.0f));
  auto lRaceEntry = std::find_if(fRaceInfo.begin(), fRaceInfo.end(), [&lRace](const std::pair<std::string_view, int> &p) { return p.first == lRace->GetName(); });
  if (lRaceEntry == fRaceInfo.end())
    fRaceInfo.push_back(std::make_pair<std::string, std::set<RE::TESRace *>>(lRace->GetName(), std::set<RE::TESRace *>{lRace}));
  else
    lRaceEntry->second.insert(lRace);
  return true;
}

bool TngSizeShape::LoadRaceShape(const std::string aRaceRecord, const std::string aShapeRecord) noexcept {
  auto lRace = LoadForm<RE::TESRace>(aRaceRecord);
  if (!lRace) {
    Tng::gLogger::error("A previously saved race {} cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  auto lShape = LoadForm<RE::TESObjectARMO>(aShapeRecord);
  if (!lShape) {
    Tng::gLogger::error("A previously saved addon {} for race {} cannot be found anymore! It's information is removed from ini file.");
    return false;
  }
  auto lIdx = FindInFormList(lShape, fMalAddLst);
  if (lIdx < 0) {
    Tng::gLogger::error("A previously installed addon {} cannot be loaded anymore! Please report this issue. {} would use original skin.", aShapeRecord, lRace->GetName());
    return true;
  }
  SetRaceShape(lRace, lIdx);
  return true;
}

float TngSizeShape::GetRaceMult(RE::TESRace *aRace) noexcept {
  if (!aRace) {
    Tng::gLogger::warn("Failure in getting a race mult!");
    return 1.0f;
  }
  for (auto lKw : aRace->GetKeywords()) {
    const std::string lKwStr(lKw->formEditorID);
    if (lKwStr.starts_with(cRaceMult)) {
      if (lKwStr.size() != 17) return 1.0f;
      const int lMult100 = std::strtol(lKwStr.substr(14, 3).data(), nullptr, 0);
      return static_cast<float>(lMult100) / 100.0f;
    }
  }
  return 1.0f;
}

bool TngSizeShape::SetRaceMult(RE::TESRace *aRace, const float aMult) noexcept {
  if (!aRace) {
    Tng::gLogger::warn("Failure in setting a race mult!");
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
    const auto lKw = *lKwIt;
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

int TngSizeShape::GetRaceShape(RE::TESRace *aRace) noexcept {
  if (!aRace) {
    Tng::gLogger::warn("Failure in getting a race shape!");
    return Tng::pgErr;
  }
  for (auto lKw : aRace->GetKeywords()) {
    const std::string lKwStr(lKw->formEditorID);
    if (lKwStr.starts_with(cRaceShape)) {
      if (lKwStr.size() != 17) return Tng::pgErr;
      return std::strtol(lKwStr.substr(15, 2).data(), nullptr, 0);
    }
  }
  return Tng::pgErr;
}

void TngSizeShape::SetRaceShape(RE::TESRace *aRace, int aRaceShape) noexcept {
  if (!aRace) {
    Tng::gLogger::warn("Failure in setting a race shape!");
    return;
  }
  if (!aRace) {
    Tng::gLogger::critical("Failure in setting a race shape!");
    return;
  }
  auto &lAllKws = fDH->GetFormArray<RE::BGSKeyword>();
  int lRaceShape = static_cast<int>(aRaceShape);
  if ((fMalAddLst->forms.size() < lRaceShape) || (lRaceShape < 0)) {
    Tng::gLogger::critical("Cannot set the race {} to use addon {}! There are only {} addons.", aRace->GetFormEditorID(), lRaceShape + 1, fMalAddLst->forms.size());
    lRaceShape = 0;
  }
  std::string lReqKw = cRaceShape + (lRaceShape < 10 ? "0" + std::to_string(lRaceShape) : std::to_string(lRaceShape));
  auto lKwIt = std::find_if(lAllKws.begin(), lAllKws.end(), [&](const auto &kw) { return kw && kw->formEditorID == lReqKw.c_str(); });
  RE::BGSKeyword *lKw{nullptr};
  if (lKwIt != lAllKws.end()) {
    const auto lKw = *lKwIt;
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
    if (lExistingKw->formEditorID.contains(cRaceShape)) aRace->RemoveKeyword(lExistingKw);
  }
  aRace->AddKeyword(lKw);
  auto lRaceEntry = std::find_if(fRaceInfo.begin(), fRaceInfo.end(), [&aRace](const std::pair<std::string_view, int> &p) { return p.first == aRace->GetName(); });
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
}

int TngSizeShape::GetRaceShape(const std::size_t aRaceIdx) noexcept {
  if (fRaceInfo.size() <= aRaceIdx) return -1.0f;
  return GetRaceShape(*fRaceInfo[aRaceIdx].second.begin());
}

bool TngSizeShape::SetRaceShape(const std::size_t aRaceIdx, int aRaceShape) noexcept {
  if (fRaceInfo.size() <= aRaceIdx) return false;
  if (aRaceShape < GetAddonCount(false) + 1) return false;
  for (const auto &lRace : fRaceInfo[aRaceIdx].second) SetRaceShape(lRace, aRaceShape);
}

std::set<RE::TESRace *> TngSizeShape::GetRacesByIdx(const std::size_t aRaceIdx) noexcept {
  std::set<RE::TESRace *> lRes{};
  if (fRaceInfo.size() < aRaceIdx + 1) return lRes;
  return fRaceInfo[aRaceIdx].second;
}

std::set<std::string> TngSizeShape::GetRaceNames() noexcept {
  std::set<std::string> lRes{};
  for (const auto &lEntry : fRaceInfo) lRes.insert(lEntry.first);
  return lRes;
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

Tng::TNGRes TngSizeShape::SetActorSkin(RE::Actor *aActor, int aGenOption) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  if (!lNPC->race) return Tng::raceErr;
  if (!lNPC->race->HasKeyword(fPRaceKey)) return Tng::raceErr;
  auto lAddLst = lNPC->IsFemale() ? fFemAddLst : fMalAddLst;
  if (!lAddLst || !fSkinWithPenisKey || !fGentified) return Tng::pgErr;
  if (aGenOption == -1) return lNPC->IsFemale() ? Tng::resOkNoGen : Tng::resOkGen;
  if (aGenOption == -2) {
    if (ResetGenital(lNPC))
      return lNPC->IsFemale() ? Tng::resOkNoGen : Tng::resOkGen;
    else
      return Tng::pgErr;
  }
  if (aGenOption >= GetAddonCount(lNPC->IsFemale())) return Tng::addonErr;
  auto lGenital = lAddLst->forms[aGenOption]->As<RE::TESObjectARMO>();
  if (!lGenital) {
    return Tng::pgErr;
  }
  if (!SetNPCGenital(lNPC, lGenital, aGenOption)) return Tng::pgErr;
  if (lGenital->HasKeyword(fSkinWithPenisKey) && !fGentified->HasForm(lNPC)) fGentified->AddForm(lNPC);
  return (!lNPC->IsFemale() || lGenital->HasKeyword(fSkinWithPenisKey)) ? Tng::resOkGen : Tng::resOkNoGen;
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

std::vector<std::string> TngSizeShape::GetAllPossibleAddons(RE::Actor *aActor) noexcept {
  std::vector<std::string> lRes{};
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC || !fFemAddLst || !fMalAddLst) return lRes;
  lRes.push_back("$TNG_AST");
  lRes.push_back(aActor->GetName());
  lRes.push_back("$TNG_ASR");
  auto lList = lNPC->IsFemale() ? fFemAddLst : fMalAddLst;
  for (auto lForm : lList->forms) {
    auto lSkin = lForm->As<RE::TESObjectARMO>();
    if (lSkin) lRes.push_back(lSkin->GetName());
  }
  return lRes;
}
RE::TESObjectARMO *TngSizeShape::GetAddonAt(bool aIsFemale, int aIdx) noexcept {
  auto lAddLst = aIsFemale ? fFemAddLst : fMalAddLst;
  if (aIdx > lAddLst->forms.size() - 1) return nullptr;
  return lAddLst->forms[aIdx]->As<RE::TESObjectARMO>();
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

bool TngSizeShape::ResetGenital(RE::TESNPC *aNPC) noexcept {
  auto lSkin = aNPC->skin;
  if (!lSkin) return true;
  if (lSkin == aNPC->race->skin) return true;
  if (!lSkin->HasKeyword(fCustomSkin)) return true;
  for (const auto &lKw : lSkin->GetKeywords()) {
    const std::string lKwStr(lKw->formEditorID);
    if (lKwStr.starts_with(cShapeSkin)) {
      const size_t lLen = lKwStr.find(Tng::cColonChar) - strlen(cShapeSkin);
      std::string lOgSkinRec = lKwStr.substr(strlen(cShapeSkin), lLen);
      auto lOgSKin = LoadForm<RE::TESObjectARMO>(lOgSkinRec);
      if (!lOgSKin) return false;
      aNPC->skin = lOgSKin;
      return true;
    }
  }
}

bool TngSizeShape::SetNPCGenital(RE::TESNPC *aNPC, RE::TESObjectARMO *aGenital, int aChoice) noexcept {
  auto lOgSkin = aNPC->skin ? aNPC->skin : aNPC->race->skin;
  std::string lReqKw = cShapeSkin + RecordToStr(aGenital) + Tng::cColonChar + std::to_string(aChoice);
  RE::TESObjectARMO *lSkin{nullptr};
  for (const auto &lShapeSkin : fSkinFactory) {
    if (lShapeSkin->HasKeywordString(lReqKw)) {
      aNPC->skin = lShapeSkin;
      return true;
    }
  }
  const auto lFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
  RE::BGSKeyword *lKw{nullptr};
  if (lKw = lFactory ? lFactory->Create() : nullptr; lKw) {
    lKw->formEditorID = lReqKw;
    fDH->GetFormArray<RE::BGSKeyword>().push_back(lKw);
  } else {
    Tng::gLogger::info("Couldn't create keyword [{}]!", lReqKw);
    return false;
  }
  lSkin = lOgSkin->CreateDuplicateForm(true, (void *)lSkin)->As<RE::TESObjectARMO>();
  lSkin->Copy(lOgSkin);
  lSkin->AddKeyword(lKw);
  lSkin->AddKeyword(fCustomSkin);
  lSkin->AddKeyword(fIAKey);
  fDH->GetFormArray<RE::TESObjectARMO>().push_back(lSkin);
  fSkinFactory.insert(lSkin);
  aNPC->skin = lSkin;
}

int TngSizeShape::FindInFormList(RE::TESForm *aForm, RE::BGSListForm *aList) noexcept {
  if (!aForm || !aList) return -1;
  if (!aList->HasForm(aForm)) return -1;
  for (int i = 0; i < aList->forms.size(); i++)
    if (aList->forms[i]->formID == aForm->formID) return i;
  return -1;
}
