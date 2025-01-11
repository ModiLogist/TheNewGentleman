#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>

void Events::RegisterEvents() {
  coverKeys.clear();
  coverKeys.push_back(Tng::ArmoKey(Tng::akeyAutoCover));
  coverKeys.push_back(Tng::ArmoKey(Tng::akeyFixCover));
  showErrMessage = true;
  const auto sesh = RE::ScriptEventSourceHolder::GetSingleton();
  sesh->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  sesh->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  sesh->AddEventSink<RE::TESSwitchRaceCompleteEvent>(GetSingleton());
  playerInfo.isInfoSet = false;
  Tng::logger::info("Registered for necessary events.");
}

void Events::SetPlayerInfo(RE::Actor* aPlayer, const int addnIdx) {
  auto npc = aPlayer->GetActorBase();
  if (!npc) return;
  playerInfo.isFemale = npc->IsFemale();
  playerInfo.race = npc->race;
  playerInfo.isInfoSet = true;
  auto addnFl = static_cast<float>(addnIdx);
  Tng::PCAddon()->value = addnFl;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto actor = aEvent->actor->As<RE::Actor>();
  auto armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(aEvent->baseObject);
  if (Core::CanModifyActor(actor) < 0 || !armor || !armor->HasKeywordInArray(coverKeys, false)) return RE::BSEventNotifyControl::kContinue;
  CheckCovering(actor, armor, aEvent->equipped);
  CheckForAddons(actor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!aEvent) return RE::BSEventNotifyControl::kContinue;
  const auto actor = RE::TESForm::LookupByID<RE::Actor>(aEvent->formID);
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return RE::BSEventNotifyControl::kContinue;
  if (Core::CanModifyActor(actor) != Tng::resOkRaceP) return RE::BSEventNotifyControl::kContinue;
  if (playerInfo.isInfoSet && actor->IsPlayerRef() && npc->skin && npc->skin != npc->race->skin) {
    if (playerInfo.isFemale != npc->IsFemale() || playerInfo.race != npc->race || Tng::PCAddon()->value >= Base::GetAddonCount(npc->IsFemale(), false)) {
      Core::SetNPCAddn(npc, Tng::cDef, true);
      SetPlayerInfo(actor, Tng::cDef);
    }
  }
  CheckCovering(actor);
  CheckForAddons(actor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* aEvent, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
  auto actor = aEvent->subject.get()->As<RE::Actor>();
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!actor || !npc || !npc->skin) return RE::BSEventNotifyControl::kContinue;
  if (npc->skin->HasKeyword(Tng::ArmoKey(Tng::akeyGenSkin)) && !npc->race->HasKeyword(Tng::RaceKey(Tng::rkeyProcessed))) {
    oldSkins.insert_or_assign(npc->GetFormID(), npc->skin);
    npc->skin = nullptr;
    return RE::BSEventNotifyControl::kContinue;
  }
  if (oldSkins.find(npc->GetFormID()) != oldSkins.end() && npc->race->HasKeyword(Tng::RaceKey(Tng::rkeyProcessed))) {
    npc->skin = oldSkins[npc->GetFormID()];
    oldSkins.erase(npc->GetFormID());
  }
  return RE::BSEventNotifyControl::kContinue;
}

void Events::CheckForAddons(RE::Actor* actor) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return;
  if (!npc->IsPlayer() || !Tng::BoolSetting(Tng::bsExcludePlayerSize)) Core::SetActorSize(actor, Tng::cNul);
  auto addnPair = Base::GetNPCAddn(npc);
  if (Base::GetRgAddn(npc->race) == 0) return;
  if (addnPair.second < 0 &&
      (npc->IsPlayer() || npc->HasKeyword(Tng::NPCKey(Tng::npckeyExclude)) || (!npc->IsFemale() && !Tng::BoolSetting(Tng::bsRandomizeMaleAddn)) || (npc->IsFemale() && Tng::WRndGlb()->value < 1)))
    return;
  if (addnPair.second == Tng::pgErr) {
    Tng::logger::critical("Faced an issue retrieving information for {}!", npc->GetName());
    return;
  }
  auto lAddn = addnPair.first ? addnPair.second : GetNPCAutoAddn(npc);
  Core::SetNPCAddn(npc, lAddn, addnPair.first);
}

int Events::GetNPCAutoAddn(RE::TESNPC* npc) {
  const auto count = Base::GetAddonCount(npc->IsFemale(), true);
  const auto chance = npc->IsFemale() ? std::floor(Tng::WRndGlb()->value) : Tng::cMalRandomPriority;
  if (count == 0 || chance == 0) return Tng::cDef;
  return npc->GetFormID() % 100 < chance ? npc->GetFormID() % count : Tng::cDef;
}

void Events::CheckCovering(RE::Actor* actor, RE::TESObjectARMO* armor, bool isEquipped) {
  if (!actor) return;
  auto cover = armor && isEquipped ? armor : GetCoveringItem(actor, isEquipped ? nullptr : armor);
  auto down = actor->GetWornArmor(Tng::cSlotGenital);
  if ((cover && down) || (!cover && !down)) return;
  auto tngCover = ForceTngCover(actor, false);
  if (!tngCover && showErrMessage) {
    showErrMessage = false;
    ShowSkyrimMessage("TNG faced an error when trying to cover genitalia. The New Gentleman won't function properly!");
  }
  if (down && FormToLoc(down) == Tng::cCover) {
    RE::ActorEquipManager::GetSingleton()->UnequipObject(actor, down, nullptr, 1, nullptr, false, true, false, true);
  }
  if (cover && !down) RE::ActorEquipManager::GetSingleton()->EquipObject(actor, tngCover);
}

RE::TESObjectARMO* Events::GetCoveringItem(RE::Actor* actor, RE::TESObjectARMO* armor) {
  auto inv = actor->GetInventory([=](RE::TESBoundObject& a_object) { return a_object.IsArmor() && a_object.HasKeywordInArray(coverKeys, false); }, true);
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;

    if (count > 0 && entry && entry->IsWorn() && item != armor) {
      return item->As<RE::TESObjectARMO>();
    }
  }
  return nullptr;
}

RE::TESBoundObject* Events::ForceTngCover(RE::Actor* actor, bool ifUpdate) {
  auto inv = actor->GetInventory([=](RE::TESBoundObject& a_object) { return a_object.IsArmor() && FormToLoc(a_object.As<RE::TESObjectARMO>()) == Tng::cCover; }, ifUpdate);
  auto cover = Tng::SEDH()->LookupForm<RE::TESObjectARMO>(Tng::cCover.first, Tng::cCover.second);
  if (!cover) return nullptr;
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count == 1 && entry) {
      return item;
    }
    if (count == 0 && entry) {
      actor->AddObjectToContainer(cover, nullptr, 1, nullptr);
      return ForceTngCover(actor, true);
    }
    if (count > 1 && entry) {
      actor->RemoveItem(cover, count - 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
      return item;
    }
  }
  if (ifUpdate) return nullptr;
  actor->AddObjectToContainer(cover, nullptr, 1, nullptr);
  return ForceTngCover(actor, true);
}
