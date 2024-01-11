#include <TngInis.h>
#include <TngSizeShape.h>
#include <TngUtil.h>

int TngUtil::fRCount;
int TngUtil::fQCount;
int TngUtil::fCCount;

bool TngUtil::FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept {
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

void TngUtil::AddGenitalToSkin(RE::TESObjectARMO* aSkin, RE::TESObjectARMA* aGenital, RE::TESRace* aRace) noexcept {
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

void TngUtil::IgnoreRace(RE::TESRace* aRace) {
  fIgnoreRaces.insert(aRace);
  if (aRace->skin) {
    fRacialSkins.insert(aRace->skin);
    fHandledSkins.insert(aRace->skin);
  }
}

bool TngUtil::CheckRace(RE::TESRace* aRace) {
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

void TngUtil::AddRace(RE::TESRace* aRace, RE::TESObjectARMA* aGenital, RE::TESRace* aRNAM) noexcept {
  if (aRace->HasPartOf(Tng::cSlotGenital)) {
    Tng::gLogger::info("The race [{}] seems to be ready for TNG. It was not modified.", aRace->GetFormEditorID());
    IgnoreRace(aRace);
    return;
  }
  if (aGenital) {
    aRace->AddSlotToMask(Tng::cSlotGenital);
    aRace->AddKeyword(fTNGRaceKey);
    AddGenitalToSkin(aRace->skin, aGenital, aRNAM);
    fRacialSkins.insert(aRace->skin);
    fHandledRaces.insert(aRace);
    return;
  }
  if (aRace->HasKeyword(fBeastKey)) {
    const auto lRaceDesc = std::string(aRace->GetFormEditorID()) + std::string(aRace->GetName());
    if ((lRaceDesc.find("Khajiit") != std::string::npos) || (lRaceDesc.find("Rhat") != std::string::npos)) {
      RE::TESRace* lRNAM = (aRace->armorParentRace && aRace->armorParentRace != fDefRace) ? aRace->armorParentRace : aRace;
      int aChoice = TngSizeShape::genitalChoices[Tng::cRaceTypes + 2] < 3 ? TngSizeShape::genitalChoices[Tng::cRaceTypes + 2] : TngSizeShape::cGenitalDefaults[Tng::cRaceTypes + 2];
      std::set<RE::TESRace*> lGenRaces{fDefKhaGenital[aChoice]->additionalRaces.begin(), fDefKhaGenital[aChoice]->additionalRaces.end()};
      fDefKhaGenital[aChoice]->additionalRaces.emplace_back(aRace);
      if (lGenRaces.find(lRNAM) == lGenRaces.end()) fDefKhaGenital[aChoice]->additionalRaces.emplace_back(lRNAM);
      Tng::gLogger::info("The race [0x{:x}:{}] from file [{}] was recognized as Khajiit. If this is wrong, a patch is required.", aRace->GetFormID(), aRace->GetFormEditorID(),
                         aRace->GetFile(0)->GetFilename());
      fExtrRaceGens.insert(std::make_pair(aRace, aGenital));
      TngSizeShape::AddRaceIndex(aRace->GetFormID(), Tng::cRaceTypes + 2);
      AddRace(aRace, fDefKhaGenital[aChoice], lRNAM);
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
        AddRace(aRace, fDefSaxGenital[aChoice], lRNAM);
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
    AddRace(aRace, fDefMnmGenital[aChoice], lRNAM);
  }
}

void TngUtil::HandleArmor(RE::TESObjectARMO* aArmor) noexcept {
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
  if (lGen) {
    aArmor->AddKeyword(fAutoCoverKey);
    if (!lBod) {
      fQCount++;
      return;
    }
    if (fCoveringAAs.find(lBod) != fCoveringAAs.end()) {
      fCCount++;
      return;
    } else {
      fQCount++;
      return;
    }
  }
  if (lBod) {
    if (aArmor->HasKeyword(fRevealingKey)) {
      if (fCoveringAAs.find(lBod) != fCoveringAAs.end()) {
        Tng::gLogger::info(
            "There is a conflict about armor [0x{:x}:{}] from file [{}]. It is markerd revealing but at the same time it uses an armor addon that is already covering!",
            aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        fCCount++;
        return;
      }
      fRevealAAs.insert(lBod);
      fRCount++;
      return;
    }
    if (fRevealAAs.find(lBod) != fRevealAAs.end()) {
      aArmor->AddKeyword(fAutoRvealKey);
      Tng::gLogger::info(
          "The armor [0x{:x}:{}] from file [{}] is markerd revealing since it shares the body slot with another revealing armor! If it should be covering, a patch is required.",
          aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      fRCount++;
      return;
    }
    if (fSkinAAs.find(lBod) != fSkinAAs.end()) {
      aArmor->AddKeyword(fAutoRvealKey);
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] is markerd revealing. If this is a mistake, a patch is required!", aArmor->GetFormID(), aArmor->GetFormEditorID(),
                         aArmor->GetFile(0)->GetFilename());
      fRCount++;
      return;
    }
    lBod->AddSlotToMask(Tng::cSlotGenital);
    aArmor->AddKeyword(fAutoCoverKey);
    fCoveringAAs.insert(lBod);
    fCCount++;
    return;
  }
  if (lBods.size() == 0) {
    aArmor->AddKeyword(fAutoCoverKey);
    fQCount++;
    return;
  }
  if (lGens.size() > 0) {
    for (const auto& lAA : lGens) {
      if (lBods.find(lAA) == lBods.end()) {
        Tng::gLogger::warn("The armor [0x{:x}:{}] from file [{}] cannot be patched automatically! If it should be covering and it is not, a patch is required.",
                           aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        return;
      }
    }
  }
  bool lbIsRevealing = false;
  for (const auto& lAA : lBods) {
    if (aArmor->HasKeyword(fRevealingKey) || aArmor->HasKeyword(fAutoRvealKey)) {
      if (fCoveringAAs.find(lAA) != fCoveringAAs.end()) {
        Tng::gLogger::info(
            "There is a conflict about armor [0x{:x}:{}] from file [{}]. It is markerd revealing but at the same time it uses an armor addon that is already covering!",
            aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
        fCCount++;
        return;
      }
      lbIsRevealing = true;
      fRevealAAs.insert(lAA);
      continue;
    }
    if (fRevealAAs.find(lAA) != fRevealAAs.end()) {
      aArmor->AddKeyword(fAutoRvealKey);
      Tng::gLogger::info(
          "The armor [0x{:x}:{}] from file [{}] is markerd revealing since it shares the body slot with another revealing armor! If it should be covering, a patch is required.",
          aArmor->GetFormID(), aArmor->GetFormEditorID(), aArmor->GetFile(0)->GetFilename());
      lbIsRevealing = true;
      continue;
    }
    if (fSkinAAs.find(lAA) != fSkinAAs.end()) {
      aArmor->AddKeyword(fAutoRvealKey);
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] is markerd revealing. If this is a mistake, a patch is required!", aArmor->GetFormID(), aArmor->GetFormEditorID(),
                         aArmor->GetFile(0)->GetFilename());
      lbIsRevealing = true;
      continue;
    }
  }
  if (lbIsRevealing) {
    fRCount++;
    return;
  }
  for (const auto& lAA : lBods) {
    if (fCoveringAAs.find(lAA) == fCoveringAAs.end()) {
      lAA->AddSlotToMask(Tng::cSlotGenital);
      fCoveringAAs.insert(lBod);
    }
  }
  aArmor->AddKeyword(fAutoCoverKey);
  fCCount++;
}

void TngUtil::CoverByArmor(RE::TESObjectARMO* aArmor) noexcept {
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

void TngUtil::ReCheckArmor(RE::TESObjectARMO* aArmor) noexcept {
  for (const auto& lAA : aArmor->armorAddons)
    if (fCoveringAAs.find(lAA) != fCoveringAAs.end()) {
      const auto lID = (std::string(aArmor->GetName()).empty()) ? aArmor->GetFormEditorID() : aArmor->GetName();
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] would be covering since it share armor addons with another covering armor.", aArmor->GetFormID(), lID,
                         aArmor->GetFile(0)->GetFilename());
      return;
    }
}

bool TngUtil::Initialize() noexcept {
  fDataHandler = RE::TESDataHandler::GetSingleton();
  if (!fDataHandler->LookupModByName(Tng::cName)) {
    Tng::gLogger::error("Mod [{}] was not found!", Tng::cName);
    return FALSE;
  }
  fNPCKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cNPCKeywID, Tng::cSkyrim);
  fCreatureKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cCrtKeywID, Tng::cSkyrim);
  fBeastKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cBstKeywID, Tng::cSkyrim);
  fAutoRvealKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cAutoRvealKeyID, Tng::cName);
  fRevealingKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  fUnderwearKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cUnderwearKeyID, Tng::cName);
  fAutoCoverKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  fCoveringKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cCoveringKeyID, Tng::cName);
  fTNGRaceKey = fDataHandler->LookupForm<RE::BGSKeyword>(Tng::cTNGRaceKeyID, Tng::cName);
  if (!(fAutoRvealKey && fRevealingKey && fUnderwearKey && fAutoCoverKey && fCoveringKey && fTNGRaceKey)) {
    Tng::gLogger::error("The original TNG keywords could not be found!");
    return FALSE;
  }
  fDefRace = RE::TESForm::LookupByID<RE::TESRace>(cDefRaceID);
  fDefSkeleton[0] = fDefRace->skeletonModels[0].model;
  fDefSkeleton[1] = fDefRace->skeletonModels[1].model;
  fDefSkinAA = RE::TESForm::LookupByID<RE::TESObjectARMA>(cDefSkinAAID);
  fDefBodyMesh[0] = fDefSkinAA->bipedModels[0].model;
  fDefBodyMesh[1] = fDefSkinAA->bipedModels[1].model;
  Tng::gLogger::info("Finding the genitals to respective races...");
  for (int i = 0; i < Tng::cRaceTypes; i++) {
    const auto lRace = fDataHandler->LookupForm<RE::TESRace>(cBaseRaceIDs[i].first, cBaseRaceIDs[i].second);
    auto aChoice = (TngSizeShape::genitalChoices[i] < 3 ? TngSizeShape::genitalChoices[i] : TngSizeShape::cGenitalDefaults[i]) * Tng::cRaceTypes + i;
    const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cGenitalIDs[aChoice], Tng::cName);
    if (!(lRace && lGenital)) {
      Tng::gLogger::error("Original information cannot be found!");
      return FALSE;
    }
    TngSizeShape::AddRaceIndex(lRace->GetFormID(), i);
    fBaseRaceGens.insert(std::make_pair(lRace, lGenital));
  }
  for (const auto& lRaceID : cEquiRaceIDs) {
    const auto lRace = fDataHandler->LookupForm<RE::TESRace>(lRaceID.first.first, lRaceID.first.second);
    auto aChoice =
        (TngSizeShape::genitalChoices[lRaceID.second] < 3 ? TngSizeShape::genitalChoices[lRaceID.second] : TngSizeShape::cGenitalDefaults[lRaceID.second]) * Tng::cRaceTypes +
        lRaceID.second;
    const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cGenitalIDs[aChoice], Tng::cName);
    if (!(lRace && lGenital)) {
      Tng::gLogger::error("Original information cannot be found!");
      return FALSE;
    }
    TngSizeShape::AddRaceIndex(lRace->GetFormID(), lRaceID.second);
    fEquiRaceGens.insert(std::make_pair(lRace, lGenital));
  }
  for (int i = 0; i < 3; i++) {
    fDefSaxGenital[i] = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalSaxID[i], Tng::cName);
    fDefKhaGenital[i] = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalKhaID[i], Tng::cName);
    fDefMnmGenital[i] = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalMnmID[i], Tng::cName);
    if (!fDefMnmGenital[i] || !fDefKhaGenital[i] || !fDefSaxGenital[i]) {
      Tng::gLogger::error("The original TNG Default-genitals cannot be found!");
      return FALSE;
    }
  }
  for (const auto& lRaceID : cExclRaceIDs) {
    auto lRace = fExclRaces.insert(fDataHandler->LookupForm<RE::TESRace>(lRaceID.first, lRaceID.second));
    if ((*lRace.first)->skin) {
      fRacialSkins.insert((*lRace.first)->skin);
      fHandledSkins.insert((*lRace.first)->skin);
    }
  }
  fAllNPCs = fDataHandler->GetFormArray<RE::TESNPC>();
  return TRUE;
}

void TngUtil::GenitalizeRaces() noexcept {
  Tng::gLogger::info("Assigning genitals to races...");
  for (const auto& lRaceGen : fBaseRaceGens) AddRace(lRaceGen.first, lRaceGen.second);
  for (const auto& lRaceGen : fEquiRaceGens) AddRace(lRaceGen.first, lRaceGen.second);
  auto& lAllRacesArray = fDataHandler->GetFormArray<RE::TESRace>();
  for (const auto& lRace : lAllRacesArray) {
    if (fHandledRaces.find(lRace) != fHandledRaces.end()) continue;
    if (fExclRaces.find(lRace) != fExclRaces.end()) continue;
    if (CheckRace(lRace)) AddRace(lRace, nullptr);
  }
  fAllRaceGens.insert(fBaseRaceGens.begin(), fBaseRaceGens.end());
  fAllRaceGens.insert(fEquiRaceGens.begin(), fEquiRaceGens.end());
  fAllRaceGens.insert(fExtrRaceGens.begin(), fExtrRaceGens.end());
  Tng::gLogger::info("TNG recognized assigned genitals to [{}] races: [{}] from base-game races and [{}] from other mods.", fAllRaceGens.size(),
                     fBaseRaceGens.size() + fEquiRaceGens.size(), fExtrRaceGens.size());
}

void TngUtil::GenitalizeNPCSkins() noexcept {
  int lAllCount = fAllNPCs.size();
  Tng::gLogger::info("Checking NPCs for custom skins: There are {} NPC records.", lAllCount);
  int lIrr = lAllCount;
  int lHdr = lAllCount;
  int lNob = lAllCount;
  int lC = 0;
  std::set<std::pair<std::string_view, int>> lCustomSkinMods;
  for (const auto& lNPC : fAllNPCs) {
    const auto lNPCRace = lNPC->race;
    if (!lNPCRace) {
      Tng::gLogger::warn("The NPC [{}] from file [{}] does not have a race! They cannot be modified by TNG.", lNPC->GetName(), lNPC->GetFile(0)->GetFilename());
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
    if (!lSkin->HasPartOf(Tng::cSlotBody)) continue;
    if (lSkin->HasPartOf(Tng::cSlotGenital)) {
      Tng::gLogger::info("The skin [0x{:x}] used in NPC [{}] from file [{}] cannot have a male genital. If this is wrong, a patch is required.", lSkin->GetFormID(),
                         lNPC->GetName(), lNPC->GetFile(0)->GetFilename());
      continue;
    }
    lNob--;
    std::set<RE::TESRace*> lSkinRaces;
    bool lHasGenital = FixSkin(lSkin, nullptr);
    if (lHasGenital) {
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
  Tng::gLogger::info("Out of the [{}] NPCs:", lAllCount);
  Tng::gLogger::info("\t{} NPCs do not have a custom skin (WNAM) or are not adult humanoids,", lIrr);
  Tng::gLogger::info("\t{} NPCs' skins are already handled with races,", lHdr - lIrr);
  if ((lNob - lHdr) > 0) Tng::gLogger::info("\t{} NPCs' skins don't have body slot or already have genital slot.", lNob - lHdr);
  if (lC > 0) {
    Tng::gLogger::info("\tTNG handled custom skins for {} NPCs from following mod(s):", lC);
    for (const auto& lMod : lCustomSkinMods) Tng::gLogger::info("\t\t[{}] skins from {}", lMod.second, lMod.first);
  }
}

void TngUtil::CheckArmorPieces() noexcept {
  Tng::gLogger::info("Checking armor pieces...");
  fRCount = 0;
  fCCount = 0;
  fQCount = 0;
  TngInis::LoadTngInis();
  auto& lAllArmor = fDataHandler->GetFormArray<RE::TESObjectARMO>();
  std::set<RE::TESObjectARMO*> lPotentialArmor;
  std::set<RE::TESObjectARMO*> lUnhandledArmor;

  bool lCheckSkinMods = (TngInis::fSkinMods.size() > 0);
  bool lCheckSkinRecords = (TngInis::fSingleSkinIDs.size() > 0);
  bool lCheckRevealMods = (TngInis::fRevealingMods.size() > 0);
  bool lCheckRevealRecords = (TngInis::fSingleRevealingIDs.size() > 0);
  bool lCheckCoverRecords = (TngInis::fSingleCoveringIDs.size() > 0);

  for (const auto& lModName : TngInis::fSkinMods)
    if (fDataHandler->LookupModByName(lModName)) Tng::gLogger::info("TheNewGentleman keeps an eye for [{}] as a skin mod.", lModName);
  for (const auto& lRevealMod : TngInis::fRevealingMods)
    if (fDataHandler->LookupModByName(lRevealMod)) Tng::gLogger::info("TheNewGentleman keeps an eye for [{}] as a revealing armor mod.", lRevealMod);

  for (const auto& lArmor : lAllArmor) {
    if (fHandledSkins.find(lArmor) != fHandledSkins.end()) continue;
    if (lArmor->armorAddons.size() == 0) continue;
    const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
    if (!lArmor->race) {
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] does not have a race! It won't be touched by Tng!", lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
      continue;
    }
    if (!(lArmor->race->HasKeyword(fNPCKey) || (lArmor->race == fDefRace))) continue;
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
      if (lCoverEntry != TngInis::fSingleCoveringIDs.end()) {
        CoverByArmor(lArmor);
        continue;
      }
    }
    if (lCheckRevealMods && (TngInis::fRevealingMods.find(std::string{lArmor->GetFile(0)->GetFilename()}) != TngInis::fRevealingMods.end())) {
      if (lArmor->HasPartOf(Tng::cSlotBody)) {
        lArmor->AddKeyword(fRevealingKey);
        HandleArmor(lArmor);
        fRCount++;
      }
      continue;
    }
    if (lCheckRevealRecords) {
      const auto lRevealEntry = TngInis::fSingleRevealingIDs.find(std::make_pair(std::string{lArmor->GetFile(0)->GetFilename()}, lArmor->GetLocalFormID()));
      if (lRevealEntry != TngInis::fSingleRevealingIDs.end()) {
        lArmor->AddKeyword(fRevealingKey);
        HandleArmor(lArmor);
        fRCount++;
        continue;
      }
    }
    if (lArmor->HasPartOf(Tng::cSlotGenital) && !lArmor->HasKeyword(fUnderwearKey)) {
      Tng::gLogger::info("The armor [0x{:x}:{}] from file [{}] would cover genitals and be covered by chest armor pieces (like an underwear). If it is wrong, a patch is required.",
                         lArmor->GetFormID(), lID, lArmor->GetFile(0)->GetFilename());
      fQCount++;
      continue;
    }
    if (lArmor->HasPartOf(Tng::cSlotBody))
      lPotentialArmor.insert(lArmor);
    else
      lUnhandledArmor.insert(lArmor);
  }
  for (const auto& lArmor : lPotentialArmor) {
    HandleArmor(lArmor);
  }
  for (const auto& lArmor : lUnhandledArmor) {
    ReCheckArmor(lArmor);
  }

  Tng::gLogger::info("Processed {} body armor pieces:", fCCount + fRCount + fQCount);
  Tng::gLogger::info("\t{}: already covering genitals", fQCount);
  Tng::gLogger::info("\t{}: revealing", fRCount);
  Tng::gLogger::info("\t{}: updated to cover genitals", fCCount);
}

void TngUtil::UpdateRace(int aRaceIdx, int aGenOption) noexcept {
  auto lRaces = TngSizeShape::GetRacesWithIndex(aRaceIdx);
  for (const auto& lRaceID : lRaces) {
    auto lRace = RE::TESForm::LookupByID<RE::TESRace>(lRaceID);
    if (!lRace) continue;
    if (aGenOption == -1) {
      lRace->skin = TngSizeShape::fAddons[0][TngSizeShape::cGenitalDefaults[aRaceIdx]];
    } else {
      lRace->skin = TngSizeShape::fAddons[0][aGenOption];
    }
  }
}

void TngUtil::UpdateSavedRaces() noexcept {
  for (int i = 0; i < Tng::cRaceTypes + 3; i++) {
    if (TngSizeShape::GetSingleton()->genitalChoices[i] > TngSizeShape::GetSingleton()->fMalAddonAAs->size()) {
      TngInis::UpdateRace(i, TngSizeShape::GetSingleton()->cGenitalDefaults[i], TngSizeShape::GetSingleton()->genitalSizes[i]);
      Tng::gLogger::info("A previously saved addon cannot be found anymore! The races using the addon are reverted to their default addon!");
      continue;
    }
    if (TngSizeShape::GetSingleton()->genitalChoices[i] > 3) UpdateRace(i, TngSizeShape::GetSingleton()->genitalChoices[i]);
  }
}
