#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>

void Events::RegisterEvents() noexcept {
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
  fIsPlayerFemale = false;
  fPlayerRace = nullptr;
  fPlayerAddn = -3;
  fPlayerInfoSet = false;
  Tng::gLogger::info("Registered for necessary events.");
}

void Events::SetPlayerInfo(RE::Actor* aPlayer, int aPlayerAddn) noexcept {
  auto lNPC = aPlayer->GetActorBase();
  if (!lNPC) return;
  fIsPlayerFemale = lNPC->IsFemale();
  fPlayerRace = lNPC->race;
  fPlayerAddn = aPlayerAddn;
  fPlayerInfoSet = true;
}

int Events::GetPlayerAddn() noexcept { return fPlayerAddn; }

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = aEvent->actor->As<RE::Actor>();
  auto lArmor = RE::TESForm::LookupByID<RE::TESObjectARMO>(aEvent->baseObject);
  if (!lArmor || !lArmor->HasPartOf(Tng::cSlotBody) || Core::CanModifyActor(lActor) < 0) return RE::BSEventNotifyControl::kContinue;
  aEvent->equipped ? CheckActorArmor(lActor, lArmor) : CheckForAddons(lActor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto lActor = RE::TESForm::LookupByID<RE::Actor>(aEvent->formID);
  if (!lActor) return RE::BSEventNotifyControl::kContinue;
  const auto lNPC = lActor ? lActor->GetActorBase() : nullptr;
  if (lActor->IsPlayerRef() && fPlayerInfoSet && lNPC && lNPC->race->HasKeyword(fPRaceKey)) {
    if (fIsPlayerFemale != lNPC->IsFemale() || fPlayerRace != lActor->GetRace() || fPlayerAddn >= Base::GetAddonCount(lNPC->IsFemale())) {
      lNPC->skin = lNPC->race->skin;
      fPlayerAddn = -1;
      return RE::BSEventNotifyControl::kContinue;
    }
  }
  if (Core::CanModifyActor(lActor) < 0) return RE::BSEventNotifyControl::kContinue;
  CheckActorArmor(lActor);
  CheckForAddons(lActor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* aEvent, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
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

void Events::CheckForAddons(RE::Actor* aActor) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!lNPC) return;
  if (!aActor->IsPlayerRef() || !Inis::GetSettingBool(Inis::excludePlayerSize)) Core::SetCharSize(aActor, lNPC, -1);
  auto lNPCAddn = Base::GetNPCAddn(lNPC);
  if (Base::GetRaceGrpAddn(lNPC->race) == 0) return;
  if (lNPCAddn.second < 0 && (lNPC->IsPlayer() || lNPC->HasKeyword(fExKey) || (!lNPC->IsFemale() && !Inis::GetSettingBool(Inis::randomizeMaleAddn)) || (lNPC->IsFemale() && fGWChance->value < 1))) return;
  if (lNPCAddn.second == Tng::pgErr) {
    Tng::gLogger::critical("Faced an issue retrieving information for {}!", lNPC->GetName());
    return;
  }
  auto lAddn = lNPCAddn.first ? lNPCAddn.second : GetNPCAutoAddn(lNPC);
  Core::SetNPCSkin(lNPC, lAddn, lNPCAddn.first);
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
    Tng::gLogger::info("The armor [0x{:x}: {}] was updated to be revealing.", lBArmo->GetLocalFormID(), lID);
  }  
  if (!lBArmo->HasPartOf(Tng::cSlotGenital) && (lBArmo->HasKeyword(fACKey) || lBArmo->HasKeyword(fCCKey))) {
    lChanged = Core::TryMakeArmorCovering(lBArmo, lBArmo->HasKeyword(fCCKey));    
  }
  if (lChanged && aActor) RE::ActorEquipManager::GetSingleton()->EquipObject(aActor, lBArmo, nullptr, 1, nullptr, false, false, false, true);
}
