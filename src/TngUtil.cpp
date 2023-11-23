#include <TngInis.h>
#include <TngUtil.h>

int TngUtil::fRCount;
int TngUtil::fQCount;
int TngUtil::fCCount;

void TngUtil::AddGenitalToSkin(RE::TESObjectARMO* aSkin, RE::TESObjectARMA* aGenital) noexcept {
  for (const auto& lAA : aSkin->armorAddons)
    if (lAA == aGenital) return;
  if (fHandledSkins.find(aSkin) == fHandledSkins.end())
    for (const auto& lAA : aSkin->armorAddons)
      if (lAA->HasPartOf(cSlotBody) && lAA->race->HasKeyword(fNPCKey)) fSkinAAs.insert(lAA);
  aSkin->AddSlotToMask(cSlotGenital);
  aSkin->armorAddons.emplace_back(aGenital);
  fHandledSkins.insert(aSkin);
}

void TngUtil::AddRace(RE::TESRace* aRace, RE::TESObjectARMA* aGenital) noexcept {
  if (aRace->HasPartOf(cSlotGenital)) {
    gLogger::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
    fIgnoreRaces.insert(aRace);
    if (aRace->skin) {
      fRacialSkins.insert(aRace->skin);
      fHandledSkins.insert(aRace->skin);
    }
    return;
  }
  if (aGenital) {
    aRace->AddSlotToMask(cSlotGenital);
    AddGenitalToSkin(aRace->skin, aGenital);
    fRacialSkins.insert(aRace->skin);
    fHandledRaces.insert(aRace);
    return;
  }
  if (!aRace->skin) {
    gLogger::warn("The race [0x{:x}:{}] from file [{}] cannot have any genitals since they do not have a skin. If they should, a patch is required.", aRace->GetFormID(),
                  aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
    fIgnoreRaces.insert(aRace);
    return;
  }
  if (aRace->HasKeyword(fBeastKey)) {
    const auto lRaceDesc = std::string(aRace->GetFormEditorID()) + std::string(aRace->GetName());
    if ((lRaceDesc.find("Khajiit") != std::string::npos) || (lRaceDesc.find("Rhat") != std::string::npos)) {
      fDefKhaGenital->additionalRaces.emplace_back(aRace);
      gLogger::info("The race [0x{:x}:{}] from file [{}] was recognized as Khajiit. If this is wrong, a patch is required.", aRace->GetFormID(), aRace->GetFormEditorID(),
                    aRace->GetFile(0)->GetFilename());
      fExtrRaceGens.insert(std::make_pair(aRace, aGenital));
      AddRace(aRace, fDefKhaGenital);
    } else {
      if ((lRaceDesc.find("Argonian") != std::string::npos) || (lRaceDesc.find("Saxhleel") != std::string::npos)) {
        fDefSaxGenital->additionalRaces.emplace_back(aRace);
        gLogger::info("The race [0x{:x}:{}] from file [{}] was recognized as Saxhleel(Argonian). If this is wrong, a patch is required.", aRace->GetFormID(),
                      aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
        fExtrRaceGens.insert(std::make_pair(aRace, aGenital));
        AddRace(aRace, fDefSaxGenital);
      } else {
        gLogger::warn("The race [0x{:x}:{}] from file [{}] could not be recognized and did not receive any genital. If they should, a patch is required.", aRace->GetFormID(),
                      aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
        fIgnoreRaces.insert(aRace);
        fRacialSkins.insert(aRace->skin);
        fHandledSkins.insert(aRace->skin);
      }
    }
  } else {
    fDefMnmGenital->additionalRaces.emplace_back(aRace);
    gLogger::info("The race [0x{:x}:{}] from file [{}] received the default genital for man and mer races. If this is wrong, a patch is required.", aRace->GetFormID(),
                  aRace->GetFormEditorID(), aRace->GetFile(0)->GetFilename());
    fExtrRaceGens.insert(std::make_pair(aRace, aGenital));
    AddRace(aRace, fDefMnmGenital);
  }
}

void TngUtil::HandleArmor(RE::TESObjectARMO* aArmor) noexcept {
  if (aArmor->HasKeyword(fRevealingKey)) {
    fRCount++;
    return;
  }
  for (const auto& lAA : aArmor->armorAddons) {
    if (fSkinAAs.find(lAA) != fSkinAAs.end()) {
      aArmor->AddKeyword(fRevealingKey);
      gLogger::info("The armor [0x{:x}:{}] from file [{}] is markerd revealing. If this is a mistake, a patch is required!", aArmor->GetFormID(), aArmor->GetFormEditorID(),
                    aArmor->GetFile(0)->GetFilename());
      fRCount++;
      return;
    }
    if (lAA->HasPartOf(cSlotGenital)) {
      if (fHandledArma.find(lAA) != fHandledArma.end()) {
        fCCount++;
        return;
      } else {
        fQCount++;
        return;
      }
    }
  }
  aArmor->armorAddons[0]->AddSlotToMask(cSlotGenital);
  fHandledArma.insert(aArmor->armorAddons[0]);
  fCCount++;
}

bool TngUtil::Initialize() noexcept {
  fDataHandler = RE::TESDataHandler::GetSingleton();
  if (!fDataHandler->LookupModByName(cTNGName)) {
    gLogger::error("Mod [{}] was not found!", cTNGName);
    return FALSE;
  }
  fBeastKey = RE::TESForm::LookupByID<RE::BGSKeyword>(cBstKeywID);
  fRevealingKey = fDataHandler->LookupForm<RE::BGSKeyword>(cRevealingKeyID, cTNGName);
  fUnderwearKey = fDataHandler->LookupForm<RE::BGSKeyword>(cUnderwearKeyID, cTNGName);
  fCoveringKey = fDataHandler->LookupForm<RE::BGSKeyword>(cCoveringKeyID, cTNGName);
  if (!(fRevealingKey && fUnderwearKey)) {
    gLogger::error("The original TNG keywords could not be found!");
    return FALSE;
  }
  fDefRace = RE::TESForm::LookupByID<RE::TESRace>(cDefRaceID);

  gLogger::info("Finding the genitals to respective races...");
  for (int i = 0; i < cRaceTypes; i++) {
    const auto lRace = RE::TESForm::LookupByID<RE::TESRace>(cBaseRaceIDs[i]);
    const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cGenitalIDs[i], cTNGName);
    if (!(lRace && lGenital)) {
      gLogger::error("Original information cannot be found!");
      return FALSE;
    }
    fBaseRaceGens.insert(std::make_pair(lRace, lGenital));
  }
  for (const auto& lRaceID : cEquiRaceIDs) {
    const auto lRace = RE::TESForm::LookupByID<RE::TESRace>(lRaceID.first);
    const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cGenitalIDs[lRaceID.second], cTNGName);
    if (!(lRace && lGenital)) {
      gLogger::error("Original information cannot be found!");
      return FALSE;
    }
    fEquiRaceGens.insert(std::make_pair(lRace, lGenital));
  }
  fDefSaxGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalSaxID, cTNGName);
  fDefKhaGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalKhaID, cTNGName);
  fDefMnmGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalMnmID, cTNGName);
  if (!fDefMnmGenital || !fDefKhaGenital || !fDefSaxGenital) {
    gLogger::error("The original TNG Default-genitals cannot be found!");
  }
  for (const auto& lID : cExclRaceIDs) {
    auto lRace = fExclRaces.insert(RE::TESForm::LookupByID<RE::TESRace>(lID));
    if ((*lRace.first)->skin) {
      fRacialSkins.insert((*lRace.first)->skin);
      fHandledSkins.insert((*lRace.first)->skin);
    }
  }
  fNPCKey = RE::TESForm::LookupByID<RE::BGSKeyword>(cNPCKeywID);
  fCreatureKey = RE::TESForm::LookupByID<RE::BGSKeyword>(cCrtKeywID);
  return TRUE;
}

void TngUtil::GenitalizeRaces() noexcept {
  gLogger::info("Assigning genitals to races...");
  for (const auto& lRaceGen : fBaseRaceGens) AddRace(lRaceGen.first, lRaceGen.second);
  for (const auto& lRaceGen : fEquiRaceGens) AddRace(lRaceGen.first, lRaceGen.second);
  auto& lAllRacesArray = fDataHandler->GetFormArray<RE::TESRace>();
  for (const auto& lRace : lAllRacesArray) {
    if (fHandledRaces.find(lRace) != fHandledRaces.end()) continue;
    if (fExclRaces.find(lRace) != fExclRaces.end()) continue;
    if (lRace->HasKeyword(fNPCKey) && !lRace->HasKeyword(fCreatureKey) && lRace->HasPartOf(cSlotBody) && !lRace->IsChildRace()) {
      AddRace(lRace, nullptr);
    }
  }
  fAllRaceGens.insert(fBaseRaceGens.begin(), fBaseRaceGens.end());
  fAllRaceGens.insert(fEquiRaceGens.begin(), fEquiRaceGens.end());
  fAllRaceGens.insert(fExtrRaceGens.begin(), fExtrRaceGens.end());
  gLogger::info("TNG recognized assigned genitals to [{}] races: [{}] from base-game races and [{}] from other mods.", fAllRaceGens.size(),
                fBaseRaceGens.size() + fEquiRaceGens.size(), fExtrRaceGens.size());
}

void TngUtil::GenitalizeNPCSkins() noexcept {
  auto& lAllNPCs = fDataHandler->GetFormArray<RE::TESNPC>();
  int lAllCount = lAllNPCs.size();
  gLogger::info("Checking NPCs for custom skins: There are {} NPC records.", lAllCount);
  int lIrr = lAllCount;
  int lHdr = lAllCount;
  int lNob = lAllCount;
  int lC = 0;
  std::set<std::pair<std::string_view, int>> lCustomSkinMods;
  for (const auto& lNPC : lAllNPCs) {
    const auto lNPCRace = lNPC->race;
    if (!lNPCRace) {
      gLogger::warn("The NPC [{}] from file [{}] does not have a race! They cannot be modified by TNG.", lNPC->GetName(), lNPC->GetFile(0)->GetFilename());
      continue;
    }
    const auto lSkin = lNPC->skin;
    if (!lSkin) continue;
    if (!lNPCRace->HasKeyword(fNPCKey) || lNPCRace->HasKeyword(fCreatureKey) || lNPCRace->IsChildRace()) continue;
    if (fExclRaces.find(lNPCRace) != fExclRaces.end()) continue;
    lIrr--;
    if ((fRacialSkins.find(lSkin) != fRacialSkins.end()) || (fIgnoreRaces.find(lNPCRace) != fIgnoreRaces.end())) continue;
    lHdr--;
    if (fHandledSkins.find(lSkin) != fHandledSkins.end()) {
      lC++;
      lNob--;
      continue;
    }
    if (!lSkin->HasPartOf(cSlotBody)) continue;
    if (lSkin->HasPartOf(cSlotGenital)) {
      gLogger::info("The skin [0x{:x}] used in NPC [{}] from file [{}] cannot have a male genital. If this is wrong, a patch is required.", lSkin->GetFormID(), lNPC->GetName(),
                    lNPC->GetFile(0)->GetFilename());
      continue;
    }
    lNob--;
    auto lRaceGen = std::find_if(fAllRaceGens.begin(), fAllRaceGens.end(), [&lNPC](const std::pair<RE::TESRace*, RE::TESObjectARMA*>& p) { return p.first == lNPC->race; });
    if (lRaceGen != fAllRaceGens.end()) {
      AddGenitalToSkin(lSkin, (*lRaceGen).second);
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
    gLogger::info("The NPC [{}] from the race [{}] does not have registered racial genitals. If they should, a patch is required.", lNPC->GetName(),
                  lNPC->GetRace()->GetFormEditorID());
  }
  gLogger::info("Out of the [{}] NPCs:", lAllCount);
  gLogger::info("\t{} NPCs do not have a custom skin (WNAM) or are not adult humanoids,", lIrr);
  gLogger::info("\t{} NPCs' skins are already handled with races,", lHdr - lIrr);
  if ((lNob - lHdr) > 0) gLogger::info("\t{} NPCs' skins don't have body slot or already have genital slot.", lNob - lHdr);
  if (lC > 0) {
    gLogger::info("\tTNG handled custom skins for {} NPCs from following mod(s):", lC);
    for (const auto& lMod : lCustomSkinMods) gLogger::info("\t\t[{}] skins from {}", lMod.second, lMod.first);
  }
}

void TngUtil::CheckArmorPieces() noexcept {
  gLogger::info("Checking armor pieces...");
  fRCount = 0;
  fCCount = 0;
  fQCount = 0;
  TngInis::LoadTngInis();
  const std::set<std::string> lSkinMods(TngInis::fSkinMods);
  const std::set<std::pair<std::string, RE::FormID>> lSingleSkinIDs(TngInis::fSingleSkinIDs);
  const std::set<std::string> lRevealingMods(TngInis::fRevealingMods);
  const std::set<std::pair<std::string, RE::FormID>> lSingleRevealingIDs(TngInis::fSingleRevealingIDs);
  const std::set<std::pair<std::string, RE::FormID>> lSingleCoveringIDs(TngInis::fSingleCoveringIDs);
  const std::set<std::pair<std::string, RE::BGSBipedObjectForm::BipedObjectSlot>> lSwapMods(TngInis::fSwapMods);

  auto& lAllArmor = fDataHandler->GetFormArray<RE::TESObjectARMO>();

  bool lCheckSkinMods = (lSkinMods.size() > 0);
  bool lCheckSkinRecords = (lSingleSkinIDs.size() > 0);
  bool lCheckSlotSwap = (lSwapMods.size() > 0);
  bool lCheckRevealMods = (lRevealingMods.size() > 0);
  bool lCheckRevealRecords = (lSingleRevealingIDs.size() > 0);
  bool lCheckCoverRecords = (lSingleCoveringIDs.size() > 0);

  for (const auto& lModName : lSkinMods)
    if (fDataHandler->LookupModByName(lModName)) gLogger::info("TheNewGentleman keeps an eye for [{}] as a skin mod.", lModName);
  for (const auto& lModSlot : lSwapMods)
    if (fDataHandler->LookupModByName(lModSlot.first)) gLogger::info("TheNewGentleman keeps an eye for [{}] as a slot swap mod.", lModSlot.first);
  for (const auto& lRevealMod : lRevealingMods)
    if (fDataHandler->LookupModByName(lRevealMod)) gLogger::info("TheNewGentleman keeps an eye for [{}] as a revealing armor mod.", lRevealMod);

  for (const auto& lArmor : lAllArmor) {
    if (fHandledSkins.find(lArmor) != fHandledSkins.end()) continue;
    if (lArmor->armorAddons.size() == 0) continue;
    if (lCheckSkinMods && (lSkinMods.find(std::string{lArmor->GetFile(0)->GetFilename()}) != lSkinMods.end())) {
      if (!lArmor->HasPartOf(cSlotBody)) continue;
      const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
      if (lArmor->HasPartOf(cSlotGenital)) {
        gLogger::info("The skin [0x{:x}:{}] from file [{}] is ready for TNG!", lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
        continue;
      }
      std::set<RE::TESRace*> lSkinRaces;
      for (const auto& lAA : lArmor->armorAddons) {
        lSkinRaces.insert(lAA->race);
        lSkinRaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
      }
      for (const auto& lRace : lSkinRaces) {
        auto lRaceGen = std::find_if(fAllRaceGens.begin(), fAllRaceGens.end(), [&lRace](const std::pair<RE::TESRace*, RE::TESObjectARMA*>& p) { return p.first == lRace; });
        if (lRaceGen != fAllRaceGens.end()) AddGenitalToSkin(lArmor, (*lRaceGen).second);
      }
      gLogger::info("The skin [0x{:x}:{}] from file [{}] added as extra skin", lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
      continue;
    }
    if (lCheckSkinRecords) {
      const auto lSkinEntry = lSingleSkinIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->formID));
      if (lSkinEntry != lSingleSkinIDs.end()) {
        std::set<RE::TESRace*> lSkinRaces;
        for (const auto& lAA : lArmor->armorAddons) {
          lSkinRaces.insert(lAA->race);
          lSkinRaces.insert(lAA->additionalRaces.begin(), lAA->additionalRaces.end());
        }
        if (lSkinRaces.find(fDefRace) != lSkinRaces.end()) lSkinRaces.erase(fDefRace);
        for (const auto& lRace : lSkinRaces) {
          auto lRaceGen = std::find_if(fAllRaceGens.begin(), fAllRaceGens.end(), [&lRace](const std::pair<RE::TESRace*, RE::TESObjectARMA*>& p) { return p.first == lRace; });
          if (lRaceGen != fAllRaceGens.end()) AddGenitalToSkin(lArmor, (*lRaceGen).second);
        }
        const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
        gLogger::info("The skin [0x{:x}:{}] from file [{}] added as extra skin", lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
        continue;
      }
    }
    if (lCheckCoverRecords) {
      const auto lCoverEntry = lSingleCoveringIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->formID));
      if (lCoverEntry != lSingleCoveringIDs.end()) {
        lArmor->AddKeyword(fCoveringKey);
        if (fHandledArma.find(lArmor->armorAddons[0]) != fHandledArma.end()) {
          lArmor->armorAddons[0]->AddSlotToMask(cSlotGenital);
          fHandledArma.insert(lArmor->armorAddons[0]);
        }
        fCCount++;
        continue;
      }
    }
    if (lCheckRevealMods && (lRevealingMods.find(std::string{lArmor->GetFile(0)->GetFilename()}) != lRevealingMods.end())) {
      if (lArmor->HasPartOf(cSlotBody)) {
        lArmor->AddKeyword(fRevealingKey);
        fRCount++;
      }
      continue;
    }
    if (lCheckRevealRecords) {
      const auto lRevealEntry = lSingleRevealingIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->formID));
      if (lRevealEntry != lSingleRevealingIDs.end()) {
        lArmor->AddKeyword(fRevealingKey);
        fRCount++;
        continue;
      }
    }
    if (lCheckSlotSwap) {
      const auto lSwapEntry = std::find_if(lSwapMods.begin(), lSwapMods.end(), [&lArmor](const std::pair<std::string_view, RE::BGSBipedObjectForm::BipedObjectSlot>& p) {
        return p.first == lArmor->GetFile(0)->GetFilename();
      });
      if (lSwapEntry != lSwapMods.end()) {
        if (lArmor->HasPartOf((*lSwapEntry).second)) HandleArmor(lArmor);
        continue;
      }
    }
    if (lArmor->HasPartOf(cSlotGenital) && !lArmor->HasKeyword(fUnderwearKey)) {
      const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
      gLogger::info("The armor [0x{:x}:{}] from file [{}] would cover genitals and be covered by chest armor pieces (like an underwear). If it is wrong, a patch is required.",
                    lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
      fQCount++;
      continue;
    }
    if (lArmor->HasPartOf(cSlotBody) || lArmor->HasKeyword(fCoveringKey)) HandleArmor(lArmor);
  }
  gLogger::info("Processed {} body armor pieces:", fCCount + fRCount + fQCount);
  gLogger::info("\t{}: already covering genitals", fQCount);
  gLogger::info("\t{}: revealing", fRCount);
  gLogger::info("\t{}: updated to cover genitals", fCCount);
}
