#include <Base.h>
#include <Core.h>
#include <Inis.h>

void Core::GenitalizeRaces() {
  SKSE::log::info("Finding the genitals for relevant races...");
  const auto& allRaces = Tng::SEDH()->GetFormArray<RE::TESRace>();
  int preprocessed = 0;
  int processed = 0;
  int ignored = 0;
  int ready = 0;
  std::set<std::string> validSkeletons;
  validSkeletons.emplace(Tng::Race(Tng::raceDefault)->skeletonModels[0].model.data());
  validSkeletons.emplace(Tng::Race(Tng::raceDefault)->skeletonModels[1].model.data());
  validSkeletons.emplace(Tng::Race(Tng::raceDefBeast)->skeletonModels[0].model.data());
  validSkeletons.emplace(Tng::Race(Tng::raceDefBeast)->skeletonModels[1].model.data());

  for (const auto& race : allRaces) {
    if (Inis::IsRaceExcluded(race)) {
      SKSE::log::debug("\tThe race [{}: xx{:x}: {}] was ignored because an ini excludes it!", race->GetFile(0)->GetFilename(), race->GetLocalFormID(), race->GetFormEditorID());
      IgnoreRace(race, false);
      ignored++;
      continue;
    }
    if (race->HasKeyword(Tng::Key(Tng::kyIgnored))) {
      ignored++;
      continue;
    }
    if (race->HasKeyword(Tng::Key(Tng::kyReady))) {
      ready++;
      continue;
    }
    if (race->HasKeyword(Tng::Key(Tng::kyProcessed))) {
      processed++;
      continue;
    }
    if (race->HasKeyword(Tng::Key(Tng::kyPreProcessed))) {
      preprocessed++;
      continue;
    }
    switch (AddPotentialRace(race, validSkeletons)) {
      case Tng::resOkRacePP:
        Base::AddRace(race, false);
        preprocessed++;
        break;
      case Tng::resOkRaceP:
        Base::AddRace(race, true);
        processed++;
        break;
      case Tng::resOkRaceR:
        ready++;
        break;
      case Tng::raceErr:
        ignored++;
        break;
    }
  }
  Base::UpdateRgSkins();
  Inis::LoadRgInfo();
  SKSE::log::info("\tRecognized assigned genitalia to [{}] races. Preprocessed [{}] races, found [{}] races to be ready and ignored [{}] races.", processed, preprocessed, ready, ignored);
}

bool Core::SetRgAddon(const size_t rgChoice, const int addnIdx, const bool onlyMCM) {
  auto res = Base::SetRgAddon(rgChoice, addnIdx, onlyMCM);
  if (!res) return res;
  Inis::SaveRgAddon(rgChoice, addnIdx);
  return true;
}

void Core::IgnoreRace(RE::TESRace* race, bool ready) {
  if (!race) return;
  if (auto& skin = race->skin; skin) skin->AddKeyword(Tng::Key(Tng::kyIgnored));
  race->RemoveKeywords(Tng::Keys(Tng::kyProcessed, Tng::kyExcluded));
  race->AddKeyword(Tng::Key(ready ? Tng::kyReady : Tng::kyIgnored));
}

Tng::TNGRes Core::AddPotentialRace(RE::TESRace* race, const std::set<std::string>& validSkeletons) {
  try {
    for (auto raceInfo : hardCodedRaces)
      if (FormToLocView(race) == raceInfo) return Tng::resOkRaceP;
    if (!race->HasKeyword(Tng::Key(Tng::kyManMer)) || race->HasKeyword(Tng::Key(Tng::kyCreature)) || race->IsChildRace()) return Tng::raceErr;
    if (!race->skin) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since they do not have a skin! It was last modified by [{}].", race->GetFormID(), race->GetFormEditorID(),
                      race->GetFile() ? race->GetFile()->GetFilename() : "Unrecognized File");
      IgnoreRace(race, false);
      return Tng::raceErr;
    }
    bool skinFound = false;
    for (const auto& aa : race->skin->armorAddons) {
      if (aa->HasPartOf(Tng::cSlotBody) && aa->IsValidRace(race)) {
        skinFound = true;
        break;
      }
    }
    if (!skinFound) {
      SKSE::log::warn("\tThe race [0x{:x}: {}] cannot have any genitals since their skin cannot be recognized!  It was last modified by [{}].", race->GetFormID(), race->GetFormEditorID(),
                      race->GetFile() ? race->GetFile()->GetFilename() : "Unrecognized File");
      IgnoreRace(race, false);
      return Tng::raceErr;
    }
    if (race->HasPartOf(Tng::cSlotGenital)) {
      auto ready = race->skin->HasPartOf(Tng::cSlotGenital);
      SKSE::log::info("\tThe race [{}] is designed to be {} TNG. It was not modified.", race->GetFormEditorID(), ready ? "ready for" : "ignored by");
      IgnoreRace(race, ready);
      return ready ? Tng::resOkRaceR : Tng::raceErr;
    }
  } catch (const std::exception& er) {
    SKSE::log::warn("\tThe race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(), race->GetFormEditorID(), er.what());
    const char* message =
        fmt::format("\tThe race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(), race->GetFormEditorID(), er.what()).c_str();
    ShowSkyrimMessage(message);
    IgnoreRace(race, false);
    return Tng::raceErr;
  }
  bool isValidSk = validSkeletons.find(race->skeletonModels[0].model.data()) != validSkeletons.end() && validSkeletons.find(race->skeletonModels[1].model.data()) != validSkeletons.end();
  return isValidSk ? Tng::resOkRaceP : Tng::resOkRacePP;
}

void Core::GenitalizeNPCSkins() {
  SKSE::log::info("Checking NPCs for custom skins.");
  std::map<std::pair<RE::TESObjectARMO*, RE::TESRace*>, std::set<RE::TESNPC*>> skinsToPatch{};
  std::map<std::string_view, size_t> customSkinMods{};
  std::map<RE::TESRace*, size_t> raceNPCCount;
  auto& allNPCs = Tng::SEDH()->GetFormArray<RE::TESNPC>();
  size_t sizeCount[Tng::cSizeCategories]{0};
  for (const auto& npc : allNPCs) {
    if (Inis::IsNPCExcluded(npc)) {
      npc->AddKeyword(Tng::Key(Tng::kyExcluded));
      continue;
    }
    const auto race = npc->race;
    if (!race) {
      SKSE::log::warn("\t\tThe NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", npc->GetFormID(), npc->GetFormEditorID());
      continue;
    }
    if (!race->HasKeyword(Tng::Key(Tng::kyProcessed)) && !race->HasKeyword(Tng::Key(Tng::kyPreProcessed))) continue;
    raceNPCCount[npc->race]++;
    if (npc->IsFemale()) continue;
    sizeCount[npc->formID % Tng::cSizeCategories]++;
    const auto skin = npc->skin;
    if (!skin) continue;
    if (skin->HasKeyword(Tng::Key(Tng::kyIgnored))) continue;
    if (!race->HasKeyword(Tng::Key(Tng::kyProcessed))) continue;
    skinsToPatch[{skin, race}].insert(npc);
  }
  for (auto& racePair : raceNPCCount) {
    if (racePair.second < 5) continue;
    Base::TryUnhideRace(racePair.first);
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
  Inis::LoadNpcInfo();
  SKSE::log::debug("TNG disributed the sizes (from smallest to largest) to [{}] npcs!", fmt::join(sizeCount, ", "));
  Base::ReportHiddenRgs();
}

Tng::TNGRes Core::CanModifyNPC(RE::TESNPC* npc) {
  auto res = Base::CanModifyNPC(npc);
  if (res < 0) return res;
  return (Inis::IsNPCExcluded(npc)) ? Tng::npcErr : res;
}

Tng::TNGRes Core::SetActorSize(RE::Actor* actor, int genSize) {
  auto res = Base::SetActorSizeCat(actor, genSize);
  if (res == Tng::resOkSizable && !actor->GetActorBase()->IsPlayer()) Inis::SaveNPCSize(actor->GetActorBase(), genSize);
  return res;
}

Tng::TNGRes Core::SetNPCAddon(RE::TESNPC* npc, const int addnIdx, const bool isUser, const bool shouldSave) {
  if (!npc->race || !npc->race->HasKeyword(Tng::Key(Tng::kyProcessed))) return Tng::raceErr;
  if (Inis::IsNPCExcluded(npc)) return Tng::npcErr;
  auto res = Base::SetNPCAddon(npc, addnIdx, isUser);
  if (res < 0) return res;
  if (!npc->IsPlayer() && shouldSave) Inis::SaveNPCAddon(npc, addnIdx);
  return res;
}

std::vector<RE::TESObjectARMO*> Core::GetActorWornArmor(RE::Actor* actor) {
  std::vector<RE::TESObjectARMO*> res{};
  res.clear();
  if (!actor) return res;
  auto inv = actor->GetInventory([=](RE::TESBoundObject& a_object) { return a_object.IsArmor(); });
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count > 0 && entry && entry->IsWorn() && FormToLocView(item) != Tng::cCover) {
      res.push_back(item->As<RE::TESObjectARMO>());
    }
  }
  return res;
}

RE::TESObjectARMO* Core::FixSkin(RE::TESObjectARMO* skin, RE::TESRace* race, const char* const name) {
  skin->RemoveKeywords(Tng::Keys(Tng::kyRevealingF, Tng::kyRevealing));
  skin->AddKeyword(Tng::Key(Tng::kyIgnored));
  if (!skin->HasPartOf(Tng::cSlotBody)) {
    SKSE::log::info("\t\tThe skin [0x{:x}] used does not have a body part. TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->HasPartOf(Tng::cSlotGenital)) {
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
  auto rgIdx = Base::GetRaceRgIdx(race);
  auto addonIdx = Base::GetRgAddon(race);
  if (rgIdx < 0 || addonIdx == Tng::pgErr) {
    SKSE::log::critical("\t\tSkin [xx{:x}] from file [{}] together with race [xx{:x}] from file [{}] caused a critical error!", skin->GetLocalFormID(),
                        skin->GetFile() ? skin->GetFile()->GetFilename() : "Unknown", race->GetLocalFormID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unknown");
    return nullptr;
  }
  if (addonIdx == Tng::cNul) return skin;
  if (name) SKSE::log::info("\t\tThe skin [0x{:x}: {}] added as extra skin.", skin->GetFormID(), name);
  return Base::GetSkinWithAddonForRg(rgIdx, skin, addonIdx, false);
}

void Core::CheckArmorPieces() {
  SKSE::log::info("Checking ARMO records...");
  auto& armorlist = Tng::SEDH()->GetFormArray<RE::TESObjectARMO>();
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
  std::vector<RE::BGSKeyword*> armorKeys = Tng::Keys(Tng::kyRevealingF, Tng::kyRevealing);
  armorKeys.push_back(Tng::Key(Tng::kyIgnored));
  std::vector<RE::BGSKeyword*> relRaceKeys{Tng::Key(Tng::kyProcessed), Tng::Key(Tng::kyPreProcessed), Tng::Key(Tng::kyReady)};
  for (auto covRec : hardCodedCovering) {
    auto armor = Tng::SEDH()->LookupForm<RE::TESObjectARMO>(covRec.first, covRec.second);
    if (!armor) continue;
    armor->RemoveKeywords(armorKeys);
    armor->AddKeyword(Tng::Key(Tng::kyCovering));
  }
  for (const auto& armor : armorlist) {
    if (!armor) {
      pa++;
      continue;
    }
    if (armor->HasKeywordInArray(armorKeys, false)) {
      hk++;
      continue;
    }
    if (armor->armorAddons.size() == 0) armor->AddKeyword(Tng::Key(Tng::kyIgnored));
    const auto armorID = (std::string(armor->GetName()).empty()) ? armor->GetFormEditorID() : armor->GetName();
    std::string modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (!armor->race) {
      if (!armor->HasKeyword(Tng::Key(Tng::kyIgnored))) SKSE::log::warn("\t\tThe armor [0x{:x}: {}] does not have a race! It won't be touched by Tng!", armor->GetFormID(), armorID);
      armor->AddKeyword(Tng::Key(Tng::kyIgnored));
      ia++;
      continue;
    }
    if (!armor->race->HasKeywordInArray(relRaceKeys, false) && armor->race != Tng::Race(Tng::raceDefault)) continue;
    if (Inis::IsSkin(armor, modName)) {
      SKSE::log::info("\t\tThe record [0x{:x}: {}] was marked as a skin.", armor->GetFormID(), armorID);
      armor->AddKeyword(Tng::Key(Tng::kyIgnored));
      ia++;
      continue;
    }
    bool has52 = armor->HasPartOf(Tng::cSlotGenital);
    for (auto& aa : armor->armorAddons) {
      if (has52) break;
      has52 = aa->HasPartOf(Tng::cSlotGenital);
    }
    if (!has52) {
      if (Inis::IsRTCovering(armor, modName)) {
        armor->AddKeyword(Tng::Key(Tng::kyCovering));
        SKSE::log::info("\t\tThe armor [[0x{:x}: {}] was marked covering since it was previously marked covering during gameplay.", armor->GetFormID(), armorID);
        ac++;
        continue;
      }
      auto keyPair = [&]() -> std::pair<RE::BGSKeyword*, std::string> {
        switch (Inis::IsRTRevealing(armor, modName)) {
          case Tng::kyRevealing:
            return {Tng::Key(Tng::kyRevealing), "all"};
          case Tng::kyRevealingF:
            return {Tng::Key(Tng::kyRevealingF), "women"};
          case Tng::kyRevealingM:
            return {Tng::Key(Tng::kyRevealingM), "men"};
          default:
            return {nullptr, ""};
        }
      }();
      if (keyPair.first) {
        if (armor->HasPartOf(Tng::cSlotBody)) {
          armor->AddKeyword(keyPair.first);
          SKSE::log::info("\t\tThe armor [[0x{:x}: {}] was marked revealing for {} since it was previously marked revealing during gameplay.", armor->GetFormID(), armorID, keyPair.second);
          ar++;
        } else {
          ia++;
        }
        continue;
      }
      if (Inis::IsCovering(armor, modName)) {
        armor->AddKeyword(Tng::Key(Tng::kyCovering));
        SKSE::log::info("\t\tThe armor [0x{:x}: {}] was marked covering.", armor->GetFormID(), armorID);
        cc++;
        continue;
      }
      keyPair = [&]() -> std::pair<RE::BGSKeyword*, std::string> {
        switch (Inis::IsRevealing(armor, modName)) {
          case Tng::kyRevealing:
            return {Tng::Key(Tng::kyRevealing), "all"};
          case Tng::kyRevealingF:
            return {Tng::Key(Tng::kyRevealingF), "women"};
          case Tng::kyRevealingM:
            return {Tng::Key(Tng::kyRevealingM), "men"};
          default:
            return {nullptr, ""};
        }
      }();
      if (keyPair.first) {
        if (armor->HasPartOf(Tng::cSlotBody)) {
          armor->AddKeyword(keyPair.first);
          SKSE::log::info("\t\tArmor [0x{:x}: {}] was marked revealing for {}.", armor->GetFormID(), armorID, keyPair.second);
          rr++;
        } else {
          ia++;
        }
        continue;
      }
      if (Inis::IsExtraRevealing(modName)) {
        if (armor->HasPartOf(Tng::cSlotBody)) {
          armor->AddKeyword(Tng::Key(Tng::kyRevealing));
          SKSE::log::info("\t\tArmor [0x{:x}: {}] was marked revealing.", armor->GetFormID(), armorID);
          ar++;
        } else {
          ia++;
        }
        continue;
      }
    } else {
      armor->RemoveKeywords(armorKeys);
      armor->AddKeyword(Tng::Key(Tng::kyCovering));
      cc++;
      if (armor->HasPartOf(Tng::cSlotBody)) continue;
      if (modName != "" && Inis::IsUnhandled(modName)) potentialSlot52Mods.insert(modName);
      SKSE::log::info("\t\tThe armor [0x{:x}: {}] would cover genitals and would have a conflict with non-revealing chest armor pieces!", armor->GetFormID(), armorID);
      continue;
    }
    if (armor->HasPartOf(Tng::cSlotBody) && modName != "") potentialMods.insert(std::string{modName});
    if (armor->HasPartOf(Tng::cSlotBody)) {
      potentialArmor.insert({modName, armor});
    } else {
      ia++;
    }
  }
  for (auto entry = potentialSlot52Mods.begin(); entry != potentialSlot52Mods.end();) potentialMods.find(*entry) == potentialMods.end() ? entry = potentialSlot52Mods.erase(entry) : ++entry;
  for (auto& modName : potentialSlot52Mods) Inis::HandleModWithSlot52(modName, Base::GetBoolSetting(Tng::bsRevealSlot52Mods));
  for (auto& armorPair : potentialArmor) {
    if (Inis::IsExtraRevealing(armorPair.first)) {
      armorPair.second->AddKeyword(Tng::Key(Tng::kyRevealing));
      ar++;
    } else {
      armorPair.second->AddKeyword(Tng::Key(Tng::kyCovering));
      ac++;
    }
  }
  SKSE::log::info("\tProcessed [{}] armor pieces:", armorlist.size());
  if (pa > 0) SKSE::log::warn("\t\t[{}]: were problematic!", pa);
  if (hk > 0) SKSE::log::info("\t\t[{}]: were already marked with TNG keywords.", hk);
  if (cc > 0) SKSE::log::info("\t\t[{}]: are covering due to ini-files or having slot 52.", cc);
  if (rr > 0) SKSE::log::info("\t\t[{}]: are marked revealing.", rr);
  if (ac > 0) SKSE::log::info("\t\t[{}]: were recognized to be covering.", ac);
  if (ar > 0) SKSE::log::info("\t\t[{}]: were recognized to be revealing.", ar);
  if (ia > 0) SKSE::log::info("\t\tThe rest [{}] are not relevant and are ignored!", ia);
  Inis::CleanIniLists();
}

void Core::RevisitRevealingArmor() {
  auto& armorlist = Tng::SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  std::vector<RE::BGSKeyword*> armorKeys = Tng::Keys(Tng::kyRevealingF, Tng::kyRevealing);
  armorKeys.push_back(Tng::Key(Tng::kyIgnored));
  for (const auto& armor : armorlist) {
    if (!armor) continue;
    auto modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (modName != "" && (armor->HasKeyword(Tng::Key(Tng::kyCovering)) || armor->HasKeyword(Tng::Key(Tng::kyRevealing)))) {
      armor->RemoveKeywords(armorKeys);
      potentialArmor.insert({modName, armor});
    }
  }
  for (const auto& armorPair : potentialArmor) armorPair.second->AddKeyword(Tng::Key(Inis::IsExtraRevealing(armorPair.first) ? Tng::kyRevealing : Tng::kyCovering));
}

bool Core::SwapRevealing(RE::Actor* actor, RE::TESObjectARMO* armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (armor->HasKeyword(Tng::Key(Tng::kyCovering))) {
    armor->RemoveKeyword(Tng::Key(Tng::kyCovering));
    if (!armor->HasPartOf(Tng::cSlotBody)) return true;
    auto revMode = npc->IsFemale() ? Tng::kyRevealingF : Tng::kyRevealingM;
    armor->AddKeyword(Tng::Key(revMode));
    Inis::SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(Tng::Key(Tng::kyRevealing))) {
    armor->RemoveKeyword(Tng::Key(Tng::kyRevealing));
    auto revMode = npc->IsFemale() ? Tng::kyRevealingM : Tng::kyRevealingF;
    armor->AddKeyword(Tng::Key(revMode));
    Inis::SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(Tng::Key(Tng::kyRevealingF))) {
    armor->RemoveKeyword(Tng::Key(Tng::kyRevealingF));
    auto revMode = npc->IsFemale() ? Tng::kyCovering : Tng::kyRevealing;
    armor->AddKeyword(Tng::Key(revMode));
    Inis::SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (armor->HasKeyword(Tng::Key(Tng::kyRevealingM))) {
    armor->RemoveKeyword(Tng::Key(Tng::kyRevealingM));
    auto revMode = npc->IsFemale() ? Tng::kyRevealing : Tng::kyCovering;
    armor->AddKeyword(Tng::Key(revMode));
    Inis::SaveRevealingArmor(armor, revMode);
    return true;
  }
  if (!armor->HasKeywordInArray(Tng::Keys(Tng::kyRevealingF, Tng::kyRevealing), false) || armor->HasKeyword(Tng::Key(Tng::kyIgnored))) {
    armor->RemoveKeyword(Tng::Key(Tng::kyIgnored));
    armor->AddKeyword(Tng::Key(Tng::kyCovering));
    Inis::SaveRevealingArmor(armor, Tng::kyCovering);
    return true;
  }
  return false;
}

void Core::SetBoolSetting(Tng::BoolSetting settingID, bool value) {
  Base::SetBoolSetting(settingID, value);
  Inis::SetBoolSetting(settingID, value);
}

void Core::SetAddonStatus(const bool isFemale, const int addnIdx, const bool status) {
  Base::SetAddonStatus(isFemale, addnIdx, status);
  Inis::SetAddonStatus(isFemale, addnIdx, status);
}

void Core::SetRgMult(const size_t rgChoice, const float mult, bool onlyMCM) {
  if (Base::SetRgMult(rgChoice, mult, onlyMCM)) Inis::SetRgMult(rgChoice, mult);
}
