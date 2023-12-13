#include <TngSizeDistr.h>

void TngSizeDistr::ScaleGenital(RE::Actor *aActor, RE::TESGlobal *aGlobal) noexcept {
  RE::NiAVObject *aBaseNode = aActor->GetNodeByName(cBaseBone);
  RE::NiAVObject *aScrtNode = aActor->GetNodeByName(cScrtBone);
  if (!aBaseNode || !aScrtNode) return;
  aBaseNode->local.scale = aGlobal->value;
  aScrtNode->local.scale = 1.0f / aGlobal->value;
}

bool TngSizeDistr::InitSizes() noexcept {
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    fSizeKeyWs[i] = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cSizeKeyWIDs[i], Tng::cName);
    fSizeGlobs[i] = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(Tng::cSizeGlobIDs[i], Tng::cName);
    fSizeLists[i] = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSListForm>(Tng::cSizeListIDs[i], Tng::cName);
    if (!fSizeKeyWs[i] || !fSizeGlobs[i] || !fSizeLists[i]) {
      Tng::gLogger::error("Could not find the information required for size distribution!");
      return false;
    }
  }
  return true;
}

void TngSizeDistr::RandomizeScale(RE::Actor *aActor) noexcept {
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
