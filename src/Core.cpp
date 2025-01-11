#include <Base.h>
#include <Core.h>
#include <Inis.h>

void Core::GenitalizeRaces() {
  Tng::logger::info("Finding the genitals for relevant races...");
  auto& lAllRacesArray = Tng::SEDH()->GetFormArray<RE::TESRace>();
  int processed = 0;
  int ignored = 0;
  int ready = 0;
  for (const auto& race : lAllRacesArray) {
    if (Inis::IsRaceExcluded(race)) {
      IgnoreRace(race);
      ignored++;
      continue;
    }
    if (race->HasKeyword(Tng::RaceKey(Tng::rkeyIgnore))) {
      ignored++;
      continue;
    }
    if (race->HasKeyword(Tng::RaceKey(Tng::rkeyReady))) {
      ready++;
      continue;
    }
    if (race->HasKeyword(Tng::RaceKey(Tng::rkeyProcessed))) {
      processed++;
      continue;
    }
    switch (AddRace(race)) {
      case Tng::resOkRaceP:
        processed++;
        break;
      case Tng::resOkRaceR:
        ready++;
        break;
      case Tng::raceErr:
        ignored++;
        break;
      default:
        break;
    }
  }
  Base::UpdateRgSkins();
  Tng::logger::info("Recognized assigned genitals to [{}] races, found [{}] races to be ready and ignored [{}] races.", processed, ready, ignored);
}

bool Core::SetRgAddn(const size_t rgChoice, const int addnIdx) {
  auto res = Base::SetRgAddn(rgChoice, addnIdx, true);
  if (!res) return res;
  Inis::SaveRgAddn(rgChoice, addnIdx);
  return true;
}

bool Core::IgnoreRace(RE::TESRace* race) {
  if (!race) return false;
  bool ready = false;
  auto& skin = race->skin;
  if (skin) {
    if (!skin->HasKeyword(Tng::ArmoKey(Tng::akeyGenSkin)) && !skin->HasKeyword(Tng::ArmoKey(Tng::akeyIgnored))) skin->AddKeyword(Tng::ArmoKey(Tng::akeyIgnored));
    for (const auto& aa : race->skin->armorAddons) {
      if (aa->HasPartOf(Tng::cSlotGenital) && aa->IsValidRace(race)) {
        ready = true;
        break;
      }
    }
  }
  race->RemoveKeywords(Tng::RaceKeys());
  race->AddKeyword(Tng::RaceKey(ready ? Tng::rkeyReady : Tng::rkeyIgnore));
  return ready;
}

bool Core::CheckRace(RE::TESRace* race) {
  try {
    if (!race->HasKeyword(Tng::RaceKey(Tng::rkeyManMer)) || race->HasKeyword(Tng::RaceKey(Tng::rkeyCreature)) || !race->HasPartOf(Tng::cSlotBody) || race->IsChildRace()) return false;
    if (Inis::IsRaceExcluded(race)) {
      Tng::logger::warn("The race [{}: 0x{:x}: {}] was ignored because an ini excludes it!", race->GetFile(0)->GetFilename(), race->GetFormID(), race->GetFormEditorID());
      return false;
    }
    if (!Inis::IsValidSkeleton(race->skeletonModels[0].model.data()) || !Inis::IsValidSkeleton(race->skeletonModels[1].model.data())) {
      Tng::logger::warn("The race [0x{:x}: {}] was ignored because it uses a custom skeleton!", race->GetFormID(), race->GetFormEditorID());
      IgnoreRace(race);
      return false;
    }
    if (!race->skin) {
      Tng::logger::warn("The race [0x{:x}: {}] cannot have any genitals since they do not have a skin!", race->GetFormID(), race->GetFormEditorID());
      IgnoreRace(race);
      return false;
    }
    RE::TESRace* lArmRace = race->armorParentRace ? race->armorParentRace : race;
    RE::TESObjectARMA* skinAA{nullptr};
    for (const auto& aa : race->skin->armorAddons) {
      std::set<RE::TESRace*> lAARaces{aa->race};
      lAARaces.insert(aa->additionalRaces.begin(), aa->additionalRaces.end());
      if (aa->HasPartOf(Tng::cSlotBody) && ((lAARaces.find(lArmRace) != lAARaces.end()) || (aa->race == Tng::Race(Tng::raceDefault)))) {
        skinAA = aa;
        break;
      }
    }
    if (!skinAA) {
      Tng::logger::warn("The race [0x{:x}: {}] cannot have any genitals since their skin cannot be recognized.", race->GetFormID(), race->GetFormEditorID());
      IgnoreRace(race);
      return false;
    }
    return true;
  } catch (const std::exception& er) {
    Tng::logger::warn("The race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(), race->GetFormEditorID(), er.what());
    const char* lMessage =
        fmt::format("The race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", race->GetFormID(), race->GetFormEditorID(), er.what()).c_str();
    ShowSkyrimMessage(lMessage);
    IgnoreRace(race);
    return false;
  }
}

Tng::TNGRes Core::AddRace(RE::TESRace* race) {
  if (!CheckRace(race)) return Tng::raceErr;
  if (race->HasPartOf(Tng::cSlotGenital) || race->skin->HasPartOf(Tng::cSlotGenital)) {
    Tng::logger::info("The race [{}] seems to be ready for TNG. It was not modified.", race->GetFormEditorID());
    return IgnoreRace(race) ? Tng::resOkRaceR : Tng::raceErr;
  }
  for (const auto aa : race->skin->armorAddons)
    if (aa->HasPartOf(Tng::cSlotGenital)) {
      Tng::logger::info("The race [{}] seems to be ready for TNG. It was not modified.", race->GetFormEditorID());
      return IgnoreRace(race) ? Tng::resOkRaceR : Tng::raceErr;
    }
  Base::AddRace(race);
  if (!race->HasKeyword(Tng::RaceKey(Tng::rkeyProcessed))) race->AddKeyword(Tng::RaceKey(Tng::rkeyProcessed));
  race->AddSlotToMask(Tng::cSlotGenital);
  auto skin = race->skin;
  if (!skin->HasKeyword(Tng::ArmoKey(Tng::akeyIgnored))) skin->AddKeyword(Tng::ArmoKey(Tng::akeyIgnored));
  return Tng::resOkRaceP;
}

void Core::GenitalizeNPCSkins() {
  Tng::logger::info("Checking NPCs for custom skins.");
  std::map<std::pair<RE::TESObjectARMO*, RE::TESRace*>, std::set<RE::TESNPC*>> skinsToPatch{};
  std::map<std::string_view, size_t> customSkinMods{};
  auto& allNPCs = Tng::SEDH()->GetFormArray<RE::TESNPC>();
  for (const auto& npc : allNPCs) {
    if (Inis::IsNPCExcluded(npc)) {
      npc->AddKeyword(Tng::NPCKey(Tng::npckeyExclude));
      continue;
    }
    const auto race = npc->race;
    if (!race) {
      Tng::logger::warn("\t\tThe NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", npc->GetFormID(), npc->GetFormEditorID());
      continue;
    }
    if (!race->HasKeyword(Tng::RaceKey(Tng::rkeyProcessed))) continue;
    if (npc->IsFemale()) continue;
    const auto skin = npc->skin;
    if (!skin) continue;
    if (skin->HasKeyword(Tng::ArmoKey(Tng::akeyIgnored))) continue;
    skinsToPatch[{skin, race}].insert(npc);
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
    Tng::logger::info("\tHandled {} custom skin-race combinations from following mod(s):", skinsToPatch.size());
    for (const auto& entry : customSkinMods) {
      Tng::logger::info("\t\t[{}] skins from {}", entry.second, entry.first);
      tot -= entry.second;
    }
    Tng::logger::info("\t\t[{}] skins were not patched.", tot);
  }
}

Tng::TNGRes Core::CanModifyActor(RE::Actor* actor) {
  auto res = Base::CanModifyActor(actor);
  if (res < 0) return res;
  return (Inis::IsNPCExcluded(actor->GetActorBase())) ? Tng::npcErr : res;
}

Tng::TNGRes Core::SetActorSize(RE::Actor* actor, int genSize) {
  auto res = Base::SetActorSizeCat(actor, genSize);
  if (res == Tng::resOkSizable && !actor->GetActorBase()->IsPlayer()) Inis::SaveNPCSize(actor->GetActorBase(), genSize);
  return res;
}

Tng::TNGRes Core::SetNPCAddn(RE::TESNPC* npc, int addnIdx, bool isUser) {
  if (!npc->race || !npc->race->HasKeyword(Tng::RaceKey(Tng::rkeyProcessed))) return Tng::raceErr;
  if (Inis::IsNPCExcluded(npc)) return Tng::npcErr;
  auto res = Base::SetNPCAddn(npc, addnIdx, isUser);
  if (res < 0) return res;
  if (!npc->IsPlayer() && isUser) Inis::SaveNPCAddn(npc, addnIdx);
  return res;
}

RE::TESObjectARMO* Core::FixSkin(RE::TESObjectARMO* skin, RE::TESRace* race, const char* const aName) {
  skin->RemoveKeywords(Tng::ArmoKeys());
  skin->AddKeyword(Tng::ArmoKey(Tng::akeyIgnored));
  if (!skin->HasPartOf(Tng::cSlotBody)) {
    Tng::logger::info("\t\tThe skin [0x{:x}] used does not have a body part. TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->HasPartOf(Tng::cSlotGenital)) {
    Tng::logger::warn("\t\tThe skin [0x{:x}] cannot have a male genital.", skin->GetFormID());
    return nullptr;
  }
  if (!skin->race) {
    Tng::logger::warn("\t\tThe skin [0x{:x}] does not have a race! TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  if (skin->armorAddons.size() == 0) {
    Tng::logger::warn("\t\tThe skin [0x{:x}] does not have any arma! TNG ignores it.", skin->GetFormID());
    return nullptr;
  }
  auto rgIdx = Base::GetRaceRgIdx(race);
  auto addonIdx = Base::GetRgAddn(race);
  if (rgIdx < 0 || addonIdx == Tng::pgErr) {
    Tng::logger::critical("\t\tSkin [0x{:x}] from file [{}] together with race [0x{:x}] from file caused a critical error!", skin->GetFormID(), skin->GetFile() ? skin->GetFile()->GetFilename() : "Unknown",
                          race->GetFormID(), race->GetFile() ? race->GetFile()->GetFilename() : "Unknown");
    return nullptr;
  }
  if (addonIdx == Tng::cNul) return skin;
  if (aName) Tng::logger::info("\t\tThe skin [0x{:x}: {}] added as extra skin.", skin->GetFormID(), aName);
  return Base::GetSkinWithAddonForRg(rgIdx, skin, addonIdx, false);
}

void Core::CheckOutfits() {
  Tng::logger::info("Checking OTFT records...");
  bool b = false;
  auto& allOutfits = Tng::SEDH()->GetFormArray<RE::BGSOutfit>();
  for (auto& outfit : allOutfits) {
    RE::TESObjectARMO* body = nullptr;
    RE::TESObjectARMO* down = nullptr;
    for (auto& item : outfit->outfitItems) {
      auto armo = item ? item->As<RE::TESObjectARMO>() : nullptr;
      if (!armo) continue;
      if (armo->HasPartOf(Tng::cSlotBody)) body = armo;
      if (armo->HasPartOf(Tng::cSlotGenital) && !armo->HasKeyword(Tng::ArmoKey(Tng::akeyUnderwear))) down = armo;
    }
    if (body && down && body != down) {
      body->AddKeyword(Tng::ArmoKey(Tng::akeyAutoReveal));
      Tng::logger::info("\tThe armor [0x{:x} : {}] was used together with an item equipped on slot 52 [0x{:x} : {}] in an outfit. It was marked revealing so the item on slot 52 won't be covered.",
                        body->GetFormID(), body->GetName(), down->GetFormID(), down->GetName());
      b = true;
    }
  }
  if (!b) Tng::logger::info("\t TNG did not find any outfit that uses items on slot 32 and 52 at the same time.");
}

void Core::CheckArmorPieces() {
  Tng::logger::info("Checking ARMO records...");
  auto& armorlist = Tng::SEDH()->GetFormArray<RE::TESObjectARMO>();
  int hk = 0;
  int ia = 0;
  int rr = 0;
  int ar = 0;
  int cc = 0;
  int ac = 0;
  int pa = 0;
  auto iaKey = Tng::ArmoKey(Tng::akeyIgnored);
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  std::set<std::string> potentialMods{};
  std::set<std::string> potentialSlot52Mods{};
  for (const auto& armor : armorlist) {
    if (!armor) {
      pa++;
      continue;
    }
    if (armor->HasKeywordInArray(Tng::ArmoKeys(), false)) {
      hk++;
      continue;
    }
    if (armor->armorAddons.size() == 0) armor->AddKeyword(iaKey);
    const auto armorID = (std::string(armor->GetName()).empty()) ? armor->GetFormEditorID() : armor->GetName();
    std::string modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (!armor->race) {
      if (!armor->HasKeyword(iaKey)) Tng::logger::warn("\t\tThe armor [0x{:x}: {}] does not have a race! It won't be touched by Tng!", armor->GetFormID(), armorID);
      armor->AddKeyword(iaKey);
      ia++;
      continue;
    }
    if (!armor->race->HasKeywordInArray(Tng::RaceKeys(Tng::rkeyIgnore), false) && armor->race != Tng::Race(Tng::raceDefault)) continue;
    if (Inis::IsSkin(armor, modName)) {
      Tng::logger::info("\t\tThe record [0x{:x}: {}] was marked as a skin.", armor->GetFormID(), armorID);
      armor->AddKeyword(iaKey);
      ia++;
      continue;
    }
    bool has52 = armor->HasPartOf(Tng::cSlotGenital);
    for (auto& aa : armor->armorAddons) {
      if (has52) break;
      has52 = aa->HasPartOf(Tng::cSlotGenital);
    }
    if (!has52) {
      if (Inis::IsCovering(armor, modName)) {
        armor->AddKeyword(Tng::ArmoKey(Tng::akeyFixCover));
        Tng::logger::info("\t\tThe armor [0x{:x}: {}] was marked covering.", armor->GetFormID(), armorID);
        cc++;
        continue;
      }
      if (Inis::IsRevealing(armor, modName) || armor->HasKeywordString(Tng::cSOSR)) {
        if (armor->HasPartOf(Tng::cSlotBody)) {
          armor->AddKeyword(Tng::ArmoKey(Tng::akeyFixReveal));
          Tng::logger::info("\t\tArmor [0x{:x}: {}] was marked revealing.", armor->GetFormID(), armorID);
          rr++;
        } else {
          ia++;
        }
        continue;
      }
      if (Inis::IsRTCovering(armor, modName)) {
        armor->AddKeyword(Tng::ArmoKey(Tng::akeyAutoCover));
        Tng::logger::info("\t\tThe armor [[0x{:x}: {}] was marked covering since it was previously marked covering during gameplay.", armor->GetFormID(), armorID);
        ac++;
        continue;
      }
      if (Inis::IsRTRevealing(armor, modName)) {
        armor->AddKeyword(Tng::ArmoKey(Tng::akeyAutoReveal));
        Tng::logger::info("\t\tThe armor [[0x{:x}: {}] was marked revealing since it was previously marked revealing during gameplay.", armor->GetFormID(), armorID);
        ar++;
        continue;
      }
      if (Inis::IsExtraRevealing(modName)) {
        if (armor->HasPartOf(Tng::cSlotBody)) {
          armor->AddKeyword(Tng::ArmoKey(Tng::akeyAutoReveal));
          Tng::logger::info("\t\tArmor [0x{:x}: {}] was marked revealing.", armor->GetFormID(), armorID);
          ar++;
        } else {
          ia++;
        }
        continue;
      }
    } else {
      armor->RemoveKeywords(Tng::ArmoKeys());
      armor->AddKeyword(Tng::ArmoKey(Tng::akeyFixCover));
      cc++;
      if (armor->HasPartOf(Tng::cSlotBody)) continue;
      if (modName != "" && Inis::IsUnhandled(modName)) potentialSlot52Mods.insert(modName);
      Tng::logger::info("\t\tThe armor [0x{:x}: {}] would cover genitals and would have a conflict with non-revealing chest armor pieces!", armor->GetFormID(), armorID);
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
  for (auto& modName : potentialSlot52Mods) Inis::HandleModWithSlot52(modName, Tng::BoolSetting(Tng::bsRevealSlot52Mods));
  for (auto& armorPair : potentialArmor) {
    if (Inis::IsExtraRevealing(armorPair.first)) {
      armorPair.second->AddKeyword(Tng::ArmoKey(Tng::akeyAutoReveal));
      ac++;
    } else {
      armorPair.second->AddKeyword(Tng::ArmoKey(Tng::akeyAutoCover));
      ar++;
    }
  }
  Tng::logger::info("\tProcessed [{}] armor pieces:", armorlist.size());
  if (pa > 0) Tng::logger::warn("\t\t[{}]: were problematic!", pa);
  if (hk > 0) Tng::logger::info("\t\t[{}]: were already marked with TNG keywords.", hk);
  if (cc > 0) Tng::logger::info("\t\t[{}]: are covering due to ini-files or having slot 52.", cc);
  if (rr > 0) Tng::logger::info("\t\t[{}]: are marked revealing.", rr);
  if (ac > 0) Tng::logger::info("\t\t[{}]: were recognized to be covering.", ac);
  if (ar > 0) Tng::logger::info("\t\t[{}]: were recognized to be revealing.", ar);
  if (ia > 0) Tng::logger::info("\t\tThe rest [{}] are not relevant and are ignored!", ia);
}

void Core::RevisitRevealingArmor() {
  auto& armorlist = Tng::SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<std::pair<std::string, RE::TESObjectARMO*>> potentialArmor = {};
  for (const auto& armor : armorlist) {
    if (!armor) continue;
    auto modName = armor->GetFile(0) ? std::string(armor->GetFile(0)->GetFilename()) : "";
    if (modName != "" && (armor->HasKeyword(Tng::ArmoKey(Tng::akeyAutoCover)) || armor->HasKeyword(Tng::ArmoKey(Tng::akeyAutoReveal)))) {
      potentialArmor.insert({modName, armor});
      armor->RemoveKeywords(Tng::ArmoKeys());
    }
  }
  for (const auto& armorPair : potentialArmor) armorPair.second->AddKeyword(Tng::ArmoKey(Inis::IsExtraRevealing(armorPair.first) ? Tng::akeyAutoReveal : Tng::akeyAutoReveal));
}

bool Core::SwapRevealing(RE::TESObjectARMO* armor) {
  if (armor->HasKeyword(Tng::ArmoKey(Tng::akeyAutoCover))) {
    armor->RemoveKeyword(Tng::ArmoKey(Tng::akeyAutoCover));
    armor->AddKeyword(Tng::ArmoKey(Tng::akeyAutoReveal));
    Inis::SaveRevealingArmor(armor);
    return true;
  }
  if (armor->HasKeyword(Tng::ArmoKey(Tng::akeyAutoReveal))) {
    armor->RemoveKeyword(Tng::ArmoKey(Tng::akeyAutoReveal));
    armor->AddKeyword(Tng::ArmoKey(Tng::akeyAutoCover));
    Inis::SaveCoveringArmor(armor);
    return true;
  }
  return false;
}
