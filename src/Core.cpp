#include <Base.h>
#include <Core.h>
#include <Inis.h>

Core* core = Core::GetSingleton();

void Core::GenitalizeRaces() {
  SKSE::log::info("Finding the genitals for relevant races...");
  const auto& allRaces = Util::SEDH()->GetFormArray<RE::TESRace>();
  int preprocessed = 0;
  int processed = 0;
  int ignored = 0;
  int ready = 0;

  std::set<std::string> validSkeletons;
  validSkeletons.emplace(Util::Race(Util::raceDefault)->skeletonModels[0].model.data());
  validSkeletons.emplace(Util::Race(Util::raceDefault)->skeletonModels[1].model.data());
  validSkeletons.emplace(Util::Race(Util::raceDefBeast)->skeletonModels[0].model.data());
  validSkeletons.emplace(Util::Race(Util::raceDefBeast)->skeletonModels[1].model.data());
  for (const auto& race : allRaces) {
    if (inis->IsRaceExcluded(race)) {
      SKSE::log::debug("\tThe race [{}: xx{:x}: {}] was ignored because an ini excludes it!", race->GetFile(0)->GetFilename(), race->GetLocalFormID(), race->GetFormEditorID());
      IgnoreRace(race, false);
      ignored++;
      continue;
    }
    if (race->HasKeyword(Util::Key(Util::kyIgnored))) {
      ignored++;
      continue;
    }
    if (race->HasKeyword(Util::Key(Util::kyReady))) {
      ready++;
      continue;
    }
    if (race->HasKeyword(Util::Key(Util::kyProcessed))) {
      processed++;
      continue;
    }
    if (race->HasKeyword(Util::Key(Util::kyPreProcessed))) {
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
      case Util::raceErr:
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
  if (auto& skin = race->skin; skin) skin->AddKeyword(Util::Key(Util::kyIgnored));
  race->RemoveKeywords(Util::Keys(Util::kyProcessed, Util::kyExcluded));
  race->AddKeyword(Util::Key(ready ? Util::kyReady : Util::kyIgnored));
}

Util::TNGRes Core::AddPotentialRace(RE::TESRace* race, const std::set<std::string>& validSkeletons) {
  try {
    for (auto raceInfo : hardCodedRaces)
      if (FormToLocView(race) == raceInfo) return Util::resOkRaceP;
    if (!race->HasKeyword(Util::Key(Util::kyManMer)) || race->HasKeyword(Util::Key(Util::kyCreature)) || race->IsChildRace()) return Util::raceErr;
    if (!race->skin) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since they do not have a skin! It was last modified by [{}].", race->GetFormID(), race->GetFormEditorID(),
                      race->GetFile() ? race->GetFile()->GetFilename() : "Unrecognized File");
      IgnoreRace(race, false);
      return Util::raceErr;
    }
    bool skinFound = false;
    for (const auto& aa : race->skin->armorAddons) {
      if (aa->HasPartOf(Util::cSlotBody) && aa->IsValidRace(race)) {
        skinFound = true;
        break;
      }
    }
    if (!skinFound) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since their skin cannot be recognized!  It was last modified by [{}].", race->GetFormID(),
                      race->GetFormEditorID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unrecognized File");
      IgnoreRace(race, false);
      return Util::raceErr;
    }
    if (race->HasPartOf(Util::cSlotGenital)) {
      auto ready = race->skin->HasPartOf(Util::cSlotGenital);
      SKSE::log::info("\tThe race [{}] is designed to be {} TNG. It was not modified.", race->GetFormEditorID(), ready ? "ready for" : "ignored by");
      IgnoreRace(race, ready);
      return ready ? Util::resOkRaceR : Util::raceErr;
    }
  } catch (const std::exception& er) {
    SKSE::log::warn("\tThe race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(),
                    race->GetFormEditorID(), er.what());
    const char* message = fmt::format("\tThe race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(),
                                      race->GetFormEditorID(), er.what())
                              .c_str();
    ShowSkyrimMessage(message);
    IgnoreRace(race, false);
    return Util::raceErr;
  }
  bool isValidSk =
      validSkeletons.find(race->skeletonModels[0].model.data()) != validSkeletons.end() && validSkeletons.find(race->skeletonModels[1].model.data()) != validSkeletons.end();
  return isValidSk ? Util::resOkRaceP : Util::resOkRacePP;
}

void Core::GenitalizeNPCSkins() {
  SKSE::log::info("Checking NPCs for custom skins.");
  std::map<std::pair<RE::TESObjectARMO*, RE::TESRace*>, std::set<RE::TESNPC*>> skinsToPatch{};
  std::map<std::string_view, size_t> customSkinMods{};
  std::map<RE::TESRace*, size_t> raceNPCCount;
  auto& allNPCs = Util::SEDH()->GetFormArray<RE::TESNPC>();
  size_t sizeCount[Util::cSizeCategories]{0};
  for (const auto& npc : allNPCs) {
    if (inis->IsNPCExcluded(npc)) {
      npc->AddKeyword(Util::Key(Util::kyExcluded));
      continue;
    }
    const auto race = npc->race;
    if (!race) {
      SKSE::log::warn("\t\tThe NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", npc->GetFormID(), npc->GetFormEditorID());
      continue;
    }
    if (!race->HasKeyword(Util::Key(Util::kyProcessed)) && !race->HasKeyword(Util::Key(Util::kyPreProcessed))) continue;
    raceNPCCount[npc->race]++;
    if (npc->IsFemale()) continue;
    sizeCount[npc->formID % Util::cSizeCategories]++;
    const auto skin = npc->skin;
    if (!skin) continue;
    if (skin->HasKeyword(Util::Key(Util::kyIgnored))) continue;
    if (!race->HasKeyword(Util::Key(Util::kyProcessed))) continue;
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

Util::TNGRes Core::CanModifyNPC(RE::TESNPC* npc) {
  auto res = base->CanModifyNPC(npc);
  if (res < 0) return res;
  return (inis->IsNPCExcluded(npc)) ? Util::npcErr : res;
}

Util::TNGRes Core::SetActorSize(RE::Actor* actor, int genSize) {
  auto res = base->SetActorSize(actor, genSize);
  if (res == Util::resOkSizable && !actor->GetActorBase()->IsPlayer()) inis->SaveNPCSize(actor->GetActorBase(), genSize);
  return res;
}

Util::TNGRes Core::SetNPCAddon(RE::TESNPC* npc, const int addnIdx, const bool isUser, const bool shouldSave) {
  if (!npc->race || !npc->race->HasKeyword(Util::Key(Util::kyProcessed))) return Util::raceErr;
  if (inis->IsNPCExcluded(npc)) return Util::npcErr;
  auto res = base->SetNPCAddon(npc, addnIdx, isUser);
  if (res < 0) return res;
  if (!npc->IsPlayer() && shouldSave) inis->SaveNPCAddon(npc, addnIdx);
  return res;
}

void Core::UpdateFormLists(RE::Actor* actor, RE::TESNPC* npc) {
  if (npc->IsFemale()) {
    if (npc->HasKeyword(Util::Key(Util::kyGentlewoman)) && !Util::TngFml(Util::flmGentleWomen)->HasForm(actor)) {
      Util::TngFml(Util::flmGentleWomen)->AddForm(actor);
    } else if (!npc->HasKeyword(Util::Key(Util::kyGentlewoman)) && Util::TngFml(Util::flmGentleWomen)->HasForm(actor)) {
      for (RE::BSTArray<RE::TESForm*>::const_iterator it = Util::TngFml(Util::flmGentleWomen)->forms.begin(); it < Util::TngFml(Util::flmGentleWomen)->forms.end(); it++) {
        if ((*it)->As<RE::Actor>() == actor) Util::TngFml(Util::flmGentleWomen)->forms.erase(it);
      }
    }
  }
  if (!npc->IsFemale()) {
    if (npc->HasKeyword(Util::Key(Util::kyExcluded)) && !Util::TngFml(Util::flmNonGentleMen)->HasForm(actor)) {
      Util::TngFml(Util::flmNonGentleMen)->AddForm(actor);
    } else if (!npc->HasKeyword(Util::Key(Util::kyExcluded)) && Util::TngFml(Util::flmNonGentleMen)->HasForm(actor)) {
      for (RE::BSTArray<RE::TESForm*>::const_iterator it = Util::TngFml(Util::flmNonGentleMen)->forms.begin(); it < Util::TngFml(Util::flmNonGentleMen)->forms.end(); it++) {
        if ((*it)->As<RE::Actor>() == actor) Util::TngFml(Util::flmNonGentleMen)->forms.erase(it);
      }
    }
  }
}

std::vector<RE::TESObjectARMO*> Core::GetActorWornArmor(RE::Actor* actor) {
  std::vector<RE::TESObjectARMO*> res{};
  res.clear();
  if (!actor) return res;
  auto inv = actor->GetInventory([=](RE::TESBoundObject& a_object) { return a_object.IsArmor(); });
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count > 0 && entry && entry->IsWorn() && FormToLocView(item) != Util::cCover) {
      res.push_back(item->As<RE::TESObjectARMO>());
    }
  }
  return res;
}

RE::TESObjectARMO* Core::FixSkin(RE::TESObjectARMO* skin, RE::TESRace* race, const char* const name) {
  skin->RemoveKeywords(Util::Keys(Util::kyRevealingF, Util::kyRevealing));
  skin->AddKeyword(Util::Key(Util::kyIgnored));
  if (!skin->HasPartOf(Util::cSlotBody)) {
    SKSE::log::info("\t\tThe skin [0x{:x}] used does not have a body part. TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->HasPartOf(Util::cSlotGenital)) {
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
  if (rgIdx < 0 || addonIdx == Util::pgErr) {
    SKSE::log::critical("\t\tSkin [xx{:x}] from file [{}] together with race [xx{:x}] from file [{}] caused a critical error!", skin->GetLocalFormID(),
                        skin->GetFile() ? skin->GetFile()->GetFilename() : "Unknown", race->GetLocalFormID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unknown");
    return nullptr;
  }
  if (addonIdx == Util::cNul) return skin;
  if (name) SKSE::log::info("\t\tThe skin [0x{:x}: {}] added as extra skin.", skin->GetFormID(), name);
  return base->GetSkinWithAddonForRg(rgIdx, skin, addonIdx, false);
}

void Core::CheckArmorPieces() {
  SKSE::log::info("Checking ARMO records...");
  auto& armorList = Util::SEDH()->GetFormArray<RE::TESObjectARMO>();
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
  std::vector<RE::BGSKeyword*> armorKeys = Util::Keys(Util::kyRevealingF, Util::kyRevealing);
  armorKeys.push_back(Util::Key(Util::kyIgnored));
  std::vector<RE::BGSKeyword*> relRaceKeys{Util::Key(Util::kyProcessed), Util::Key(Util::kyPreProcessed), Util::Key(Util::kyReady)};
  for (auto covRec : hardCodedCovering) {
    auto armor = Util::SEDH()->LookupForm<RE::TESObjectARMO>(covRec.first, covRec.second);
    if (!armor) continue;
    armor->RemoveKeywords(armorKeys);
    armor->AddKeyword(Util::Key(Util::kyCovering));
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
    if (armor->armorAddons.size() == 0) armor->AddKeyword(Util::Key(Util::kyIgnored));
    const auto armorID = (std::string(armor->GetName()).empty()) ? armor->GetFormEditorID() : armor->GetName();
    std::string modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (!armor->race) {
      if (!armor->HasKeyword(Util::Key(Util::kyIgnored)))
        SKSE::log::warn("\t\tThe armor [0x{:x}: {}] does not have a race! It won't be touched by Tng!", armor->GetFormID(), armorID);
      armor->AddKeyword(Util::Key(Util::kyIgnored));
      ia++;
      continue;
    }
    if (!armor->race->HasKeywordInArray(relRaceKeys, false) && armor->race != Util::Race(Util::raceDefault)) continue;
    if (inis->IsSkin(armor, modName)) {
      SKSE::log::info("\t\tThe record [0x{:x}: {}] was marked as a skin.", armor->GetFormID(), armorID);
      armor->AddKeyword(Util::Key(Util::kyIgnored));
      ia++;
      continue;
    }
    bool has52 = armor->HasPartOf(Util::cSlotGenital);
    for (auto& aa : armor->armorAddons) {
      if (has52) break;
      has52 = aa->HasPartOf(Util::cSlotGenital);
    }
    if (!has52) {
      if (inis->IsRTCovering(armor, modName)) {
        armor->AddKeyword(Util::Key(Util::kyCovering));
        SKSE::log::info("\t\tThe armor [[0x{:x}: {}] was marked covering since it was previously marked covering during gameplay.", armor->GetFormID(), armorID);
        ac++;
        continue;
      }
      auto keyPair = [&]() -> std::pair<RE::BGSKeyword*, std::string> {
        switch (inis->IsRTRevealing(armor, modName)) {
          case Util::kyRevealing:
            return {Util::Key(Util::kyRevealing), "all"};
          case Util::kyRevealingF:
            return {Util::Key(Util::kyRevealingF), "women"};
          case Util::kyRevealingM:
            return {Util::Key(Util::kyRevealingM), "men"};
          default:
            return {nullptr, ""};
        }
      }();
      if (keyPair.first) {
        if (armor->HasPartOf(Util::cSlotBody)) {
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
        armor->AddKeyword(Util::Key(Util::kyCovering));
        SKSE::log::info("\t\tThe armor [0x{:x}: {}] was marked covering.", armor->GetFormID(), armorID);
        cc++;
        continue;
      }
      keyPair = [&]() -> std::pair<RE::BGSKeyword*, std::string> {
        switch (inis->IsRevealing(armor, modName)) {
          case Util::kyRevealing:
            return {Util::Key(Util::kyRevealing), "all"};
          case Util::kyRevealingF:
            return {Util::Key(Util::kyRevealingF), "women"};
          case Util::kyRevealingM:
            return {Util::Key(Util::kyRevealingM), "men"};
          default:
            return {nullptr, ""};
        }
      }();
      if (keyPair.first) {
        if (armor->HasPartOf(Util::cSlotBody)) {
          armor->AddKeyword(keyPair.first);
          SKSE::log::info("\t\tArmor [0x{:x}: {}] was marked revealing for {}.", armor->GetFormID(), armorID, keyPair.second);
          rr++;
        } else {
          ia++;
        }
        continue;
      }
      if (inis->IsExtraRevealing(modName)) {
        if (armor->HasPartOf(Util::cSlotBody)) {
          armor->AddKeyword(Util::Key(Util::kyRevealing));
          SKSE::log::info("\t\tArmor [0x{:x}: {}] was marked revealing.", armor->GetFormID(), armorID);
          ar++;
        } else {
          ia++;
        }
        continue;
      }
    } else {
      armor->RemoveKeywords(armorKeys);
      armor->AddKeyword(Util::Key(Util::kyCovering));
      cc++;
      if (armor->HasPartOf(Util::cSlotBody)) continue;
      if (modName != "" && inis->IsUnhandled(modName)) potentialSlot52Mods.insert(modName);
      SKSE::log::info("\t\tThe armor [0x{:x}: {}] would cover genitals and would have a conflict with non-revealing chest armor pieces!", armor->GetFormID(), armorID);
      continue;
    }
    if (armor->HasPartOf(Util::cSlotBody) && modName != "") potentialMods.insert(std::string{modName});
    if (armor->HasPartOf(Util::cSlotBody)) {
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
      armorPair.second->AddKeyword(Util::Key(Util::kyRevealing));
      ar++;
    } else {
      armorPair.second->AddKeyword(Util::Key(Util::kyCovering));
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
  auto& armorList = Util::SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  std::vector<RE::BGSKeyword*> armorKeys = Util::Keys(Util::kyRevealingF, Util::kyRevealing);
  armorKeys.push_back(Util::Key(Util::kyIgnored));
  for (const auto& armor : armorList) {
    if (!armor) continue;
    auto modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (modName != "" && (armor->HasKeyword(Util::Key(Util::kyCovering)) || armor->HasKeyword(Util::Key(Util::kyRevealing)))) {
      armor->RemoveKeywords(armorKeys);
      potentialArmor.insert({modName, armor});
    }
  }
  for (const auto& armorPair : potentialArmor) armorPair.second->AddKeyword(Util::Key(inis->IsExtraRevealing(armorPair.first) ? Util::kyRevealing : Util::kyCovering));
}

bool Core::SwapRevealing(RE::Actor* actor, RE::TESObjectARMO* armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (armor->HasKeyword(Util::Key(Util::kyCovering))) {
    armor->RemoveKeyword(Util::Key(Util::kyCovering));
    if (!armor->HasPartOf(Util::cSlotBody)) return true;
    auto revMode = npc->IsFemale() ? Util::kyRevealingF : Util::kyRevealingM;
    armor->AddKeyword(Util::Key(revMode));
    inis->SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(Util::Key(Util::kyRevealing))) {
    armor->RemoveKeyword(Util::Key(Util::kyRevealing));
    auto revMode = npc->IsFemale() ? Util::kyRevealingM : Util::kyRevealingF;
    armor->AddKeyword(Util::Key(revMode));
    inis->SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(Util::Key(Util::kyRevealingF))) {
    armor->RemoveKeyword(Util::Key(Util::kyRevealingF));
    auto revMode = npc->IsFemale() ? Util::kyCovering : Util::kyRevealing;
    armor->AddKeyword(Util::Key(revMode));
    inis->SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(Util::Key(Util::kyRevealingM))) {
    armor->RemoveKeyword(Util::Key(Util::kyRevealingM));
    auto revMode = npc->IsFemale() ? Util::kyRevealing : Util::kyCovering;
    armor->AddKeyword(Util::Key(revMode));
    inis->SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (!armor->HasKeywordInArray(Util::Keys(Util::kyRevealingF, Util::kyRevealing), false) || armor->HasKeyword(Util::Key(Util::kyIgnored))) {
    armor->RemoveKeyword(Util::Key(Util::kyIgnored));
    armor->AddKeyword(Util::Key(Util::kyCovering));
    inis->SaveRevealingArmor(armor, Util::kyCovering);
    return true;
  }
  return false;
}

void Core::SetBoolSetting(Util::BoolSetting settingID, bool value) {
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
