#include <Core.h>
#include <SEEvents.h>
#include <Util.h>

SEEvents* events = SEEvents::GetSingleton();

void SEEvents::RegisterEvents() {
  const auto sesh = RE::ScriptEventSourceHolder::GetSingleton();
  sesh->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  sesh->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  sesh->AddEventSink<RE::TESSwitchRaceCompleteEvent>(GetSingleton());
  SKSE::log::info("Registered for necessary events.");
}

RE::BSEventNotifyControl SEEvents::ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = event->actor ? event->actor->As<RE::Actor>() : nullptr;
  auto armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(event->baseObject);
  if (!armor || ut->IsBlock(armor) || (!ut->IsCovering(actor, armor) && !armor->HasPartOf(Common::genitalSlot))) return RE::BSEventNotifyControl::kContinue;
  core->UpdateActor(actor, armor, event->equipped);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl SEEvents::ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!event || !event->loaded) return RE::BSEventNotifyControl::kContinue;
  if (const auto actor = RE::TESForm::LookupByID<RE::Actor>(event->formID); actor) core->UpdateActor(actor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl SEEvents::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
  auto actor = event->subject.get()->As<RE::Actor>();
  bool isPlayer = actor->IsPlayerRef();
  ut->DoDelayed(
      [actor, isPlayer]() {
        auto ac = isPlayer ? RE::PlayerCharacter::GetSingleton() : actor;
        if (!ac) return;
        core->UpdateActor(ac);
      },
      []() { return true; }, isPlayer * 500, false);
  return RE::BSEventNotifyControl::kContinue;
}