#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>
#include <Util.h>

Events* events = Events::GetSingleton();

void Events::RegisterEvents() {
  coverKeys.clear();
  coverKeys.push_back(ut->Key(Util::kyCovering));
  coverKeys.push_back(ut->Key(Util::kyRevealingF));
  coverKeys.push_back(ut->Key(Util::kyRevealingM));
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
  if (npc->race->HasKeyword(ut->Key(Util::kyPreProcessed)) && !base->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (ut->FormToLocView(armor) == Util::coverID) return RE::BSEventNotifyControl::kContinue;
  if (armor->HasKeyword(ut->Key(Util::kyCovering)) || (armor->HasKeyword(ut->Key(Util::kyRevealingF)) && !npc->IsFemale()) ||
      (armor->HasKeyword(ut->Key(Util::kyRevealingM)) && npc->IsFemale()))
    DoChecks(actor, armor, event->equipped);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = RE::TESForm::LookupByID<RE::Actor>(event->formID);
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return RE::BSEventNotifyControl::kContinue;
  if (core->CanModifyNPC(npc) < 0) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(ut->Key(Util::kyPreProcessed)) && !base->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (actor->IsPlayerRef() && base->HasPlayerChanged(actor)) {
    base->SetPlayerInfo(actor, Util::def);
  }
  DoChecks(actor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
  auto actor = event->subject.get()->As<RE::Actor>();
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!actor || !npc || !npc->skin || !npc->race || !npc->race->skin) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(ut->Key(Util::kyPreProcessed)) && !base->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (npc->skin->HasKeyword(ut->Key(Util::kyTngSkin)) && !npc->race->HasKeyword(ut->Key(Util::kyProcessed))) {
    oldSkins.insert_or_assign(npc->GetFormID(), npc->skin);
    npc->skin = nullptr;
    return RE::BSEventNotifyControl::kContinue;
  }
  if (oldSkins.find(npc->GetFormID()) != oldSkins.end() && npc->race->HasKeyword(ut->Key(Util::kyProcessed))) {
    npc->skin = oldSkins[npc->GetFormID()];
    oldSkins.erase(npc->GetFormID());
    return RE::BSEventNotifyControl::kContinue;
  }
  if (GetNPCAutoAddon(npc).second && npc->race->HasKeyword(ut->Key(Util::kyProcessed)) && !npc->HasKeyword(ut->Key(Util::kyProcessed))) {
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
      if (res->HasKeyword(ut->Key(Util::kyCovering)) || (res->HasKeyword(ut->Key(Util::kyRevealingF)) && !npc->IsFemale()) ||
          (res->HasKeyword(ut->Key(Util::kyRevealingM)) && npc->IsFemale()))
        return res;
    }
  }
  return nullptr;
}

void Events::CheckForAddons(RE::Actor* actor) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race || !npc->race->HasKeyword(ut->Key(Util::kyProcessed))) return;
  if (npc->HasKeyword(ut->Key(Util::kyProcessed))) return;
  npc->AddKeyword(ut->Key(Util::kyProcessed));
  if (!npc->IsPlayer() || !base->GetBoolSetting(Util::bsExcludePlayerSize)) core->SetActorSize(actor, Util::nul);
  auto addnPair = GetNPCAutoAddon(npc);
  if (addnPair.first == Util::err40) {
    SKSE::log::critical("Faced an issue retrieving information for {}!", npc->GetName());
    return;
  }
  bool requiresUpdate = false;
  switch (addnPair.first) {
    case Util::nul:
      if (npc->IsFemale()) {
        if (!npc->skin || !npc->skin->HasKeyword(ut->Key(Util::kyTngSkin))) break;
      } else {
        if (npc->skin && !npc->skin->HasKeyword(ut->Key(Util::kyTngSkin))) break;
      }
      requiresUpdate = true;
      break;
    case Util::def:
      if (base->GetRgAddon(npc->race) < 0) break;
      if (npc->IsFemale()) {
        if ((addnPair.first < 0 && npc->skin && npc->skin->HasKeyword(ut->Key(Util::kyTngSkin))) ||
            (addnPair.first >= 0 && (!npc->skin || !npc->skin->HasKeyword(ut->Key(Util::kyTngSkin)))))
          requiresUpdate = true;
      } else {
        if (addnPair.first < 0 && (!npc->skin || npc->skin->HasKeyword(ut->Key(Util::kyTngSkin)))) break;
        requiresUpdate = true;
      }
      break;
    default:
      if (npc->skin && npc->skin->HasKeyword(ut->Key(Util::kyTngSkin))) break;
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
  auto down = actor->GetWornArmor(Util::genitalSlot);
  auto cover = armor && isEquipped ? armor : GetCoveringItem(actor, isEquipped ? nullptr : armor);
  bool needsCover = NeedsCover(actor);
  if (!needsCover || (down && ut->FormToLocView(down) != Util::coverID) || (!cover && down && ut->FormToLocView(down) == Util::coverID)) {
    if (down && ut->FormToLocView(down) == Util::coverID) {
      RE::ActorEquipManager::GetSingleton()->UnequipObject(actor, down, nullptr, 1, nullptr, false, true, false, true);
    }
    actor->RemoveItem(ut->Block(), 10, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
    return;
  }
  auto tngCover = ForceTngCover(actor, false);
  if ((cover && down) || (!cover && !down)) return;
  if (!tngCover && showErrMessage) {
    showErrMessage = false;
    ut->ShowSkyrimMessage("TNG faced an error when trying to cover genitalia. The New Gentleman won't function properly!");
  }
  if (down && ut->FormToLocView(down) == Util::coverID) {
    RE::ActorEquipManager::GetSingleton()->UnequipObject(actor, down, nullptr, 1, nullptr, false, true, false, true);
  }
  if (cover && !down) {
    RE::ActorEquipManager::GetSingleton()->EquipObject(actor, tngCover);
  }
}

std::pair<int, bool> Events::GetNPCAutoAddon(RE::TESNPC* npc) {
  auto res = base->GetNPCAddon(npc);
  if (res.first != Util::def) return res;
  auto list = base->GetRgAddonList(npc->race, npc->IsFemale(), true);
  const auto count = list.size();
  const auto malChance = base->GetBoolSetting(Util::bsRandomizeMaleAddon) ? ut->malRndChance : 0;
  const size_t chance = npc->IsFemale() ? static_cast<size_t>(std::floor(base->GetFloatSetting(ut->ssWomenChance) + 0.1f)) : malChance;
  if (count == 0 || chance == 0) return {Util::def, false};
  auto addon = npc->GetFormID() % 100 < chance ? static_cast<int>(list[npc->GetFormID() % count]) : Util::def;
  return {addon, false};
}

bool Events::NeedsCover(RE::Actor* actor) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (base->CanModifyNPC(npc) < 0) return false;
  if (npc->IsFemale()) {
    return npc->HasKeyword(ut->Key(Util::kyGentlewoman)) || (base->GetNPCAddon(npc).first >= 0);
  } else {
    return (!npc->HasKeyword(ut->Key(Util::kyExcluded)));
  }
}

RE::TESBoundObject* Events::ForceTngCover(RE::Actor* actor, bool ifUpdate) {
  auto inv =
      actor->GetInventory([=](RE::TESBoundObject& a_object) { return a_object.IsArmor() && ut->FormToLocView(a_object.As<RE::TESObjectARMO>()) == Util::coverID; }, ifUpdate);
  if (!ut->Block()) return nullptr;
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count == 1 && entry && item) {
      return item;
    }
    if (count > 1 && entry && item) {
      actor->RemoveItem(ut->Block(), count - 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
      return item;
    }
  }
  if (ifUpdate) return nullptr;
  actor->AddObjectToContainer(ut->Block(), nullptr, 1, nullptr);
  return ForceTngCover(actor, true);
}