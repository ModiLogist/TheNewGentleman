#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>

Events* events = Events::GetSingleton();

void Events::RegisterEvents() {
  coverKeys.clear();
  coverKeys.push_back(Tng::Key(Tng::kyCovering));
  coverKeys.push_back(Tng::Key(Tng::kyRevealingF));
  coverKeys.push_back(Tng::Key(Tng::kyRevealingM));
  showErrMessage = true;
  const auto sesh = RE::ScriptEventSourceHolder::GetSingleton();
  sesh->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  sesh->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  sesh->AddEventSink<RE::TESSwitchRaceCompleteEvent>(GetSingleton());

  SKSE::log::info("Registered for necessary events.");
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = event->actor ? event->actor->As<RE::Actor>() : nullptr;
  auto npc = actor ? actor->GetActorBase() : nullptr;
  auto armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(event->baseObject);
  if (core->CanModifyNPC(npc) < 0 || !armor || !armor->HasKeywordInArray(coverKeys, false)) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(Tng::Key(Tng::kyPreProcessed)) && !base->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (FormToLocView(armor) == Tng::cCover) return RE::BSEventNotifyControl::kContinue;
  if (armor->HasKeyword(Tng::Key(Tng::kyCovering)) || (armor->HasKeyword(Tng::Key(Tng::kyRevealingF)) && !npc->IsFemale()) ||
      (armor->HasKeyword(Tng::Key(Tng::kyRevealingM)) && npc->IsFemale()))
    DoChecks(actor, armor, event->equipped);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = RE::TESForm::LookupByID<RE::Actor>(event->formID);
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return RE::BSEventNotifyControl::kContinue;
  if (core->CanModifyNPC(npc) < 0) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(Tng::Key(Tng::kyPreProcessed)) && !base->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (actor->IsPlayerRef() && base->HasPlayerChanged(actor)) {
    base->SetPlayerInfo(actor, Tng::cDef);
  }
  DoChecks(actor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
  auto actor = event->subject.get()->As<RE::Actor>();
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!actor || !npc || !npc->skin) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(Tng::Key(Tng::kyPreProcessed)) && !base->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (npc->skin->HasKeyword(Tng::Key(Tng::kyTngSkin)) && !npc->race->HasKeyword(Tng::Key(Tng::kyProcessed))) {
    oldSkins.insert_or_assign(npc->GetFormID(), npc->skin);
    npc->skin = nullptr;
    return RE::BSEventNotifyControl::kContinue;
  }
  if (oldSkins.find(npc->GetFormID()) != oldSkins.end() && npc->race->HasKeyword(Tng::Key(Tng::kyProcessed))) {
    npc->skin = oldSkins[npc->GetFormID()];
    oldSkins.erase(npc->GetFormID());
    return RE::BSEventNotifyControl::kContinue;
  }
  if (GetNPCAutoAddon(npc).second && npc->race->HasKeyword(Tng::Key(Tng::kyProcessed)) && !npc->HasKeyword(Tng::Key(Tng::kyProcessed))) {
    DoChecks(actor);
  }
  return RE::BSEventNotifyControl::kContinue;
}

void Events::DoChecks(RE::Actor* actor, RE::TESObjectARMO* armor, bool isEquipped) {
  CheckForAddons(actor);
  CheckCovering(actor, armor, isEquipped);
}

RE::TESObjectARMO* Events::GetCoveringItem(RE::Actor* actor, RE::TESObjectARMO* armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return nullptr;
  auto inv = actor->GetInventory([=](RE::TESBoundObject& a_object) { return a_object.IsArmor() && a_object.HasKeywordInArray(coverKeys, false); }, true);
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count > 0 && entry && entry->IsWorn() && item != armor) {
      auto res = item->As<RE::TESObjectARMO>();
      if (res->HasKeyword(Tng::Key(Tng::kyCovering)) || (res->HasKeyword(Tng::Key(Tng::kyRevealingF)) && !npc->IsFemale()) ||
          (res->HasKeyword(Tng::Key(Tng::kyRevealingM)) && npc->IsFemale()))
        return res;
    }
  }
  return nullptr;
}

void Events::CheckForAddons(RE::Actor* actor) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return;
  if (npc->HasKeyword(Tng::Key(Tng::kyProcessed))) return;
  npc->AddKeyword(Tng::Key(Tng::kyProcessed));
  if (!npc->IsPlayer() || !base->GetBoolSetting(Tng::bsExcludePlayerSize)) core->SetActorSize(actor, Tng::cNul);
  auto addnPair = GetNPCAutoAddon(npc);
  if (addnPair.first == Tng::pgErr) {
    SKSE::log::critical("Faced an issue retrieving information for {}!", npc->GetName());
    return;
  }
  bool requiresUpdate = false;
  switch (addnPair.first) {
    case Tng::cNul:
      if (npc->IsFemale()) {
        if (!npc->skin || !npc->skin->HasKeyword(Tng::Key(Tng::kyTngSkin))) break;
      } else {
        if (npc->skin && !npc->skin->HasKeyword(Tng::Key(Tng::kyTngSkin))) break;
      }
      requiresUpdate = true;
      break;
    case Tng::cDef:
      if (base->GetRgAddon(npc->race) < 0) break;
      if (npc->IsFemale()) {
        if ((addnPair.first < 0 && npc->skin && npc->skin->HasKeyword(Tng::Key(Tng::kyTngSkin))) ||
            (addnPair.first >= 0 && (!npc->skin || !npc->skin->HasKeyword(Tng::Key(Tng::kyTngSkin)))))
          requiresUpdate = true;
      } else {
        if (addnPair.first < 0 && (!npc->skin || npc->skin->HasKeyword(Tng::Key(Tng::kyTngSkin)))) break;
        requiresUpdate = true;
      }
      break;
    default:
      if (npc->skin && npc->skin->HasKeyword(Tng::Key(Tng::kyTngSkin))) break;
      requiresUpdate = true;
      break;
  }
  if (requiresUpdate) {
    core->SetNPCAddon(npc, addnPair.first, addnPair.second, false);
  }
  core->UpdateFormLists(actor, npc);
}

void Events::CheckCovering(RE::Actor* actor, RE::TESObjectARMO* armor, bool isEquipped) {
  if (!actor) return;
  auto down = actor->GetWornArmor(Tng::cSlotGenital);
  auto cover = armor && isEquipped ? armor : GetCoveringItem(actor, isEquipped ? nullptr : armor);
  bool needsCover = NeedsCover(actor);
  if (!needsCover || (down && FormToLocView(down) != Tng::cCover) || (!cover && down && FormToLocView(down) == Tng::cCover)) {
    if (down && FormToLocView(down) == Tng::cCover) {
      RE::ActorEquipManager::GetSingleton()->UnequipObject(actor, down, nullptr, 1, nullptr, false, true, false, true);
    }
    actor->RemoveItem(Tng::Block(), 10, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
    return;
  }
  auto tngCover = ForceTngCover(actor, false);
  if ((cover && down) || (!cover && !down)) return;
  if (!tngCover && showErrMessage) {
    showErrMessage = false;
    ShowSkyrimMessage("TNG faced an error when trying to cover genitalia. The New Gentleman won't function properly!");
  }
  if (down && FormToLocView(down) == Tng::cCover) {
    RE::ActorEquipManager::GetSingleton()->UnequipObject(actor, down, nullptr, 1, nullptr, false, true, false, true);
  }
  if (cover && !down) {
    RE::ActorEquipManager::GetSingleton()->EquipObject(actor, tngCover);
  }
}

std::pair<int, bool> Events::GetNPCAutoAddon(RE::TESNPC* npc) {
  auto res = base->GetNPCAddon(npc);
  if (res.first != Tng::cDef) return res;
  auto list = base->GetRgAddonList(npc->race, npc->IsFemale(), true);
  const auto count = list.size();
  const auto malChance = base->GetBoolSetting(Tng::bsRandomizeMaleAddon) ? Tng::cMalRandomPriority : 0;
  const size_t chance = npc->IsFemale() ? static_cast<size_t>(std::floor(Tng::WRndGlb()->value + 0.1)) : malChance;
  if (count == 0 || chance == 0) return {Tng::cDef, false};
  auto addon = npc->GetFormID() % 100 < chance ? static_cast<int>(list[npc->GetFormID() % count]) : Tng::cDef;
  return {addon, false};
}

bool Events::NeedsCover(RE::Actor* actor) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (base->CanModifyNPC(npc) < 0) return false;
  if (npc->IsFemale()) {
    return npc->HasKeyword(Tng::Key(Tng::kyGentlewoman)) || (base->GetNPCAddon(npc).first >= 0);
  } else {
    return (!npc->HasKeyword(Tng::Key(Tng::kyExcluded)));
  }
}

RE::TESBoundObject* Events::ForceTngCover(RE::Actor* actor, bool ifUpdate) {
  auto inv = actor->GetInventory([=](RE::TESBoundObject& a_object) { return a_object.IsArmor() && FormToLocView(a_object.As<RE::TESObjectARMO>()) == Tng::cCover; }, ifUpdate);
  if (!Tng::Block()) return nullptr;
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count == 1 && entry) {
      return item;
    }
    if (count == 0 && entry) {
      actor->AddObjectToContainer(Tng::Block(), nullptr, 1, nullptr);
      return ForceTngCover(actor, true);
    }
    if (count > 1 && entry) {
      actor->RemoveItem(Tng::Block(), count - 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
      return item;
    }
  }
  if (ifUpdate) return nullptr;
  actor->AddObjectToContainer(Tng::Block(), nullptr, 1, nullptr);
  return ForceTngCover(actor, true);
}