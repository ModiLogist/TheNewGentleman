#include <TngCore.h>
#include <TngEvents.h>
#include <TngInis.h>
#include <TngSizeShape.h>

void TngEvents::RegisterEvents() noexcept {
  const auto lSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
  RE::TESDataHandler* fDH = RE::TESDataHandler::GetSingleton();
  static RE::BGSKeyword* fPRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  static RE::BGSKeyword* fCCKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  static RE::BGSKeyword* fACKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  static RE::BGSKeyword* fARKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  static RE::BGSKeyword* fRRKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  static RE::BGSKeyword* fPAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProblemArmoKeyID, Tng::cName);
  static RE::BGSKeyword* fIAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  static RE::BGSKeyword* fUAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  static RE::BGSKeyword* fGWKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cGentleWomanKeyID, Tng::cName);
  static RE::BGSKeyword* fPSKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  static RE::TESGlobal* fGWChance = fDH->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
  static RE::BGSListForm* fGentified = fGentified ? fGentified : fDH->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  if (!(fPRaceKey && fCCKey && fACKey && fARKey && fRRKey && fPAKey && fIAKey && fUAKey && fGWKey && fGWKey && fPSKey && fGWChance && fGentified)) {
    Tng::gLogger::critical("Failed to register events. There might be functionality issues. Please report this issue.");
    return;
  }
  lSourceHolder->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  lSourceHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  Tng::gLogger::info("Registered for necessary events.");
}

void TngEvents::CheckForRevealing(RE::TESObjectARMO* aBodyArmor, RE::TESObjectARMO* aPelvisArmor) noexcept {
  if (!aBodyArmor || !aPelvisArmor) return;
  if (aBodyArmor == aPelvisArmor) return;
  if (!aBodyArmor->HasKeyword(fACKey) || aPelvisArmor->HasKeyword(fUAKey)) return;
  if (aBodyArmor->GetFile(0)->GetFilename() != aPelvisArmor->GetFile(0)->GetFilename()) return;
  if (aBodyArmor->armorAddons.size() == 0) return;
  for (const auto& lAA : aBodyArmor->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotGenital) && lAA->HasPartOf(Tng::cSlotBody)) lAA->RemoveSlotFromMask(Tng::cSlotGenital);
    const auto lID = (std::string(aBodyArmor->GetName()).empty()) ? aBodyArmor->GetFormEditorID() : aBodyArmor->GetName();
    Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] was recognized revealing.", aBodyArmor->GetLocalFormID(), lID, aBodyArmor->GetFile(0)->GetFilename());
    aBodyArmor->RemoveKeyword(fACKey);
    aBodyArmor->AddKeyword(fARKey);
    return;
  }
}

void TngEvents::CheckForClipping(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept {
  if (!aActor || !aArmor) return;
  fInternal = true;
  static RE::ActorEquipManager* lEquipManager = lEquipManager ? lEquipManager : RE::ActorEquipManager::GetSingleton();
  lEquipManager->EquipObject(aActor, aArmor, nullptr, 1, nullptr, false, false, false, true);
}

void TngEvents::CheckActor(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  std::vector<RE::BGSKeyword*> lArmoKeys = {fCCKey, fACKey, fARKey, fRRKey, fPAKey, fIAKey};
  if (!lNPC->race->HasKeyword(fPRaceKey)) return;
  TngSizeShape::RandomizeScale(aActor);
  const auto lGArmo = aActor->GetWornArmor(Tng::cSlotGenital);
  if (aArmor && !lGArmo) {
    if (!aArmor->HasKeywordInArray(lArmoKeys, false)) TngCore::HandleArmor(aArmor, false);
    if (aArmor->HasKeyword(fCCKey) || aArmor->HasKeyword(fACKey)) CheckForClipping(aActor, aArmor);
    return;
  }
  const auto lBArmo = aActor->GetWornArmor(Tng::cSlotBody);
  if ((lNPC->IsFemale() && !TngInis::GetAutoReveal(true)) || (!lNPC->IsFemale() && !TngInis::GetAutoReveal(false))) return;
  CheckForRevealing(lBArmo, lGArmo);
}

void TngEvents::CheckGentlewomen(RE::Actor* aActor) noexcept {
  if (TngSizeShape::CanModifyActor(aActor) != Tng::resOkRaceP) return;
  if (aActor->IsPlayerRef()) return;
  const auto lNPC = aActor->GetActorBase();
  if (!lNPC->IsFemale()) return;
  if (fGWChance->value < 1) return;
  const auto lFAddonCount = TngSizeShape::GetAddonCount(true);
  if (lFAddonCount == 0) return;
  if ((lNPC->GetFormID() % 100) < (std::floor(fGWChance->value) + 1)) {
    auto lSkin = TngSizeShape::GetAddonAt(true, lNPC->GetFormID() % lFAddonCount);
    if (!lSkin) return;
    lNPC->skin = lSkin;
    if (lSkin->HasKeyword(fPSKey)) {
      lNPC->AddKeyword(fGWKey);
      fGentified->AddForm(lNPC);
    }
  }
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>* aSource) {
  if (!aEvent || fInternal) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = aEvent->actor->As<RE::Actor>();
  auto lArmor = RE::TESForm::LookupByID<RE::TESObjectARMO>(aEvent->baseObject);
  if (!lArmor) return RE::BSEventNotifyControl::kContinue;
  if (aEvent->equipped) {
    aSource->notifying = false;
    if (!lArmor->GetPlayable()) return RE::BSEventNotifyControl::kContinue;
    if (!lArmor->HasPartOf(Tng::cSlotBody)) return RE::BSEventNotifyControl::kContinue;
    CheckActor(lActor, lArmor);
    fInternal = false;
    aSource->notifying = true;
  } else {
    TngSizeShape::RandomizeScale(lActor);
  }
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!aEvent || fInternal) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = RE::TESForm::LookupByID<RE::Actor>(aEvent->formID);
  CheckGentlewomen(lActor);
  CheckActor(lActor);
  return RE::BSEventNotifyControl::kContinue;
}
