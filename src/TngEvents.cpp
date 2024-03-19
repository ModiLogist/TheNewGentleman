#include <TngCore.h>
#include <TngCoreBase.h>
#include <TngEvents.h>
#include <TngInis.h>

void TngEvents::RegisterEvents() noexcept {
  const auto lSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
  RE::TESDataHandler* fDH = RE::TESDataHandler::GetSingleton();
  fPRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  fCCKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  fACKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  fARKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  fRRKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  fUAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  fGWKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cGentleWomanKeyID, Tng::cName);
  fPSKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  fExKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cExcludeKeyID, Tng::cName);
  fGenSkinKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCustomSkinID, Tng::cName);
  fGWChance = fDH->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
  fGentified = fDH->LookupForm<RE::BGSListForm>(Tng::cGentifiedID, Tng::cName);
  if (!(fPRaceKey && fCCKey && fACKey && fARKey && fRRKey && fUAKey && fGWKey && fPSKey && fExKey && fGenSkinKey && fGWChance && fGentified)) {
    Tng::gLogger::critical("Failed to register events. There might be functionality issues. Please report this issue.");
    return;
  }
  lSourceHolder->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  lSourceHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  lSourceHolder->AddEventSink<RE::TESSwitchRaceCompleteEvent>(GetSingleton());
  Tng::gLogger::info("Registered for necessary events.");
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = aEvent->actor->As<RE::Actor>();
  auto lArmor = RE::TESForm::LookupByID<RE::TESObjectARMO>(aEvent->baseObject);
  if (!lArmor || !lActor) return RE::BSEventNotifyControl::kContinue;
  if (!((1 << TngCore::CanModifyActor(lActor)) & ((1 << Tng::resOkRaceP) | (1 << Tng::resOkRaceR)))) return RE::BSEventNotifyControl::kContinue;
  if (!lArmor->HasPartOf(Tng::cSlotBody)) return RE::BSEventNotifyControl::kContinue;
  aEvent->equipped ? CheckActorArmor(lActor, lArmor) : CheckForAddons(lActor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = RE::TESForm::LookupByID<RE::Actor>(aEvent->formID);
  if (!lActor) return RE::BSEventNotifyControl::kContinue;
  if (!((1 << TngCore::CanModifyActor(lActor)) & ((1 << Tng::resOkRaceP) | (1 << Tng::resOkRaceR)))) return RE::BSEventNotifyControl::kContinue;
  CheckActorArmor(lActor);
  CheckForAddons(lActor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* aEvent, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
  auto lActor = aEvent->subject.get()->As<RE::Actor>();
  auto lNPC = lActor ? lActor->GetActorBase() : nullptr;
  if (!lActor || !lNPC || !lNPC->skin) return RE::BSEventNotifyControl::kContinue;
  if (lNPC->skin->HasKeyword(fGenSkinKey) && !lNPC->race->HasKeyword(fPRaceKey)) {
    fOldSkins.insert_or_assign(lNPC->GetFormID(), lNPC->skin);
    lNPC->skin = lNPC->race->skin;
    return RE::BSEventNotifyControl::kContinue;
  }
  if (auto lIt = fOldSkins.find(lNPC->GetFormID()); lIt != fOldSkins.end()) {
    lNPC->skin = lIt->second;
  }
  return RE::BSEventNotifyControl::kContinue;
}

void TngEvents::CheckForAddons(RE::Actor* aActor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return;
  auto lNPCAddn = TngCoreBase::GetNPCAddn(lNPC);
  if (lNPCAddn.second < 0) {
    if (!lNPC->IsFemale() && lNPC->HasKeyword(fExKey)) TngCore::RevertNPCSkin(lNPC);
    if (lNPC->IsPlayer() || !lNPC->IsFemale() || lNPC->HasKeyword(fExKey) || fGWChance->value < 1) return;
  }
  if (lNPCAddn.second == Tng::pgErr) {
    Tng::gLogger::critical("Faced an issue retrieving information for {}!", lNPC->GetName());
    return;
  }
  auto lAddn = lNPCAddn.first ? lNPCAddn.second : GetNPCAutoAddn(lNPC);
  TngCore::SetNPCSkin(lNPC, lAddn, lNPCAddn.first);
}

int TngEvents::GetNPCAutoAddn(RE::TESNPC* aNPC) noexcept {
  const auto lFDistAddnCount = TngCoreBase::GetActiveFAddnCount();
  if (lFDistAddnCount == 0) return -1;
  return (((aNPC->GetFormID() % 100) < (std::floor(fGWChance->value) + 1))) ? aNPC->GetFormID() % lFDistAddnCount : -1;
}

void TngEvents::CheckActorArmor(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept {
  if (aActor && aActor->HasKeyword(fExKey)) return;  
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!lNPC || !lNPC->race) return;
  if (!(lNPC->race->HasKeyword(fPRaceKey) || lNPC->race->HasKeyword(fRRKey))) return;
  if (!aActor->IsPlayerRef() || !TngInis::GetExcludePlayer()) TngCore::SetCharSize(aActor, lNPC, -1);
  if (aArmor->HasPartOf(Tng::cSlotBody)) TngCore::TryMakeArmorCovering(aArmor, aArmor->HasKeyword(fCCKey));
  const auto lGArmo = aActor->GetWornArmor(Tng::cSlotGenital);
  const auto lBArmo = aActor->GetWornArmor(Tng::cSlotBody);
  if ((lNPC->IsFemale() && !TngInis::GetAutoReveal(true)) || (!lNPC->IsFemale() && !TngInis::GetAutoReveal(false))) return;
  if (!lBArmo || !(lBArmo->HasKeyword(fRRKey) || lBArmo->HasKeywordString("SOS_Revealing"))) {
    if (!lBArmo || !lGArmo || lBArmo == lGArmo || !lBArmo->GetFile(0) || !lGArmo->GetFile(0) || !lBArmo->HasKeyword(fACKey) || lGArmo->HasKeyword(fUAKey)) return;
    if (lBArmo->GetFile(0)->GetFilename() != lGArmo->GetFile(0)->GetFilename()) return;
  }
  if (lBArmo->armorAddons.size() == 0) return;
  bool lChanged = false;
  for (const auto& lAA : lBArmo->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotGenital) && lAA->HasPartOf(Tng::cSlotBody)) {
      lAA->RemoveSlotFromMask(Tng::cSlotGenital);
      lChanged = true;
    }
    const auto lID = (std::string(lBArmo->GetName()).empty()) ? lBArmo->GetFormEditorID() : lBArmo->GetName();
    Tng::gLogger::info("The armor [0x{:x}: {}] was updated to be revealing.", lBArmo->GetLocalFormID(), lID);
    lBArmo->RemoveKeyword(fACKey);
    lBArmo->AddKeyword(fARKey);
  }
  if (lChanged && aActor) RE::ActorEquipManager::GetSingleton()->EquipObject(aActor, lBArmo, nullptr, 1, nullptr, false, false, false, true);
}
