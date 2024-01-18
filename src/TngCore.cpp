#include <TngCore.h>
#include <TngInis.h>
#include <TngSizeShape.h>

bool TngCore::Initialize() noexcept {
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  if (!lDataHandler->LookupModByName(Tng::cName)) {
    Tng::gLogger::critical("Mod [{}] was not found!", Tng::cName);
    return false;
  }
  auto lAutoRvealKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  auto lRevealingKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  auto lUnderwearKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  auto lAutoCoverKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  auto lCoveringKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  auto lPRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  auto lRRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  auto lIRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cIgnoredRaceKeyID, Tng::cName);
  auto lIArmoKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  if (!(lAutoRvealKey && lRevealingKey && lUnderwearKey && lAutoCoverKey && lCoveringKey && lPRaceKey && lRRaceKey && lIRaceKey && lIArmoKey)) {
    Tng::gLogger::critical("The original TNG keywords could not be found!");
    return false;
  }
  return true;
}

void TngCore::GenitalizeRaces() noexcept {
  Tng::gLogger::info("Finding the genitals to for installed races...");
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  for (int i = 0; i < cVanillaRaceTypes; i++) HandleVanillaRace(cBaseRaceIDs[i], cVanillaRaceDefaults[i]);
  for (const auto& lRaceID : cEquiRaceIDs) HandleVanillaRace(lRaceID.first, cVanillaRaceDefaults[lRaceID.second]);
  for (const auto& lRaceID : cExclRaceIDs) IgnoreRace(lDataHandler->LookupForm<RE::TESRace>(lRaceID.first, lRaceID.second));
  auto& lAllRacesArray = lDataHandler->GetFormArray<RE::TESRace>();
  auto lPRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  auto lRRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  auto lIRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cIgnoredRaceKeyID, Tng::cName);
  int lPR = 0;
  int lIR = 0;
  int lRR = 0;
  for (const auto& lRace : lAllRacesArray) {
    if (lRace->HasKeyword(lIRaceKey)) {
      lIR++;
      continue;
    }
    if (lRace->HasKeyword(lRRaceKey)) {
      lRR++;
      continue;
    }
    if (lRace->HasKeyword(lPRaceKey)) {
      lPR++;
      continue;
    }
    if (CheckRace(lRace)) switch (AddRace(lRace, nullptr)) {
        case 0:
          lPR++;
        case 1:
          lRR++;
        case -1:
          lIR++;
        default:
          Tng::gLogger::critical("There is a mistake in the programming for races! Please report this.");
      }
  }
  Tng::gLogger::info("TNG recognized assigned genitals to [{}] races, found [{}] races to be ready and ignored [{}] races.", lPR, lRR, lIR);
}

void TngCore::GenitalizeNPCSkins() noexcept {
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  RE::BSTArray<RE::TESNPC*> lAllNPCs = lDataHandler->GetFormArray<RE::TESNPC>();
  auto lPRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  auto lRRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  auto lIRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cIgnoredRaceKeyID, Tng::cName);
  auto lIArmoKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  Tng::gLogger::info("Checking NPCs for custom skins.");
  int lC = 0;
  std::set<std::pair<std::string_view, int>> lCustomSkinMods{};
  for (const auto& lNPC : lAllNPCs) {
    const auto lNPCRace = lNPC->race;
    if (!lNPCRace) {
      Tng::gLogger::warn("The NPC [{}] from file [{}] does not have a race! They cannot be modified by TNG.", lNPC->GetName(), lNPC->GetFile(0)->GetFilename());
      continue;
    }
    if (!lNPCRace->HasKeyword(lPRaceKey) && !lNPCRace->HasKeyword(lRRaceKey)) continue;
    const auto lSkin = lNPC->skin;
    if (!lSkin) continue;
    if (lSkin->HasKeyword(lIArmoKey)) continue;
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

void TngCore::CheckArmorPieces() noexcept {
  Tng::gLogger::info("Checking ARMO records...");
  TngInis::LoadTngInis();
  auto lDataHandler = RE::TESDataHandler::GetSingleton();
  auto& lAllArmor = lDataHandler->GetFormArray<RE::TESObjectARMO>();
  auto lPRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cProcessedRaceKeyID, Tng::cName);
  auto lRRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  auto lIRaceKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cIgnoredRaceKeyID, Tng::cName);
  auto lIArmoKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  auto lDefRace = lDataHandler->LookupForm<RE::TESRace>(cDefRaceID, Tng::cSkyrim);
  auto lARKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  auto lRRKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  auto lUwKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  auto lACKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  auto lCCKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  auto lPAKey = lDataHandler->LookupForm<RE::BGSKeyword>(Tng::cProblemArmoKeyID, Tng::cName);
  std::set<RE::TESObjectARMO*> lPotentialArmor;
  std::set<RE::TESObjectARMO*> lUnhandledArmor;
  std::set<RE::TESRace*> lArmoRaces;
  bool lCheckSkinMods = (TngInis::fSkinMods.size() > 0);
  bool lCheckSkinRecords = (TngInis::fSingleSkinIDs.size() > 0);
  bool lCheckRevealMods = (TngInis::fRevealingMods.size() > 0);
  bool lCheckRevealRecords = (TngInis::fSingleRevealingIDs.size() > 0);
  bool lCheckCoverRecords = (TngInis::fSingleCoveringIDs.size() > 0);
  int lRR = 0;
  int lAR = 0;
  int lCC = 0;
  int lAC = 0;
  int lPA = 0;

  for (const auto& lModName : TngInis::fSkinMods)
    if (lDataHandler->LookupModByName(lModName)) Tng::gLogger::info("TheNewGentleman keeps an eye for [{}] as a skin mod.", lModName);
  for (const auto& lRevealMod : TngInis::fRevealingMods)
    if (lDataHandler->LookupModByName(lRevealMod)) Tng::gLogger::info("TheNewGentleman keeps an eye for [{}] as a revealing armor mod.", lRevealMod);

  for (const auto& lArmor : lAllArmor) {
    if (lArmor->HasKeyword(lIArmoKey)) continue;
    if (lArmor->armorAddons.size() == 0) continue;
    const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
    if (!lArmor->race) {
      Tng::gLogger::warn("The armor [0x{:x}:{}] from file [{}] does not have a race! It won't be touched by Tng!", lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
      lArmor->AddKeyword(lIArmoKey);
      continue;
    }
    if (!(lArmor->race->HasKeyword(lPRaceKey) || lArmor->race->HasKeyword(lRRaceKey) || (lArmor->race != lDefRace))) continue;
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
        lArmor->AddKeyword(lRRKey);
        HandleArmor(lArmor);
        if (lArmor->HasKeyword(lPAKey))
          lPA++;
        else
          lRR++;
      }
      continue;
    }
    if (lCheckRevealRecords) {
      const auto lRevealEntry = TngInis::fSingleRevealingIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->GetLocalFormID()));
      if (lRevealEntry != TngInis::fSingleRevealingIDs.end()) {
        lArmor->AddKeyword(lRRKey);
        HandleArmor(lArmor);
        if (lArmor->HasKeyword(lPAKey))
          lPA++;
        else
          lRR++;
        continue;
      }
    }
    if (lArmor->HasPartOf(Tng::cSlotGenital) && !lArmor->HasKeyword(lUwKey)) {
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] would cover genitals and be covered by chest armor pieces (like an underwear). If it is wrong, a patch is required.",
                         lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
      lCC++;
      continue;
    }
    if (lArmor->HasPartOf(Tng::cSlotBody)) lPotentialArmor.insert(lArmor);
  }
  for (const auto& lArmor : lPotentialArmor) {
    HandleArmor(lArmor);
    if (lArmor->HasKeyword(lRRKey)) lRR++;
    if (lArmor->HasKeyword(lARKey)) lAR++;
    if (lArmor->HasKeyword(lACKey)) lAC++;
    if (lArmor->HasKeyword(lCCKey)) lCC++;
    if (lArmor->HasKeyword(lPAKey)) lPA++;
  }
  Tng::gLogger::info("Processed {} armor pieces with slot 32 or 52:", lRR + lAR + lAC + lCC + lPA);
  if (lPA > 0) Tng::gLogger::warn("\t{}: seems to be problematic!", lPA);
  if (lCC > 0) Tng::gLogger::info("\t{}: are already covering genitals,", lCC);
  if (lRR > 0) Tng::gLogger::info("\t{}: are already revealing", lRR);
  if (lAC > 0) Tng::gLogger::info("\t{}: were updated to cover genitals", lAC);
  if (lAR > 0) Tng::gLogger::info("\t{}: were updated to be revealing", lAR);
}

bool TngCore::UpdateRace(RE::TESRace* aRace, int aGenOption) noexcept {
  
  auto lGenital = TngSizeShape::GetAddonAt(false, aGenOption);
  if (!lGenital) return false;
  TngSizeShape::SetRaceShape(aRace, aGenOption);
  auto lSkin = aRace->skin;
  for (auto i = lSkin->armorAddons.begin(); i != lSkin->armorAddons.end(); i++)
    if ((*i)->HasPartOf(Tng::cSlotGenital)) lSkin->armorAddons.erase(i);
  UpdateGenRace(aRace, lGenital);
}

void TngCore::UpdateGenRace(RE::TESRace* aRace, RE::TESObjectARMO* aGenital, const bool aIsCustomRace) noexcept {
  RE::TESObjectARMA* lGenitalAA{nullptr};
  for (const auto& lAA : aGenital->armorAddons) {
    std::set<RE::TESRace*> lAARaces{lAA->race};
    if (lAA->additionalRaces.size() > 0) lAARaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
    if (lAARaces.find(aRace) != lAARaces.end()) {
      lGenitalAA = lAA;
      break;
    }
    if (aRace->armorParentRace)
      if (lAARaces.find(aRace->armorParentRace) != lAARaces.end()) {
        lGenitalAA = lAA;
        break;
      }
  }
  if (!lGenitalAA)
    if (aIsCustomRace) {
      UpdateEqRaceAddon(aRace, aGenital);
    } else {
      Tng::gLogger::critical("The race {} is not supported by the addon {} from file {}.", aRace->GetFormEditorID(), aGenital->GetName(), aGenital->GetFile(0)->GetFilename());
    }
}

void TngCore::HandleVanillaRace(std::pair<RE::FormID, std::string_view> aRaceRecord, const int aDefaultChoice) {
  const auto lRace = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESRace>(aRaceRecord.first, aRaceRecord.second);
  if (!(lRace)) {
    Tng::gLogger::critical("An original game race cannot be found!");
    return;
  }
  auto lChoice = TngSizeShape::GetRaceShape(lRace) > -1 ? TngSizeShape::GetRaceShape(lRace) : aDefaultChoice;
  const auto lGenital = TngSizeShape::GetAddonAt(false, lChoice);
  if (!(lGenital)) {
    Tng::gLogger::critical("Original genitalia from TNG cannot be found!");
    return;
  }
  UpdateGenRace(lRace, lGenital);
}

bool TngCore::UpdateEqRaceAddon(RE::TESRace* aRace, RE::TESObjectARMO* aGenital) noexcept {
  const auto lRaceDesc = std::string(aRace->GetFormEditorID()) + std::string(aRace->GetName());
  int lEqRace = -1;
  for (int i = 0; i < cVanillaRaceTypes; i++) {
    if (lRaceDesc.contains(cRaceNames[i][0]) || lRaceDesc.contains(cRaceNames[i][1])) {
      lEqRace = i;
      break;
    }
  }
  if (lEqRace == -1) return false;
  auto lDH = RE::TESDataHandler::GetSingleton();
  auto lBeastKey = lDH->LookupForm<RE::BGSKeyword>(Tng::cBstKeywID, Tng::cSkyrim);
  if ((lEqRace == 9 || lEqRace == 8) != aRace->HasKeyword(lBeastKey)) {
    UpdateGenRace(aRace, aGenital);
    return false;
  }
  auto lGenRace = lDH->LookupForm<RE::TESRace>(cBaseRaceIDs[lEqRace].first, cBaseRaceIDs[lEqRace].second);
  RE::TESObjectARMA* lGenitalAA{nullptr};
  for (const auto& lAA : aGenital->armorAddons) {
    std::set<RE::TESRace*> lAARaces{lAA->race};
    if (lAA->additionalRaces.size() > 0) lAARaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
    if (lAARaces.find(lGenRace) != lAARaces.end()) {
      lGenitalAA = lAA;
      break;
    }
    if (lGenRace->armorParentRace)
      if (lAARaces.find(lGenRace->armorParentRace) != lAARaces.end()) {
        lGenitalAA = lAA;
        break;
      }
  }
  lGenitalAA->additionalRaces.emplace_back(aRace->armorParentRace ? aRace->armorParentRace : aRace);
  UpdateGenRace(aRace, aGenital);
}

bool TngCore::FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept {
  if (!aSkin->HasPartOf(Tng::cSlotBody)) return false;
  if (aSkin->HasPartOf(Tng::cSlotGenital)) {
    if (aName) Tng::gLogger::info("The skin [0x{:x}:{}] from file [{}] is ready for TNG!", aSkin->GetFormID(), aName, aSkin->GetFile(0)->GetFilename());
    fHandledSkins.insert(aSkin);
    return true;
  }

  auto lHasGenital = false;
  std::set<RE::TESRace*> lSkinRaces;
  for (const auto& lAA : aSkin->armorAddons) {
    lSkinRaces.insert(lAA->race);
    lSkinRaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
  }
  if (lSkinRaces.find(fDefRace) != lSkinRaces.end()) lSkinRaces.erase(fDefRace);
  for (const auto& lRace : lSkinRaces) {
    auto lRaceGen = std::find_if(fAllRaceGens.begin(), fAllRaceGens.end(), [&lRace](const std::pair<RE::TESRace*, RE::TESObjectARMA*>& p) { return p.first == lRace; });
    {
      if (lRaceGen != fAllRaceGens.end()) AddGenitalToSkin(aSkin, (*lRaceGen).second);
      lHasGenital = true;
    }
  }
  if (aName) Tng::gLogger::info("The skin [0x{:x}:{}] from file [{}] added as extra skin.", aSkin->GetFormID(), aName, aSkin->GetFile(0)->GetFilename());
  return lHasGenital;
}

void TngCore::AddGenitalToSkin(RE::TESObjectARMO* aSkin, RE::TESObjectARMA* aGenital, RE::TESRace* aRace) noexcept {
  for (const auto& lAA : aSkin->armorAddons) {
    if (lAA == aGenital) {
      fHandledSkins.insert(aSkin);
      return;
    }
    if (!aRace) continue;
    if (lAA->GetFile(0)->GetFilename() == Tng::cName) {
      std::set<RE::TESRace*> lGenRaces{lAA->race};
      lGenRaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
      if (lGenRaces.find(aRace) != lGenRaces.end()) {
        fHandledSkins.insert(aSkin);
        return;
      }
    }
  }
  if (fHandledSkins.find(aSkin) == fHandledSkins.end()) {
    for (const auto lAA : aSkin->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody)) fSkinAAs.insert(lAA);
  }
  if (!aSkin->HasPartOf(Tng::cSlotGenital)) aSkin->AddSlotToMask(Tng::cSlotGenital);
  aSkin->armorAddons.emplace_back(aGenital);
  fHandledSkins.insert(aSkin);
}

bool TngCore::IgnoreRace(RE::TESRace* aRace) {
  static RE::TESDataHandler* lDH = lDH ? lDH : RE::TESDataHandler::GetSingleton();
  static RE::BGSKeyword* lRRaceKey = lRRaceKey ? lRRaceKey : lDH->LookupForm<RE::BGSKeyword>(Tng::cReadyRaceKeyID, Tng::cName);
  static RE::BGSKeyword* lIRaceKey = lIRaceKey ? lIRaceKey : lDH->LookupForm<RE::BGSKeyword>(Tng::cIgnoredRaceKeyID, Tng::cName);
  static RE::BGSKeyword* lIArmoKey = lIArmoKey ? lIArmoKey : lDH->LookupForm<RE::BGSKeyword>(Tng::cIgnoredArmoKeyID, Tng::cName);
  if (aRace->skin) {
    aRace->skin->AddKeyword(lIArmoKey);
    for (const auto& lAA : aRace->skin->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotGenital)) {
        aRace->AddKeyword(lRRaceKey);
        return true;
      }
  }
  aRace->AddKeyword(lIRaceKey);
  return false;
}

bool TngCore::CheckRace(RE::TESRace* aRace) {
  if (!aRace->HasKeyword(fNPCKey) || aRace->HasKeyword(fCreatureKey) || !aRace->HasPartOf(Tng::cSlotBody) || aRace->IsChildRace()) return false;
  if ((aRace->skeletonModels[0].model != fDefSkeleton[0]) || (aRace->skeletonModels[1].model != fDefSkeleton[1])) {
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
  if ((lRaceSkinAA->bipedModels[0].model != fDefBodyMesh[0]) || (lRaceSkinAA->bipedModels[1].model != fDefBodyMesh[1])) {
    IgnoreRace(aRace);
    Tng::gLogger::info("The race [0x{:x}:{}] from file [{}] was ignored because it uses a custom mesh!", aRace->GetFormID(), aRace->GetFormEditorID(),
                       aRace->GetFile(0)->GetFilename());
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

char TngCore::AddRace(RE::TESRace* aRace, RE::TESObjectARMA* aGenital, RE::TESRace* aRNAM) noexcept {
  if (aRace->HasPartOf(Tng::cSlotGenital)) {
    Tng::gLogger::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
    return IgnoreRace(aRace) ? 1 : -1;
  }
  if (aGenital) {
    aRace->AddSlotToMask(Tng::cSlotGenital);
    aRace->AddKeyword(lTNGRaceKey);
    AddGenitalToSkin(aRace->skin, aGenital, aRNAM);
    fRacialSkins.insert(aRace->skin);
    fHandledRaces.insert(aRace);
    return 0;
  }
  if (aRace->HasKeyword(fBeastKey)) {
    const auto lRaceDesc = std::string(aRace->GetFormEditorID()) + std::string(aRace->GetName());
    if ((lRaceDesc.find("Khajiit") != std::string::npos) || (lRaceDesc.find("Rhat") != std::string::npos)) {
      RE::TESRace* lRNAM = (aRace->armorParentRace && aRace->armorParentRace != fDefRace) ? aRace->armorParentRace : aRace;
      int aChoice = TngSizeShape::GetRaceShape(aRace) > -1 ? TngSizeShape::GetRaceShape(aRace) : cVanillaRaceDefaults[9];
      std::set<RE::TESRace*> lGenRaces{fDefKhaGenital[aChoice]->additionalRaces.begin(), fDefKhaGenital[aChoice]->additionalRaces.end()};
      fDefKhaGenital[aChoice]->additionalRaces.emplace_back(aRace);
      if (lGenRaces.find(lRNAM) == lGenRaces.end()) fDefKhaGenital[aChoice]->additionalRaces.emplace_back(lRNAM);
      Tng::gLogger::info("The race [0x{:x}:{}] from file [{}] was recognized as Khajiit. If this is wrong, a patch is required.", aRace->GetFormID(), aRace->GetFormEditorID(),
                         aRace->GetFile(0)->GetFilename());
      fExtrRaceGens.insert(std::make_pair(aRace, aGenital));
      return AddRace(aRace, fDefKhaGenital[aChoice], lRNAM);
    } else {
      if ((lRaceDesc.find("Argonian") != std::string::npos) || (lRaceDesc.find("Saxhleel") != std::string::npos)) {
        RE::TESRace* lRNAM = (aRace->armorParentRace && aRace->armorParentRace != fDefRace) ? aRace->armorParentRace : aRace;
        int aChoice =
            TngSizeShape::genitalChoices[Tng::cRaceTypes + 1] < 3 ? TngSizeShape::genitalChoices[Tng::cRaceTypes + 1] : TngSizeShape::cGenitalDefaults[Tng::cRaceTypes + 1];
        std::set<RE::TESRace*> lGenRaces{fDefSaxGenital[aChoice]->additionalRaces.begin(), fDefSaxGenital[aChoice]->additionalRaces.end()};
        fDefSaxGenital[aChoice]->additionalRaces.emplace_back(aRace);
        if (lGenRaces.find(lRNAM) == lGenRaces.end()) fDefSaxGenital[aChoice]->additionalRaces.emplace_back(lRNAM);
        Tng::gLogger::info("The race [0x{:x}:{}] from file [{}] was recognized as Saxhleel(Argonian). If this is wrong, a patch is required.", aRace->GetFormID(),
                           aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
        fExtrRaceGens.insert(std::make_pair(aRace, aGenital));
        TngSizeShape::AddRaceIndex(aRace->GetFormID(), Tng::cRaceTypes + 1);
        return AddRace(aRace, fDefSaxGenital[aChoice], lRNAM);
      } else {
        Tng::gLogger::warn("The race [0x{:x}:{}] from file [{}] could not be recognized and did not receive any genital. If they should, a patch is required.", aRace->GetFormID(),
                           aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
        IgnoreRace(aRace);
      }
    }
  } else {
    RE::TESRace* lRNAM = (aRace->armorParentRace && aRace->armorParentRace != fDefRace) ? aRace->armorParentRace : aRace;
    int aChoice = TngSizeShape::genitalChoices[Tng::cRaceTypes] < 3 ? TngSizeShape::genitalChoices[Tng::cRaceTypes] : TngSizeShape::cGenitalDefaults[Tng::cRaceTypes];
    std::set<RE::TESRace*> lGenRaces{fDefMnmGenital[aChoice]->additionalRaces.begin(), fDefMnmGenital[aChoice]->additionalRaces.end()};
    fDefMnmGenital[aChoice]->additionalRaces.emplace_back(aRace);
    if (lGenRaces.find(lRNAM) == lGenRaces.end()) fDefMnmGenital[aChoice]->additionalRaces.emplace_back(lRNAM);
    Tng::gLogger::info("The race [0x{:x}:{}] from file [{}] received the default genital for man and mer races. If this is wrong, a patch is required.", aRace->GetFormID(),
                       aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
    fExtrRaceGens.insert(std::make_pair(aRace, aGenital));
    TngSizeShape::AddRaceIndex(aRace->GetFormID(), Tng::cRaceTypes);
    return AddRace(aRace, fDefMnmGenital[aChoice], lRNAM);
  }
}

void TngCore::HandleArmor(RE::TESObjectARMO* aArmor) noexcept {
  static RE::TESDataHandler* lDH = lDH ? lDH : RE::TESDataHandler::GetSingleton();
  static RE::BGSKeyword* lARKey = lARKey ? lARKey : lDH->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  static RE::BGSKeyword* lRRKey = lRRKey ? lRRKey : lDH->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  static RE::BGSKeyword* lACKey = lACKey ? lACKey : lDH->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  static RE::BGSKeyword* lCCKey = lCCKey ? lCCKey : lDH->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  static RE::BGSKeyword* lPAKey = lPAKey ? lPAKey : lDH->LookupForm<RE::BGSKeyword>(Tng::cProblemArmoKeyID, Tng::cName);
  static std::set<RE::TESObjectARMA*> lCAAs{};
  static std::set<RE::TESObjectARMA*> lRAAs{};
  std::set<RE::TESObjectARMA*> lBods{};
  std::set<RE::TESObjectARMA*> lGens{};
  RE::TESObjectARMA* lBod{nullptr};
  RE::TESObjectARMA* lGen{nullptr};
  for (const auto& lAA : aArmor->armorAddons) {
    if (fSkinAAs.find(lAA) != fSkinAAs.end()) {
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
      aArmor->AddKeyword(lCCKey);
      return;
    }
    if (aArmor->HasKeyword(lRRKey)) {
      Tng::gLogger::error(
          "There is a conflict about armor [0x{:x}:{}] from file [{}]. It is markerd revealing but at the same time it uses an armor addon that is already covering!",
          aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      aArmor->RemoveKeyword(lRRKey);
      aArmor->AddKeyword(lPAKey);
      return;
    }
    if (lBod == lGen)
      aArmor->AddKeyword(lACKey);
    else
      aArmor->AddKeyword(lCCKey);
    return;
  }
  if (lBod) {
    if (aArmor->HasKeyword(lRRKey)) {
      if ((lCAAs.find(lBod) != lCAAs.end()) || (lGens.size() > 0)) {
        Tng::gLogger::error(
            "There is a conflict about armor [0x{:x}:{}] from file [{}]. It is markerd revealing but at the same time it uses an armor addon that is already covering!",
            aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        aArmor->RemoveKeyword(lRRKey);
        aArmor->AddKeyword(lPAKey);
        return;
      }
      lRAAs.insert(lBod);
      return;
    }
    if ((lRAAs.find(lBod) != lRAAs.end()) && (lGens.size() == 0)) {
      aArmor->AddKeyword(lARKey);
      Tng::gLogger::info(
          "The armor [0x{:x}:{}] from file [{}] is markerd revealing since it shares the body slot with another revealing armor! If it should be covering, a patch is required.",
          aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      return;
    }
    if (fSkinAAs.find(lBod) != fSkinAAs.end() && (lGens.size() == 0)) {
      aArmor->AddKeyword(lARKey);
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] is markerd revealing. If this is a mistake, a patch is required!", aArmor->GetFormID(), aArmor->GetFormEditorID(),
                         aArmor->GetFile(0)->GetFilename());
      return;
    }
    if (lGens.size() > 0) {
      Tng::gLogger::warn("The armor [0x{:x}:{}] from file [{}] cannot be patched automatically! If it should be covering and it is not, a patch is required.", aArmor->GetFormID(),
                         aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      aArmor->AddKeyword(lCCKey);
      return;
    }
    lBod->AddSlotToMask(Tng::cSlotGenital);
    aArmor->AddKeyword(lACKey);
    lCAAs.insert(lBod);
    return;
  }
  if (lBods.size() == 0) {
    aArmor->AddKeyword(lCCKey);
    return;
  }
  if (lGens.size() > 0) {
    for (const auto& lAA : lGens) {
      if (lBods.find(lAA) == lBods.end()) {
        Tng::gLogger::warn("The armor [0x{:x}:{}] from file [{}] cannot be patched automatically! If it should be covering and it is not, a patch is required.",
                           aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        aArmor->AddKeyword(lCCKey);
        return;
      }
    }
  }
  bool lbIsRevealing = false;
  for (const auto& lAA : lBods) {
    if (aArmor->HasKeyword(lRRKey) || aArmor->HasKeyword(lARKey) || lbIsRevealing) {
      if (lCAAs.find(lAA) != lCAAs.end()) {
        Tng::gLogger::error(
            "There is a conflict about armor [0x{:x}:{}] from file [{}]. It is markerd revealing but at the same time it uses an armor addon that is already covering!",
            aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        if (aArmor->HasKeyword(lRRKey)) aArmor->RemoveKeyword(lRRKey);
        if (aArmor->HasKeyword(lARKey)) aArmor->RemoveKeyword(lARKey);
        aArmor->AddKeyword(lPAKey);
        return;
      }
      lbIsRevealing = true;
      lRAAs.insert(lAA);
      continue;
    }
    if (lRAAs.find(lAA) != lRAAs.end()) {
      aArmor->AddKeyword(lARKey);
      Tng::gLogger::info(
          "The armor [0x{:x}:{}] from file [{}] is markerd revealing since it shares the body slot with another revealing armor! If it should be covering, a patch is required.",
          aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      lbIsRevealing = true;
      continue;
    }
    if (fSkinAAs.find(lAA) != fSkinAAs.end()) {
      aArmor->AddKeyword(lARKey);
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] is markerd revealing. If this is a mistake, a patch is required!", aArmor->GetFormID(), aArmor->GetFormEditorID(),
                         aArmor->GetFile(0)->GetFilename());
      lbIsRevealing = true;
      continue;
    }
  }
  if (lbIsRevealing) {
    if (!aArmor->HasKeyword(lARKey) && !aArmor->HasKeyword(lRRKey)) aArmor->AddKeyword(lARKey);
    return;
  }
  for (const auto& lAA : lBods) {
    if (lCAAs.find(lAA) == lCAAs.end()) {
      lAA->AddSlotToMask(Tng::cSlotGenital);
      lCAAs.insert(lBod);
    }
  }
  aArmor->AddKeyword(lACKey);
}

void TngCore::CoverByArmor(RE::TESObjectARMO* aArmor) noexcept {
  aArmor->AddKeyword(fCoveringKey);
  const auto lID = (std::string(aArmor->GetName()).empty()) ? aArmor->GetFormEditorID() : aArmor->GetName();
  if (!(aArmor->HasPartOf(Tng::cSlotGenital) || aArmor->armorAddons[0]->HasPartOf(Tng::cSlotGenital))) {
    aArmor->armorAddons[0]->AddSlotToMask(Tng::cSlotGenital);
    fCoveringAAs.insert(aArmor->armorAddons[0]);
    Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] would cover genitalia and any other armor on slot 52.", aArmor->GetFormID(), lID, aArmor->GetFile(0)->GetFilename());
  } else {
    Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] already covers genitalia and any other armor on slot 52.", aArmor->GetFormID(), lID,
                       aArmor->GetFile(0)->GetFilename());
  }
}
