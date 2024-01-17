#include <TngSizeShape.h>

bool TngSizeShape::LoadAddons() noexcept {
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  auto lFemAddKey = lDataHandler->LookupForm<RE::BGSKeyword>(cFemAddKeyID, Tng::cName);
  auto lMalAddKey = lDataHandler->LookupForm<RE::BGSKeyword>(cMalAddKeyID, Tng::cName);
  auto lFemAddLst = lDataHandler->LookupForm<RE::BGSListForm>(cFemAddLstID, Tng::cName);
  auto lMalAddLst = lDataHandler->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  if (!lFemAddKey || !lMalAddKey || !lFemAddLst || !lMalAddLst) {
    Tng::gLogger::error("Could not find the base information required for shape variations!");
    return false;
  }
  for (int i = 0; i < 3; i++) {
    auto lArmor = lDataHandler->LookupForm<RE::TESObjectARMO>(cSkinIDs[i], Tng::cName);
    if (!lArmor) {
      Tng::gLogger::error("Could not find the information required for shape variations!");
      return false;
    }
  }
  int lFAddonCount = lFemAddLst->forms.size();
  int lMAddonCount = lMalAddLst->forms.size();
  auto &lAllArmor = lDataHandler->GetFormArray<RE::TESObjectARMO>();
  for (const auto &lArmor : lAllArmor) {
    if (lArmor->HasKeyword(lFemAddKey)) {
      if (!lFemAddLst->HasForm(lArmor)) lFemAddLst->AddForm(lArmor);
      lFAddonCount++;
    }
    if (lArmor->HasKeyword(lMalAddKey)) {
      lMalAddLst->AddForm(lArmor);
      lMAddonCount++;
    }
  }
  return true;
}

void TngSizeShape::LoadNPCSize(const std::string aNPCRecord, const int aSize) noexcept {
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  const size_t lSepLoc = aNPCRecord.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aNPCRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aNPCRecord.substr(lSepLoc + 1);
  RE::TESNPC *lNPC = lDataHandler->LookupForm<RE::TESNPC>(lFormID, lModName);
  RE::BGSKeyword *lSizeKws[Tng::cSizeCategories]{};
  RE::TESGlobal *lSizeGlbs[Tng::cSizeCategories]{};
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    lSizeKws[i] = lDataHandler->LookupForm<RE::BGSKeyword>(cSizeKeyWIDs[i], Tng::cName);
    lSizeGlbs[i] = lDataHandler->LookupForm<RE::TESGlobal>(cSizeGlobIDs[i], Tng::cName);
    if (!lSizeKws[i] || !lSizeGlbs[i]) {
      Tng::gLogger::error("Could not find the information required for size distribution among NPCs!");
      return;
    }
  }
  if (lNPC) {
    for (int i = 0; i < Tng::cSizeCategories; i++)
      if (lNPC->HasKeyword(lSizeKws[i])) lNPC->RemoveKeyword(lSizeKws[i]);
    lNPC->AddKeyword(lSizeKws[aSize]);
  }
}

void TngSizeShape::LoadNPCShape(const std::string aNPCRecord, const std::string aShapeRecord) noexcept {
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  const size_t lSepLoc = aNPCRecord.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aNPCRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aNPCRecord.substr(lSepLoc + 1);
  RE::TESNPC *lNPC = lDataHandler->LookupForm<RE::TESNPC>(lFormID, lModName);
  if (!lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->skin) return;
  if (lNPC->skin && lNPC->skin != lNPC->race->skin) return;
  const size_t lShapeSepLoc = aShapeRecord.find(Tng::cDelimChar);
  const RE::FormID lShapeFormID = std::strtol(aShapeRecord.substr(0, lShapeSepLoc).data(), nullptr, 0);
  const std::string lShapeModName = aShapeRecord.substr(lShapeSepLoc + 1);
  RE::TESNPC *lNPC = lDataHandler->LookupForm<RE::TESNPC>(lShapeFormID, lShapeModName);
  if (!lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->skin) return;
  if (lNPC->skin && lNPC->skin != lNPC->race->skin) return;

  auto lFemAddLst = lDataHandler->LookupForm<RE::BGSListForm>(cFemAddLstID, Tng::cName);
  auto lMalAddLst = lDataHandler->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  auto lTNGRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cTNGRaceKeyID, Tng::cName);
  auto lGentified = lDataHandler->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  auto lSkinWithPenisKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  if (!lTNGRaceKey || !lGentified || !lSkinWithPenisKey || !lFemAddLst || !lMalAddLst) {
    Tng::gLogger::error("Could not find the information required for size distribution!");
    return;
  }
  auto lAddLst = lNPC->IsFemale() ? lFemAddLst : lMalAddLst;
  if (lAddLst->forms.size() < aShape) {
    Tng::gLogger::error("A previously installed addon cannot be found anymore! {} would use original skin.", lNPC->GetName());
    return;
  }
  auto lSkin = lAddLst->forms[aShape]->As<RE::TESObjectARMO>();
  if (!lSkin) {
    Tng::gLogger::error("A previously installed addon cannot be found anymore! {} would use original skin.", lNPC->GetName());
    return;
  }
  if (aShape == GetRaceShape(lNPC->race)) return;
  auto &lAllKws = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSKeyword>();
  std::string lReqKw = cActorShape + (aShape < 10 ? "0" + std::to_string(aShape) : std::to_string(aShape));
  auto lKwIt = std::find_if(lAllKws.begin(), lAllKws.end(), [&](const auto &kw) { return kw && kw->formEditorID == lReqKw.c_str(); });
  RE::BGSKeyword *lKw{nullptr};
  if (lKwIt != lAllKws.end()) {
    const auto lKw = *lKwIt;
    if (!lKw) {
      Tng::gLogger::critical("Couldn't get existing keyword [{}]!", lReqKw);
      return;
    }
  } else {
    const auto lFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
    if (lKw = lFactory ? lFactory->Create() : nullptr; lKw) {
      lKw->formEditorID = lReqKw;
      lAllKws.push_back(lKw);
    } else {
      Tng::gLogger::critical("Couldn't create keyword [{}]!", lReqKw);
      return;
    }
  }
  if (!lKw) {
    Tng::gLogger::info("Couldn't set the shape for actor [{}]!", lNPC->GetFormEditorID());
    return;
  }
  for (const auto &lExistingKw : lNPC->GetKeywords()) {
    if (lExistingKw->formEditorID.contains(cActorShape)) lNPC->RemoveKeyword(lExistingKw);
  }
  lNPC->AddKeyword(lKw);
}

void TngSizeShape::LoadRaceMult(const std::string aRaceRecord, const int aSize100) noexcept {
  const size_t lSepLoc = aRaceRecord.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aRaceRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aRaceRecord.substr(lSepLoc + 1);
  auto *lRace = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESRace>(lFormID, lModName);
  if (!lRace) return;
  SetRaceMult(lRace, static_cast<float>(aSize100/100.0f));
}

void TngSizeShape::LoadRaceShape(const std::string aRaceRecord, const std::string aShapeRecord) noexcept {
  const size_t lSepLoc = aRaceRecord.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aRaceRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aRaceRecord.substr(lSepLoc + 1);
  auto *lRace = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESRace>(lFormID, lModName);
  if (!lRace) return;
  SetRaceShape(lRace, aShape);
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

void TngSizeShape::SetRaceMult(RE::TESRace *aRace, const float aMult) noexcept {
  if (!aRace) {
    Tng::gLogger::warn("Failure in setting a race mult!");
    return;
  }
  auto &lAllKws = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSKeyword>();
  int lRaceMult100 = static_cast<int>(aMult*100);
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
    if (!lKw) {
      Tng::gLogger::info("Couldn't get existing keyword [{}]!", lReqKw);
      return;
    }
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
  if (!lKw) {
    Tng::gLogger::info("Couldn't set the size multiplier for race [{}]!", aRace->GetFormEditorID());
    return;
  }
  for (const auto &lExistingKw : aRace->GetKeywords()) {
    if (lExistingKw->formEditorID.contains(cRaceMult)) aRace->RemoveKeyword(lExistingKw);
  }
  aRace->AddKeyword(lKw);
}

int TngSizeShape::GetRaceShape(RE::TESRace *aRace) noexcept {
  if (!aRace) {
    Tng::gLogger::warn("Failure in getting a race shape!");
    return -1;
  }
  auto lMalAddLst = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  if (!lMalAddLst) {
    Tng::gLogger::critical("Required formlist for shapes cannot be accessed!");
    return -1;
  }
  for (auto lKw : aRace->GetKeywords()) {
    const std::string lKwStr(lKw->formEditorID);
    if (lKwStr.starts_with(cRaceShape)) {
      if (lKwStr.size() != 17) return 0;
      return std::strtol(lKwStr.substr(15, 2).data(), nullptr, 0);
    }
  }
  return -1;
}

void TngSizeShape::SetRaceShape(RE::TESRace *aRace, int aRaceShape) noexcept {
  if (!aRace) {
    Tng::gLogger::warn("Failure in setting a race shape!");
    return;
  }
  auto lMalAddLst = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  if (!lMalAddLst) {
    Tng::gLogger::critical("Required formlist for shapes cannot be accessed!");
    return;
  }
  if (!aRace) {
    Tng::gLogger::critical("Failure in setting a race shape!");
    return;
  }
  auto &lAllKws = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::BGSKeyword>();
  int lRaceShape = static_cast<int>(aRaceShape);
  if ((lMalAddLst->forms.size() < lRaceShape) || (lRaceShape < 0)) {
    Tng::gLogger::critical("Cannot set the race {} to use addon {}! There are only {} addons.", aRace->GetFormEditorID(), lRaceShape + 1, lMalAddLst->forms.size());
    lRaceShape = 0;
  }
  std::string lReqKw = cRaceShape + (lRaceShape < 10 ? "0" + std::to_string(lRaceShape) : std::to_string(lRaceShape));
  auto lKwIt = std::find_if(lAllKws.begin(), lAllKws.end(), [&](const auto &kw) { return kw && kw->formEditorID == lReqKw.c_str(); });
  RE::BGSKeyword *lKw{nullptr};
  if (lKwIt != lAllKws.end()) {
    const auto lKw = *lKwIt;
    if (!lKw) {
      Tng::gLogger::info("Couldn't get existing keyword [{}]!", lReqKw);
      return;
    }
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
  if (!lKw) {
    Tng::gLogger::info("Couldn't set the shape for race [{}]!", aRace->GetFormEditorID());
    return;
  }
  for (const auto &lExistingKw : aRace->GetKeywords()) {
    if (lExistingKw->formEditorID.contains(cRaceShape)) aRace->RemoveKeyword(lExistingKw);
  }
  aRace->AddKeyword(lKw);
}

int TngSizeShape::CanModifyActor(RE::Actor *aActor, bool aAllowOverwrite) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return -1;
  if (!lNPC->race) return -1;
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  if (!lDataHandler) return -1;
  auto lTNGRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cTNGRaceKeyID, Tng::cName);
  auto lFemAddLst = lDataHandler->LookupForm<RE::BGSListForm>(cFemAddLstID, Tng::cName);
  auto lMalAddLst = lDataHandler->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  if (!lNPC->race->HasKeyword(lTNGRaceKey)) return -1;
  if (aAllowOverwrite) return 1;
  auto lSkin = aActor->GetSkin();
  if (!lSkin) return 1;
  if (lSkin == aActor->GetRace()->skin) return 1;
  for (const auto &lTngSkin : lFemAddLst->forms)
    if (lSkin == lTngSkin->As<RE::TESObjectARMO>()) return 1;
  for (const auto &lTngSkin : lMalAddLst->forms)
    if (lSkin == lTngSkin) return 1;
  return lNPC->IsFemale() ? -1 : 0;
}

float TngSizeShape::GetGlobalSize(int aIdx) noexcept {
  if (aIdx < 0 || aIdx > Tng::cSizeCategories) return 1.0f;
  RE::TESGlobal *lSizeGlb{nullptr};
  lSizeGlb = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cSizeGlobIDs[aIdx], Tng::cName);
  if (!lSizeGlb) {
    Tng::gLogger::critical("Could not find the information required for size distribution among NPCs!");
    return 1.0f;
  }
  return lSizeGlb->value;
}

void TngSizeShape::SetGlobalSize(int aIdx, float aSize) noexcept {
  if (aIdx < 0 || aIdx > Tng::cSizeCategories) return;
  RE::TESGlobal *lSizeGlb{nullptr};
  lSizeGlb = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cSizeGlobIDs[aIdx], Tng::cName);
  if (!lSizeGlb) {
    Tng::gLogger::critical("Could not find the information required for size distribution among NPCs!");
    return;
  }
  lSizeGlb->value = aSize;
}

void TngSizeShape::RandomizeScale(RE::Actor *aActor) noexcept {
  RE::BGSKeyword *lSizeKws[Tng::cSizeCategories]{};
  RE::TESGlobal *lSizeGlbs[Tng::cSizeCategories]{};
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    lSizeKws[i] = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(cSizeKeyWIDs[i], Tng::cName);
    lSizeGlbs[i] = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cSizeGlobIDs[i], Tng::cName);
    if (!lSizeKws[i] || !lSizeGlbs[i]) {
      Tng::gLogger::error("Could not find the information required for size distribution among NPCs!");
      return;
    }
  }
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

bool TngSizeShape::SetActorSkin(RE::Actor *aActor, int aGenOption) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return false;
  if (!lNPC->race) return false;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return false;
  auto lAddLst = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(lNPC->IsFemale() ? cFemAddLstID : cMalAddLstID, Tng::cName);
  auto lSkinWithPenisKey = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  auto lGentified = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  if (!lAddLst || !lSkinWithPenisKey || !lGentified) return false;
  if (aGenOption == -1) return !lNPC->IsFemale();
  if (aGenOption == -2) {
    lNPC->skin = lNPC->race->skin;
    return !lNPC->IsFemale();
  }
  auto lSkin = lAddLst->forms[aGenOption]->As<RE::TESObjectARMO>();
  if (!lSkin) {
    lNPC->skin = lNPC->race->skin;
    return !lNPC->IsFemale();
  }
  lNPC->skin = lSkin;
  if (lSkin->HasKeyword(lSkinWithPenisKey)) lGentified->AddForm(lNPC);
  return (!lNPC->IsFemale() || lSkin->HasKeyword(lSkinWithPenisKey));
}

void TngSizeShape::SetActorSize(RE::Actor *aActor, int aGenSize) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return;
  RE::BGSKeyword *lSizeKws[Tng::cSizeCategories]{};
  RE::TESGlobal *lSizeGlbs[Tng::cSizeCategories]{};
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    lSizeKws[i] = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(cSizeKeyWIDs[i], Tng::cName);
    lSizeGlbs[i] = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cSizeGlobIDs[i], Tng::cName);
    if (!lSizeKws[i] || !lSizeGlbs[i]) {
      Tng::gLogger::error("Could not find the information required for size distribution among NPCs!");
      return;
    }
    if (lNPC->HasKeyword(lSizeKws[i])) {
      lNPC->RemoveKeyword(lSizeKws[i]);
    }
  }
  lNPC->AddKeyword(lSizeKws[aGenSize]);
  ScaleGenital(aActor, lSizeGlbs[aGenSize]);
}

std::vector<std::string> TngSizeShape::GetAllPossibleAddons(RE::Actor *aActor) noexcept {
  std::vector<std::string> lRes{};
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  auto lFemAddLst = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(cFemAddLstID, Tng::cName);
  auto lMalAddLst = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  if (!aActor || !lNPC || !lFemAddLst || !lMalAddLst) return lRes;
  lRes.push_back("$TNG_AST");
  lRes.push_back(aActor->GetName());
  lRes.push_back("$TNG_ASR");
  auto lList = lNPC->IsFemale() ? lFemAddLst : lMalAddLst;
  for (auto lForm : lList->forms) {
    auto lSkin = lForm->As<RE::TESObjectARMO>();
    if (lSkin) lRes.push_back(lSkin->GetName());
  }
  return lRes;
}

int TngSizeShape::GetAddonCount(bool aIsFemale) noexcept {
  auto lAddLst = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(aIsFemale ? cFemAddLstID : cMalAddLstID, Tng::cName);
  if (!lAddLst) return 0;
  return lAddLst->forms.size();
}

RE::TESObjectARMO *TngSizeShape::GetAddonAt(bool aIsFemale, int aIdx) noexcept {
  auto lAddLst = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(aIsFemale ? cFemAddLstID : cMalAddLstID, Tng::cName);
  if (!lAddLst) return nullptr;
  if (aIdx > lAddLst->forms.size()) return nullptr;
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