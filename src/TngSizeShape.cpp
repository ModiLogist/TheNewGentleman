#include <TngSizeShape.h>

int TngSizeShape::fFemAddonCount;
int TngSizeShape::fMalAddonCount;

bool TngSizeShape::LoadAddons() noexcept {
  fFemAddKey = fDataHandler->LookupForm<RE::BGSKeyword>(cFemAddKeyID, Tng::cName);
  fMalAddKey = fDataHandler->LookupForm<RE::BGSKeyword>(cMalAddKeyID, Tng::cName);
  fFemAddLst = fDataHandler->LookupForm<RE::BGSListForm>(cFemAddLstID, Tng::cName);
  fMalAddLst = fDataHandler->LookupForm<RE::BGSListForm>(cMalAddLstID, Tng::cName);
  if (!fFemAddKey || !fMalAddKey || !fFemAddLst || !fMalAddLst) {
    Tng::gLogger::error("Could not find the base information required for shape variations!");
    return false;
  }
  for (int i = 0; i < 3; i++) {
    fMalAddons[i] = fDataHandler->LookupForm<RE::TESObjectARMO>(cSkinIDs[i], Tng::cName);
    if (!fMalAddons[i]) {
      Tng::gLogger::error("Could not find the information required for shape variations!");
      return false;
    }
  }
  fFemAddonCount = 0;
  fMalAddonCount = 3;
  auto &lAllArmor = fDataHandler->GetFormArray<RE::TESObjectARMO>();
  for (const auto &lArmor : lAllArmor) {
    if (lArmor->HasKeyword(fFemAddKey)) {
      fFemAddons[fFemAddonCount] = lArmor;
      fFemAddonCount++;
      fFemAddLst->AddForm(lArmor);
    }
    if (lArmor->HasKeyword(fMalAddKey)) {
      fMalAddons[fMalAddonCount] = lArmor;
      fMalAddonCount++;
      fMalAddLst->AddForm(lArmor);
    }
  }
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
  if (!fTNGRaceKey) {
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
  if (!lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->skin) return;
  if (lNPC->skin && lNPC->skin != lNPC->race->skin) return;
  if (lNPC->IsFemale()) {
    lNPC->skin = fFemAddons[aShape];
  } else {
    lNPC->skin = fMalAddons[aShape];
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
  for (const auto &lTngSkin : fMalAddons)
    if (lSkin == lTngSkin) return 1;
  for (const auto &lTngSkin : fFemAddons)
    if (lSkin == lTngSkin) return 1;
  return 0;
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
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return;
  if (lNPC->IsFemale()) {
    lNPC->skin = fFemAddons[aGenOption];
  } else {
    lNPC->skin = fMalAddons[aGenOption];
  }
}
