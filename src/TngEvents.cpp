#include <TngEvents.h>

void TngEvents::MakeArmorRevealing(RE::TESObjectARMO* aArmo) noexcept {
  if (aArmo->armorAddons.size() == 0) return;

  for (const auto& lAA : aArmo->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotGenital) && lAA->HasPartOf(Tng::cSlotBody)) lAA->RemoveSlotFromMask(Tng::cSlotGenital);
    const auto lID = (std::string(aArmo->GetName()).empty()) ? aArmo->GetFormEditorID() : aArmo->GetName();
    Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] was recognized revealing.", aArmo->GetLocalFormID(), lID, aArmo->GetFile(0)->GetFilename());
    aArmo->AddKeyword(fRevealingKey);
    return;
  }
}

void TngEvents::CheckActor(RE::Actor* aActor) noexcept {
  const auto lNpc = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNpc) return;
  if (!lNpc->race) return;
  if (!lNpc->race->HasKeyword(fNPCKey)) return;
  const auto lBArmo = aActor->GetWornArmor(Tng::cSlotBody);
  const auto lGArmo = aActor->GetWornArmor(Tng::cSlotGenital);
  if (!lBArmo || !lGArmo) return;
  if (lBArmo == lGArmo) return;
  if (lGArmo->HasKeyword(fUnderwearKey)) return;
  if (lBArmo->HasKeyword(fRevealingKey)) return;
  if (lBArmo->GetFile(0)->GetFilename() != lGArmo->GetFile(0)->GetFilename()) return;
  MakeArmorRevealing(lBArmo);
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = aEvent->actor->As<RE::Actor>();
  CheckActor(lActor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = RE::TESForm::LookupByID<RE::Actor>(aEvent->formID);
  CheckActor(lActor);
  return RE::BSEventNotifyControl::kContinue;
}

void TngEvents::RegisterEvents() noexcept {
  const auto lSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
  fEquipManager = RE::ActorEquipManager::GetSingleton();
  fNPCKey = RE::TESForm::LookupByID<RE::BGSKeyword>(Tng::cNPCKeywID);
  fRevealingKey = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  fUnderwearKey = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  fCoveringKey = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  if (!lSourceHolder || !fNPCKey || !fRevealingKey || !fUnderwearKey) {
    Tng::gLogger::error("Mod cannot find necessary info for events, no events registered!");
    return;
  }
  lSourceHolder->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  lSourceHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  Tng::gLogger::info("Registered for necessary events.");
}
