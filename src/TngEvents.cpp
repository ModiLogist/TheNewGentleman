#include <TngEvents.h>

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = RE::TESForm::LookupByID<RE::Actor>(aEvent->formID);
  if (!lActor) return RE::BSEventNotifyControl::kContinue;
  if (lActor->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
  const auto lNpc = lActor ? lActor->GetActorBase() : nullptr;
  if (lActor && lNpc) {
    if (lNpc->HasKeyword(fNPCKey)) return RE::BSEventNotifyControl::kContinue;
    ProcessActor(lActor);
  }
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl TngEvents::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) {
  const auto lActor = aEvent->actor->As<RE::Actor>();
  if (!lActor) return RE::BSEventNotifyControl::kContinue;
  const auto lArmor = RE::TESForm::LookupByID<RE::TESObjectARMO>(aEvent->baseObject);
  const auto lNpc = lActor ? lActor->GetActorBase() : nullptr;
  if (!lActor || !lArmor || !lNpc) return RE::BSEventNotifyControl::kContinue;
  return RE::BSEventNotifyControl::kContinue;
}

bool TngEvents::IsCovering(const RE::TESBoundObject* aItem, const std::unique_ptr<RE::InventoryEntryData>& aEntry) noexcept {
  if (!aEntry->IsWorn()) return false;
  const auto lArmor = aItem->As<RE::TESObjectARMO>();
  if (!lArmor) return false;
  if (lArmor->HasPartOf(Tng::cSlotBody)) return true;
  return false;
}

void TngEvents::ProcessActor(RE::Actor* aActor) noexcept {
  const auto lActInv = aActor->GetInventory();
  Tng::gLogger::info("{} has {} items in inventory.", aActor->GetName(), lActInv.size());
  for (const auto& [lItem, lData] : lActInv) {
    if (lItem->Is(RE::FormType::LeveledItem)) continue;
    const auto& [lCount, lEntry] = lData;
    if ((lCount > 0) && IsCovering(lItem, lEntry)) {
      aActor->AddObjectToContainer(fCover, nullptr, 1, nullptr);
      fEquipManager->EquipObject(aActor, fCover);
      Tng::gLogger::info("Fixed {}", aActor->GetName());
      return;
    }
  }
  fWaitingActors.insert(aActor);
}

void TngEvents::RegisterEvents() noexcept {
  const auto lSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
  fEquipManager = RE::ActorEquipManager::GetSingleton();
  fCover = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectARMO>(cCoverID, Tng::cName);
  fNPCKey = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cNPCKeywID, Tng::cName);
  if (!lSourceHolder || !fEquipManager) return;
  lSourceHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  /*lSourceHolder->AddEventSink<RE::TESEquipEvent>(GetSingleton());*/
  Tng::gLogger::info("Registered for necessary events.");
}
