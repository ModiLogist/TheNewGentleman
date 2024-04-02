#include <Base.h>
#include <Core.h>
#include <Inis.h>

bool Core::Initialize() noexcept {
  fDH = RE::TESDataHandler::GetSingleton();
  if (!fDH->LookupModByName(Tng::cName)) {
    Tng::gLogger::critical("Mod [{}] was not found! Please report this issue!", Tng::cName);
    return false;
  }
  fPRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  fRRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  fIRaceKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cIgnoredRaceKeyID, Tng::cName);
  fARKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  fRRKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  fACKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  fCCKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  fPAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cProblemArmoKeyID, Tng::cName);
  fIAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  fUAKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  fExKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cExcludeKeyID, Tng::cName);
  fNPCKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cNPCKeywID, Tng::cSkyrim);
  fCrtKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCrtKeywID, Tng::cSkyrim);
  fSwPKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  fGenSkinKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCustomSkinID, Tng::cName);
  fDefRace = fDH->LookupForm<RE::TESRace>(Tng::cDefRaceID, Tng::cSkyrim);
  fBeastDef = fDH->LookupForm<RE::TESRace>(Tng::cBstRaceID, Tng::cSkyrim);
  fAllNPCs = fDH->GetFormArray<RE::TESNPC>();
  if (!(fPRaceKey && fRRaceKey && fIRaceKey && fARKey && fRRKey && fACKey && fCCKey && fPAKey && fIAKey && fUAKey && fExKey && fNPCKey && fCrtKey && fSwPKey && fGenSkinKey &&
        fDefRace && fBeastDef)) {
    Tng::gLogger::critical("The original TNG information could not be found!");
    return false;
  }
  for (std::size_t i = Tng::raceManMer; i < Tng::raceSnowElf; i++) {
    fExSkins[i] = fDH->LookupForm<RE::TESObjectARMO>(cExSkinIDs[i], Tng::cName);
    if (!fExSkins) {
      Tng::gLogger::critical("Skins for excluded NPCs cannot be loaded.");
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
  Tng::gLogger::info("Finding the genitals for relevant races...");
  for (const auto& lRaceID : cExclRaceIDs) IgnoreRace(fDH->LookupForm<RE::TESRace>(lRaceID.first, lRaceID.second));
  auto& lAllRacesArray = fDH->GetFormArray<RE::TESRace>();
  int lPR = 0;
  int lIR = 0;
  int lRR = 0;
  int lPP = 0;
  for (const auto& lRace : lAllRacesArray) {
    if (lRace->HasKeyword(fIRaceKey)) {
      lIR++;
      continue;
    }
    if (lRace->HasKeyword(fRRaceKey)) {
      lRR++;
      continue;
    }
    if (lRace->HasKeyword(fPRaceKey)) {
      lPR++;
      continue;
    }
    if (CheckRace(lRace)) {
      switch (AddRace(lRace)) {
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
    auto lRacialSkin = Base::GetRaceGrpSkin(i);
    lRacialSkin->RemoveKeywords(fArmoKeys);
    lRacialSkin->AddKeyword(fIAKey);
    fSAAs.insert(lSkinAAs.begin(), lSkinAAs.end());
  }
  Tng::gLogger::info("Recognized assigned genitals to [{}] races, found [{}] races to be ready and ignored [{}] races.", lPR, lRR, lIR);
  if (lPP > 0) Tng::gLogger::error("Faced errors for [{}] races.", lPP);
}

bool Core::UpdateRaceGrpAddn(const std::size_t aRaceIdx, int aAddon) noexcept {
  int lChoice = aAddon >= 0 ? aAddon : Base::GetRaceGrpDefAddn(aRaceIdx);
  Base::UpdateRaceGrpAddn(aRaceIdx, lChoice);
  Inis::SaveRaceAddn(aRaceIdx, aAddon);
  return true;
}

bool Core::IgnoreRace(RE::TESRace* aRace) noexcept {
  bool lIsReady = false;
  if (aRace->skin) {
    aRace->skin->RemoveKeywords(fArmoKeys);
    aRace->skin->AddKeyword(fIAKey);
    for (const auto& lAA : aRace->skin->armorAddons) {
      if (lAA->HasPartOf(Tng::cSlotBody)) fSAAs.insert(lAA);
      if (lAA->HasPartOf(Tng::cSlotGenital)) lIsReady = true;
    }
  }
  aRace->AddKeyword(lIsReady ? fRRaceKey : fIRaceKey);
  return lIsReady;
}

bool Core::CheckRace(RE::TESRace* aRace) {
  if (!aRace->HasKeyword(fNPCKey) || aRace->HasKeyword(fCrtKey) || !aRace->HasPartOf(Tng::cSlotBody) || aRace->IsChildRace()) return false;
  if (aRace->GetFile(0) && Inis::fRaceExMods.size() > 0 && Inis::fRaceExMods.find(std::string{aRace->GetFile(0)->GetFilename()}) != Inis::fRaceExMods.end()) {
    Tng::gLogger::info("The race [{}: 0x{:x}: {}] was ignored because an ini excludes it!", aRace->GetFile(0)->GetFilename(), aRace->GetFormID(), aRace->GetFormEditorID());
    return false;
  }
  if (!Inis::IsValidSkeleton(aRace->skeletonModels[0].model.data()) || !Inis::IsValidSkeleton(aRace->skeletonModels[1].model.data())) {
    IgnoreRace(aRace);
    Tng::gLogger::info("The race [0x{:x}: {}] was ignored because it uses a custom skeleton!", aRace->GetFormID(), aRace->GetFormEditorID());
    return false;
  }
  if (!aRace->skin) {
    Tng::gLogger::warn("The race [0x{:x}: {}] cannot have any genitals since they do not have a skin.", aRace->GetFormID(), aRace->GetFormEditorID());
    IgnoreRace(aRace);
    return false;
  }
  RE::TESRace* lArmRace = aRace->armorParentRace ? aRace->armorParentRace : aRace;
  RE::TESObjectARMA* lRaceSkinAA{nullptr};
  for (const auto& lAA : aRace->skin->armorAddons) {
    std::set<RE::TESRace*> lAARaces{lAA->race};
    lAARaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
    if (lAA->HasPartOf(Tng::cSlotBody) && ((lAARaces.find(lArmRace) != lAARaces.end()) || (lAA->race == fDefRace))) {
      lRaceSkinAA = lAA;
      break;
    }
  }
  if (!lRaceSkinAA) {
    Tng::gLogger::warn("The race [0x{:x}: {}] cannot have any genitals since their skin cannot be recognized.", aRace->GetFormID(), aRace->GetFormEditorID());
    IgnoreRace(aRace);
    return false;
  }
  return true;
}

Tng::TNGRes Core::AddRace(RE::TESRace* aRace) noexcept {
  if (aRace->HasPartOf(Tng::cSlotGenital) || aRace->skin->HasPartOf(Tng::cSlotGenital)) {
    Tng::gLogger::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
    return IgnoreRace(aRace) ? Tng::resOkRaceR : Tng::raceErr;
  }
  for (const auto lAA : aRace->skin->armorAddons)
    if (lAA->HasPartOf(Tng::cSlotGenital)) {
      Tng::gLogger::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
      return IgnoreRace(aRace) ? Tng::resOkRaceR : Tng::raceErr;
    }
  auto lRaceIdx = Base::GetRaceGrp(aRace);
  if (aRace != Base::GetRaceByIdx(lRaceIdx)) {
    auto lRaceName = Base::GetRaceName(lRaceIdx);
    Tng::gLogger::info("The race [0x{:x}: {}] was recognized as {}.", aRace->GetFormID(), aRace->GetFormEditorID(), lRaceName);
  } else {
    Tng::gLogger::info("The race [0x{:x}: {}] was recognized as a new group.", aRace->GetFormID(), aRace->GetFormEditorID());
  }
  aRace->AddKeyword(fPRaceKey);
  aRace->AddSlotToMask(Tng::cSlotGenital);
  auto lSkin = aRace->skin;
  lSkin->RemoveKeywords(fArmoKeys);
  lSkin->AddKeyword(fIAKey);
  auto lRaceToPatch = aRace->armorParentRace ? aRace->armorParentRace : aRace;
  if (lRaceToPatch != fDefRace) Base::UpdateAddons(lRaceToPatch);
  return Tng::resOkRaceP;
}

RE::TESObjectARMO* Core::ProduceAddonSkin(RE::TESObjectARMO* aOgSkin, int aAddonChoice, bool aIsFemale) noexcept {
  auto& lSkins = aIsFemale ? fFemAddonSkins : fMalAddonSkins;
  if (auto lIt = lSkins.find(aOgSkin); (lIt != lSkins.end()) && lIt->second[aAddonChoice]) return lIt->second[aAddonChoice];
  RE::TESObjectARMO* lSkin = nullptr;
  lSkin = aOgSkin->CreateDuplicateForm(true, (void*)lSkin)->As<RE::TESObjectARMO>();
  lSkin->Copy(aOgSkin);
  lSkin->AddKeyword(fGenSkinKey);
  if (Base::GetAddonAt(aIsFemale, aAddonChoice)->HasKeyword(fSwPKey)) lSkin->AddKeyword(fSwPKey);
  aIsFemale ? Base::GentifyFemSkin(lSkin, aAddonChoice) : Base::GentifyMalSkin(lSkin, aAddonChoice);
  if (auto lIt = lSkins.find(aOgSkin); lIt != lSkins.end()) {
    lIt->second[aAddonChoice] = lSkin;
  } else {
    auto lInsert = lSkins.insert_or_assign(aOgSkin, std::vector<RE::TESObjectARMO*>(Base::GetAddonCount(aIsFemale)));
    lInsert.first->second[aAddonChoice] = lSkin;
  }
  return lSkin;
}

void Core::GenitalizeNPCSkins() noexcept {
  Tng::gLogger::info("Checking NPCs for custom skins.");
  std::set<RE::TESObjectARMO*> lSkinsToPatch{};
  std::map<std::string_view, int> lCustomSkinMods{};
  if (Inis::fHardExcluded.size() > 0) LoadHardExcluded();
  for (const auto& lNPC : fAllNPCs) {
    if (fHardExcludedNPCs.find(lNPC) != fHardExcludedNPCs.end()) continue;
    const auto lNPCRace = lNPC->race;
    if (!lNPCRace) {
      Tng::gLogger::warn("The NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", lNPC->GetFormID(), lNPC->GetName());
      continue;
    }
    if (!lNPCRace->HasKeyword(fPRaceKey) && !lNPCRace->HasKeyword(fRRaceKey)) continue;
    const auto lSkin = lNPC->skin;
    if (!lSkin) continue;
    if (lSkin->HasKeyword(fIAKey)) continue;
    lSkin->AddKeyword(fIAKey);
    if (!lSkin->HasPartOf(Tng::cSlotBody)) continue;
    if (lSkin->HasPartOf(Tng::cSlotGenital)) {
      Tng::gLogger::info("The skin [0x{:x}] used in NPC [{}] cannot have a male genital.", lSkin->GetFormID(), lNPC->GetName());
      continue;
    }
    lSkinsToPatch.insert(lSkin);
  }
  for (auto lSkin : lSkinsToPatch) {
    if (!FixSkin(lSkin, nullptr)) continue;
    auto lSkinName = lSkin->GetFile(0) ? lSkin->GetFile(0)->GetFilename() : "Other";
    auto lIt = lCustomSkinMods.insert({lSkinName, 0});
    lIt.first->second++;
  }
  if (lSkinsToPatch.size() > 0) {
    auto lTot = lSkinsToPatch.size();
    Tng::gLogger::info("\tHandled custom skins for {} NPCs from following mod(s):", lSkinsToPatch.size());
    for (const auto& lMod : lCustomSkinMods) {
      Tng::gLogger::info("\t\t[{}] skins from {}", lMod.second, lMod.first);
      lTot -= lMod.second;
    }
    Tng::gLogger::info("\t\t[{}] skins were not patched.", lTot);
  }
  for (const auto& lNPC : fAllNPCs)
    if (auto lAddn = Base::GetNPCAddn(lNPC); lAddn.second >= 0) SetNPCSkin(lNPC, lAddn.second, lAddn.first);
}

Tng::TNGRes Core::CanModifyActor(RE::Actor* aActor) noexcept {
  auto lRes = Base::CanModifyActor(aActor);
  if (lRes < 0) return lRes;
  auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!lNPC || !aActor) return Tng::npcErr;
  return (fHardExcludedNPCs.find(lNPC) != fHardExcludedNPCs.end()) ? Tng::npcErr : lRes;
}

Tng::TNGRes Core::SetNPCSkin(RE::TESNPC* aNPC, int aAddon, bool aIsUser) noexcept {
  auto lMaxRes = (aNPC->IsPlayer() && Inis::GetExcludePlayer()) ? Tng::resOkNoGen : Tng::resOkGen;
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
    Tng::gLogger::critical("Failed to get the previously created skin for an NPC!");
    return Tng::pgErr;
  }
  if (lOgSkin->armorAddons.size() == 0 || !lOgSkin->race) return Tng::skinErr;
  auto lActualAddon = aNPC->IsFemale() && !aIsUser ? Base::GetActualAddon(aAddon) : aAddon;
  auto lSkin = ProduceAddonSkin(lOgSkin, lActualAddon, aNPC->IsFemale());
  Base::SetNPCAddn(aNPC, aAddon, aIsUser);
  if (lSkin != lCurrSkin) {
    aNPC->skin = lSkin;
    if (!aNPC->IsPlayer() && aIsUser) Inis::SaveNPCAddn(aNPC, aAddon);
  }
  return !aNPC->IsFemale() || aNPC->skin->HasKeyword(fSwPKey) ? lMaxRes : Tng::resOkNoGen;
}

Tng::TNGRes Core::SetCharSize(RE::Actor* aActor, RE::TESNPC* aNPC, int aGenSize) noexcept {
  if (!aNPC->race) return Tng::raceErr;
  if (!(aNPC->race->HasKeyword(fPRaceKey) || aNPC->race->HasKeyword(fRRaceKey))) return Tng::raceErr;
  int lGenSize = (aNPC->IsPlayer() && Inis::GetExcludePlayer()) ? -2 : aGenSize;
  auto lRes = Base::SetCharSize(aActor, aNPC, lGenSize);
  if (!aNPC->IsPlayer() && lRes == Tng::resOkGen && lGenSize >= 0) Inis::SaveNPCSize(aNPC, lGenSize);
  return lRes;
}

RE::TESObjectARMO* Core::GetOgSkin(RE::TESNPC* aNPC) noexcept {
  auto lSkin = aNPC->skin ? aNPC->skin : aNPC->race->skin;
  if (!lSkin->HasKeyword(fGenSkinKey)) return lSkin;
  auto& lSkinsToLook = aNPC->IsFemale() ? fFemAddonSkins : fMalAddonSkins;
  for (const auto& lAddonEntry : lSkinsToLook)
    for (const auto& lAddnSkin : lAddonEntry.second)
      if (lAddnSkin == lSkin) return lAddonEntry.first;
  return nullptr;
}

bool Core::FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept {
  aSkin->RemoveKeywords(fArmoKeys);
  aSkin->AddKeyword(fIAKey);
  if (!aSkin->HasPartOf(Tng::cSlotBody)) return false;
  if (!aSkin->race) return false;
  if (aSkin->armorAddons.size() == 0) return false;
  if (aSkin->HasPartOf(Tng::cSlotGenital)) {
    if (aName) Tng::gLogger::info("The skin [0x{:x}: {}] is ready for TNG! It won't be modified.", aSkin->GetFormID(), aName);
    for (const auto& lAA : aSkin->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody)) fSAAs.insert(lAA);
    return true;
  }
  auto lSkinAAs = Base::GentifyMalSkin(aSkin);
  fSAAs.insert(lSkinAAs.begin(), lSkinAAs.end());
  if (aName) Tng::gLogger::info("The skin [0x{:x}: {}] added as extra skin.", aSkin->GetFormID(), aName);
  return true;
}

void Core::LoadHardExcluded() noexcept {
  for (const auto& lRecord : Inis::fHardExcluded) {
    auto lNPC = fDH->LookupForm<RE::TESNPC>(lRecord.second, lRecord.first);
    if (!lNPC) continue;
    Tng::gLogger::info("The NPC [{}] would be excluded from TNG.", lNPC->GetName());
    fHardExcludedNPCs.insert(lNPC);
    lNPC->AddKeyword(fExKey);
    if (lNPC->race) lNPC->skin = fExSkins[Base::GetRaceType(lNPC->race)];
  }
}

void Core::RevertNPCSkin(RE::TESNPC* aNPC) {
  if (fHardExcludedNPCs.find(aNPC) != fHardExcludedNPCs.end()) return;
  auto lOgSkin = GetOgSkin(aNPC);
  if (lOgSkin)
    aNPC->skin = lOgSkin == aNPC->race->skin ? nullptr : lOgSkin;
  else {
    Tng::gLogger::critical("Failed to revert the skin of [{}] to its original value!", aNPC->GetName());
  }
}

void Core::CheckArmorPieces() noexcept {
  Tng::gLogger::info("Checking ARMO records...");
  auto& lAllArmor = fDH->GetFormArray<RE::TESObjectARMO>();
  std::set<RE::TESObjectARMO*> lPotentialArmor;
  std::set<RE::TESObjectARMO*> lUnhandledArmor;
  std::set<RE::TESRace*> lArmoRaces;
  bool lCheckSkinMods = Inis::fSkinMods.size() > 0;
  bool lCheckSkinRecords = Inis::fSingleSkinIDs.size() > 0;
  bool lCheckRevealMods = Inis::fRevealingMods.size() > 0;
  bool lCheckRevealRecords = Inis::fSingleRevealingIDs.size() > 0;
  bool lCheckCoverRecords = Inis::fSingleCoveringIDs.size() > 0;
  bool lCheckRuntimeRecords = Inis::fRunTimeRevealingIDs.size() > 0;
  int lIA = lAllArmor.size();
  int lRR = 0;
  int lAR = 0;
  int lCC = 0;
  int lAC = 0;
  int lPA = 0;
  for (const auto& lModName : Inis::fSkinMods)
    if (fDH->LookupModByName(lModName)) Tng::gLogger::info("TheNewGentleman keeps an eye for [{}] as a skin mod.", lModName);
  for (const auto& lRevealMod : Inis::fRevealingMods)
    if (fDH->LookupModByName(lRevealMod)) Tng::gLogger::info("TheNewGentleman keeps an eye for [{}] as a revealing armor mod.", lRevealMod);
  for (const auto& lArmor : lAllArmor) {
    if (!lArmor) continue;
    if (lArmor->armorAddons.size() == 0) lArmor->AddKeyword(fIAKey);
    const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
    if (!lArmor->race) {
      Tng::gLogger::warn("The armor [0x{:x}: {}] does not have a race! It won't be touched by Tng!", lArmor->GetFormID(), lID);
      lArmor->AddKeyword(fIAKey);
      continue;
    }
    if (!(lArmor->race->HasKeyword(fPRaceKey) || lArmor->race->HasKeyword(fRRaceKey) || lArmor->race == fDefRace)) continue;
    if (lArmor->HasKeyword(fIAKey) || lArmor->HasKeyword(fUAKey)) continue;
    auto lFN = lArmor->GetFile(0) ? lArmor->GetFile(0)->GetFilename() : "NoFile";
    if ((lCheckSkinMods && Inis::fSkinMods.find(std::string{lFN}) != Inis::fSkinMods.end()) ||
        (lCheckSkinRecords && Inis::fSingleSkinIDs.find(std::make_pair(std::string{lFN}, lArmor->GetLocalFormID())) != Inis::fSingleSkinIDs.end())) {
      if (FixSkin(lArmor, lID)) {
        Tng::gLogger::info("The skin [{:x}] from file [{}] was patched.", lArmor->GetLocalFormID(), lFN);
      } else {
        Tng::gLogger::error("The skin [{:x}] from file [{}] has issues. It was not touched by TNG.", lArmor->GetLocalFormID(), lFN);
      }
      continue;
    }
    lIA--;
    if ((lCheckCoverRecords && Inis::fSingleCoveringIDs.find(std::make_pair(std::string{lFN}, lArmor->GetLocalFormID())) != Inis::fSingleCoveringIDs.end()) ||
        lArmor->HasKeyword(fCCKey)) {
      TryMakeArmorCovering(lArmor, true);
      Tng::gLogger::info("The armor [0x{:x}: {}] was marked covering.", lArmor->GetFormID(), lID);
      lCC++;
      continue;
    }
    if ((lCheckRevealMods && Inis::fRevealingMods.find(std::string{lFN}) != Inis::fRevealingMods.end()) ||
        (lCheckRevealRecords && Inis::fSingleRevealingIDs.find(std::make_pair(std::string{lFN}, lArmor->GetLocalFormID())) != Inis::fSingleRevealingIDs.end()) ||
        lArmor->HasKeywordString(Tng::cSOSR) || lArmor->HasKeyword(fRRKey)) {
      if (lArmor->HasPartOf(Tng::cSlotBody)) {
        if (TryMakeArmorRevealing(lArmor, true)) {
          Tng::gLogger::info("Armor [0x{:x}: {}] was marked revealing.", lArmor->GetFormID(), lID);
          lRR++;
        } else {
          Tng::gLogger::error("The armor [0x{:x}: {}] was assigned revealing but it has arma on slot 52!", lArmor->GetFormID(), lID);
          lPA++;
        }
      } else {
        lIA++;
      }
      continue;
    }
    if (lCheckRuntimeRecords && Inis::fRunTimeRevealingIDs.find(std::make_pair(std::string{lFN}, lArmor->GetLocalFormID())) != Inis::fRunTimeRevealingIDs.end()) {
      if (TryMakeArmorRevealing(lArmor, false)) {
        Tng::gLogger::info("The armor [[0x{:x}: {}] was marked revealing since it was previously marked revealing during gameplay.", lArmor->GetFormID(), lID);
        lAR++;
      } else {
        Tng::gLogger::warn("The armor [0x{:x}: {}] was marked revealing in a previous gameplay but now it must be covering!", lArmor->GetFormID(), lID);
        lPA++;
      }
      continue;
    }
    if (lArmor->HasPartOf(Tng::cSlotGenital) && !lArmor->HasKeyword(fUAKey) && !lArmor->HasKeyword(fIAKey)) {
      lArmor->AddKeyword(fIAKey);
      Tng::gLogger::warn("The armor [0x{:x}: {}] would cover genitals and would have a conflict with non-revealing chest armor pieces!", lArmor->GetFormID(), lID);
      lIA++;
      continue;
    }
    lPotentialArmor.insert(lArmor);
  }
  for (auto& lAA : fRAAs) {
    if (fCAAs.find(lAA) != fCAAs.end()) {
      Tng::gLogger::error("The armor addon [:x] is shared between revealing and covering armor! Any armor using that addon would be covering.");
      fRAAs.erase(lAA);
    }
  }
  for (auto& lArmor : lPotentialArmor) {
    switch (HandleArmor(lArmor)) {
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
        lArmor->AddKeyword(fPAKey);
        break;
    }
  }
  for (auto& lAA : fCAAs) {
    if (fRAAs.find(lAA) != fRAAs.end() || fSAAs.find(lAA) != fSAAs.end()) {
      Tng::gLogger::warn("The armor addon [0x{:x}] is also part of a skin/revealing armor! Any armor using that addon would be revealing.", lAA->GetFormID());
      fCAAs.erase(lAA);
      continue;
    }
    lAA->AddSlotToMask(Tng::cSlotGenital);
    if (lAA->data.priorities[0] == 0 || lAA->data.priorities[1] == 0)
      Tng::gLogger::warn("The armor addon [0x{:x}] might have wrong priorities. This can cause genital clipping through.", lAA->GetFormID());
  }
  Tng::gLogger::info("Processed [{}] armor pieces:", lAllArmor.size());
  if (lPA > 0) Tng::gLogger::warn("\t[{}]: seems to be problematic!", lPA);
  if (lCC > 0) Tng::gLogger::info("\t[{}]: are marked covering,", lCC);
  if (lRR > 0) Tng::gLogger::info("\t[{}]: are marked revealing", lRR);
  if (lAC > 0) Tng::gLogger::info("\t[{}]: were recognized to be covering", lAC);
  if (lAR > 0) Tng::gLogger::info("\t[{}]: were recognized to be revealing", lAR);
  if (lIA > 0) Tng::gLogger::info("\tThe rest [{}] are not relevant and are ignored!", lIA);
}

Tng::TNGRes Core::HandleArmor(RE::TESObjectARMO* aArmor, const bool aIfLog) noexcept {
  const auto lID = (std::string(aArmor->GetName()).empty()) ? aArmor->GetFormEditorID() : aArmor->GetName();
  std::set<RE::TESObjectARMA*> lBods{};
  std::set<RE::TESObjectARMA*> lGens{};
  bool lS = false;
  bool lR = false;
  bool lC = false;
  for (const auto& lAA : aArmor->armorAddons) {
    if (fSAAs.find(lAA) != fSAAs.end()) lS = true;
    if (fRAAs.find(lAA) != fRAAs.end()) lR = true;
    if (fCAAs.find(lAA) != fCAAs.end() && lAA->HasPartOf(Tng::cSlotGenital)) lC = true;
    if (lAA->HasPartOf(Tng::cSlotBody)) lBods.insert(lAA);
    if (lAA->HasPartOf(Tng::cSlotGenital) && fCAAs.find(lAA) == fCAAs.end()) lGens.insert(lAA);
  }
  if ((lR || lS) && lC) {
    if (aIfLog) Tng::gLogger::warn("The armor [0x{:x}: {}] uses both covering and revealing armature at the same time!", aArmor->GetFormID(), lID);
    aArmor->RemoveKeywords(fArmoKeys);
    aArmor->AddKeyword(fPAKey);
    return Tng::armoErr;
  }
  if (lR || lS) {
    if (TryMakeArmorRevealing(aArmor, false)) {
      Tng::gLogger::info("The armor [0x{:x}: {}] is markerd revealing since it has {} armature!", aArmor->GetFormID(), lID, lR ? "revealing armor" : "full body");
      return Tng::resOkAR;
    } else {
      Tng::gLogger::warn("The armor [0x{:x}: {}] has revealing armature but also armature on slot 52!", aArmor->GetFormID(), lID);
      return Tng::resOkIA;
    }
  }
  if (lC) {
    if (lGens.size() == 0) {
      TryMakeArmorCovering(aArmor, false);
      Tng::gLogger::info("The armor [0x{:x}: {}] uses covering armature from another armor and is marked covering.", aArmor->GetFormID(), lID);
      return Tng::resOkAC;
    } else {
      Tng::gLogger::warn("The armor [0x{:x}: {}] uses covering armature from another armor but it also has items on genitlia slot!", aArmor->GetFormID(), lID);
      aArmor->AddKeyword(fIAKey);
      return Tng::resOkIA;
    }
  }
  if (!aArmor->HasPartOf(Tng::cSlotBody)) return Tng::resOkIA;
  aArmor->RemoveKeywords(fArmoKeys);
  if (lGens.size() > 0) {
    Tng::gLogger::warn("The armor [0x{:x}: {}] has armature on the body slot and also the genitlia slot!", aArmor->GetFormID(), lID);
    aArmor->AddKeyword(fIAKey);
    return Tng::resOkIA;
  } else {
    aArmor->AddKeyword(fACKey);
    aArmor->AddSlotToMask(Tng::cSlotGenital);
    fCAAs.insert(lBods.begin(), lBods.end());
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
    Inis::RemoveRevealingArmor(aArmor);
  }
  return true;
}

bool Core::TryMakeArmorCovering(RE::TESObjectARMO* aArmor, bool aIsCC) noexcept {
  if (aArmor->HasKeywordInArray(fArmoKeys, false) && !aArmor->HasKeyword(fCCKey) && !aArmor->HasKeyword(fACKey)) return false;
  if (aArmor->HasPartOf(Tng::cSlotGenital)) {
    aArmor->RemoveKeywords(fArmoKeys);
    aArmor->AddKeyword(fIAKey);
    return false;
  }
  auto lArmoPrimSlot = aArmor->HasPartOf(Tng::cSlotBody) ? Tng::cSlotBody : aArmor->GetSlotMask();
  for (auto& lAA : aArmor->armorAddons)
    if (lAA->HasPartOf(lArmoPrimSlot) && !lAA->HasPartOf(Tng::cSlotGenital)) {
      lAA->AddSlotToMask(Tng::cSlotGenital);
      fCAAs.insert(lAA);
      fRAAs.erase(lAA);
    }
  if (!aArmor->HasKeywordInArray(fArmoKeys, false)) aArmor->AddKeyword(aIsCC ? fCCKey : fACKey);
  if (!aArmor->HasPartOf(Tng::cSlotGenital)) aArmor->AddSlotToMask(Tng::cSlotGenital);
  return true;
}

bool Core::TryMakeArmorRevealing(RE::TESObjectARMO* aArmor, bool aIsRR) noexcept {
  std::set<RE::TESObjectARMA*> lNewRs{};
  if (aArmor->HasKeywordInArray(fArmoKeys, false) && !aArmor->HasKeyword(fRRKey) && !aArmor->HasKeyword(fARKey) && !aArmor->HasKeywordString(Tng::cSOSR)) return false;
  for (const auto& lAA : aArmor->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotBody)) lNewRs.insert(lAA);
    if (lAA->HasPartOf(Tng::cSlotGenital)) return false;
  }
  for (auto& lAA : lNewRs) {
    fRAAs.insert(lAA);
    fCAAs.erase(lAA);
  }
  if (aArmor->HasKeyword(fACKey)) aArmor->RemoveKeyword(fACKey);
  if (aArmor->HasPartOf(Tng::cSlotGenital)) aArmor->RemoveSlotFromMask(Tng::cSlotGenital);
  if (!aArmor->HasKeywordInArray(fArmoKeys, false)) aArmor->AddKeyword(aIsRR ? fRRKey : fARKey);
  return true;
}
