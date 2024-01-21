#include <TngCore.h>
#include <TngInis.h>
#include <TngSizeShape.h>

bool TngCore::Initialize() noexcept {
  auto fDH = RE::TESDataHandler::GetSingleton();
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
  fNPCKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cNPCKeywID, Tng::cSkyrim);
  fBstKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cBstKeywID, Tng::cSkyrim);
  fCrtKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCrtKeywID, Tng::cSkyrim);
  fDefRace = fDH->LookupForm<RE::TESRace>(cDefRaceID, Tng::cSkyrim);
  fAllNPCs = fDH->GetFormArray<RE::TESNPC>();
  if (!(fPRaceKey && fRRaceKey && fIRaceKey && fARKey && fRRKey && fACKey && fCCKey && fPAKey && fIAKey && fUAKey && fNPCKey && fBstKey && fCrtKey && fDefRace)) {
    Tng::gLogger::critical("The original TNG information could not be found!");
    return false;
  }
  for (int i = 0; i < cVanillaRaceTypes; i++) fBaseRaces[i] = fDH->LookupForm<RE::TESRace>(cBaseRaceIDs[i].first, cBaseRaceIDs[i].second);
  for (int i = 0; i < cEqRaceTypes; i++) fEqRaces[i] = fDH->LookupForm<RE::TESRace>(cEquiRaceIDs[i].first.first, cEquiRaceIDs[i].first.second);

  return true;
}

void TngCore::GenitalizeRaces() noexcept {
  Tng::gLogger::info("Finding the genitals to for installed races...");
  for (int i = 0; i < cVanillaRaceTypes; i++) HandleVanillaRace(fBaseRaces[i], cVanillaRaceDefaults[i]);
  for (int i = 0; i < cEqRaceTypes; i++) HandleVanillaRace(fEqRaces[i], cVanillaRaceDefaults[cEquiRaceIDs[i].second]);
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
    if (CheckRace(lRace)) switch (AddRace(lRace)) {
        case 0:
          lPR++;
        case 1:
          lRR++;
        case -1:
          lIR++;
        case -2:
          lPP++;
        default:
          Tng::gLogger::critical("There is a mistake in the programming for races! Please report this.");
      }
  }
  Tng::gLogger::info("TNG recognized assigned genitals to [{}] races, found [{}] races to be ready and ignored [{}] races.", lPR, lRR, lIR);
  if (lPP > 0) Tng::gLogger::error("TNG faced errors for [{}] races.", lPP);
}

bool TngCore::UpdateRaces(const std::size_t aRaceIdx, int aGenOption) noexcept {
  auto lRaces = TngSizeShape::GetRacesByIdx(aRaceIdx);
  for (const auto& lRace : lRaces) {
    if (!lRace->HasKeyword(fPRaceKey)) return false;
    RE::TESObjectARMO* lGenital{nullptr};
    int lChoice = aGenOption;
    if (lChoice < 0) {
      for (int i = 0; (i < cVanillaRaceTypes && lChoice < 0); i++)
        if (lRace == fBaseRaces[i]) lChoice = cVanillaRaceDefaults[i];
      for (int i = 0; i < cEqRaceTypes && lChoice < 0; i++)
        if (lRace == fEqRaces[i]) lChoice = cVanillaRaceDefaults[cEquiRaceIDs[i].second];
      if (lChoice < 0) lChoice = 0;
    }
    lGenital = TngSizeShape::GetAddonAt(false, lChoice);
    if (!lGenital) return false;
    auto lSkin = lRace->skin;
    if (!lSkin) return false;
    for (auto i = lSkin->armorAddons.begin(); i != lSkin->armorAddons.end(); i++)
      if ((*i)->HasPartOf(Tng::cSlotGenital)) lSkin->armorAddons.erase(i);
    AddGenitalToSkin(lSkin, UpdateEqRaceAddon(lRace, lGenital));
    TngSizeShape::SetRaceShape(lRace, aGenOption);
  }
  TngInis::SaveRaceShape(aRaceIdx, aGenOption);
  return true;
}

bool TngCore::IgnoreRace(RE::TESRace* aRace) noexcept {
  if (aRace->skin) {
    aRace->skin->AddKeyword(fIAKey);
    for (const auto& lAA : aRace->skin->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotGenital)) {
        aRace->AddKeyword(fRRaceKey);
        return true;
      }
  }
  aRace->AddKeyword(fIRaceKey);
  return false;
}

void TngCore::HandleVanillaRace(RE::TESRace* aRace, const int aDefaultChoice) noexcept {
  if (!(aRace)) {
    Tng::gLogger::critical("An original game race cannot be found!");
    return;
  }
  auto lChoice = TngSizeShape::GetRaceShape(aRace) > -1 ? TngSizeShape::GetRaceShape(aRace) : aDefaultChoice;
  const auto lGenital = TngSizeShape::GetAddonAt(false, lChoice);
  if (!(lGenital)) {
    Tng::gLogger::critical("Original genitalia from TNG cannot be found!");
    return;
  }
  TngSizeShape::SetRaceShape(aRace, lChoice);
  AddGenitalToSkin(aRace->skin, UpdateEqRaceAddon(aRace, lGenital));
}

RE::TESObjectARMA* TngCore::UpdateEqRaceAddon(RE::TESRace* aRace, RE::TESObjectARMO* aGenital) noexcept {
  const auto lRaceDesc = std::string(aRace->GetFormEditorID()) + std::string(aRace->GetName());
  int lEqRace = -1;
  for (int i = 0; i < cVanillaRaceTypes; i++) {
    if (lRaceDesc.contains(cRaceNames[i][0]) || lRaceDesc.contains(cRaceNames[i][1])) {
      lEqRace = i;
      break;
    }
  }
  if (lEqRace == -1) return nullptr;
  if ((lEqRace == 9 || lEqRace == 8) != aRace->HasKeyword(fBstKey)) {
    return nullptr;
  }
  auto lGenRace = fDH->LookupForm<RE::TESRace>(cBaseRaceIDs[lEqRace].first, cBaseRaceIDs[lEqRace].second);
  RE::TESObjectARMA* lGenitalAA{nullptr};
  for (const auto& lAA : aGenital->armorAddons) {
    std::set<RE::TESRace*> lAARaces{lAA->race};
    if (lAA->additionalRaces.size() > 0) lAARaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
    if (lAARaces.find(lGenRace) != lAARaces.end()) {
      if (lAARaces.find(aRace) != lAARaces.end()) return lAA;
      lGenitalAA = lAA;
      break;
    }
  }
  if (lGenitalAA) lGenitalAA->additionalRaces.emplace_back(aRace->armorParentRace ? aRace->armorParentRace : aRace);
  return lGenitalAA;
}

bool TngCore::CheckRace(RE::TESRace* aRace) {
  RE::BSFixedString lDefSks[2]{fDefRace->skeletonModels[0].model, fDefRace->skeletonModels[1].model};
  if (!aRace->HasKeyword(fNPCKey) || aRace->HasKeyword(fCrtKey) || !aRace->HasPartOf(Tng::cSlotBody) || aRace->IsChildRace()) return false;
  if (!TngInis::IsValidSkeleton(aRace->skeletonModels[0].model, lDefSks) || !TngInis::IsValidSkeleton(aRace->skeletonModels[1].model, lDefSks)) {
    IgnoreRace(aRace);
    Tng::gLogger::info("The race [0x{:x}:{}] from file [{}] was ignored because it uses a custom skeleton!", aRace->GetFormID(), aRace->GetFormEditorID(),
                       aRace->GetFile(0)->GetFilename());
    return false;
  }
  if (!aRace->skin) {
    Tng::gLogger::warn("The race [0x{:x}:{}] from file [{}] cannot have any genitals since they do not have a skin. If they should, a patch is required.", aRace->GetFormID(),
                       aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
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
    Tng::gLogger::warn("The race [0x{:x}:{}] from file [{}] cannot have any genitals since their skin cannot be recognized. If they should, a patch is required.",
                       aRace->GetFormID(), aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
    IgnoreRace(aRace);
    return false;
  }
  if (aRace->GetPlayable()) return true;
  bool lHasMaleNPCs = false;
  bool lHasNPCs = false;

  for (const auto& lNPC : fAllNPCs) {
    if (lNPC->race == aRace) {
      if (lNPC->IsFemale())
        lHasNPCs = true;
      else
        lHasMaleNPCs = true;
      if (lHasMaleNPCs) break;
    }
  }
  bool lAdd = lHasMaleNPCs || (!lHasNPCs);
  if (!lAdd) {
    IgnoreRace(aRace);
    Tng::gLogger::info("The race [0x{:x}:{}] from file [{}] has only female NPC references and ignored by TNG.", aRace->GetFormID(), aRace->GetFormEditorID(),
                       aRace->GetFile(0)->GetFilename());
  }
  return lAdd;
}

bool TngCore::UpdateAddonsForRace(RE::TESRace* aRace, const int aChoice) noexcept {
  int lAddCount = TngSizeShape::GetAddonCount(false);
  RE::TESObjectARMA* lRes{nullptr};
  for (int i = 0; i < lAddCount; i++) {
    auto lGenital = TngSizeShape::GetAddonAt(false, i);
    if (!lGenital) {
      Tng::gLogger::critical("The previously loaded addons cannot be accessed!");
      continue;
    }
    auto lAA = UpdateEqRaceAddon(aRace, lGenital);
    if (lAA && !lRes) lRes = lAA;
    if (i == aChoice && lAA) lRes = lAA;
  }
  if (!lRes) return false;
  TngSizeShape::SetRaceShape(aRace, aChoice);
  AddGenitalToSkin(aRace->skin, lRes);
  return true;
}

char TngCore::AddRace(RE::TESRace* aRace) noexcept {
  if (aRace->HasPartOf(Tng::cSlotGenital)) {
    Tng::gLogger::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
    return IgnoreRace(aRace) ? 1 : -1;
  }
  int lRaceIdx = -1;
  if (aRace->HasKeyword(fBstKey)) {
    const auto lRaceDesc = std::string(aRace->GetFormEditorID()) + std::string(aRace->GetName());

    if ((lRaceDesc.contains(cRaceNames[9][0])) || (lRaceDesc.contains(cRaceNames[9][1]))) lRaceIdx = 9;
    if ((lRaceDesc.contains(cRaceNames[8][0])) || (lRaceDesc.contains(cRaceNames[8][1]))) lRaceIdx = 8;
  } else {
    lRaceIdx = 0;
  }
  if (lRaceIdx > -1) {
    int aChoice = TngSizeShape::GetRaceShape(aRace) > -1 ? TngSizeShape::GetRaceShape(aRace) : cVanillaRaceDefaults[lRaceIdx];
    if (!UpdateAddonsForRace(aRace->armorParentRace ? aRace->armorParentRace : aRace, aChoice)) {
      Tng::gLogger::critical("Original genitalia from TNG or an addon cannot be found!");
      return -2;
    }
    aRace->AddSlotToMask(Tng::cSlotGenital);
    aRace->AddKeyword(fPRaceKey);
    Tng::gLogger::info("The race [0x{:x}:{}] from file [{}] was recognized as {}. If this is wrong, a patch is required.", aRace->GetFormID(), aRace->GetFormEditorID(),
                       cRaceNames[lRaceIdx][0], aRace->GetFile(0)->GetFilename());
    return 0;
  } else {
    Tng::gLogger::warn("The race [0x{:x}:{}] from file [{}] could not be recognized and did not receive any genital. If they should, a patch is required.", aRace->GetFormID(),
                       aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
    IgnoreRace(aRace);
    return -1;
  }
}

void TngCore::AddGenitalToSkin(RE::TESObjectARMO* aSkin, RE::TESObjectARMA* aGenital) noexcept {
  if (!aGenital) return;
  for (const auto& lAA : aSkin->armorAddons)
    if (lAA == aGenital) return;
  for (const auto lAA : aSkin->armorAddons)
    if (lAA->HasPartOf(Tng::cSlotBody)) fSAAs.insert(lAA);
  if (!aSkin->HasPartOf(Tng::cSlotGenital)) aSkin->AddSlotToMask(Tng::cSlotGenital);
  aSkin->armorAddons.emplace_back(aGenital);
}

void TngCore::GenitalizeNPCSkins() noexcept {
  Tng::gLogger::info("Checking NPCs for custom skins.");
  int lC = 0;
  std::set<std::pair<std::string_view, int>> lCustomSkinMods{};
  for (const auto& lNPC : fAllNPCs) {
    const auto lNPCRace = lNPC->race;
    if (!lNPCRace) {
      Tng::gLogger::warn("The NPC [{}] from file [{}] does not have a race! They cannot be modified by TNG.", lNPC->GetName(), lNPC->GetFile(0)->GetFilename());
      continue;
    }
    if (!lNPCRace->HasKeyword(fPRaceKey) && !lNPCRace->HasKeyword(fRRaceKey)) continue;
    const auto lSkin = lNPC->skin;
    if (!lSkin) continue;
    if (lSkin->HasKeyword(fIAKey)) continue;
    if (!lSkin->HasPartOf(Tng::cSlotBody)) continue;
    if (lSkin->HasPartOf(Tng::cSlotGenital)) {
      Tng::gLogger::info("The skin [0x{:x}] used in NPC [{}] from file [{}] cannot have a male genital. If this is wrong, a patch is required.", lSkin->GetFormID(),
                         lNPC->GetName(), lNPC->GetFile(0)->GetFilename());
      continue;
    }
    if (FixSkin(lSkin, nullptr)) {
      auto lFoundMod =
          std::find_if(lCustomSkinMods.begin(), lCustomSkinMods.end(), [&lSkin](const std::pair<std::string_view, int>& p) { return p.first == lSkin->GetFile(0)->GetFilename(); });
      if (lFoundMod == lCustomSkinMods.end()) {
        lCustomSkinMods.insert(std::make_pair(lSkin->GetFile(0)->GetFilename(), 1));
      } else {
        lCustomSkinMods.insert(std::make_pair(lSkin->GetFile(0)->GetFilename(), (*lFoundMod).second + 1));
        lCustomSkinMods.erase(lFoundMod);
      }
      lC++;
      continue;
    }
    Tng::gLogger::info("The NPC [{}] from the race [{}] does not have registered racial genitals. If they should, a patch is required.", lNPC->GetName(),
                       lNPC->GetRace()->GetFormEditorID());
  }
  if (lC > 0) {
    Tng::gLogger::info("\tTNG handled custom skins for {} NPCs from following mod(s):", lC);
    for (const auto& lMod : lCustomSkinMods) Tng::gLogger::info("\t\t[{}] skins from {}", lMod.second, lMod.first);
  }
}

bool TngCore::FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept {
  if (!aSkin->HasPartOf(Tng::cSlotBody)) return false;
  if (aSkin->HasPartOf(Tng::cSlotGenital)) {
    if (aName) Tng::gLogger::info("The skin [0x{:x}:{}] from file [{}] is ready for TNG!", aSkin->GetFormID(), aName, aSkin->GetFile(0)->GetFilename());
    aSkin->AddKeyword(fIAKey);
    for (const auto& lAA : aSkin->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody)) fSAAs.insert(lAA);
    return true;
  }
  std::set<RE::TESRace*> lSkinRaces;
  for (const auto& lAA : aSkin->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotBody)) fSAAs.insert(lAA);
    lSkinRaces.insert(lAA->race);
    lSkinRaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
  }
  for (const auto& lRace : lSkinRaces) {
    if (!lRace->HasKeyword(fPRaceKey)) continue;
    auto lGenID = TngSizeShape::GetRaceShape(lRace);
    if (lGenID < 0) {
      Tng::gLogger::error("There is an error loading the genital for skin [0x{:x}:{}] from file [{}]!", aSkin->GetFormID(), aName, aSkin->GetFile(0)->GetFilename());
      return false;
    }
    auto lShape = TngSizeShape::GetAddonAt(false, lGenID);
    if (!lShape) {
      Tng::gLogger::error("There is an error loading the genital for skin [0x{:x}:{}] from file [{}]!", aSkin->GetFormID(), aName, aSkin->GetFile(0)->GetFilename());
      return false;
    }
    AddGenitalToSkin(aSkin, UpdateEqRaceAddon(lRace, lShape));
  }
  if (aName) Tng::gLogger::info("The skin [0x{:x}:{}] from file [{}] added as extra skin.", aSkin->GetFormID(), aName, aSkin->GetFile(0)->GetFilename());
  return true;
}

void TngCore::CheckArmorPieces() noexcept {
  Tng::gLogger::info("Checking ARMO records...");
  auto& lAllArmor = fDH->GetFormArray<RE::TESObjectARMO>();
  std::set<RE::TESObjectARMO*> lPotentialArmor;
  std::set<RE::TESObjectARMO*> lUnhandledArmor;
  std::set<RE::TESRace*> lArmoRaces;
  bool lCheckSkinMods = TngInis::fSkinMods.size() > 0;
  bool lCheckSkinRecords = TngInis::fSingleSkinIDs.size() > 0;
  bool lCheckRevealMods = TngInis::fRevealingMods.size() > 0;
  bool lCheckRevealRecords = TngInis::fSingleRevealingIDs.size() > 0;
  bool lCheckCoverRecords = TngInis::fSingleCoveringIDs.size() > 0;
  bool lCheckRuntimeRecords = TngInis::fRunTimeRevealingIDs.size() > 0;
  int lRR = 0;
  int lAR = 0;
  int lCC = 0;
  int lAC = 0;
  int lPA = 0;

  for (const auto& lModName : TngInis::fSkinMods)
    if (fDH->LookupModByName(lModName)) Tng::gLogger::info("TheNewGentleman keeps an eye for [{}] as a skin mod.", lModName);
  for (const auto& lRevealMod : TngInis::fRevealingMods)
    if (fDH->LookupModByName(lRevealMod)) Tng::gLogger::info("TheNewGentleman keeps an eye for [{}] as a revealing armor mod.", lRevealMod);

  for (const auto& lArmor : lAllArmor) {
    if (lArmor->HasKeyword(fIAKey)) continue;
    if (lArmor->armorAddons.size() == 0) continue;
    const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
    if (!lArmor->race) {
      Tng::gLogger::warn("The armor [0x{:x}:{}] from file [{}] does not have a race! It won't be touched by Tng!", lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
      lArmor->AddKeyword(fIAKey);
      continue;
    }
    if (!(lArmor->race->HasKeyword(fPRaceKey) || lArmor->race->HasKeyword(fRRaceKey) || (lArmor->race != fDefRace))) continue;
    if (lCheckSkinMods && (TngInis::fSkinMods.find(std::string{lArmor->GetFile(0)->GetFilename()}) != TngInis::fSkinMods.end())) {
      FixSkin(lArmor, lID);
      continue;
    }
    if (lCheckSkinRecords) {
      const auto lSkinEntry = TngInis::fSingleSkinIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->GetLocalFormID()));
      if (lSkinEntry != TngInis::fSingleSkinIDs.end()) {
        FixSkin(lArmor, lID);
        continue;
      }
    }
    if (lCheckCoverRecords) {
      const auto lCoverEntry = TngInis::fSingleCoveringIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->GetLocalFormID()));
      if (lArmor->HasPartOf(Tng::cSlotBody)) {
        Tng::gLogger::warn("The armor [0x{:x}:{}] from file [{}] is marked covering by an ini but it has slot 32 and would be handled automatically.", lArmor->GetFormID(), lID,
                           lArmor->GetFile(0)->GetFilename());
        continue;
      }
      if (lCoverEntry != TngInis::fSingleCoveringIDs.end()) {
        CoverByArmor(lArmor);
        continue;
      }
    }
    if (lCheckRevealMods && (TngInis::fRevealingMods.find(std::string{lArmor->GetFile(0)->GetFilename()}) != TngInis::fRevealingMods.end())) {
      if (lArmor->HasPartOf(Tng::cSlotBody)) {
        lArmor->AddKeyword(fRRKey);
        HandleArmor(lArmor);
        if (lArmor->HasKeyword(fPAKey))
          lPA++;
        else
          lRR++;
      }
      continue;
    }
    if (lCheckRevealRecords) {
      const auto lRevealEntry = TngInis::fSingleRevealingIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->GetLocalFormID()));
      if (lRevealEntry != TngInis::fSingleRevealingIDs.end()) {
        lArmor->AddKeyword(fRRKey);
        HandleArmor(lArmor);
        if (lArmor->HasKeyword(fPAKey))
          lPA++;
        else
          lRR++;
        continue;
      }
    }
    if (lCheckRuntimeRecords) {
      const auto lRevealEntry = TngInis::fRunTimeRevealingIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->GetLocalFormID()));
      if (lRevealEntry != TngInis::fRunTimeRevealingIDs.end()) {
        lArmor->AddKeyword(fARKey);
        HandleArmor(lArmor);
        if (lArmor->HasKeyword(fPAKey))
          lPA++;
        else
          lRR++;
        continue;
      }
    }
    if (lArmor->HasPartOf(Tng::cSlotGenital) && !lArmor->HasKeyword(fUAKey)) {
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] would cover genitals and be covered by chest armor pieces (like an underwear). If it is wrong, a patch is required.",
                         lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
      lCC++;
      continue;
    }
    if (lArmor->HasPartOf(Tng::cSlotBody)) lPotentialArmor.insert(lArmor);
  }
  for (const auto& lArmor : lPotentialArmor) {
    HandleArmor(lArmor);
    if (lArmor->HasKeyword(fRRKey)) lRR++;
    if (lArmor->HasKeyword(fARKey)) lAR++;
    if (lArmor->HasKeyword(fACKey)) lAC++;
    if (lArmor->HasKeyword(fCCKey)) lCC++;
    if (lArmor->HasKeyword(fPAKey)) lPA++;
  }
  Tng::gLogger::info("Processed {} armor pieces with slot 32 or 52:", lRR + lAR + lAC + lCC + lPA);
  if (lPA > 0) Tng::gLogger::warn("\t{}: seems to be problematic!", lPA);
  if (lCC > 0) Tng::gLogger::info("\t{}: are already covering genitals,", lCC);
  if (lRR > 0) Tng::gLogger::info("\t{}: are already revealing", lRR);
  if (lAC > 0) Tng::gLogger::info("\t{}: were updated to cover genitals", lAC);
  if (lAR > 0) Tng::gLogger::info("\t{}: were updated to be revealing", lAR);
}

void TngCore::HandleArmor(RE::TESObjectARMO* aArmor, const bool aIfLog) noexcept {
  std::set<RE::TESObjectARMA*> lBods{};
  std::set<RE::TESObjectARMA*> lGens{};
  RE::TESObjectARMA* lBod{nullptr};
  RE::TESObjectARMA* lGen{nullptr};
  for (const auto& lAA : aArmor->armorAddons) {
    if (fSAAs.find(lAA) != fSAAs.end()) {
      lBod = lAA;
      break;
    }
    if (lAA->HasPartOf(Tng::cSlotBody)) lBods.insert(lAA);
    if (lAA->HasPartOf(Tng::cSlotGenital)) lGens.insert(lAA);
  }
  if (lBods.size() == 1 && !lBod) lBod = *lBods.begin();
  if (lGens.size() == 1) lGen = *lGens.begin();
  if (lBods.size() == 0 && lGens.size() == 0) return;
  if (lGen) {
    if (!lBod) {
      aArmor->AddKeyword(fCCKey);
      return;
    }
    if (aArmor->HasKeyword(fRRKey)) {
      if (aIfLog)
        Tng::gLogger::error(
            "There is a conflict about armor [0x{:x}:{}] from file [{}]. It is markerd revealing but at the same time it uses an armor addon that is already covering!",
            aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      aArmor->RemoveKeyword(fRRKey);
      aArmor->AddKeyword(fPAKey);
      return;
    }
    if (lBod == lGen)
      aArmor->AddKeyword(fACKey);
    else
      aArmor->AddKeyword(fCCKey);
    return;
  }
  if (lBod) {
    if (aArmor->HasKeyword(fRRKey) || aArmor->HasKeyword(fARKey)) {
      if ((fCAAs.find(lBod) != fCAAs.end()) || (lGens.size() > 0)) {
        if (aIfLog)
          Tng::gLogger::error(
              "There is a conflict about armor [0x{:x}:{}] from file [{}]. It is markerd revealing but at the same time it uses an armor addon that is already covering!",
              aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        aArmor->RemoveKeyword(fRRKey);
        aArmor->AddKeyword(fPAKey);
        return;
      }
      fRAAs.insert(lBod);
      return;
    }
    if ((fRAAs.find(lBod) != fRAAs.end()) && (lGens.size() == 0)) {
      aArmor->AddKeyword(fARKey);
      if (aIfLog)
        Tng::gLogger::info(
            "The armor [0x{:x}:{}] from file [{}] is markerd revealing since it shares the body slot with another revealing armor! If it should be covering, a patch is "
            "required.",
            aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      return;
    }
    if (fSAAs.find(lBod) != fSAAs.end() && (lGens.size() == 0)) {
      aArmor->AddKeyword(fARKey);
      if (aIfLog)
        Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] is markerd revealing. If this is a mistake, a patch is required!", aArmor->GetFormID(), aArmor->GetFormEditorID(),
                           aArmor->GetFile(0)->GetFilename());
      return;
    }
    if (lGens.size() > 0) {
      if (aIfLog)
        Tng::gLogger::warn("The armor [0x{:x}:{}] from file [{}] cannot be patched automatically! If it should be covering and it is not, a patch is required.",
                           aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      aArmor->AddKeyword(fCCKey);
      return;
    }
    lBod->AddSlotToMask(Tng::cSlotGenital);
    aArmor->AddKeyword(fACKey);
    fCAAs.insert(lBod);
    return;
  }
  if (lBods.size() == 0) {
    aArmor->AddKeyword(fCCKey);
    return;
  }
  if (lGens.size() > 0) {
    for (const auto& lAA : lGens) {
      if (lBods.find(lAA) == lBods.end()) {
        if (aIfLog)
          Tng::gLogger::warn("The armor [0x{:x}:{}] from file [{}] cannot be patched automatically! If it should be covering and it is not, a patch is required.",
                             aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        aArmor->AddKeyword(fCCKey);
        return;
      }
    }
  }
  bool lbIsRevealing = false;
  for (const auto& lAA : lBods) {
    if (aArmor->HasKeyword(fRRKey) || aArmor->HasKeyword(fARKey) || lbIsRevealing) {
      if (fCAAs.find(lAA) != fCAAs.end()) {
        if (aIfLog)
          Tng::gLogger::error(
              "There is a conflict about armor [0x{:x}:{}] from file [{}]. It is markerd revealing but at the same time it uses an armor addon that is already covering!",
              aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        if (aArmor->HasKeyword(fRRKey)) aArmor->RemoveKeyword(fRRKey);
        if (aArmor->HasKeyword(fARKey)) aArmor->RemoveKeyword(fARKey);
        aArmor->AddKeyword(fPAKey);
        return;
      }
      lbIsRevealing = true;
      fRAAs.insert(lAA);
      continue;
    }
    if (fRAAs.find(lAA) != fRAAs.end()) {
      aArmor->AddKeyword(fARKey);
      if (aIfLog)
        Tng::gLogger::info(
            "The armor [0x{:x}:{}] from file [{}] is markerd revealing since it shares the body slot with another revealing armor! If it should be covering, a patch is "
            "required.",
            aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      lbIsRevealing = true;
      continue;
    }
    if (fSAAs.find(lAA) != fSAAs.end()) {
      aArmor->AddKeyword(fARKey);
      if (aIfLog)
        Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] is markerd revealing. If this is a mistake, a patch is required!", aArmor->GetFormID(), aArmor->GetFormEditorID(),
                           aArmor->GetFile(0)->GetFilename());
      lbIsRevealing = true;
      continue;
    }
  }
  if (lbIsRevealing) {
    if (!aArmor->HasKeyword(fARKey) && !aArmor->HasKeyword(fRRKey)) aArmor->AddKeyword(fARKey);
    return;
  }
  for (const auto& lAA : lBods) {
    if (fCAAs.find(lAA) == fCAAs.end()) {
      lAA->AddSlotToMask(Tng::cSlotGenital);
      fCAAs.insert(lBod);
    }
  }
  aArmor->AddKeyword(fACKey);
}

bool TngCore::SwapRevealing(RE::TESObjectARMO* aArmor) noexcept {
  if (!(aArmor->HasKeyword(fACKey) || aArmor->HasKeyword(fARKey))) return false;
  if (aArmor->HasKeyword(fACKey)) {
    for (const auto& lAA : aArmor->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody)) lAA->RemoveSlotFromMask(Tng::cSlotGenital);
    aArmor->RemoveKeyword(fACKey);
    aArmor->AddKeyword(fARKey);
    TngInis::SaveRevealingArmor(aArmor);
  } else {
    for (const auto& lAA : aArmor->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody)) lAA->AddSlotToMask(Tng::cSlotGenital);
    aArmor->RemoveKeyword(fARKey);
    aArmor->AddKeyword(fACKey);
    TngInis::RemoveRevealingArmor(aArmor);
  }
}

void TngCore::CoverByArmor(RE::TESObjectARMO* aArmor) noexcept {
  aArmor->AddKeyword(fCCKey);
  const auto lID = (std::string(aArmor->GetName()).empty()) ? aArmor->GetFormEditorID() : aArmor->GetName();
  if (!(aArmor->HasPartOf(Tng::cSlotGenital) || aArmor->armorAddons[0]->HasPartOf(Tng::cSlotGenital))) {
    aArmor->armorAddons[0]->AddSlotToMask(Tng::cSlotGenital);
    Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] would cover genitalia and any other armor on slot 52.", aArmor->GetFormID(), lID, aArmor->GetFile(0)->GetFilename());
  } else {
    Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] already covers genitalia and any other armor on slot 52.", aArmor->GetFormID(), lID,
                       aArmor->GetFile(0)->GetFilename());
  }
}
