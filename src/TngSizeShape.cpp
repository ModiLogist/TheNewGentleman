#include <TngSizeShape.h>

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
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (fSizeKeyWs[i] && fSizeGlobs[i] && fSizeLists[i]) continue;
    fSizeKeyWs[i] = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cSizeKeyWIDs[i], Tng::cName);
    fSizeGlobs[i] = fDataHandler->LookupForm<RE::TESGlobal>(Tng::cSizeGlobIDs[i], Tng::cName);
    fSizeLists[i] = fDataHandler->LookupForm<RE::BGSListForm>(Tng::cSizeListIDs[i], Tng::cName);
    if (!fSizeKeyWs[i] || !fSizeGlobs[i] || !fSizeLists[i]) {
      Tng::gLogger::error("Could not find the information required for size distribution!");
      return false;
    }
  }
  for (int i = 0; i < 3; i++) {
    if (fShapeSkins[i]) continue;
    fShapeSkins[i] = fDataHandler->LookupForm<RE::TESObjectARMO>(Tng::cSkinIDs[i], Tng::cName);
    if (!fShapeSkins[i]) {
      Tng::gLogger::error("Could not find the information required for shape variations!");
      return false;
    }
  }
  return true;
}

void TngSizeShape::UpdateSavedSize(const std::string aNPCRecord, const long aSize) noexcept {
  const size_t lSepLoc = aNPCRecord.find(cDelimChar);
  const RE::FormID lFormID = std::strtol(aNPCRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aNPCRecord.substr(lSepLoc + 1);
  RE::TESNPC *lNPC = fDataHandler->LookupForm<RE::TESNPC>(lFormID, lModName);
  if (lNPC) {
    if (!fSizeLists[aSize]->HasForm(lNPC)) fSizeLists[aSize]->AddForm(lNPC);
  }
}

void TngSizeShape::UpdateSavedShape(const std::string aNPCRecord, const long aShape) noexcept {
  const size_t lSepLoc = aNPCRecord.find(cDelimChar);
  const RE::FormID lFormID = std::strtol(aNPCRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aNPCRecord.substr(lSepLoc + 1);
  RE::TESNPC *lNPC = fDataHandler->LookupForm<RE::TESNPC>(lFormID, lModName);
  if (!lNPC->race) return;
  if (!lNPC->race->skin) return;
  if (!lNPC->skin || lNPC->skin == lNPC->race->skin) lNPC->skin = fShapeSkins[aShape];
}

void TngSizeShape::AddRaceIndex(RE::FormID aRaceID, int aRaceIndex) noexcept { fRaceIndices.insert(std::make_pair(aRaceID, aRaceIndex)); }

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
    if (fSizeLists[i]->HasForm(lNPC)) {
      lNPC->AddKeyword(fSizeKeyWs[i]);
      ScaleGenital(aActor, fSizeGlobs[i]);
      return;
    }
  }
  const int lRnd = rand() % Tng::cSizeCategories;
  lNPC->AddKeyword(fSizeKeyWs[lRnd]);
  fSizeLists[lRnd]->AddForm(lNPC);
  ScaleGenital(aActor, fSizeGlobs[lRnd]);
}

bool TngSizeShape::CanModifyActor(RE::Actor *aActor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return false;
  if (!lNPC->race) return false;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return false;
  auto lSkin = aActor->GetSkin();
  if (!lSkin) return true;
  if (lSkin == aActor->GetRace()->skin) return true;
  for (const auto &lTngSkin : fShapeSkins)
    if (lSkin == lTngSkin) return true;
  return false;
}

void TngSizeShape::SetActorSize(RE::Actor *aActor, int aGenSize) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return;
  for (const auto &lKw : fSizeKeyWs)
    if (lNPC->HasKeyword(lKw)) lNPC->RemoveKeyword(lKw);
  lNPC->AddKeyword(fSizeKeyWs[aGenSize]);
  ScaleGenital(aActor, fSizeGlobs[aGenSize]);
}

void TngSizeShape::SetActorSkin(RE::Actor *aActor, int aGenOption) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasPartOf(Tng::cSlotGenital)) return;
  lNPC->skin = fShapeSkins[aGenOption];
}
