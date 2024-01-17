#include <TngEvents.h>
#include <TngInis.h>
#include <TngSizeShape.h>

void TngEvents::CheckForRevealing(RE::TESObjectARMO* aBodyArmor, RE::TESObjectARMO* aPelvisArmor) noexcept {
  if (!aBodyArmor || !aPelvisArmor) return;
  if (aBodyArmor == aPelvisArmor) return;
  if (!aBodyArmor->HasKeyword(fAutoCoverKey) || aBodyArmor->HasKeyword(fRevealingKey) || aPelvisArmor->HasKeyword(fUnderwearKey)) return;
  if (aBodyArmor->GetFile(0)->GetFilename() != aPelvisArmor->GetFile(0)->GetFilename()) return;
  if (aBodyArmor->armorAddons.size() == 0) return;
  for (const auto& lAA : aBodyArmor->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotGenital) && lAA->HasPartOf(Tng::cSlotBody)) lAA->RemoveSlotFromMask(Tng::cSlotGenital);
    const auto lID = (std::string(aBodyArmor->GetName()).empty()) ? aBodyArmor->GetFormEditorID() : aBodyArmor->GetName();
    Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] was recognized revealing.", aBodyArmor->GetLocalFormID(), lID, aBodyArmor->GetFile(0)->GetFilename());
    aBodyArmor->RemoveKeyword(fAutoCoverKey);
    aBodyArmor->AddKeyword(fAutoRvealKey);
    return;
  }
}

void TngEvents::CheckForClipping(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept {
  if (!aActor || !aArmor) return;
  fInternal = true;
  fEquipManager->EquipObject(aActor, aArmor, nullptr, 1, nullptr, false, false, false, true);
}

void TngEvents::CheckActor(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  if (!lNPC->race) return;
  if (!lNPC->race->HasKeyword(lTNGRaceKey)) return;
  TngSizeShape::RandomizeScale(aActor);
  const auto lGArmo = aActor->GetWornArmor(Tng::cSlotGenital);
  if (aArmor && !lGArmo) {
    if (aArmor->HasKeyword(fCoveringKey) || aArmor->HasKeyword(fAutoCoverKey)) CheckForClipping(aActor, aArmor);
    return;
  }
  const auto lBArmo = aActor->GetWornArmor(Tng::cSlotBody);
  if ((lNPC->IsFemale() && !TngInis::GetAutoReveal(true)) || (!lNPC->IsFemale() && !TngInis::GetAutoReveal(false))) return;
  CheckForRevealing(lBArmo, lGArmo);
}

void TngEvents::CheckGentlewomen(RE::Actor* aActor) noexcept {
  if (TngSizeShape::CanModifyActor(aActor, false) != 1) return;
  if (aActor->IsPlayerRef()) return;
  const auto lNPC = aActor->GetActorBase();
  if (!lNPC->IsFemale()) return;
  if (fWomenChance->value < 1) return;
  const auto lFAddonCount = TngSizeShape::GetAddonCount(true);
  if (lFAddonCount == 0) return;
  if ((lNPC->GetFormID() % 100) < (std::floor(fWomenChance->value) + 1)) {
    auto lSkin = TngSizeShape::GetAddonAt(true,lNPC->GetFormID() % lFAddonCount);
    if (!lSkin) return;
    lNPC->skin = lSkin;
    if (lSkin->HasKeyword(lSkinWithPenisKey)) {
      lNPC->AddKeyword(fGentleWomanKey);
      lGentified->AddForm(lNPC);
    }
  }
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>* aSource) {
  if (!aEvent || fInternal) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = aEvent->actor->As<RE::Actor>();
  auto lArmor = RE::TESForm::LookupByID<RE::TESObjectARMO>(aEvent->baseObject);
  if (!lArmor) return RE::BSEventNotifyControl::kContinue;
  if (lArmor->HasKeyword(fCoveringKey) || lArmor->HasKeyword(fAutoCoverKey) || lArmor->HasPartOf(Tng::cSlotGenital)) {
    if (aEvent->equipped) {
      aSource->notifying = false;
      CheckActor(lActor, lArmor);
      fInternal = false;
      aSource->notifying = true;
    } else {
      TngSizeShape::RandomizeScale(lActor);
    }
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

void TngEvents::RegisterEvents() noexcept {
  const auto lSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
  fEquipManager = RE::ActorEquipManager::GetSingleton();
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  if (!lDataHandler) {
    Tng::gLogger::error("Cannot find necessary SKSE information.");
    return;
  }
  fNPCKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cNPCKeywID, Tng::cSkyrim);
  fAutoRvealKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  fRevealingKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  fUnderwearKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  fAutoCoverKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  fCoveringKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  lTNGRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cTNGRaceKeyID, Tng::cName);
  fGentleWomanKey = lDataHandler->LookupForm<RE::BGSKeyword>(cGentleWomanKeyID, Tng::cName);
  lSkinWithPenisKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  fWomenChance = lDataHandler->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
  lGentified = lDataHandler->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  if (!(lSourceHolder && fNPCKey && fAutoRvealKey && fRevealingKey && fUnderwearKey && fAutoCoverKey && fCoveringKey && lTNGRaceKey && fGentleWomanKey && lSkinWithPenisKey &&
        fWomenChance && lGentified)) {
    Tng::gLogger::error("Mod cannot find necessary info for events, no events registered!");
    return;
  }
  lSourceHolder->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  lSourceHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  Tng::gLogger::info("Registered for necessary events.");
}
