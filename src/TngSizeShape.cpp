#include <TngSizeShape.h>

int TngSizeShape::fAddonCount[2];

bool TngSizeShape::LoadAddons() noexcept {
  fFemAddKey = fDataHandler->LookupForm<RE::BGSKeyword>(cFemAddKeyID, Tng::cName);
  fMalAddKey = fDataHandler->LookupForm<RE::BGSKeyword>(cMalAddKeyID, Tng::cName);
  fFemAddLst = fDataHandler->LookupForm<RE::BGSListForm>(cFemAddLstID, Tng::cName);
  fMalAddLst = fDataHandler->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  fAddonGlb[1] = fDataHandler->LookupForm<RE::TESGlobal>(cFemAddConID, Tng::cName);
  fAddonGlb[0] = fDataHandler->LookupForm<RE::TESGlobal>(cMalAddConID, Tng::cName);
  fMsg[1] = fDataHandler->LookupForm<RE::BGSMessage>(cShapeMessageFID, Tng::cName);
  fMsg[0] = fDataHandler->LookupForm<RE::BGSMessage>(cShapeMessageMID, Tng::cName);
  fShowNext[1] = fDataHandler->LookupForm<RE::TESGlobal>(cShowNextFID, Tng::cName);
  fShowNext[0] = fDataHandler->LookupForm<RE::TESGlobal>(cShowNextMID, Tng::cName);  
  if (!fFemAddKey || !fMalAddKey || !fFemAddLst || !fMalAddLst || !fAddonGlb[0] || !fAddonGlb[1] || !fMsg[0] || !fMsg[1] || !fShowNext[0] || !fShowNext[1]) {
    Tng::gLogger::error("Could not find the base information required for shape variations!");
    return false;
  }
  for (int i = 0; i < 3; i++) {
    fAddons[0][i] = fDataHandler->LookupForm<RE::TESObjectARMO>(cSkinIDs[i], Tng::cName);
    if (!fAddons[0][i]) {
      Tng::gLogger::error("Could not find the information required for shape variations!");
      return false;
    }
  }
  fAddonCount[0] = 3;
  fAddonCount[1] = 0;
  auto &lAllArmor = fDataHandler->GetFormArray<RE::TESObjectARMO>();
  for (const auto &lArmor : lAllArmor) {
    if (lArmor->HasKeyword(fFemAddKey)) {
      fAddons[1][fAddonCount[1]] = lArmor;
      fFemAddLst->AddForm(lArmor);
      auto lBtn = *std::next(fMsg[1]->menuButtons.begin(), fAddonCount[1]);
      lBtn->text = lArmor->GetName();
      fAddonCount[1]++;
    }
    if (lArmor->HasKeyword(fMalAddKey)) {
      fAddons[0][fAddonCount[0]] = lArmor;
      fMalAddLst->AddForm(lArmor);
      auto lBtn = *std::next(fMsg[0]->menuButtons.begin(), fAddonCount[0]);
      lBtn->text = lArmor->GetName();
      fAddonCount[0]++;
    }
  }
  fAddonGlb[0]->value = static_cast<float>(fAddonCount[0]);
  fShowNext[0]->value = fAddonCount[0] > 4 ? 2.0f : 0.0f;
  fAddonGlb[1]->value = static_cast<float>(fAddonCount[1]);
  fShowNext[1]->value = fAddonCount[1] > 4 ? 2.0f : 0.0f;
  return true;
}

float TngSizeShape::GetRacialMult(RE::Actor *aActor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!lNPC) return 1.0f;
  const auto lRaceID = lNPC->race->GetFormID();
  auto lRaceIndex = std::find_if(fRaceIndices.begin(), fRaceIndices.end(), [&lRaceID](const std::pair<RE::FormID, int> &p) { return p.first == lRaceID; });
  if (lRaceIndex != fRaceIndices.end()) return genitalSizes[lRaceIndex->second];
  return 1.0f;
}

void TngSizeShape::ScaleGenital(RE::Actor *aActor, RE::TESGlobal *aGlobal) noexcept {
  auto lScale = GetRacialMult(aActor) * aGlobal->value;
  RE::NiAVObject *aBaseNode = aActor->GetNodeByName(cBaseBone);
  RE::NiAVObject *aScrtNode = aActor->GetNodeByName(cScrtBone);
  if (!aBaseNode || !aScrtNode) return;
  aBaseNode->local.scale = lScale;
  aScrtNode->local.scale = 1.0f / lScale;
}

bool TngSizeShape::InitSizes() noexcept {
  fDataHandler = RE::TESDataHandler::GetSingleton();
  fTNGRaceKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cTNGRaceKeyID, Tng::cName);
  fGentified = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  fSkinWithPenisKey = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  if (!fTNGRaceKey || !fGentified) {
    Tng::gLogger::error("Could not find the information required for size distribution!");
    return false;
  }
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (fSizeKeyWs[i] && fSizeGlobs[i]) continue;
    fSizeKeyWs[i] = fDataHandler->LookupForm<RE::BGSKeyword>(cSizeKeyWIDs[i], Tng::cName);
    fSizeGlobs[i] = fDataHandler->LookupForm<RE::TESGlobal>(cSizeGlobIDs[i], Tng::cName);
    if (!fSizeKeyWs[i] || !fSizeGlobs[i]) {
      Tng::gLogger::error("Could not find the information required for size distribution!");
      return false;
    }
  }
  return LoadAddons();
}

void TngSizeShape::UpdateSavedSize(const std::string aNPCRecord, const long aSize) noexcept {
  const size_t lSepLoc = aNPCRecord.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aNPCRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aNPCRecord.substr(lSepLoc + 1);
  RE::TESNPC *lNPC = fDataHandler->LookupForm<RE::TESNPC>(lFormID, lModName);
  if (lNPC) {
    for (int i = 0; i < Tng::cSizeCategories; i++)
      if (lNPC->HasKeyword(fSizeKeyWs[i])) lNPC->RemoveKeyword(fSizeKeyWs[i]);
    lNPC->AddKeyword(fSizeKeyWs[aSize]);
  }
}

void TngSizeShape::UpdateSavedShape(const std::string aNPCRecord, const long aShape) noexcept {
  const size_t lSepLoc = aNPCRecord.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aNPCRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aNPCRecord.substr(lSepLoc + 1);
  RE::TESNPC *lNPC = fDataHandler->LookupForm<RE::TESNPC>(lFormID, lModName);
  Tng::gLogger::info("Looking for {:x} in {}",lFormID,lModName);
  if (!lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->skin) return;
  if (lNPC->skin && lNPC->skin != lNPC->race->skin) return;
  if (lNPC->IsFemale()) {
    lNPC->skin = fAddons[1][aShape];
    if (fAddons[1][aShape]->HasKeyword(fSkinWithPenisKey)) fGentified->AddForm(lNPC);
  } else {
    lNPC->skin = fAddons[0][aShape];
  }
}

void TngSizeShape::AddRaceIndex(RE::FormID aRaceID, int aRaceIndex) noexcept { fRaceIndices.insert(std::make_pair(aRaceID, aRaceIndex)); }

std::set<RE::FormID> TngSizeShape::GetRacesWithIndex(int aRaceIndex) noexcept {
  std::set<RE::FormID> res{};
  for (const auto &lRaceIdx : fRaceIndices) {
    if (lRaceIdx.second == aRaceIndex) res.insert(lRaceIdx.first);
  }
  return res;
}

void TngSizeShape::RandomizeScale(RE::Actor *aActor) noexcept {
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (!fSizeKeyWs[i] || !fSizeGlobs[i]) {
      Tng::gLogger::error("TNG could not find the information required for size distribution!");
      return;
    }
  }
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return;

  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (lNPC->HasKeyword(fSizeKeyWs[i])) {
      ScaleGenital(aActor, fSizeGlobs[i]);
      return;
    }
  }
  const int lDefSize = lNPC->formID % 5;
  lNPC->AddKeyword(fSizeKeyWs[lDefSize]);
  ScaleGenital(aActor, fSizeGlobs[lDefSize]);
}

int TngSizeShape::CanModifyActor(RE::Actor *aActor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return -1;
  if (!lNPC->race) return -1;
  if (!lNPC->race->HasKeyword(fTNGRaceKey)) return -1;
  auto lSkin = aActor->GetSkin();
  if (!lSkin) return 1;
  if (lSkin == aActor->GetRace()->skin) return 1;
  for (const auto &lTngSkin : fAddons[0])
    if (lSkin == lTngSkin) return 1;
  for (const auto &lTngSkin : fAddons[1])
    if (lSkin == lTngSkin) return 1;
  return 0;
}

void TngSizeShape::UpdateMessage(bool aIsFemale) noexcept {
  int lFM = aIsFemale ? 1 : 0;
  for (int i = 0; i < 4; i++) {
    auto lBtn = *std::next(fMsg[lFM]->menuButtons.begin(), i);
    if (fMessagePage[lFM] == 0) fOgMessageBtns[i] = lBtn->text;
    int lIdx = 4 * fMessagePage[lFM] + i;
    if (lIdx < fAddonCount[lFM]) lBtn->text = fAddons[lFM][lIdx]->GetName();
  }
  fAddonGlb[lFM]->value = static_cast<float>(fAddonCount[lFM] - (4 * fMessagePage[lFM]));
  fMessagePage[lFM]++;
  fShowNext[lFM]->value = fAddonCount[lFM] > fMessagePage[lFM] * 4 ? 2.0f : 0.0f;
}

void TngSizeShape::ResetMessage(bool aIsFemale) noexcept {
  int lFM = aIsFemale ? 1 : 0;
  for (int i = 0; i < 4; i++) {
    auto lBtn = *std::next(fMsg[lFM]->menuButtons.begin(), i);
    lBtn->text = fOgMessageBtns[i];
    fAddonGlb[lFM]->value = static_cast<float>(fAddonCount[lFM]);
    fShowNext[0]->value = fAddonCount[0] > 4 ? 2.0f : 0.0f;
    fMessagePage[lFM] = 0;
  }
}

void TngSizeShape::SetActorSize(RE::Actor *aActor, int aGenSize) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return;
  for (int i = 0; i < Tng::cSizeCategories; i++)
    if (lNPC->HasKeyword(fSizeKeyWs[i])) {
      lNPC->RemoveKeyword(fSizeKeyWs[i]);
    }
  lNPC->AddKeyword(fSizeKeyWs[aGenSize]);
  ScaleGenital(aActor, fSizeGlobs[aGenSize]);
}

void TngSizeShape::SetActorSkin(RE::Actor *aActor, int aGenOption) noexcept {
  if (aGenOption == -1) return;
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return;
  if (aGenOption == -2) {
    lNPC->skin = lNPC->race->skin;
    return;
  }
  if (lNPC->IsFemale()) {
    lNPC->skin = fAddons[1][aGenOption];
    if (fAddons[1][aGenOption]->HasKeyword(fSkinWithPenisKey)) fGentified->AddForm(lNPC);
  } else {
    lNPC->skin = fAddons[0][aGenOption];
  }
}
