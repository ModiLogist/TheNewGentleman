#include <Core.h>
#include <SEEvents.h>
#include <Util.h>

SEEvents* events = SEEvents::GetSingleton();

void SEEvents::RegisterEvents() {
  showErrMessage = true;
  const auto sesh = RE::ScriptEventSourceHolder::GetSingleton();
  sesh->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  sesh->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  sesh->AddEventSink<RE::TESSwitchRaceCompleteEvent>(GetSingleton());

  SKSE::log::info("Registered for necessary events.");
}

RE::BSEventNotifyControl SEEvents::ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = event->actor ? event->actor->As<RE::Actor>() : nullptr;
  auto npc = actor ? actor->GetActorBase() : nullptr;
  auto armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(event->baseObject);
  if (core->CanModifyActor(actor) < 0 || !armor || !ut->IsCovering(npc, armor) || !armor->HasPartOf(Common::genitalSlot)) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(ut->Key(Common::kyPreProcessed)) && !core->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (ut->IsBlock(armor)) return RE::BSEventNotifyControl::kContinue;
  DoChecks(actor, armor, event->equipped);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl SEEvents::ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = RE::TESForm::LookupByID<RE::Actor>(event->formID);
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return RE::BSEventNotifyControl::kContinue;
  if (core->CanModifyActor(actor) < 0) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(ut->Key(Common::kyPreProcessed)) && !core->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (actor->IsPlayerRef()) core->SetActorAddon(actor, Common::defPlayer, false, false);
  DoChecks(actor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl SEEvents::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
  auto actor = event->subject.get()->As<RE::Actor>();
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!actor || !npc || !npc->skin || !npc->race || !npc->race->skin) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(ut->Key(Common::kyPreProcessed)) && !core->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (npc->skin->HasKeyword(ut->Key(Common::kyTngSkin)) && !npc->race->HasKeyword(ut->Key(Common::kyProcessed))) {
    oldSkins.insert_or_assign(npc->GetFormID(), npc->skin);
    npc->skin = nullptr;
    return RE::BSEventNotifyControl::kContinue;
  }
  if (oldSkins.find(npc->GetFormID()) != oldSkins.end() && npc->race->HasKeyword(ut->Key(Common::kyProcessed))) {
    npc->skin = oldSkins[npc->GetFormID()];
    oldSkins.erase(npc->GetFormID());
    return RE::BSEventNotifyControl::kContinue;
  }
  if (GetNPCAutoAddon(npc).second && npc->race->HasKeyword(ut->Key(Common::kyProcessed)) && !npc->HasKeyword(ut->Key(Common::kyProcessed))) {
    DoChecks(actor);
  }
  return RE::BSEventNotifyControl::kContinue;
}

bool SEEvents::NeedsCover(RE::Actor* actor) {
  if (core->CanModifyActor(actor) < 0) return false;
  auto npc = actor->GetActorBase();
  return npc->skin && npc->skin->HasPartOf(Common::genitalSlot);
}