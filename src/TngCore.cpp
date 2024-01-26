#include <TngCore.h>
#include <TngInis.h>
#include <TngSizeShape.h>

bool TngCore::Initialize() noexcept {
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
  fNPCKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cNPCKeywID, Tng::cSkyrim);
  fBstKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cBstKeywID, Tng::cSkyrim);
  fCrtKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCrtKeywID, Tng::cSkyrim);
  fSwPKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cSkinWithPenisKeyID, Tng::cName);
  fGenSkinKey = fDH->LookupForm<RE::BGSKeyword>(Tng::cCustomSkinID, Tng::cName);
  fDefRace = fDH->LookupForm<RE::TESRace>(Tng::cDefRaceID, Tng::cSkyrim);
  fBeastDef = fDH->LookupForm<RE::TESRace>(cBaseRaceIDs[9].first, cBaseRaceIDs[9].second);
  fAllNPCs = fDH->GetFormArray<RE::TESNPC>();
  if (!(fPRaceKey && fRRaceKey && fIRaceKey && fARKey && fRRKey && fACKey && fCCKey && fPAKey && fIAKey && fUAKey && fNPCKey && fBstKey && fCrtKey && fSwPKey && fGenSkinKey &&
        fDefRace && fBeastDef)) {
    Tng::gLogger::critical("The original TNG information could not be found!");
    return false;
  }
  std::set<std::string> lDefSks{};
  lDefSks.insert(fDefRace->skeletonModels[0].model.data());
  lDefSks.insert(fDefRace->skeletonModels[1].model.data());
  lDefSks.insert(fBeastDef->skeletonModels[0].model.data());
  lDefSks.insert(fBeastDef->skeletonModels[1].model.data());
  TngInis::UpdateValidSkeletons(lDefSks);
  for (int i = 0; i < TngSizeShape::GetAddonCount(false); i++) fMalAddonSkins.push_back({});
  for (int i = 0; i < TngSizeShape::GetAddonCount(true); i++) fFemAddonSkins.push_back({});
  for (int i = 0; i < cVanillaRaceTypes; i++) {
    fBaseRaces[i] = fDH->LookupForm<RE::TESRace>(cBaseRaceIDs[i].first, cBaseRaceIDs[i].second);
    fBaseRaces[i]->AddKeyword(fPRaceKey);
  }
  for (int i = 0; i < cEqRaceTypes; i++) {
    fEqRaces[i] = fDH->LookupForm<RE::TESRace>(cEquiRaceIDs[i].first.first, cEquiRaceIDs[i].first.second);
    fEqRaces[i]->AddKeyword(fPRaceKey);
  }
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
  UpdateRacialAddons();
  Tng::gLogger::info("Recognized assigned genitals to [{}] races, found [{}] races to be ready and ignored [{}] races.", lPR, lRR, lIR);
  if (lPP > 0) Tng::gLogger::error("Faced errors for [{}] races.", lPP);
}

bool TngCore::UpdateRaces(const std::size_t aRaceIdx, int aAddon) noexcept {
  int lChoice = aAddon >= 0 ? aAddon : TngSizeShape::GetRaceGrpDefAddn(aRaceIdx);
  auto lOgFormID = TngSizeShape::GetRaceGrpSkinOg(aRaceIdx)->formID;
  auto& lAddonSkins = fMalAddonSkins[lChoice];
  auto lSkinEntry = std::find_if(lAddonSkins.begin(), lAddonSkins.end(), [&lOgFormID](const std::pair<RE::FormID, RE::TESObjectARMO*>& p) { return p.first == lOgFormID; });
  if (lSkinEntry != lAddonSkins.end()) TngSizeShape::SetRaceGrpSkin(aRaceIdx, lSkinEntry->second);
  TngSizeShape::SetRaceGrpAddn(aRaceIdx, aAddon);
  TngInis::SaveRaceAddn(aRaceIdx, aAddon);
  return true;
}

bool TngCore::IgnoreRace(RE::TESRace* aRace) noexcept {
  if (aRace->skin) {
    aRace->skin->RemoveKeywords(fArmoKeys);
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
  auto lChoice = TngSizeShape::GetRaceGrpAddn(aRace) > -1 ? TngSizeShape::GetRaceGrpAddn(aRace) : aDefaultChoice;
  TngSizeShape::SetRaceGrpAddn(aRace, lChoice);
  TngSizeShape::SetRaceGrpDefAddn(aRace, aDefaultChoice);
}

bool TngCore::CheckRace(RE::TESRace* aRace) {
  if (!aRace->HasKeyword(fNPCKey) || aRace->HasKeyword(fCrtKey) || !aRace->HasPartOf(Tng::cSlotBody) || aRace->IsChildRace()) return false;
  if (!TngInis::IsValidSkeleton(aRace->skeletonModels[0].model.data()) || !TngInis::IsValidSkeleton(aRace->skeletonModels[1].model.data())) {
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
    Tng::gLogger::info("The race [0x{:x}: {}] has only female NPC references and is ignored by TNG.", aRace->GetFormID(), aRace->GetFormEditorID());
  }
  return lAdd;
}

Tng::TNGRes TngCore::AddRace(RE::TESRace* aRace) noexcept {
  if (aRace->HasPartOf(Tng::cSlotGenital)) {
    Tng::gLogger::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
    return IgnoreRace(aRace) ? Tng::resOkRaceR : Tng::raceErr;
  }
  auto lRaceIdx = TngSizeShape::GetRaceGrp(aRace);
  if (aRace != TngSizeShape::GetRaceByIdx(lRaceIdx)) {
    auto lRaceName = TngSizeShape::GetRaceName(lRaceIdx);
    Tng::gLogger::info("The race [0x{:x}: {}] was recognized as {}.", aRace->GetFormID(), aRace->GetFormEditorID(), lRaceName);
  } else {
    Tng::gLogger::info("The race [0x{:x}: {}] was recognized as a new group.", aRace->GetFormID(), aRace->GetFormEditorID());
  }
  int lChoice = TngSizeShape::GetRaceGrpAddn(aRace) < 0 ? 0 : TngSizeShape::GetRaceGrpAddn(aRace);
  aRace->AddKeyword(fPRaceKey);
  aRace->AddSlotToMask(Tng::cSlotGenital);
  TngSizeShape::SetRaceGrpAddn(aRace, lChoice);
  TngSizeShape::SetRaceGrpDefAddn(aRace, 0);
  return Tng::resOkRaceP;
}

void TngCore::UpdateRacialAddons() noexcept {
  for (std::size_t i = 0; i < TngSizeShape::GroupCount(); i++) {
    auto lRace = TngSizeShape::GetRaceByIdx(i);
    if (lRace->armorParentRace) continue;
    int lSubEqRace = FindEqVanilla(lRace);
    for (int j = 0; j < TngSizeShape::GetAddonCount(false); j++) {
      auto lGen = TngSizeShape::GetAddonAt(false, j);
      for (auto& lGenAA : lGen->armorAddons) {
        std::set<RE::TESRace*> lGenAARaces{lGenAA->race};
        lGenAARaces.insert(lGenAA->additionalRaces.begin(), lGenAA->additionalRaces.end());
        if (lGenAARaces.find(fBaseRaces[lSubEqRace]) != lGenAARaces.end() && lGenAARaces.find(lRace) == lGenAARaces.end()) {
          lGenAA->additionalRaces.emplace_back(lRace);
          break;
        }
      }
    }
    for (int j = 0; j < TngSizeShape::GetAddonCount(true); j++) {
      auto lGen = TngSizeShape::GetAddonAt(true, j);
      for (auto& lGenAA : lGen->armorAddons) {
        std::set<RE::TESRace*> lGenAARaces{lGenAA->race};
        lGenAARaces.insert(lGenAA->additionalRaces.begin(), lGenAA->additionalRaces.end());
        if (lGenAARaces.find(fBaseRaces[lSubEqRace]) != lGenAARaces.end() && lGenAARaces.find(lRace) == lGenAARaces.end()) {
          lGenAA->additionalRaces.emplace_back(lRace);
          break;
        }
      }
    }
  }
  for (int i = 0; i < TngSizeShape::GroupCount(); i++) TngSizeShape::SetRaceGrpSkin(i, GentifySkin(TngSizeShape::GetRaceGrpSkinOg(i), TngSizeShape::GetRaceGrpAddn(i)));
}

RE::TESObjectARMO* TngCore::GentifySkin(RE::TESObjectARMO* aOgSkin, int aAddonChoice) noexcept {
  fOgSkins.insert(aOgSkin);
  aOgSkin->RemoveKeywords(fArmoKeys);
  aOgSkin->AddKeyword(fIAKey);
  RE::TESObjectARMO* lRes{nullptr};
  for (const auto lAA : aOgSkin->armorAddons)
    if (lAA->HasPartOf(Tng::cSlotBody)) fSAAs.insert(lAA);
  std::set<RE::TESRace*> lSkinRaces{};
  bool lIsBeastSkin = aOgSkin->race->HasKeyword(fBstKey);
  for (const auto lAA : aOgSkin->armorAddons) {
    if (!lAA->HasPartOf(Tng::cSlotBody)) continue;
    if (lAA->race != fDefRace && !lAA->race->HasKeyword(fPRaceKey)) continue;
    std::set<RE::TESRace*> lAARaces{lAA->race};
    lAARaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
    for (auto& lRace : lAARaces)
      if (lRace->HasKeyword(fPRaceKey) && (lRace->HasKeyword(fBstKey) == lIsBeastSkin)) lSkinRaces.insert(lRace);
    if (lAA->race == fDefRace && lAA->additionalRaces.size() == 0) lSkinRaces.insert(fDefRace);
  }
  for (int i = 0; i < TngSizeShape::GetAddonCount(false); i++) {
    RE::TESObjectARMO* lSkin = nullptr;
    lSkin = aOgSkin->CreateDuplicateForm(true, (void*)lSkin)->As<RE::TESObjectARMO>();
    lSkin->Copy(aOgSkin);
    lSkin->AddKeyword(fGenSkinKey);
    lSkin->AddKeyword(fIAKey);
    lSkin->AddSlotToMask(Tng::cSlotGenital);
    auto lGen = TngSizeShape::GetAddonAt(false, i);
    for (const auto& lGenAA : lGen->armorAddons)
      if (lSkinRaces.find(lGenAA->race) != lSkinRaces.end()) lSkin->armorAddons.emplace_back(lGenAA);
    if (lSkin->race) fMalAddonSkins[i].insert(std::make_pair(aOgSkin->GetFormID(), lSkin));
    fDH->GetFormArray<RE::TESObjectARMO>().push_back(lSkin);
    if (i == aAddonChoice) lRes = lSkin;
  }
  for (int i = 0; i < TngSizeShape::GetAddonCount(true); i++) {
    RE::TESObjectARMO* lSkin = nullptr;
    lSkin = aOgSkin->CreateDuplicateForm(true, (void*)lSkin)->As<RE::TESObjectARMO>();
    lSkin->Copy(aOgSkin);
    lSkin->AddKeyword(fGenSkinKey);
    lSkin->AddKeyword(fIAKey);
    lSkin->AddSlotToMask(Tng::cSlotGenital);
    auto lGen = TngSizeShape::GetAddonAt(true, i);
    for (const auto& lGenAA : lGen->armorAddons)
      if (lSkinRaces.find(lGenAA->race) != lSkinRaces.end()) lSkin->armorAddons.emplace_back(lGenAA);
    fFemAddonSkins[i].insert(std::make_pair(aOgSkin->GetFormID(), lSkin));
    fDH->GetFormArray<RE::TESObjectARMO>().push_back(lSkin);
  }
  return lRes;
}

int TngCore::FindEqVanilla(RE::TESRace* aRace) noexcept {
  for (const auto& lRace : fBaseRaces)
    if (aRace == lRace) return -1;
  for (const auto& lRace : fEqRaces)
    if (aRace == lRace) return -1;
  const auto lRaceDesc = std::string(aRace->GetFormEditorID()) + std::string(aRace->GetName());
  for (int i = 0; i < cVanillaRaceTypes; i++) {
    if (lRaceDesc.contains(cRaceNames[i].first[0]) || lRaceDesc.contains(cRaceNames[i].first[1]) && (aRace->HasKeyword(fBstKey) == cRaceNames[i].second)) return i;
  }
  return aRace->HasKeyword(fBstKey) ? 9 : 0;
}

void TngCore::GenitalizeNPCSkins() noexcept {
  Tng::gLogger::info("Checking NPCs for custom skins.");
  int lC = 0;
  std::set<std::pair<std::string_view, int>> lCustomSkinMods{};
  for (const auto& lNPC : fAllNPCs) {
    const auto lNPCRace = lNPC->race;
    if (!lNPCRace) {
      Tng::gLogger::warn("The NPC [0x{:x}: {}] does not have a race! They cannot be modified by TNG.", lNPC->GetFormID(), lNPC->GetName());
      continue;
    }
    if (!lNPCRace->HasKeyword(fPRaceKey) && !lNPCRace->HasKeyword(fRRaceKey)) continue;
    const auto lSkin = lNPC->skin;
    if (!lSkin) continue;
    if (lSkin->HasKeyword(fIAKey)) continue;
    if (!lSkin->HasPartOf(Tng::cSlotBody)) continue;
    if (lSkin->HasPartOf(Tng::cSlotGenital)) {
      Tng::gLogger::info("The skin [0x{:x}] used in NPC [{}] cannot have a male genital.", lSkin->GetFormID(), lNPC->GetName());
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
    Tng::gLogger::info("The NPC [{}] from the race [{}] does not have registered racial genitals.", lNPC->GetName(), lNPC->GetRace()->GetFormEditorID());
  }
  if (lC > 0) {
    Tng::gLogger::info("\tHandled custom skins for {} NPCs from following mod(s):", lC);
    for (const auto& lMod : lCustomSkinMods) Tng::gLogger::info("\t\t[{}] skins from {}", lMod.second, lMod.first);
  }
}

Tng::TNGRes TngCore::SetActorSkin(RE::Actor* aActor, int aAddon) noexcept {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  if (!lNPC->race) return Tng::raceErr;
  if (!lNPC->race->HasKeyword(fPRaceKey)) return Tng::raceErr;
  if (aAddon == -1) return !lNPC->IsFemale() || lNPC->skin->HasKeyword(fSwPKey) ? Tng::resOkGen : Tng::resOkNoGen;
  if (aAddon == -2 && lNPC->IsFemale()) {
    TngSizeShape::SetNPCAddn(lNPC, aAddon);
    RevertNPCSkin(lNPC);
    if (!aActor->IsPlayerRef()) TngInis::SaveNPCAddn(lNPC, -3);
    return Tng::resOkNoGen;
  }
  auto lCurrSkin = lNPC->skin ? lNPC->skin : lNPC->race->skin;
  auto lOgFormID = GetOgSkinID(lCurrSkin, lNPC->IsFemale());
  if (lOgFormID == 0) {
    Tng::gLogger::critical("Failed to get the previously created skin for an NPC!");
    return Tng::pgErr;
  }
  int lChoice = aAddon == -2 ? TngSizeShape::GetRaceGrpAddn(lNPC->race) : aAddon;
  auto& lSkinsToLook = lNPC->IsFemale() ? fFemAddonSkins[lChoice] : fMalAddonSkins[lChoice];
  auto lSkinP = std::find_if(lSkinsToLook.begin(), lSkinsToLook.end(), [&lOgFormID](const std::pair<RE::FormID, RE::TESObjectARMO*>& p) { return p.first == lOgFormID; });
  TngSizeShape::SetNPCAddn(lNPC, aAddon);
  auto lSkin = lSkinP->second;
  if (lSkin != lCurrSkin) {
    lNPC->skin = lSkin;
    if (!aActor->IsPlayerRef()) TngInis::SaveNPCAddn(lNPC, aAddon);
  }
  return !lNPC->IsFemale() || lNPC->skin->HasKeyword(fSwPKey) ? Tng::resOkGen : Tng::resOkNoGen;
}

RE::FormID TngCore::GetOgSkinID(RE::TESObjectARMO* aSkin, const bool aIsFemale) noexcept {
  if (aSkin->HasKeyword(fGenSkinKey)) {
    auto& lSkinsToLook = aIsFemale ? fFemAddonSkins : fMalAddonSkins;
    for (const auto& lAddonSkins : lSkinsToLook) {
      auto lSkinEntry = std::find_if(lAddonSkins.begin(), lAddonSkins.end(), [&](const std::pair<RE::FormID, RE::TESObjectARMO*>& p) { return p.second == aSkin; });
      if (lSkinEntry == lAddonSkins.end()) continue;
      return lSkinEntry->first;
    }
    if (aIsFemale) {
      for (const auto& lAddonSkins : fMalAddonSkins) {
        auto lSkinEntry = std::find_if(lAddonSkins.begin(), lAddonSkins.end(), [&](const std::pair<RE::FormID, RE::TESObjectARMO*>& p) { return p.second == aSkin; });
        if (lSkinEntry == lAddonSkins.end()) continue;
        return lSkinEntry->first;
      }
    }
    return RE::FormID{0};
  } else {
    return aSkin->GetFormID();
  }
}

RE::TESObjectARMO* TngCore::GetOgSkin(RE::TESObjectARMO* aSkin, const bool aIsFemale) noexcept {
  if (!aSkin->HasKeyword(fGenSkinKey)) return aSkin;
  auto lID = GetOgSkinID(aSkin, aIsFemale);
  if (lID > 0) {
    auto lSkinEntry = std::find_if(fOgSkins.begin(), fOgSkins.end(), [&](const auto& p) { return p->GetFormID() == lID; });
    if (lSkinEntry != fOgSkins.end()) return *lSkinEntry;
  }
  return nullptr;
}

bool TngCore::FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept {
  if (fOgSkins.find(aSkin) != fOgSkins.end()) return true;
  if (!aSkin->HasPartOf(Tng::cSlotBody)) return false;
  if (aSkin->HasPartOf(Tng::cSlotGenital)) {
    if (aName) Tng::gLogger::info("The skin [0x{:x}: {}] is ready for TNG!", aSkin->GetFormID(), aName);
    aSkin->RemoveKeywords(fArmoKeys);
    aSkin->AddKeyword(fIAKey);
    for (const auto& lAA : aSkin->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody)) fSAAs.insert(lAA);
    return true;
  }
  GentifySkin(aSkin);
  if (aName) Tng::gLogger::info("The skin [0x{:x}: {}] added as extra skin.", aSkin->GetFormID(), aName);
  return true;
}

void TngCore::RevertNPCSkin(RE::TESNPC* aNPC) {
  auto lOgSkin = GetOgSkin(aNPC->skin, aNPC->IsFemale());
  if (lOgSkin)
    aNPC->skin = lOgSkin;
  else {
    Tng::gLogger::critical("Failed to revert the skin of [{}] to its original value!", aNPC->GetName());
  }
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
    if (!lArmor->HasPartOf(Tng::cSlotBody) && !lArmor->HasPartOf(Tng::cSlotGenital)) continue;
    const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
    if (!lArmor->race) {
      Tng::gLogger::warn("The armor [0x{:x}: {}] does not have a race! It won't be touched by Tng!", lArmor->GetFormID(), lID);
      lArmor->AddKeyword(fIAKey);
      continue;
    }
    if (!(lArmor->race->HasKeyword(fPRaceKey) || lArmor->race->HasKeyword(fRRaceKey) || lArmor->race == fDefRace)) continue;
    ProcessArmor(lArmor);
    if (lArmor->HasKeyword(fIAKey) || lArmor->HasKeyword(fUAKey)) continue;
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
        Tng::gLogger::warn("The armor [0x{:x}: {}] is marked covering by an ini but it has slot 32 and would be handled automatically.", lArmor->GetFormID(), lID);
        continue;
      }
      if (lCoverEntry != TngInis::fSingleCoveringIDs.end()) {
        CoverByArmor(lArmor);
        continue;
      }
    }
    if (lCheckRevealMods && (TngInis::fRevealingMods.find(std::string{lArmor->GetFile(0)->GetFilename()}) != TngInis::fRevealingMods.end())) {
      if (lArmor->HasPartOf(Tng::cSlotBody)) {
        Tng::gLogger::info("Armor [0x{:x}: {}] was marked revealing by a TNG ini!", lArmor->GetFormID(), lID);
        lArmor->AddKeyword(fRRKey);
        ProcessArmor(lArmor);
        lRR++;
      }
      continue;
    }
    if (lCheckRevealRecords) {
      const auto lRevealEntry = TngInis::fSingleRevealingIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->GetLocalFormID()));
      if (lRevealEntry != TngInis::fSingleRevealingIDs.end()) {
        Tng::gLogger::info("Armor [0x{:x}: {}] was marked revealing by a TNG ini!", lArmor->GetFormID(), lID);
        lArmor->AddKeyword(fRRKey);
        ProcessArmor(lArmor);
        lRR++;
        continue;
      }
    }
    if (lCheckRuntimeRecords) {
      const auto lRevealEntry = TngInis::fRunTimeRevealingIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->GetLocalFormID()));
      if (lRevealEntry != TngInis::fRunTimeRevealingIDs.end()) {
        lArmor->AddKeyword(fARKey);
        ProcessArmor(lArmor, true);
        Tng::gLogger::info("The armor [[0x{:x}: {}] was marked revealing since it was marked revealing before.", lArmor->GetFormID(), lID);
        lAR++;
        continue;
      }
    }
    if (lArmor->HasPartOf(Tng::cSlotGenital) && !lArmor->HasKeyword(fUAKey)) {
      ProcessArmor(lArmor);
      Tng::gLogger::warn("The armor [0x{:x}: {}] would cover genitals and might cause conflicts with chest armor pieces!", lArmor->GetFormID(), lID);
      lCC++;
      continue;
    }
    if (lArmor->HasPartOf(Tng::cSlotBody)) lPotentialArmor.insert(lArmor);
  }
  for (auto& lAA : fRAAs) {
    if (fCAAs.find(lAA) != fCAAs.end()) {
      Tng::gLogger::error(
          "The armor addon [:x] is shared between two or more armors; and some are marked revealing and others are marked covering! Any armor using that addon would be covering.");
      fRAAs.erase(lAA);
    }
  }
  for (auto& lArmor : lPotentialArmor) {
    switch (HandleArmor(lArmor)) {
      case Tng::resOkAC:
        lAC++;
        break;
      case Tng::resOkCC:
        lCC++;
        break;
      case Tng::resOkAR:
        lAR++;
        break;
      case Tng::resOkIR:
        lAC++;
        break;
      default:
        lArmor->AddKeyword(fPAKey);
        lPA++;
        break;
    }
  }
  for (auto& lAA : fCAAs) {
    if (fRAAs.find(lAA) != fRAAs.end()) {
      Tng::gLogger::error(
          "The armor addon [:x] is shared between two or more armors; and some are marked revealing and others are marked covering! Any armor using that addon would be "
          "revealing.");
      fCAAs.erase(lAA);
    }
    lAA->AddSlotToMask(Tng::cSlotGenital);
  }
  Tng::gLogger::info("Processed {} armor pieces with slot 32 or 52:", lRR + lAR + lAC + lCC + lPA);
  if (lPA > 0) Tng::gLogger::warn("\t{}: seems to be problematic!", lPA);
  if (lCC > 0) Tng::gLogger::info("\t{}: are already covering genitals,", lCC);
  if (lRR > 0) Tng::gLogger::info("\t{}: are already revealing", lRR);
  if (lAC > 0) Tng::gLogger::info("\t{}: were updated to cover genitals", lAC);
  if (lAR > 0) Tng::gLogger::info("\t{}: were updated to be revealing", lAR);
}

Tng::TNGRes TngCore::HandleArmor(RE::TESObjectARMO* aArmor, const bool aIfLog) noexcept {
  if (aArmor->HasKeyword(fIAKey)) return Tng::resOkIR;
  if (aArmor->HasKeyword(fCCKey)) return Tng::resOkCC;
  if (aArmor->HasKeyword(fRRKey) || aArmor->HasKeyword(fARKey)) {
    for (auto& lAA : aArmor->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody) && fRAAs.find(lAA) == fRAAs.end()) {
        aArmor->RemoveKeywords(fArmoKeys);
        aArmor->AddKeyword(fPAKey);
        if (aIfLog)
          Tng::gLogger::error("Conflict about armor [0x{:x}: {}]. It is markerd revealing but uses a covering armor addon!", aArmor->GetFormID(), aArmor->GetFormEditorID());
        return Tng::armoErr;
      }
    return Tng::resOkRR;
  }
  std::set<RE::TESObjectARMA*> lBods{};
  std::set<RE::TESObjectARMA*> lGens{};
  for (const auto& lAA : aArmor->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotBody)) lBods.insert(lAA);
    if (lAA->HasPartOf(Tng::cSlotGenital)) lGens.insert(lAA);
  }
  bool lS = false;
  bool lR = false;
  bool lC = false;
  for (const auto& lAA : lBods) {
    if (fSAAs.find(lAA) != fSAAs.end()) lS = true;
    if (fRAAs.find(lAA) != fRAAs.end()) lR = true;
    if (fCAAs.find(lAA) != fCAAs.end()) lC = true;
  }
  if ((lR || lS) && lC) {
    if (aIfLog)
      Tng::gLogger::error("Conflict about armor [0x{:x}: {}]. It uses both covering and revealing addons at the same time!", aArmor->GetFormID(), aArmor->GetFormEditorID());
    aArmor->RemoveKeywords(fArmoKeys);
    aArmor->AddKeyword(fPAKey);
    return Tng::armoErr;
  }
  if (lR || lS) {
    fRAAs.insert(lBods.begin(), lBods.end());
    aArmor->AddKeyword(fARKey);
    Tng::gLogger::info("The armor [0x{:x}: {}] is markerd revealing since it has {} addons!", aArmor->GetFormID(), aArmor->GetFormEditorID(), lR ? "revealing armor" : "full body");
    return Tng::resOkAR;
  }
  for (const auto& lAA : lBods) {
    if (fCAAs.find(lAA) == fCAAs.end()) fCAAs.insert(lAA);
  }
  aArmor->AddKeyword(fACKey);
  return Tng::resOkAC;
}

void TngCore::ProcessArmor(RE::TESObjectARMO* aArmor, bool aAcceptAR) noexcept {
  int lKwCount = 0;
  for (const auto& lKw : fArmoKeys)
    if (aArmor->HasKeyword(lKw)) lKwCount++;
  if (lKwCount > 1) {
    aArmor->RemoveKeywords(fArmoKeys);
    Tng::gLogger::error("The armor [0x{:x}: {}] has more than one TNG keyword. They were removed!", aArmor->GetFormID(), aArmor->GetFormEditorID());
  }
  if (aArmor->HasKeyword(fIAKey) || aArmor->HasKeyword(fUAKey)) return;
  std::set<RE::TESObjectARMA*> lBods;
  std::set<RE::TESObjectARMA*> lGens;
  for (auto& lAA : aArmor->armorAddons) {
    if (lAA->HasPartOf(Tng::cSlotGenital)) lGens.insert(lAA);
    if (lAA->HasPartOf(Tng::cSlotBody)) lBods.insert(lAA);
  }
  if (lBods.size() == 0) {
    aArmor->RemoveKeywords(fArmoKeys);
    if (lGens.size() == 0) {
      Tng::gLogger::info("The armor [0x{:x}: {}] would be neglected.", aArmor->GetFormID(), aArmor->GetFormEditorID());
      aArmor->AddKeyword(fIAKey);
    } else {
      Tng::gLogger::info("The armor [0x{:x}: {}] would cover genitals.", aArmor->GetFormID(), aArmor->GetFormEditorID());
      aArmor->AddKeyword(fCCKey);
    }
    return;
  }
  if (aArmor->HasKeywordInArray(fArmoKeys, false)) {
    if (aArmor->HasKeyword(fRRKey)) fRAAs.insert(lBods.begin(), lBods.end());
    if (aArmor->HasKeyword(fCCKey)) fCAAs.insert(lBods.begin(), lBods.end());
    if (aArmor->HasKeyword(fARKey) && aAcceptAR) fRAAs.insert(lBods.begin(), lBods.end());
    if (aArmor->HasKeyword(fPAKey) || (aArmor->HasKeyword(fARKey) && !aAcceptAR) || aArmor->HasKeyword(fACKey)) {
      Tng::gLogger::error("The armor [0x {:x}:{}] has some keywords that should not be distributed manually!", aArmor->GetFormID(), aArmor->GetFormEditorID());
      aArmor->RemoveKeywords(fArmoKeys);
    }
    return;
  }
  if (lGens.size() > 0) {
    aArmor->AddKeyword(fCCKey);
    return;
  }
}

bool TngCore::SwapRevealing(RE::TESObjectARMO* aArmor) noexcept {
  if (!(aArmor->HasKeyword(fACKey) || aArmor->HasKeyword(fARKey))) return false;
  if (aArmor->HasKeyword(fACKey)) {
    for (const auto& lAA : aArmor->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody)) {
        lAA->RemoveSlotFromMask(Tng::cSlotGenital);
        if (fCAAs.find(lAA) != fCAAs.end()) fCAAs.erase(lAA);
        fRAAs.insert(lAA);
      }
    aArmor->RemoveKeyword(fACKey);
    aArmor->AddKeyword(fARKey);
    TngInis::SaveRevealingArmor(aArmor);
  } else {
    for (const auto& lAA : aArmor->armorAddons)
      if (lAA->HasPartOf(Tng::cSlotBody)) {
        lAA->AddSlotToMask(Tng::cSlotGenital);
        if (fRAAs.find(lAA) != fRAAs.end()) fRAAs.erase(lAA);
        fCAAs.insert(lAA);
      }
    aArmor->RemoveKeyword(fARKey);
    aArmor->AddKeyword(fACKey);
    TngInis::RemoveRevealingArmor(aArmor);
  }
  return true;
}

void TngCore::FixArmor(RE::TESObjectARMO* aArmor) noexcept {
  if (aArmor->HasKeywordInArray(fArmoKeys, false) && !aArmor->HasKeyword(fCCKey) && !aArmor->HasKeyword(fACKey)) return;
  if (aArmor->HasPartOf(Tng::cSlotGenital)) return;
  for (auto& lAA : aArmor->armorAddons)
    if (lAA->HasPartOf(Tng::cSlotGenital)) return;
  for (auto& lAA : aArmor->armorAddons)
    if (lAA->HasPartOf(Tng::cSlotBody)) {
      lAA->AddSlotToMask(Tng::cSlotGenital);
      fCAAs.insert(lAA);
    }
  if (!aArmor->HasKeyword(fCCKey) && !aArmor->HasKeyword(fACKey)) aArmor->AddKeyword(fACKey);
}

void TngCore::CoverByArmor(RE::TESObjectARMO* aArmor) noexcept {
  aArmor->AddKeyword(fCCKey);
  const auto lID = (std::string(aArmor->GetName()).empty()) ? aArmor->GetFormEditorID() : aArmor->GetName();
  if (!(aArmor->HasPartOf(Tng::cSlotGenital) || aArmor->armorAddons[0]->HasPartOf(Tng::cSlotGenital))) {
    aArmor->armorAddons[0]->AddSlotToMask(Tng::cSlotGenital);
    Tng::gLogger::info("The armor [0x{:x}: {}] was made covering by a TNG ini.", aArmor->GetFormID(), lID);
  } else {
    Tng::gLogger::info("The armor [0x{:x}: {}] is marked covering by a TNG ini but it already covers genitalia.", aArmor->GetFormID(), lID);
  }
}
