#include <Base.h>
#include <Core.h>
#include <Inis.h>

bool Core::Initialize() noexcept {
  Tng::SEDH() = RE::TESDataHandler::GetSingleton();
  if (!Tng::SEDH()->LookupModByName(Tng::cName)) {
    SKSE::log::critical("Mod [{}] was not found! Please report this issue!", Tng::cName);
    return false;
  }
  fPRaceKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  fRRaceKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  fIRaceKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cIgnoredRaceKeyID, Tng::cName);
  fARKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  fRRKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  fACKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  fCCKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  fPAKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cProblemArmoKeyID, Tng::cName);
  fIAKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  fUAKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  Tng::NexKey() = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cExcludeKeyID, Tng::cName);
  fNPCKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cNPCKeywID, Tng::cSkyrim);
  fCrtKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cCrtKeywID, Tng::cSkyrim);
  fGenSkinKey = Tng::SEDH()->LookupForm<RE::BGSKeyword>(Tng::cCustomSkinID, Tng::cName);
  fDefRace = Tng::SEDH()->LookupForm<RE::TESRace>(Tng::cDefRaceID, Tng::cSkyrim);
  fBeastDef = Tng::SEDH()->LookupForm<RE::TESRace>(Tng::cBstRaceID, Tng::cSkyrim);
  fAllNPCs = Tng::SEDH()->GetFormArray<RE::TESNPC>();
  if (!(fPRaceKey && fRRaceKey && fIRaceKey && fARKey && fRRKey && fACKey && fCCKey && fPAKey && fIAKey && fUAKey && Tng::NexKey() && fNPCKey && fCrtKey && fSwPKey && fGenSkinKey &&
        fDefRace && fBeastDef)) {
    SKSE::log::critical("The original TNG information could not be found!");
    return false;
  }
  for (std::size_t i = Tng::raceManMer; i < Tng::cRaceTypeCount; i++) {
    fExSkins[i] = Tng::SEDH()->LookupForm<RE::TESObjectARMO>(cExSkinIDs[i], Tng::cName);
    if (!fExSkins) {
      SKSE::log::critical("Skins for excluded NPCs cannot be loaded.");
      return false;
    }
  }

  std::set<std::string> lDefSks{};
  lDefSks.insert(fDefRace->skeletonModels[0].model.data());
  lDefSks.insert(fDefRace->skeletonModels[1].model.data());
  lDefSks.insert(fBeastDef->skeletonModels[0].model.data());
  lDefSks.insert(fBeastDef->skeletonModels[1].model.data());
  Inis::UpdateValidSkeletons(lDefSks);
  fArmoKeys = {fARKey, fRRKey, fACKey, fCCKey, fPAKey, fIAKey, fUAKey};
  return true;
}

void Core::GenitalizeRaces() noexcept {
  SKSE::log::info("Finding the genitals for relevant races...");
  for (const auto& lRaceID : cExclRaceIDs) IgnoreRace(Tng::SEDH()->LookupForm<RE::TESRace>(lRaceID.first, lRaceID.second));
  auto& lAllRacesArray = Tng::SEDH()->GetFormArray<RE::TESRace>();
  int lPR = 0;
  int lIR = 0;
  int lRR = 0;
  int lPP = 0;
  for (const auto& race : lAllRacesArray) {
    if (race->HasKeyword(fIRaceKey)) {
      lIR++;
      continue;
    }
    if (race->HasKeyword(fRRaceKey)) {
      lRR++;
      continue;
    }
    if (race->HasKeyword(fPRaceKey)) {
      lPR++;
      continue;
    }
    if (CheckRace(race)) {
      switch (AddRace(race)) {
        case Tng::resOkRaceP:
          lPR++;
          break;
        case Tng::resOkRaceR:
          lRR++;
          break;
        case Tng::raceErr:
          lIR++;
          break;
        default:
          lPP++;
          break;
      }
    }
  }
  for (int i = 0; i < Base::GroupCount(); i++) {
    auto lSkinAAs = Base::GentifyGrpSkin(i);
    auto lRacialSkin = Base::GetRGSkin(i);
    fSAAs.insert(lSkinAAs.begin(), lSkinAAs.end());
  }
  SKSE::log::info("Recognized assigned genitals to [{}] races, found [{}] races to be ready and ignored [{}] races.", lPR, lRR, lIR);
  if (lPP > 0) SKSE::log::error("Faced errors for [{}] races.", lPP);
}

bool Core::UpdateRGAddn(const std::size_t aRgId, int aAddon) noexcept {
  int lChoice = aAddon >= 0 ? aAddon : Base::GetRGDefAddn(aRgId);
  Base::UpdateRGAddn(aRgId, lChoice);
  Inis::SaveRaceAddn(aRgId, aAddon);
  return true;
}

bool Core::IgnoreRace(RE::TESRace* aRace) noexcept {
  bool lIsReady = false;
  if (aRace->skin) {
    aRace->skin->RemoveKeywords(fArmoKeys);
    aRace->skin->AddKeyword(fIAKey);
    for (const auto& aa : aRace->skin->armorAddons) {
      if (aa->HasPartOf(Tng::cSlotBody)) fSAAs.insert(aa);
      if (aa->HasPartOf(Tng::cSlotGenital)) lIsReady = true;
    }
  }
  aRace->AddKeyword(lIsReady ? fRRaceKey : fIRaceKey);
  return lIsReady;
}

bool Core::CheckRace(RE::TESRace* aRace) {
  try {
    if (!aRace->HasKeyword(fNPCKey) || aRace->HasKeyword(fCrtKey) || !aRace->HasPartOf(Tng::cSlotBody) || aRace->IsChildRace()) return false;
    if (aRace->GetFile(0) && Inis::fRaceExMods.size() > 0 && Inis::fRaceExMods.find(std::string{aRace->GetFile(0)->GetFilename()}) != Inis::fRaceExMods.end()) {
      SKSE::log::info("The race [{}: 0x{:x}: {}] was ignored because an ini excludes it!", aRace->GetFile(0)->GetFilename(), aRace->GetFormID(), aRace->GetFormEditorID());
      return false;
    }
    if (!Inis::IsValidSkeleton(aRace->skeletonModels[0].model.data()) || !Inis::IsValidSkeleton(aRace->skeletonModels[1].model.data())) {
      IgnoreRace(aRace);
      SKSE::log::info("The race [0x{:x}: {}] was ignored because it uses a custom skeleton!", aRace->GetFormID(), aRace->GetFormEditorID());
      return false;
    }
    if (!aRace->skin) {
      SKSE::log::warn("The race [0x{:x}: {}] cannot have any genitals since they do not have a skin.", aRace->GetFormID(), aRace->GetFormEditorID());
      IgnoreRace(aRace);
      return false;
    }
    RE::TESRace* lArmRace = aRace->armorParentRace ? aRace->armorParentRace : aRace;
    RE::TESObjectARMA* lRaceSkinAA{nullptr};
    for (const auto& aa : aRace->skin->armorAddons) {
      std::set<RE::TESRace*> lAARaces{aa->race};
      lAARaces.insert(aa->additionalRaces.begin(), aa->additionalRaces.end());
      if (aa->HasPartOf(Tng::cSlotBody) && ((lAARaces.find(lArmRace) != lAARaces.end()) || (aa->race == fDefRace))) {
        lRaceSkinAA = aa;
        break;
      }
    }
    if (!lRaceSkinAA) {
      SKSE::log::warn("The race [0x{:x}: {}] cannot have any genitals since their skin cannot be recognized.", aRace->GetFormID(), aRace->GetFormEditorID());
      IgnoreRace(aRace);
      return false;
    }
    return true;
  } catch (const std::exception& er) {
    SKSE::log::warn("The race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", aRace->GetFormID(), aRace->GetFormEditorID(), er.what());
    const char* lMessage =
        fmt::format("The race [0x{:x}: {}] caused an error [{}] in the process. TNG tries to ignore it but it might not work properly!", aRace->GetFormID(), aRace->GetFormEditorID(), er.what()).c_str();
    ShowSkyrimMessage(lMessage);
    IgnoreRace(aRace);
  }
}

Tng::TNGRes Core::AddRace(RE::TESRace* aRace) noexcept {
  if (aRace->HasPartOf(Tng::cSlotGenital) || aRace->skin->HasPartOf(Tng::cSlotGenital)) {
    SKSE::log::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
    return IgnoreRace(aRace) ? Tng::resOkRaceR : Tng::raceErr;
  }
  for (const auto aa : aRace->skin->armorAddons)
    if (aa->HasPartOf(Tng::cSlotGenital)) {
      SKSE::log::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
      return IgnoreRace(aRace) ? Tng::resOkRaceR : Tng::raceErr;
    }
  auto lRaceIdx = Base::GetRG(aRace);
  
  aRace->AddKeyword(fPRaceKey);
  aRace->AddSlotToMask(Tng::cSlotGenital);
  auto skin = aRace->skin;
  skin->RemoveKeywords(fArmoKeys);
  skin->AddKeyword(fIAKey);
  auto lRaceToPatch = aRace->armorParentRace ? aRace->armorParentRace : aRace;
  if (lRaceToPatch != fDefRace) Base::UpdateAddons(lRaceToPatch);
  return Tng::resOkRaceP;
}

RE::TESObjectARMO* Core::ProduceAddonSkin(RE::TESObjectARMO* aOgSkin, int aAddonChoice, bool aIsFemale) noexcept {
  auto& lSkins = aIsFemale ? fFemAddonSkins : fMalAddonSkins;
  if (auto it = lSkins.find(aOgSkin); (it != lSkins.end()) && it->second[aAddonChoice]) return it->second[aAddonChoice];
  RE::TESObjectARMO* skin = nullptr;
  skin = aOgSkin->CreateDuplicateForm(true, (void*)skin)->As<RE::TESObjectARMO>();
  skin->Copy(aOgSkin);
  skin->AddKeyword(fGenSkinKey);
  if (Base::GetAddonAt(aIsFemale, aAddonChoice)->HasKeyword(fSwPKey)) skin->AddKeyword(fSwPKey);
  aIsFemale ? Base::GentifyFemSkin(skin, aAddonChoice) : Base::GentifyMalSkin(skin, aAddonChoice);
  if (auto it = lSkins.find(aOgSkin); it != lSkins.end()) {
    it->second[aAddonChoice] = skin;
  } else {
    auto lInsert = lSkins.insert_or_assign(aOgSkin, std::vector<RE::TESObjectARMO*>(Base::GetAddonCount(aIsFemale)));
    lInsert.first->second[aAddonChoice] = skin;
  }
  return skin;
}

void Core::GenitalizeNPCSkins() noexcept {
  SKSE::log::info("Checking NPCs for custom skins.");
  std::set<RE::TESObjectARMO*> lSkinsToPatch{};
  std::map<std::string_view, int> lCustomSkinMods{};
  if (Inis::fHardExcluded.size() > 0) LoadHardExcluded();
  for (const auto& npc : fAllNPCs) {
    if (fHardExcludedNPCs.find(npc) != fHardExcludedNPCs.end()) continue;
    const auto lNPCRace = npc->race;
    if (!lNPCRace) {
      SKSE::log::warn("The NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", npc->GetFormID(), npc->GetName());
      continue;
    }
    if (!lNPCRace->HasKeyword(fPRaceKey) && !lNPCRace->HasKeyword(fRRaceKey)) continue;
    const auto skin = npc->skin;
    if (!skin) continue;
    if (skin->HasKeyword(fIAKey)) continue;
    skin->AddKeyword(fIAKey);
    if (!skin->HasPartOf(Tng::cSlotBody)) continue;
    if (skin->HasPartOf(Tng::cSlotGenital)) {
      SKSE::log::info("The skin [0x{:x}] used in NPC [{}] cannot have a male genital.", skin->GetFormID(), npc->GetName());
      continue;
    }
    lSkinsToPatch.insert(skin);
  }
  for (auto skin : lSkinsToPatch) {
    if (!FixSkin(skin, nullptr)) continue;
    auto lSkinName = skin->GetFile(0) ? skin->GetFile(0)->GetFilename() : "Other";
    auto it = lCustomSkinMods.insert({lSkinName, 0});
    it.first->second++;
  }
  if (lSkinsToPatch.size() > 0) {
    auto lTot = lSkinsToPatch.size();
    SKSE::log::info("\tHandled custom skins for {} NPCs from following mod(s):", lSkinsToPatch.size());
    for (const auto& lMod : lCustomSkinMods) {
      SKSE::log::info("\t\t[{}] skins from {}", lMod.second, lMod.first);
      lTot -= lMod.second;
    }
    SKSE::log::info("\t\t[{}] skins were not patched.", lTot);
  }
  for (const auto& npc : fAllNPCs)
    if (auto lAddn = Base::GetNPCAddn(npc); lAddn.second >= 0) SetNPCSkin(npc, lAddn.second, lAddn.first);
}

Tng::TNGRes Core::CanModifyActor(RE::Actor* aActor) noexcept {
  auto res = Base::CanModifyActor(aActor);
  if (res < 0) return res;
  return (fHardExcludedNPCs.find(aActor->GetActorBase()) != fHardExcludedNPCs.end()) ? Tng::npcErr : res;
}

Tng::TNGRes Core::SetNPCSkin(RE::TESNPC* aNPC, int aAddon, bool aIsUser) noexcept {
  auto lMaxRes = (aNPC->IsPlayer() && Inis::GetSettingBool(Inis::excludePlayerSize)) ? Tng::resOkNoGen : Tng::resOkGen;
  if (!aNPC->race || !aNPC->race->HasKeyword(fPRaceKey)) return Tng::raceErr;
  if (fHardExcludedNPCs.find(aNPC) != fHardExcludedNPCs.end()) return Tng::npcErr;
  if (aAddon == -1) return !aNPC->IsFemale() || (aNPC->skin && aNPC->skin->HasKeyword(fSwPKey)) ? lMaxRes : Tng::resOkNoGen;
  if (aAddon == -2) {
    RevertNPCSkin(aNPC);
    Base::SetNPCAddn(aNPC, aAddon, true);
    if (!aNPC->IsPlayer()) Inis::SaveNPCAddn(aNPC, aNPC->IsFemale() ? -3 : aAddon);
    return !aNPC->IsFemale() ? lMaxRes : Tng::resOkNoGen;
  }
  auto lCurrSkin = aNPC->skin ? aNPC->skin : aNPC->race->skin;
  auto lOgSkin = GetOgSkin(aNPC);
  if (!lOgSkin) {
    SKSE::log::critical("Failed to get the previously created skin for an NPC!");
    return Tng::pgErr;
  }
  if (lOgSkin->armorAddons.size() == 0 || !lOgSkin->race) return Tng::skinErr;
  auto lActualAddon = aIsUser ? aAddon : Base::GetActualAddon(aNPC->IsFemale(), aAddon);
  auto skin = ProduceAddonSkin(lOgSkin, lActualAddon, aNPC->IsFemale());
  Base::SetNPCAddn(aNPC, aAddon, aIsUser);
  if (skin != lCurrSkin) {
    aNPC->skin = skin;
    if (!aNPC->IsPlayer() && aIsUser) Inis::SaveNPCAddn(aNPC, aAddon);
  }
  return !aNPC->IsFemale() || aNPC->skin->HasKeyword(fSwPKey) ? lMaxRes : Tng::resOkNoGen;
}

Tng::TNGRes Core::SetCharSize(RE::Actor* aActor, RE::TESNPC* aNPC, int aGenSize) noexcept {
  if (!aNPC->race) return Tng::raceErr;
  if (!(aNPC->race->HasKeyword(fPRaceKey) || aNPC->race->HasKeyword(fRRaceKey))) return Tng::raceErr;
  int lGenSize = (aNPC->IsPlayer() && Inis::GetSettingBool(Inis::excludePlayerSize)) ? -2 : aGenSize;
  auto res = Base::SetCharSize(aActor, aNPC, lGenSize);
  if (!aNPC->IsPlayer() && res == Tng::resOkGen && lGenSize >= 0) Inis::SaveNPCSize(aNPC, lGenSize);
  return res;
}

RE::TESObjectARMO* Core::GetOgSkin(RE::TESNPC* aNPC) noexcept {
  auto skin = aNPC->skin ? aNPC->skin : aNPC->race->skin;
  if (!skin->HasKeyword(fGenSkinKey)) return skin;
  auto& lSkinsToLook = aNPC->IsFemale() ? fFemAddonSkins : fMalAddonSkins;
  for (const auto& lAddonEntry : lSkinsToLook)
    for (const auto& lAddnSkin : lAddonEntry.second)
      if (lAddnSkin == skin) return lAddonEntry.first;
  return nullptr;
}

bool Core::FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept {
  aSkin->RemoveKeywords(fArmoKeys);
  aSkin->AddKeyword(fIAKey);
  if (!aSkin->HasPartOf(Tng::cSlotBody)) return false;
  if (!aSkin->race) return false;
  if (aSkin->armorAddons.size() == 0) return false;
  if (aSkin->HasPartOf(Tng::cSlotGenital)) {
    if (aName) SKSE::log::info("The skin [0x{:x}: {}] is ready for TNG! It won't be modified.", aSkin->GetFormID(), aName);
    for (const auto& aa : aSkin->armorAddons)
      if (aa->HasPartOf(Tng::cSlotBody)) fSAAs.insert(aa);
    return true;
  }
  auto lSkinAAs = Base::GentifyMalSkin(aSkin);
  fSAAs.insert(lSkinAAs.begin(), lSkinAAs.end());
  if (aName) SKSE::log::info("The skin [0x{:x}: {}] added as extra skin.", aSkin->GetFormID(), aName);
  return true;
}

void Core::LoadHardExcluded() noexcept {
  for (const auto& lRecord : Inis::fHardExcluded) {
    auto npc = Tng::SEDH()->LookupForm<RE::TESNPC>(lRecord.second, lRecord.first);
    if (!npc) continue;
    SKSE::log::info("The NPC [{}] would be excluded from TNG.", npc->GetName());
    fHardExcludedNPCs.insert(npc);
    npc->AddKeyword(Tng::NexKey());
    if (npc->race) npc->skin = fExSkins[Base::GetRaceType(npc->race)];
  }
}

void Core::RevertNPCSkin(RE::TESNPC* aNPC) {
  if (fHardExcludedNPCs.find(aNPC) != fHardExcludedNPCs.end()) return;
  auto lOgSkin = GetOgSkin(aNPC);
  if (lOgSkin)
    aNPC->skin = lOgSkin == aNPC->race->skin ? nullptr : lOgSkin;
  else {
    SKSE::log::critical("Failed to revert the skin of [{}] to its original value!", aNPC->GetName());
  }
}

void Core::CheckOutfits() noexcept {
  SKSE::log::info("Checking OTFT records...");
  auto& lAllOutfits = Tng::SEDH()->GetFormArray<RE::BGSOutfit>();
  for (auto& lOutfit : lAllOutfits) {
    RE::TESObjectARMO* lBArmo = nullptr;
    RE::TESObjectARMO* lGArmo = nullptr;
    for (auto& lItem : lOutfit->outfitItems) {
      auto lArmo = lItem ? lItem->As<RE::TESObjectARMO>() : nullptr;
      if (!lArmo) continue;
      if (lArmo->HasPartOf(Tng::cSlotBody)) lBArmo = lArmo;
      if (lArmo->HasPartOf(Tng::cSlotGenital) && !lArmo->HasKeyword(fUAKey)) lGArmo = lArmo;
    }
    if (lBArmo && lGArmo) fR4Os.insert(lBArmo);
  }
}

void Core::CheckArmorPieces() noexcept {
  SKSE::log::info("Checking ARMO records...");
  auto& armorlist = Tng::SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<RE::TESObjectARMO*> lPotentialArmor;
  bool lCheckSkinMods = Inis::fSkinMods.size() > 0;
  bool lCheckSkinRecords = Inis::fSingleSkinIDs.size() > 0;
  bool lCheckRevealMods = Inis::fRevealingMods.size() > 0;
  bool lCheckRevealRecords = Inis::fSingleRevealingIDs.size() > 0;
  bool lCheckCoverRecords = Inis::fSingleCoveringIDs.size() > 0;
  bool lCheckRuntimeRRecords = Inis::fRunTimeRevealingIDs.size() > 0;
  bool lCheckRuntimeCRecords = Inis::fRuntimeCoveringIDs.size() > 0;
  bool lCheckExtraRevealing = Inis::fExtraRevealing.size() > 0;
  if (lCheckExtraRevealing) fSlot52Mods.insert(Inis::fExtraRevealing.begin(), Inis::fExtraRevealing.end());
  std::set<std::string> lSlot52ReadyMods{Inis::fSkinMods.begin(), Inis::fSkinMods.end()};
  std::set<std::string> lModsWithPotential;
  auto lERMCount = Inis::fExtraRevealing.size();
  int lIA = armorlist.size();
  int lRR = 0;
  int lAR = 0;
  int lCC = 0;
  int lAC = 0;
  int lPA = 0;
  for (const auto& lModName : Inis::fSkinMods)
    if (Tng::SEDH()->LookupModByName(lModName)) SKSE::log::info("TheNewGentleman keeps an eye for [{}] as a skin mod.", lModName);
  for (const auto& lRevealMod : Inis::fRevealingMods)
    if (Tng::SEDH()->LookupModByName(lRevealMod)) SKSE::log::info("TheNewGentleman keeps an eye for [{}] as a revealing armor mod.", lRevealMod);
  for (const auto& armor : armorlist) {
    if (!armor) continue;
    if (armor->armorAddons.size() == 0) armor->AddKeyword(fIAKey);
    const auto lID = (std::string(armor->GetName()).empty()) ? armor->GetFormEditorID() : armor->GetName();
    if (!armor->race) {
      if (!armor->HasKeyword(fIAKey)) SKSE::log::warn("The armor [0x{:x}: {}] does not have a race! It won't be touched by Tng!", armor->GetFormID(), lID);
      armor->AddKeyword(fIAKey);
      continue;
    }
    if (!(armor->race->HasKeyword(fPRaceKey) || armor->race->HasKeyword(fRRaceKey) || armor->race == fDefRace)) continue;
    if (armor->HasKeyword(fIAKey) || armor->HasKeyword(fUAKey)) continue;
    auto lFN = armor->GetFile(0) ? armor->GetFile(0)->GetFilename() : cNoFile;
    if ((lCheckSkinMods && Inis::fSkinMods.find(std::string{lFN}) != Inis::fSkinMods.end()) ||
        (lCheckSkinRecords && Inis::fSingleSkinIDs.find(std::make_pair(std::string{lFN}, armor->GetLocalFormID())) != Inis::fSingleSkinIDs.end())) {
      if (FixSkin(armor, lID)) {
        SKSE::log::info("The skin [{:x}] from file [{}] was patched.", armor->GetLocalFormID(), lFN);
      } else {
        SKSE::log::error("The skin [{:x}] from file [{}] has issues. It was not touched by TNG.", armor->GetLocalFormID(), lFN);
      }
      continue;
    }
    lIA--;
    if ((lCheckCoverRecords && Inis::fSingleCoveringIDs.find(std::make_pair(std::string{lFN}, armor->GetLocalFormID())) != Inis::fSingleCoveringIDs.end()) ||
        armor->HasKeyword(fCCKey)) {
      TryMakeArmorCovering(armor, true);
      SKSE::log::info("The armor [0x{:x}: {}] was marked covering.", armor->GetFormID(), lID);
      lCC++;
      continue;
    }
    if ((lCheckRevealMods && Inis::fRevealingMods.find(std::string{lFN}) != Inis::fRevealingMods.end()) ||
        (lCheckRevealRecords && Inis::fSingleRevealingIDs.find(std::make_pair(std::string{lFN}, armor->GetLocalFormID())) != Inis::fSingleRevealingIDs.end()) ||
        armor->HasKeywordString(Tng::cSOSR) || armor->HasKeyword(fRRKey)) {
      lSlot52ReadyMods.insert(std::string{lFN});
      if (armor->HasPartOf(Tng::cSlotBody)) {
        if (TryMakeArmorRevealing(armor, true)) {
          SKSE::log::info("Armor [0x{:x}: {}] was marked revealing.", armor->GetFormID(), lID);
          lRR++;
        } else {
          SKSE::log::error("The armor [0x{:x}: {}] was assigned revealing but it has arma on slot 52!", armor->GetFormID(), lID);
          lPA++;
        }
      } else {
        lIA++;
      }
      continue;
    }
    if (lCheckRuntimeRRecords && Inis::fRunTimeRevealingIDs.find(std::make_pair(std::string{lFN}, armor->GetLocalFormID())) != Inis::fRunTimeRevealingIDs.end()) {
      if (TryMakeArmorRevealing(armor, false)) {
        SKSE::log::info("The armor [[0x{:x}: {}] was marked revealing since it was previously marked revealing during gameplay.", armor->GetFormID(), lID);
        lAR++;
      } else {
        SKSE::log::warn("The armor [0x{:x}: {}] was marked revealing in a previous gameplay but now it must be covering!", armor->GetFormID(), lID);
        lPA++;
      }
      continue;
    }
    if (lCheckRuntimeCRecords && Inis::fRuntimeCoveringIDs.find(std::make_pair(std::string{lFN}, armor->GetLocalFormID())) != Inis::fRuntimeCoveringIDs.end()) {
      if (TryMakeArmorCovering(armor, false)) {
        SKSE::log::info("The armor [[0x{:x}: {}] was marked covering since it was previously marked covering during gameplay.", armor->GetFormID(), lID);
        lAR++;
      } else {
        SKSE::log::warn("The armor [0x{:x}: {}] was marked covering in a previous gameplay but TNG failed to apply that again!", armor->GetFormID(), lID);
        lPA++;
      }
      continue;
    }
    if (fR4Os.find(armor) != fR4Os.end()) {
      if (TryMakeArmorRevealing(armor, true)) {
        SKSE::log::info("The armor [[0x{:x}: {}] was marked revealing since it used together with another piece on slot 52 in an outfit.", armor->GetFormID(), lID);
        lAR++;
      } else {
        SKSE::log::warn("The armor [0x{:x}: {}] is used together with another piece on slot 52 in an outfit but cannot be made revealing!", armor->GetFormID(), lID);
        lPA++;
      }
      continue;
    }
    if ((lCheckExtraRevealing && Inis::fExtraRevealing.find(std::string{lFN}) != Inis::fExtraRevealing.end())) {
      if (armor->HasPartOf(Tng::cSlotBody)) {
        if (TryMakeArmorRevealing(armor, false)) {
          SKSE::log::info("Armor [0x{:x}: {}] was marked revealing.", armor->GetFormID(), lID);
          lRR++;
        } else {
          SKSE::log::error("The armor [0x{:x}: {}] was assigned revealing but it has arma on slot 52!", armor->GetFormID(), lID);
          lPA++;
        }
      } else {
        lIA++;
      }
      continue;
    }
    if (armor->HasPartOf(Tng::cSlotGenital)) {
      armor->AddKeyword(fCCKey);
      lCC++;
      if (armor->HasPartOf(Tng::cSlotBody)) continue;
      if (lFN != cNoFile && lSlot52ReadyMods.find(std::string{lFN}) == lSlot52ReadyMods.end()) {
        fSlot52Mods.insert(std::string{lFN});
        if (Inis::GetSettingBool(Inis::revealSlot52Mods)) {
          Inis::Slot52ModBehavior(std::string{lFN}, true);
          continue;
        }
      }
      SKSE::log::warn("The armor [0x{:x}: {}] would cover genitals and would have a conflict with non-revealing chest armor pieces!", armor->GetFormID(), lID);
      continue;
    }
    if (armor->HasPartOf(Tng::cSlotBody)) lModsWithPotential.insert(std::string{lFN});
    lPotentialArmor.insert(armor);
  }
  for (auto lMod = fSlot52Mods.begin(); lMod != fSlot52Mods.end();) lModsWithPotential.find(*lMod) == lModsWithPotential.end() ? lMod = fSlot52Mods.erase(lMod) : ++lMod;
  if (Inis::fExtraRevealing.size() > lERMCount) lAR += VisitRevealingArmor(lPotentialArmor);
  for (auto& aa : fRAAs) {
    if (fCAAs.find(aa) != fCAAs.end()) {
      fRAAs.erase(aa);
    }
  }
  for (auto& armor : lPotentialArmor) {
    switch (HandleArmor(armor)) {
      case Tng::resOkAC:
        lAC++;
        break;
      case Tng::resOkAR:
        lAR++;
        break;
      case Tng::resOkIA:
        lIA++;
        break;
      default:
        lPA++;
        armor->AddKeyword(fPAKey);
        break;
    }
  }
  for (auto& aa : fCAAs) {
    if (fRAAs.find(aa) != fRAAs.end() || fSAAs.find(aa) != fSAAs.end()) {
      SKSE::log::warn("The armor addon [0x{:x}] is also part of a skin/revealing armor! Any armor using that addon would be revealing.", aa->GetFormID());
      fCAAs.erase(aa);
      continue;
    }
    if (aa->data.priorities[0] == 0 || aa->data.priorities[1] == 0)
      SKSE::log::warn("The armor addon [0x{:x}] might have wrong priorities. This can cause genital clipping through.", aa->GetFormID());
  }
  SKSE::log::info("Processed [{}] armor pieces:", armorlist.size());
  if (lPA > 0) SKSE::log::warn("\t[{}]: seems to be problematic!", lPA);
  if (lCC > 0) SKSE::log::info("\t[{}]: are covering due to ini-files or having slot 52,", lCC);
  if (lRR > 0) SKSE::log::info("\t[{}]: are marked revealing", lRR);
  if (lAC > 0) SKSE::log::info("\t[{}]: were recognized to be covering", lAC);
  if (lAR > 0) SKSE::log::info("\t[{}]: were recognized to be revealing", lAR);
  if (lIA > 0) SKSE::log::info("\tThe rest [{}] are not relevant and are ignored!", lIA);
}

void Core::RevisitRevealingArmor() noexcept {
  auto& armorlist = Tng::SEDH()->GetFormArray<RE::TESObjectARMO>();
  std::set<RE::TESObjectARMO*> lPotentialArmor;
  bool lCheckRuntimeRRecords = Inis::fRunTimeRevealingIDs.size() > 0;
  bool lCheckRuntimeCRecords = Inis::fRuntimeCoveringIDs.size() > 0;
  for (const auto& armor : armorlist) {
    auto lFN = armor->GetFile(0) ? armor->GetFile(0)->GetFilename() : cNoFile;
    if (armor && (armor->HasKeyword(fACKey) || armor->HasKeyword(fARKey))) {
      if (lCheckRuntimeRRecords && Inis::fRunTimeRevealingIDs.find(std::make_pair(std::string{lFN}, armor->GetLocalFormID())) != Inis::fRunTimeRevealingIDs.end()) continue;
      if (lCheckRuntimeCRecords && Inis::fRuntimeCoveringIDs.find(std::make_pair(std::string{lFN}, armor->GetLocalFormID())) != Inis::fRuntimeCoveringIDs.end()) continue;
      lPotentialArmor.insert(armor);
      armor->RemoveKeywords(fArmoKeys);
    }
  }
  VisitRevealingArmor(lPotentialArmor);
  for (const auto& armor : lPotentialArmor) TryMakeArmorCovering(armor, false);
}

int Core::VisitRevealingArmor(std::set<RE::TESObjectARMO*> aPotentialArmor) noexcept {
  int res = 0;
  for (const auto& armor : aPotentialArmor) {
    auto lFN = armor->GetFile(0) ? armor->GetFile(0)->GetFilename() : cNoFile;
    if (!armor->HasPartOf(Tng::cSlotBody)) continue;
    if (Inis::fExtraRevealing.find(std::string{lFN}) == Inis::fExtraRevealing.end()) continue;
    const auto lID = (std::string(armor->GetName()).empty()) ? armor->GetFormEditorID() : armor->GetName();
    if (TryMakeArmorRevealing(armor, false)) {
      SKSE::log::info("Armor [0x{:x}: {}] was marked revealing since it belongs to a mod with revealing armor.", armor->GetFormID(), lID);
      res++;
      aPotentialArmor.erase(armor);
    } else {
      SKSE::log::error("The armor [0x{:x}: {}] was assigned revealing but it has arma on slot 52!", armor->GetFormID(), lID);
    }
  }
  return res;
}

Tng::TNGRes Core::HandleArmor(RE::TESObjectARMO* aArmor) noexcept {
  const auto lID = (std::string(aArmor->GetName()).empty()) ? aArmor->GetFormEditorID() : aArmor->GetName();
  std::set<RE::TESObjectARMA*> lBods{};
  std::set<RE::TESObjectARMA*> lGens{};
  bool lS = false;
  bool lR = false;
  bool lC = false;
  for (const auto& aa : aArmor->armorAddons) {
    if (fSAAs.find(aa) != fSAAs.end()) lS = true;
    if (fRAAs.find(aa) != fRAAs.end()) lR = true;
    if (fCAAs.find(aa) != fCAAs.end()) lC = true;
    if (aa->HasPartOf(Tng::cSlotBody)) lBods.insert(aa);
    if (aa->HasPartOf(Tng::cSlotGenital)) lGens.insert(aa);
  }
  if ((lR || lS) && lC) {
    SKSE::log::warn("The armor [0x{:x}: {}] uses both covering and revealing armature at the same time!", aArmor->GetFormID(), lID);
    aArmor->RemoveKeywords(fArmoKeys);
    aArmor->AddKeyword(fPAKey);
    return Tng::armoErr;
  }
  if (lR || lS) {
    if (TryMakeArmorRevealing(aArmor, false)) {
      SKSE::log::info("The armor [0x{:x}: {}] is markerd revealing since it has {} armature!", aArmor->GetFormID(), lID, lR ? "revealing armor" : "full body");
      return Tng::resOkAR;
    } else {
      SKSE::log::warn("The armor [0x{:x}: {}] has revealing armature but also armature on slot 52!", aArmor->GetFormID(), lID);
      return Tng::resOkIA;
    }
  }
  if (lC) {
    if (lGens.size() == 0) {
      TryMakeArmorCovering(aArmor, false);
      if (!aArmor->HasPartOf(Tng::cSlotBody))
        SKSE::log::info("The armor [0x{:x}: {}] uses covering armature from another armor and is marked covering.", aArmor->GetFormID(), lID);
      return Tng::resOkAC;
    } else {
      aArmor->AddKeyword(fIAKey);
      return Tng::resOkIA;
    }
  }
  if (!aArmor->HasPartOf(Tng::cSlotBody)) return Tng::resOkIA;
  aArmor->RemoveKeywords(fArmoKeys);
  if (lGens.size() > 0) {
    SKSE::log::warn("The armor [0x{:x}: {}] has armature on the body slot and also the genitlia slot without ARMO on slot 52. TNG would ignore it!", aArmor->GetFormID(), lID);
    aArmor->AddKeyword(fIAKey);
    return Tng::resOkIA;
  } else {
    aArmor->AddKeyword(fACKey);
    aArmor->AddSlotToMask(Tng::cSlotGenital);
    return Tng::resOkAC;
  }
}

bool Core::SwapRevealing(RE::TESObjectARMO* aArmor) noexcept {
  if (!(aArmor->HasKeyword(fACKey) || aArmor->HasKeyword(fARKey))) return false;
  if (aArmor->HasKeyword(fACKey)) {
    aArmor->RemoveKeyword(fACKey);
    TryMakeArmorRevealing(aArmor, false);
    Inis::SaveRevealingArmor(aArmor);
  } else {
    aArmor->RemoveKeyword(fARKey);
    TryMakeArmorCovering(aArmor, false);
    Inis::SaveCoveringArmor(aArmor);
  }
  return true;
}

bool Core::TryMakeArmorCovering(RE::TESObjectARMO* aArmor, bool aIsCC) noexcept {
  if (aArmor->HasKeywordInArray(fArmoKeys, false) && !aArmor->HasKeyword(fCCKey) && !aArmor->HasKeyword(fACKey)) return false;
  if (aIsCC && !aArmor->HasPartOf(Tng::cSlotGenital)) {
    auto lArmoPrimSlot = aArmor->HasPartOf(Tng::cSlotBody) ? Tng::cSlotBody : aArmor->GetSlotMask();
    for (auto& aa : aArmor->armorAddons)
      if (aa->HasPartOf(lArmoPrimSlot) && !aa->HasPartOf(Tng::cSlotGenital)) {
        fCAAs.insert(aa);
        fRAAs.erase(aa);
      }
  }
  if (!aArmor->HasKeywordInArray(fArmoKeys, false)) aArmor->AddKeyword(aIsCC ? fCCKey : fACKey);
  if (!aArmor->HasPartOf(Tng::cSlotGenital)) aArmor->AddSlotToMask(Tng::cSlotGenital);
  return true;
}

bool Core::TryMakeArmorRevealing(RE::TESObjectARMO* aArmor, bool aIsRR) noexcept {
  std::set<RE::TESObjectARMA*> lNewRs{};
  if (aArmor->HasKeywordInArray(fArmoKeys, false) && !aArmor->HasKeyword(fRRKey) && !aArmor->HasKeyword(fARKey) && !aArmor->HasKeywordString(Tng::cSOSR)) return false;
  for (const auto& aa : aArmor->armorAddons) {
    if (aa->HasPartOf(Tng::cSlotBody)) lNewRs.insert(aa);
    if (aa->HasPartOf(Tng::cSlotGenital)) return false;
  }
  for (auto& aa : lNewRs) {
    fRAAs.insert(aa);
    fCAAs.erase(aa);
  }
  if (aArmor->HasPartOf(Tng::cSlotGenital)) aArmor->RemoveSlotFromMask(Tng::cSlotGenital);
  if (!aArmor->HasKeywordInArray(fArmoKeys, false)) aArmor->AddKeyword(aIsRR ? fRRKey : fARKey);
  return true;
}

std::set<std::string> Core::GetSlot52Mods() noexcept { return fSlot52Mods; }
