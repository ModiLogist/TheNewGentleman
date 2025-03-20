#include <Base.h>
#include <Core.h>
#include <Inis.h>

Core* core = Core::GetSingleton();

void Core::ProcessRaces() {
  SKSE::log::info("Finding the genitals for relevant races...");
  const auto& allRaces = ut->SEDH()->GetFormArray<RE::TESRace>();
  int preprocessed = 0;
  int processed = 0;
  int ignored = 0;
  int ready = 0;

  std::set<std::string> validSkeletons;
  validSkeletons.emplace(ut->Race(Util::raceDefault)->skeletonModels[0].model.data());
  validSkeletons.emplace(ut->Race(Util::raceDefault)->skeletonModels[1].model.data());
  validSkeletons.emplace(ut->Race(Util::raceDefBeast)->skeletonModels[0].model.data());
  validSkeletons.emplace(ut->Race(Util::raceDefBeast)->skeletonModels[1].model.data());
  for (const auto& race : allRaces) {
    if (inis->IsRaceExcluded(race)) {
      SKSE::log::debug("\tThe race [{}: xx{:x}: {}] was ignored because an ini excludes it!", race->GetFile(0)->GetFilename(), race->GetLocalFormID(), race->GetFormEditorID());
      IgnoreRace(race, false);
      ignored++;
      continue;
    }
    if (race->HasKeyword(ut->Key(Util::kyIgnored))) {
      ignored++;
      continue;
    }
    if (race->HasKeyword(ut->Key(Util::kyReady))) {
      ready++;
      continue;
    }
    if (race->HasKeyword(ut->Key(Util::kyProcessed))) {
      processed++;
      continue;
    }
    if (race->HasKeyword(ut->Key(Util::kyPreProcessed))) {
      preprocessed++;
      continue;
    }
    switch (AddPotentialRace(race, validSkeletons)) {
      case Util::resOkRacePP:
        base->AddRace(race, false);
        preprocessed++;
        break;
      case Util::resOkRaceP:
        base->AddRace(race, true);
        processed++;
        break;
      case Util::resOkRaceR:
        ready++;
        break;
      case Util::errRace:
        ignored++;
        break;
    }
  }
  base->UpdateRgSkins();
  inis->LoadRgInfo();
  SKSE::log::info("\tRecognized assigned genitalia to [{}] races. Preprocessed [{}] races, found [{}] races to be ready and ignored [{}] races.", processed, preprocessed, ready,
                  ignored);
}

bool Core::SetRgAddon(const size_t rgChoice, const int addnIdx, const bool onlyMCM) {
  auto res = base->SetRgAddon(rgChoice, addnIdx, onlyMCM);
  if (!res) return res;
  inis->SaveRgAddon(rgChoice, addnIdx);
  return true;
}

void Core::IgnoreRace(RE::TESRace* race, bool ready) {
  if (!race) return;
  if (auto& skin = race->skin; skin) skin->AddKeyword(ut->Key(Util::kyIgnored));
  race->RemoveKeywords(ut->Keys(Util::kyProcessed, Util::kyExcluded));
  race->AddKeyword(ut->Key(ready ? Util::kyReady : Util::kyIgnored));
}

Util::eRes Core::AddPotentialRace(RE::TESRace* race, const std::set<std::string>& validSkeletons) {
  try {
    for (auto raceInfo : hardCodedRaces)
      if (ut->FormToLocView(race) == raceInfo) return Util::resOkRaceP;
    if (!race->HasKeyword(ut->Key(Util::kyManMer)) || race->HasKeyword(ut->Key(Util::kyCreature)) || race->IsChildRace()) return Util::errRace;
    if (!race->skin) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since they do not have a skin! It was last modified by [{}].", race->GetFormID(), race->GetFormEditorID(),
                      race->GetFile() ? race->GetFile()->GetFilename() : "Unrecognized File");
      IgnoreRace(race, false);
      return Util::errRace;
    }
    bool skinFound = false;
    for (const auto& aa : race->skin->armorAddons) {
      if (aa->HasPartOf(Util::bodySlot) && aa->IsValidRace(race)) {
        skinFound = true;
        break;
      }
    }
    if (!skinFound) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since their skin cannot be recognized!  It was last modified by [{}].", race->GetFormID(),
                      race->GetFormEditorID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unrecognized File");
      IgnoreRace(race, false);
      return Util::errRace;
    }
    if (race->HasPartOf(Util::genitalSlot)) {
      auto ready = race->skin->HasPartOf(Util::genitalSlot);
      SKSE::log::info("\tThe race [{}] is designed to be {} TNG. It was not modified.", race->GetFormEditorID(), ready ? "ready for" : "ignored by");
      IgnoreRace(race, ready);
      return ready ? Util::resOkRaceR : Util::errRace;
    }
  } catch (const std::exception& er) {
    SKSE::log::warn("\tThe race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(),
                    race->GetFormEditorID(), er.what());
    const char* message = fmt::format("\tThe race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(),
                                      race->GetFormEditorID(), er.what())
                              .c_str();
    ut->ShowSkyrimMessage(message);
    IgnoreRace(race, false);
    return Util::errRace;
  }
  bool isValidSk =
      validSkeletons.find(race->skeletonModels[0].model.data()) != validSkeletons.end() && validSkeletons.find(race->skeletonModels[1].model.data()) != validSkeletons.end();
  return isValidSk ? Util::resOkRaceP : Util::resOkRacePP;
}

void Core::ProcessSkins() {
  SKSE::log::info("Checking NPCs for custom skins.");
  std::map<std::pair<RE::TESObjectARMO*, RE::TESRace*>, std::set<RE::TESNPC*>> skinsToPatch{};
  std::map<std::string_view, size_t> customSkinMods{};
  std::map<RE::TESRace*, size_t> raceNPCCount;
  auto& allNPCs = ut->SEDH()->GetFormArray<RE::TESNPC>();
  size_t sizeCount[Util::sizeKeyCount]{0};
  for (const auto& npc : allNPCs) {
    if (inis->IsNPCExcluded(npc)) {
      npc->AddKeyword(ut->Key(Util::kyExcluded));
      continue;
    }
    const auto race = npc->race;
    if (!race) {
      SKSE::log::warn("\t\tThe NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", npc->GetFormID(), npc->GetFormEditorID());
      continue;
    }
    if (!race->HasKeyword(ut->Key(Util::kyProcessed)) && !race->HasKeyword(ut->Key(Util::kyPreProcessed))) continue;
    raceNPCCount[npc->race]++;
    if (npc->IsFemale()) continue;
    sizeCount[npc->formID % Util::sizeKeyCount]++;
    const auto skin = npc->skin;
    if (!skin) continue;
    if (skin->HasKeyword(ut->Key(Util::kyIgnored))) continue;
    if (!race->HasKeyword(ut->Key(Util::kyProcessed))) continue;
    skinsToPatch[{skin, race}].insert(npc);
  }
  for (auto& racePair : raceNPCCount) {
    if (racePair.second < 5) continue;
    base->TryUnhideRace(racePair.first);
  }
  for (auto& skinPair : skinsToPatch) {
    auto& oldSkin = skinPair.first.first;
    auto& race = skinPair.first.second;
    auto newSkin = FixSkin(oldSkin, race, nullptr);
    if (!newSkin) continue;
    auto modName = oldSkin->GetFile(0) ? oldSkin->GetFile(0)->GetFilename() : "(no mod name)";
    customSkinMods[modName]++;
    for (auto& npc : skinPair.second) npc->skin = newSkin;
  }
  if (skinsToPatch.size() > 0) {
    auto tot = skinsToPatch.size();
    SKSE::log::info("\tHandled {} custom skin-race combinations from following mod(s):", skinsToPatch.size());
    for (const auto& entry : customSkinMods) {
      SKSE::log::info("\t\t[{}] skins from {}", entry.second, entry.first);
      tot -= entry.second;
    }
    SKSE::log::info("\t\t[{}] skins were not patched.", tot);
  }
  inis->LoadNpcInfo();
  SKSE::log::debug("TNG distributed the sizes (from smallest to largest) to [{}] NPCs!", fmt::join(sizeCount, ", "));
  base->ReportHiddenRgs();
}

Util::eRes Core::CanModifyNPC(RE::TESNPC* npc) {
  auto res = base->CanModifyNPC(npc);
  if (res < 0) return res;
  return (inis->IsNPCExcluded(npc)) ? Util::errNPC : res;
}

Util::eRes Core::SetActorSize(RE::Actor* actor, int genSize) {
  auto res = base->SetActorSize(actor, genSize);
  if (res == Util::resOkSizable && !actor->GetActorBase()->IsPlayer()) inis->SaveNPCSize(actor->GetActorBase(), genSize);
  return res;
}

Util::eRes Core::SetNPCAddon(RE::TESNPC* npc, const int addnIdx, const bool isUser, const bool shouldSave) {
  if (!npc->race || !npc->race->HasKeyword(ut->Key(Util::kyProcessed))) return Util::errRace;
  if (inis->IsNPCExcluded(npc)) return Util::errNPC;
  auto res = base->SetNPCAddon(npc, addnIdx, isUser);
  if (res < 0) return res;
  if (!npc->IsPlayer() && shouldSave) inis->SaveNPCAddon(npc, addnIdx);
  return res;
}

void Core::UpdateFormLists(RE::Actor* actor, RE::TESNPC* npc) {
  if (npc->IsFemale()) {
    if (npc->HasKeyword(ut->Key(Util::kyGentlewoman)) && !ut->TngFml(Util::flmGentleWomen)->HasForm(actor)) {
      ut->TngFml(Util::flmGentleWomen)->AddForm(actor);
    } else if (!npc->HasKeyword(ut->Key(Util::kyGentlewoman)) && ut->TngFml(Util::flmGentleWomen)->HasForm(actor)) {
      for (RE::BSTArray<RE::TESForm*>::const_iterator it = ut->TngFml(Util::flmGentleWomen)->forms.begin(); it < ut->TngFml(Util::flmGentleWomen)->forms.end(); it++) {
        if ((*it)->As<RE::Actor>() == actor) ut->TngFml(Util::flmGentleWomen)->forms.erase(it);
      }
    }
  }
  if (!npc->IsFemale()) {
    if (npc->HasKeyword(ut->Key(Util::kyExcluded)) && !ut->TngFml(Util::flmNonGentleMen)->HasForm(actor)) {
      ut->TngFml(Util::flmNonGentleMen)->AddForm(actor);
    } else if (!npc->HasKeyword(ut->Key(Util::kyExcluded)) && ut->TngFml(Util::flmNonGentleMen)->HasForm(actor)) {
      for (RE::BSTArray<RE::TESForm*>::const_iterator it = ut->TngFml(Util::flmNonGentleMen)->forms.begin(); it < ut->TngFml(Util::flmNonGentleMen)->forms.end(); it++) {
        if ((*it)->As<RE::Actor>() == actor) ut->TngFml(Util::flmNonGentleMen)->forms.erase(it);
      }
    }
  }
}

std::vector<RE::TESObjectARMO*> Core::GetActorWornArmor(RE::Actor* actor) {
  std::vector<RE::TESObjectARMO*> res{};
  res.clear();
  if (!actor) return res;
  auto inv = actor->GetInventory([=](RE::TESBoundObject& obj) { return obj.IsArmor(); });
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count > 0 && entry && entry->IsWorn() && ut->FormToLocView(item) != Util::coverID) {
      res.push_back(item->As<RE::TESObjectARMO>());
    }
  }
  return res;
}

RE::TESObjectARMO* Core::FixSkin(RE::TESObjectARMO* skin, RE::TESRace* race, const char* const name) {
  skin->RemoveKeywords(ut->Keys(Util::kyRevealingF, Util::kyRevealing));
  skin->AddKeyword(ut->Key(Util::kyIgnored));
  if (!skin->HasPartOf(Util::bodySlot)) {
    SKSE::log::info("\t\tThe skin [0x{:x}] used does not have a body part. TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->HasPartOf(Util::genitalSlot)) {
    SKSE::log::warn("\t\tThe skin [0x{:x}] cannot have a male genital.", skin->GetFormID());
    return nullptr;
  }
  if (!skin->race) {
    SKSE::log::warn("\t\tThe skin [0x{:x}] does not have a race! TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->armorAddons.size() == 0) {
    SKSE::log::warn("\t\tThe skin [0x{:x}] does not have any arma! TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  auto rgIdx = base->GetRaceRgIdx(race);
  auto addonIdx = base->GetRgAddon(race);
  if (rgIdx < 0 || addonIdx == Util::err40) {
    SKSE::log::critical("\t\tSkin [xx{:x}] from file [{}] together with race [xx{:x}] from file [{}] caused a critical error!", skin->GetLocalFormID(),
                        skin->GetFile() ? skin->GetFile()->GetFilename() : "Unknown", race->GetLocalFormID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unknown");
    return nullptr;
  }
  if (addonIdx == Util::nul) return skin;
  if (name) SKSE::log::info("\t\tThe skin [0x{:x}: {}] added as extra skin.", skin->GetFormID(), name);
  return base->GetSkinWithAddonForRg(rgIdx, skin, addonIdx, false);
}

void Core::CheckArmorPieces() {
  SKSE::log::info("Checking ARMO records...");
  auto& armorList = ut->SEDH()->GetFormArray<RE::TESObjectARMO>();
  int hk = 0;
  int ia = 0;
  int rr = 0;
  int ar = 0;
  int cc = 0;
  int ac = 0;
  int pa = 0;
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  std::set<std::string> potentialMods{};
  std::set<std::string> potentialSlot52Mods{};
  std::vector<RE::BGSKeyword*> armorKeys = ut->Keys(Util::kyRevealingF, Util::kyRevealing);
  armorKeys.push_back(ut->Key(Util::kyIgnored));
  std::vector<RE::BGSKeyword*> relRaceKeys{ut->Key(Util::kyProcessed), ut->Key(Util::kyPreProcessed), ut->Key(Util::kyReady)};
  for (auto covRec : hardCodedCovering) {
    auto armor = ut->SEDH()->LookupForm<RE::TESObjectARMO>(covRec.first, covRec.second);
    if (!armor) continue;
    armor->RemoveKeywords(armorKeys);
    armor->AddKeyword(ut->Key(Util::kyCovering));
  }
  for (const auto& armor : armorList) {
    if (!armor) {
      pa++;
      continue;
    }
    if (armor->HasKeywordInArray(armorKeys, false)) {
      hk++;
      continue;
    }
    if (armor->armorAddons.size() == 0) armor->AddKeyword(ut->Key(Util::kyIgnored));
    const auto armorID = (std::string(armor->GetName()).empty()) ? armor->GetFormEditorID() : armor->GetName();
    std::string modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (!armor->race) {
      if (!armor->HasKeyword(ut->Key(Util::kyIgnored)))
        SKSE::log::warn("\t\tThe armor [0x{:x}: {}] does not have a race! It won't be touched by Tng!", armor->GetFormID(), armorID);
      armor->AddKeyword(ut->Key(Util::kyIgnored));
      ia++;
      continue;
    }
    if (!armor->race->HasKeywordInArray(relRaceKeys, false) && armor->race != ut->Race(Util::raceDefault)) continue;
    if (inis->IsSkin(armor, modName)) {
      SKSE::log::info("\t\tThe record [0x{:x}: {}] was marked as a skin.", armor->GetFormID(), armorID);
      armor->AddKeyword(ut->Key(Util::kyIgnored));
      ia++;
      continue;
    }
    bool has52 = armor->HasPartOf(Util::genitalSlot);
    for (auto& aa : armor->armorAddons) {
      if (has52) break;
      has52 = aa->HasPartOf(Util::genitalSlot);
    }
    if (!has52) {
      if (inis->IsRTCovering(armor, modName)) {
        armor->AddKeyword(ut->Key(Util::kyCovering));
        SKSE::log::info("\t\tThe armor [[0x{:x}: {}] was marked covering since it was previously marked covering during gameplay.", armor->GetFormID(), armorID);
        ac++;
        continue;
      }
      auto keyPair = [&]() -> std::pair<RE::BGSKeyword*, std::string> {
        switch (inis->IsRTRevealing(armor, modName)) {
          case Util::kyRevealing:
            return {ut->Key(Util::kyRevealing), "all"};
          case Util::kyRevealingF:
            return {ut->Key(Util::kyRevealingF), "women"};
          case Util::kyRevealingM:
            return {ut->Key(Util::kyRevealingM), "men"};
          default:
            return {nullptr, ""};
        }
      }();
      if (keyPair.first) {
        if (armor->HasPartOf(Util::bodySlot)) {
          armor->AddKeyword(keyPair.first);
          SKSE::log::info("\t\tThe armor [[0x{:x}: {}] was marked revealing for {} since it was previously marked revealing during gameplay.", armor->GetFormID(), armorID,
                          keyPair.second);
          ar++;
        } else {
          ia++;
        }
        continue;
      }
      if (inis->IsCovering(armor, modName)) {
        armor->AddKeyword(ut->Key(Util::kyCovering));
        SKSE::log::info("\t\tThe armor [0x{:x}: {}] was marked covering.", armor->GetFormID(), armorID);
        cc++;
        continue;
      }
      keyPair = [&]() -> std::pair<RE::BGSKeyword*, std::string> {
        switch (inis->IsRevealing(armor, modName)) {
          case Util::kyRevealing:
            return {ut->Key(Util::kyRevealing), "all"};
          case Util::kyRevealingF:
            return {ut->Key(Util::kyRevealingF), "women"};
          case Util::kyRevealingM:
            return {ut->Key(Util::kyRevealingM), "men"};
          default:
            return {nullptr, ""};
        }
      }();
      if (keyPair.first) {
        if (armor->HasPartOf(Util::bodySlot)) {
          armor->AddKeyword(keyPair.first);
          SKSE::log::info("\t\tArmor [0x{:x}: {}] was marked revealing for {}.", armor->GetFormID(), armorID, keyPair.second);
          rr++;
        } else {
          ia++;
        }
        continue;
      }
      if (inis->IsExtraRevealing(modName)) {
        if (armor->HasPartOf(Util::bodySlot)) {
          armor->AddKeyword(ut->Key(Util::kyRevealing));
          SKSE::log::info("\t\tArmor [0x{:x}: {}] was marked revealing.", armor->GetFormID(), armorID);
          ar++;
        } else {
          ia++;
        }
        continue;
      }
    } else {
      armor->RemoveKeywords(armorKeys);
      armor->AddKeyword(ut->Key(Util::kyIgnored));
      cc++;
      if (armor->HasPartOf(Util::bodySlot)) continue;
      if (modName != "" && inis->IsUnhandled(modName)) potentialSlot52Mods.insert(modName);
      SKSE::log::info("\t\tThe armor [0x{:x}: {}] would cover genitals and would have a conflict with non-revealing chest armor pieces!", armor->GetFormID(), armorID);
      continue;
    }
    if (armor->HasPartOf(Util::bodySlot) && modName != "") potentialMods.insert(std::string{modName});
    if (armor->HasPartOf(Util::bodySlot)) {
      potentialArmor.insert({modName, armor});
    } else {
      ia++;
    }
  }
  for (auto entry = potentialSlot52Mods.begin(); entry != potentialSlot52Mods.end();)
    potentialMods.find(*entry) == potentialMods.end() ? entry = potentialSlot52Mods.erase(entry) : ++entry;
  for (auto& modName : potentialSlot52Mods) inis->HandleModWithSlot52(modName, base->GetBoolSetting(Util::bsRevealSlot52Mods));
  for (auto& armorPair : potentialArmor) {
    if (inis->IsExtraRevealing(armorPair.first)) {
      armorPair.second->AddKeyword(ut->Key(Util::kyRevealing));
      ar++;
    } else {
      armorPair.second->AddKeyword(ut->Key(Util::kyCovering));
      ac++;
    }
  }
  SKSE::log::info("\tProcessed [{}] armor pieces:", armorList.size());
  if (pa > 0) SKSE::log::warn("\t\t[{}]: were problematic!", pa);
  if (hk > 0) SKSE::log::info("\t\t[{}]: were already marked with TNG keywords.", hk);
  if (cc > 0) SKSE::log::info("\t\t[{}]: are covering due to ini-files or having slot 52.", cc);
  if (rr > 0) SKSE::log::info("\t\t[{}]: are marked revealing.", rr);
  if (ac > 0) SKSE::log::info("\t\t[{}]: were recognized to be covering.", ac);
  if (ar > 0) SKSE::log::info("\t\t[{}]: were recognized to be revealing.", ar);
  if (ia > 0) SKSE::log::info("\t\tThe rest [{}] are not relevant and are ignored!", ia);
  inis->CleanIniLists();
}

void Core::RevisitRevealingArmor() {
  auto& armorList = ut->SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  std::vector<RE::BGSKeyword*> armorKeys = ut->Keys(Util::kyRevealingF, Util::kyRevealing);
  armorKeys.push_back(ut->Key(Util::kyIgnored));
  for (const auto& armor : armorList) {
    if (!armor) continue;
    auto modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (modName != "" && (armor->HasKeyword(ut->Key(Util::kyCovering)) || armor->HasKeyword(ut->Key(Util::kyRevealing)))) {
      armor->RemoveKeywords(armorKeys);
      potentialArmor.insert({modName, armor});
    }
  }
  for (const auto& armorPair : potentialArmor) armorPair.second->AddKeyword(ut->Key(inis->IsExtraRevealing(armorPair.first) ? Util::kyRevealing : Util::kyCovering));
}

bool Core::SwapRevealing(RE::Actor* actor, RE::TESObjectARMO* armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (armor->HasKeyword(ut->Key(Util::kyCovering))) {
    armor->RemoveKeyword(ut->Key(Util::kyCovering));
    if (!armor->HasPartOf(Util::bodySlot)) return true;
    auto revMode = npc->IsFemale() ? Util::kyRevealingF : Util::kyRevealingM;
    armor->AddKeyword(ut->Key(revMode));
    inis->SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(ut->Key(Util::kyRevealing))) {
    armor->RemoveKeyword(ut->Key(Util::kyRevealing));
    auto revMode = npc->IsFemale() ? Util::kyRevealingM : Util::kyRevealingF;
    armor->AddKeyword(ut->Key(revMode));
    inis->SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(ut->Key(Util::kyRevealingF))) {
    armor->RemoveKeyword(ut->Key(Util::kyRevealingF));
    auto revMode = npc->IsFemale() ? Util::kyCovering : Util::kyRevealing;
    armor->AddKeyword(ut->Key(revMode));
    inis->SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(ut->Key(Util::kyRevealingM))) {
    armor->RemoveKeyword(ut->Key(Util::kyRevealingM));
    auto revMode = npc->IsFemale() ? Util::kyRevealing : Util::kyCovering;
    armor->AddKeyword(ut->Key(revMode));
    inis->SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (!armor->HasKeywordInArray(ut->Keys(Util::kyRevealingF, Util::kyRevealing), false) || armor->HasKeyword(ut->Key(Util::kyIgnored))) {
    armor->RemoveKeyword(ut->Key(Util::kyIgnored));
    armor->AddKeyword(ut->Key(Util::kyCovering));
    inis->SaveRevealingArmor(armor, Util::kyCovering);
    return true;
  }
  return false;
}

void Core::SetBoolSetting(Util::eBoolSetting settingID, bool value) {
  base->SetBoolSetting(settingID, value);
  inis->SetBoolSetting(settingID, value);
}

void Core::SetAddonStatus(const bool isFemale, const int addnIdx, const bool status) {
  base->SetAddonStatus(isFemale, addnIdx, status);
  inis->SetAddonStatus(isFemale, addnIdx, status);
}

void Core::SetRgMult(const size_t rgChoice, const float mult, bool onlyMCM) {
  if (base->SetRgMult(rgChoice, mult, onlyMCM)) inis->SetRgMult(rgChoice, mult);
}
