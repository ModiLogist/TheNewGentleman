#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>
#include <Util.h>

Events* events = Events::GetSingleton();

void Events::RegisterEvents() {
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
  if (core->CanModifyNPC(npc) < 0 || !armor || !ut->IsCovering(npc, armor) || !armor->HasPartOf(Common::genitalSlot)) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(ut->Key(Common::kyPreProcessed)) && !core->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (ut->IsBlock(armor)) return RE::BSEventNotifyControl::kContinue;
  DoChecks(actor, armor, event->equipped);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = RE::TESForm::LookupByID<RE::Actor>(event->formID);
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return RE::BSEventNotifyControl::kContinue;
  if (core->CanModifyNPC(npc) < 0) return RE::BSEventNotifyControl::kContinue;
  if (npc->race->HasKeyword(ut->Key(Common::kyPreProcessed)) && !core->ReevaluateRace(npc->race, actor)) return RE::BSEventNotifyControl::kContinue;
  if (actor->IsPlayerRef() && core->HasPlayerChanged(actor)) {
    core->SetPlayerInfo(actor, Common::def);
  }
  DoChecks(actor);
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) {
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

void Events::DoChecks(RE::Actor* actor, RE::TESObjectARMO* armor, bool isEquipped) {
  CheckForAddons(actor);
  CheckCovering(actor, armor, isEquipped);
  CheckDF(actor);
}

RE::TESObjectARMO* Events::GetCoveringItem(RE::Actor* actor, RE::TESObjectARMO* exception) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return nullptr;
  auto inv = actor->GetInventory([=](RE::TESBoundObject& a_object) { return a_object.IsArmor() && ut->IsCovering(npc, a_object.As<RE::TESObjectARMO>()); }, true);
  for (const auto& [item, invData] : inv) {
    if (item && invData.first && invData.second && invData.second->IsWorn() && item != exception) {
      return item->As<RE::TESObjectARMO>();
    }
  }
  return nullptr;
}

void Events::CheckForAddons(RE::Actor* actor) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race || !npc->race->HasKeyword(ut->Key(Common::kyProcessed))) return;
  if (npc->HasKeyword(ut->Key(Common::kyProcessed))) return;
  npc->AddKeyword(ut->Key(Common::kyProcessed));
  if (!npc->IsPlayer() || !core->GetBoolSetting(Common::bsExcludePlayerSize)) core->SetActorSize(actor, Common::nul);
  auto addnPair = GetNPCAutoAddon(npc);
  if (addnPair.first == Common::err40) {
    SKSE::log::critical("Faced an issue retrieving information for {}!", npc->GetName());
    return;
  }
  bool requiresUpdate = false;
  switch (addnPair.first) {
    case Common::nul:
      if (npc->IsFemale()) {
        if (!npc->skin || !npc->skin->HasKeyword(ut->Key(Common::kyTngSkin))) break;
      } else {
        if (npc->skin && !npc->skin->HasKeyword(ut->Key(Common::kyTngSkin))) break;
      }
      requiresUpdate = true;
      break;
    case Common::def:
      if (core->GetRgAddon(npc->race) < 0) break;
      if (npc->IsFemale()) {
        if ((addnPair.first < 0 && npc->skin && npc->skin->HasKeyword(ut->Key(Common::kyTngSkin))) ||
            (addnPair.first >= 0 && (!npc->skin || !npc->skin->HasKeyword(ut->Key(Common::kyTngSkin)))))
          requiresUpdate = true;
      } else {
        if (addnPair.first < 0 && (!npc->skin || npc->skin->HasKeyword(ut->Key(Common::kyTngSkin)))) break;
        requiresUpdate = true;
      }
      break;
    default:
      if (npc->skin && npc->skin->HasKeyword(ut->Key(Common::kyTngSkin))) break;
      requiresUpdate = true;
      break;
  }
  if (requiresUpdate) {
    core->SetActorAddon(actor, addnPair.first, addnPair.second, false);
  }
  core->UpdateFormLists(actor, npc);
}

void Events::CheckCovering(RE::Actor* actor, RE::TESObjectARMO* armor, bool isEquipped) {
  if (!actor) return;
  auto down = armor && isEquipped && armor->HasPartOf(Common::genitalSlot) ? armor : actor->GetWornArmor(Common::genitalSlot);
  if (down && down == armor && !isEquipped) down = nullptr;
  auto cover = armor && isEquipped && !armor->HasPartOf(Common::genitalSlot) ? armor : GetCoveringItem(actor, isEquipped ? nullptr : armor);
  bool needsCover = NeedsCover(actor);
  if (!needsCover || (down && (!ut->IsBlock(down) || !cover))) {
    actor->RemoveItem(ut->Block(), 10, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
    return;
  }
  if ((cover && down) || (!cover && !down)) return;
  auto tngBlock = ut->Block();
  if (!tngBlock) {
    if (showErrMessage) {
      showErrMessage = false;
      ut->ShowSkyrimMessage("TNG faced an error when trying to cover genitalia. The New Gentleman won't function properly!");
    }
    return;
  }
  actor->AddObjectToContainer(tngBlock, nullptr, 1, nullptr);
  RE::ActorEquipManager::GetSingleton()->EquipObject(actor, tngBlock);
}
std::pair<int, bool> Events::GetNPCAutoAddon(RE::TESNPC* npc) {
  auto res = core->GetNPCAddon(npc);
  if (res.first != Common::def) return res;
  auto list = core->GetRgAddonList(npc->race, npc->IsFemale(), true);
  const auto count = list.size();
  const auto malChance = core->GetBoolSetting(Common::bsRandomizeMaleAddon) ? ut->malRndChance : 0;
  const size_t chance = npc->IsFemale() ? static_cast<size_t>(std::floor(core->GetFloatSetting(ut->fsWomenChance) + 0.1f)) : malChance;
  if (count == 0 || chance == 0) return {Common::def, false};
  auto addon = npc->GetFormID() % 100 < chance ? static_cast<int>(list[npc->GetFormID() % count]) : Common::def;
  return {addon, false};
}

bool Events::NeedsCover(RE::Actor* actor) {
  const auto npc = actor ? actor->GetActorBase() : nullptr;
  if (core->CanModifyNPC(npc) < 0) return false;
  if (npc->IsFemale()) {
    return npc->HasKeyword(ut->Key(Common::kyGentlewoman)) || (core->GetNPCAddon(npc).first >= 0);
  } else {
    return (!npc->HasKeyword(ut->Key(Common::kyExcluded)));
  }
}