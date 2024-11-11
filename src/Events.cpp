#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>

void Events::RegisterEvents() noexcept {
  const auto lSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
  RE::TESDataHandler* Tng::SEDH() = RE::TESDataHandler::GetSingleton();
  fPRaceKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  fCCKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  fACKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  fARKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  fRRKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  fUAKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  fPSKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  Tng::NexKey() = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cExcludeKeyID, Tng::cName);
  fGenSkinKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cCustomSkinID, Tng::cName);
  fGWChance = Tng::SEDH()->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
  fPCAddon = Tng::SEDH()->LookupForm<RE::TESGlobal>(Tng::cPCAddon, Tng::cName);
  if (!(fPRaceKey && fCCKey && fACKey && fARKey && fRRKey && fUAKey && fPSKey && Tng::NexKey() && fGenSkinKey && fGWChance && fPCAddon)) {
    SKSE::log::critical("Failed to register events. There might be functionality issues. Please report this issue.");
    return;
  }
  lSourceHolder->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  lSourceHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  lSourceHolder->AddEventSink<RE::TESSwitchRaceCompleteEvent>(GetSingleton());
  fIsPlayerFemale = false;
  fPlayerRace = nullptr;
  fPlayerInfoSet = false;
  SKSE::log::info("Registered for necessary events.");
}

void Events::SetPlayerInfo(RE::Actor* aPlayer, const int aAddon) noexcept {
  auto npc = aPlayer->GetActorBase();
  if (!npc) return;
  fIsPlayerFemale = npc->IsFemale();
  fPlayerRace = npc->race;
  fPCAddon->value = static_cast<float>(aAddon);
  fPlayerInfoSet = true;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = aEvent->actor->As<RE::Actor>();
  auto armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(aEvent->baseObject);
  if (!armor || !armor->HasPartOf(Tng::cSlotBody) || Core::CanModifyActor(lActor) < 0) return RE::BSEventNotifyControl::kContinue;
  aEvent->equipped ? CheckActorArmor(lActor, armor) : CheckForAddons(lActor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = RE::TESForm::LookupByID<RE::Actor>(aEvent->formID);
  if (!lActor) return RE::BSEventNotifyControl::kContinue;
  if (Core::CanModifyActor(lActor) < 0) return RE::BSEventNotifyControl::kContinue;
  const auto npc = lActor ? lActor->GetActorBase() : nullptr;
  if (fPlayerInfoSet && lActor->IsPlayerRef() && npc && npc->race->HasKeyword(fPRaceKey) && npc->skin && npc->skin != npc->race->skin) {
    if (fIsPlayerFemale != npc->IsFemale() || fPlayerRace != npc->race || fPCAddon->value >= Base::GetAddonCount(npc->IsFemale())) {
      Core::SetNPCSkin(npc, -2, true);
      SetPlayerInfo(lActor, -2);
    }
  }
  CheckActorArmor(lActor);
  CheckForAddons(lActor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* aEvent, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
  auto lActor = aEvent->subject.get()->As<RE::Actor>();
  auto npc = lActor ? lActor->GetActorBase() : nullptr;
  if (!lActor || !npc || !npc->skin) return RE::BSEventNotifyControl::kContinue;
  if (npc->skin->HasKeyword(fGenSkinKey) && !npc->race->HasKeyword(fPRaceKey)) {
    fOldSkins.insert_or_assign(npc->GetFormID(), npc->skin);
    npc->skin = nullptr;
    return RE::BSEventNotifyControl::kContinue;
  }
  if (auto it = fOldSkins.find(npc->GetFormID()); it != fOldSkins.end()) {
    npc->skin = it->second;
  }
  return RE::BSEventNotifyControl::kContinue;
}

void Events::CheckForAddons(RE::Actor* aActor) noexcept {
  const auto npc = aActor ? aActor->GetActorBase() : nullptr;
  if (!npc) return;
  if (!npc->IsPlayer() || !Inis::GetSettingBool(Inis::excludePlayerSize)) Core::SetCharSize(aActor, npc, -1);
  auto lNPCAddn = Base::GetNPCAddn(npc);
  if (Base::GetRGAddn(npc->race) == 0) return;
  if (lNPCAddn.second < 0 &&
      (npc->IsPlayer() || npc->HasKeyword(Tng::NexKey()) || (!npc->IsFemale() && !Inis::GetSettingBool(Inis::randomizeMaleAddn)) || (npc->IsFemale() && fGWChance->value < 1)))
    return;
  if (lNPCAddn.second == Tng::pgErr) {
    SKSE::log::critical("Faced an issue retrieving information for {}!", npc->GetName());
    return;
  }
  auto lAddn = lNPCAddn.first ? lNPCAddn.second : GetNPCAutoAddn(npc);
  Core::SetNPCSkin(npc, lAddn, lNPCAddn.first);
}

int Events::GetNPCAutoAddn(RE::TESNPC* aNPC) noexcept {
  if (aNPC->IsFemale()) {
    const auto lFDistAddnCount = Base::GetActiveFAddnCount();
    if (lFDistAddnCount == 0) return -1;
    return (((aNPC->GetFormID() % 100) < (std::floor(fGWChance->value) + 1))) ? aNPC->GetFormID() % lFDistAddnCount : -1;
  } else {
    const auto lMDistAddnCount = Base::GetActiveMAddnCount();
    return (((aNPC->GetFormID() % 100) > Tng::cMalDefAddnPriority)) ? (aNPC->GetFormID() % lMDistAddnCount) + 1 : -1;
  }
}

void Events::CheckActorArmor(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept {
  const auto lBArmo = aArmor ? aArmor : aActor->GetWornArmor(Tng::cSlotBody);
  if (!lBArmo || lBArmo->armorAddons.size() == 0) return;
  bool lChanged = {false};
  if (lBArmo->HasKeyword(fRRKey) || lBArmo->HasKeywordString(Tng::cSOSR) || lBArmo->HasKeyword(fARKey)) {
    if (!lBArmo->HasPartOf(Tng::cSlotGenital)) return;
    lChanged = Core::TryMakeArmorRevealing(lBArmo, lBArmo->HasKeyword(fRRKey) || lBArmo->HasKeywordString(Tng::cSOSR));
    const auto lID = (std::string(lBArmo->GetName()).empty()) ? lBArmo->GetFormEditorID() : lBArmo->GetName();
    SKSE::log::info("The armor [0x{:x}: {}] was updated to be revealing.", lBArmo->GetLocalFormID(), lID);
  }
  if (!lBArmo->HasPartOf(Tng::cSlotGenital) && (lBArmo->HasKeyword(fACKey) || lBArmo->HasKeyword(fCCKey))) {
    lChanged = Core::TryMakeArmorCovering(lBArmo, lBArmo->HasKeyword(fCCKey));
  }
  if (lChanged && aActor) RE::ActorEquipManager::GetSingleton()->EquipObject(aActor, lBArmo, nullptr, 1, nullptr, false, false, false, true);
}
