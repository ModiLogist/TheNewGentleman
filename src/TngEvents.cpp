#include <TngCore.h>
#include <TngEvents.h>
#include <TngInis.h>
#include <TngSizeShape.h>

void TngEvents::RegisterEvents() noexcept {
  const auto lSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
  RE::TESDataHandler* fDH = RE::TESDataHandler::GetSingleton();
  fPRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  fCCKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  fACKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  fARKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  fRRKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  fPAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProblemArmoKeyID, Tng::cName);
  fIAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  fUAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  fGWKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cGentleWomanKeyID, Tng::cName);
  fPSKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  fExKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cExcludeKeyID, Tng::cName);
  fGWChance = fDH->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
  fGentified = fGentified ? fGentified : fDH->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  if (!(fPRaceKey && fCCKey && fACKey && fARKey && fRRKey && fPAKey && fIAKey && fUAKey && fGWKey && fPSKey && fExKey && fGWChance && fGentified)) {
    Tng::gLogger::critical("Failed to register events. There might be functionality issues. Please report this issue.");
    return;
  }
  lSourceHolder->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  lSourceHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  Tng::gLogger::info("Registered for necessary events.");
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>* aSource) {
  if (!aEvent || fInternal) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = aEvent->actor->As<RE::Actor>();
  auto lArmor = RE::TESForm::LookupByID<RE::TESObjectARMO>(aEvent->baseObject);
  if (!lArmor || !lActor) return RE::BSEventNotifyControl::kContinue;
  if (!lArmor->HasPartOf(Tng::cSlotBody)) return RE::BSEventNotifyControl::kContinue;
  if (aEvent->equipped) {
    aSource->notifying = false;
    if (!lArmor->GetPlayable()) return RE::BSEventNotifyControl::kContinue;
    CheckActor(lActor, lArmor);
    fInternal = false;
    aSource->notifying = true;
  } else {
    CheckForAddons(lActor);
    TngSizeShape::RandomizeScale(lActor);
  }
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!aEvent || fInternal) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = RE::TESForm::LookupByID<RE::Actor>(aEvent->formID);
  if (!lActor) return RE::BSEventNotifyControl::kContinue;  
  CheckForAddons(lActor);
  CheckActor(lActor);
  return RE::BSEventNotifyControl::kContinue;
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
    Tng::gLogger::info("The armor [0x{:x}: {}] was updated to be revealing.", aBodyArmor->GetLocalFormID(), lID);
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
  if (!lNPC->race->HasKeyword(fPRaceKey)) return;
  TngSizeShape::RandomizeScale(aActor);
  const auto lGArmo = aActor->GetWornArmor(Tng::cSlotGenital);
  if (aArmor && !lGArmo) {
    TngCore::FixArmor(aArmor);
    if (aArmor->HasKeyword(fCCKey) || aArmor->HasKeyword(fACKey)) CheckForClipping(aActor, aArmor);
    return;
  }
  const auto lBArmo = aActor->GetWornArmor(Tng::cSlotBody);
  if ((lNPC->IsFemale() && !TngInis::GetAutoReveal(true)) || (!lNPC->IsFemale() && !TngInis::GetAutoReveal(false))) return;
  CheckForRevealing(lBArmo, lGArmo);
}

void TngEvents::CheckForAddons(RE::Actor* aActor) noexcept {
  if (TngSizeShape::CanModifyActor(aActor) != Tng::resOkRaceP) return;
  const auto lNPC = aActor->GetActorBase();
  int lNPCAddn = TngSizeShape::GetNPCAddn(lNPC);
  if (lNPCAddn > 0 && lNPCAddn != Tng::resOkNoAddon) {
    TngCore::SetActorSkin(aActor, lNPCAddn);
    return;
  }
  if (lNPCAddn == Tng::pgErr) {
    Tng::gLogger::critical("Faced an issue retrieving information for {}!", lNPC->GetName());
    return;
  }
  if (!lNPC->IsFemale() && lNPC->HasKeyword(fExKey)) TngCore::RevertNPCSkin(lNPC);
  if (!lNPC->IsFemale() || lNPC->HasKeyword(fExKey) || fGWChance->value < 1) return;
  const auto lFAddonCount = TngSizeShape::GetAddonCount(true);
  if (lFAddonCount == 0) return;
  if ((lNPC->GetFormID() % 100) < (std::floor(fGWChance->value) + 1)) {
    TngCore::SetActorSkin(aActor, lNPC->GetFormID() % lFAddonCount);
    if (lNPC->skin->HasKeyword(fPSKey)) {
      lNPC->AddKeyword(fGWKey);
      if (!fGentified->HasForm(lNPC)) fGentified->AddForm(lNPC);
    }
  }
}
