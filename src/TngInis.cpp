#include <TngInis.h>
#include <TngSizeShape.h>

bool TngInis::Init() noexcept {
  fDH = RE::TESDataHandler::GetSingleton();
  fNPCCtrl = fDH->LookupForm<RE::TESGlobal>(cNPCCtrlID, Tng::cName);
  fUPGCtrl = fDH->LookupForm<RE::TESGlobal>(cUPGCtrlID, Tng::cName);
  fDOWCtrl = fDH->LookupForm<RE::TESGlobal>(cDOWCtrlID, Tng::cName);
  fREVCtrl = fDH->LookupForm<RE::TESGlobal>(cREVCtrlID, Tng::cName);
  fINTCtrl = fDH->LookupForm<RE::TESGlobal>(cINTCtrlID, Tng::cName);
  fWomenChance = fDH->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
  if (!(fNPCCtrl && fUPGCtrl && fDOWCtrl && fREVCtrl && fINTCtrl && fWomenChance)) {
    Tng::gLogger::critical("Information required for saving and loading ini files could not be found. Please report this issue!");
    return false;
  }
  return true;
}

void TngInis::LoadMainIni() noexcept {
  if (!std::filesystem::exists(cSettings)) {
    std::ofstream lTngSettings(cSettings);
    lTngSettings << ";TNG Settings File" << std::endl;
    lTngSettings.close();
  }
  Tng::gLogger::info("Loading TNG settings...");
  UpdateIniVersion();
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  fAutoReveal[0] = lIni.GetBoolValue(cGeneral, cFAutoReveal, true);
  fAutoReveal[1] = lIni.GetBoolValue(cGeneral, cMAutoReveal, false);
  fClipCheck = lIni.GetBoolValue(cGeneral, cDoubleCheck, true);
  fExlPC = lIni.GetBoolValue(cGeneral, cExlPC, false);
  for (std::size_t i = 0; i < Tng::cSizeCategories; i++) {
    TngSizeShape::SetGlobalSize(i, static_cast<float>(lIni.GetDoubleValue(cGlobalSize, cSizeNames[i], 1.0)));
  }
  CSimpleIniA::TNamesDepend lRaceRecords;
  CSimpleIniA::TNamesDepend::const_iterator lEntry;
  lIni.GetAllKeys(cRacialGenital, lRaceRecords);
  for (lEntry = lRaceRecords.begin(); lEntry != lRaceRecords.end(); lEntry++) {
    std::string lAddon = lIni.GetValue(cRacialGenital, lEntry->pItem);
    const std::string lRaceRecord(lEntry->pItem);
    if (!TngSizeShape::LoadRaceAddn(lRaceRecord, lAddon)) lIni.Delete(cRacialGenital, lEntry->pItem);
  }
  lRaceRecords.clear();
  lIni.GetAllKeys(cRacialSize, lRaceRecords);
  for (lEntry = lRaceRecords.begin(); lEntry != lRaceRecords.end(); lEntry++) {
    auto lMult = lIni.GetDoubleValue(cRacialSize, lEntry->pItem);
    const std::string lRaceRecord(lEntry->pItem);
    if (!TngSizeShape::LoadRaceMult(lRaceRecord, static_cast<float>(lMult))) lIni.Delete(cRacialSize, lEntry->pItem);
  }
  CSimpleIniA::TNamesDepend lSizeRecords;
  lIni.GetAllKeys(cNPCSizeSection, lSizeRecords);
  for (lEntry = lSizeRecords.begin(); lEntry != lSizeRecords.end(); lEntry++) {
    auto lSize = lIni.GetLongValue(cNPCSizeSection, lEntry->pItem);
    const std::string lNPCRecord(lEntry->pItem);
    if (!TngSizeShape::LoadNPCSize(lNPCRecord, lSize)) lIni.Delete(cNPCSizeSection, lEntry->pItem);
  }
  CSimpleIniA::TNamesDepend lAddonRecords;
  lIni.GetAllKeys(cNPCAddnSection, lAddonRecords);
  for (lEntry = lAddonRecords.begin(); lEntry != lAddonRecords.end(); lEntry++) {
    const std::string lNPCRecord(lEntry->pItem);
    std::string lAddon = lIni.GetValue(cNPCAddnSection, lEntry->pItem);
    if (!TngSizeShape::LoadNPCAddn(lNPCRecord, lAddon)) lIni.Delete(cNPCAddnSection, lEntry->pItem);
  }
  CSimpleIniA::TNamesDepend lExRecords;
  lIni.GetAllKeys(cExcludeSection, lExRecords);
  for (lEntry = lExRecords.begin(); lEntry != lExRecords.end(); lEntry++) {
    const std::string lNPCRecord(lEntry->pItem);
    TngSizeShape::ExcludeNPC(lNPCRecord);
  }
  CSimpleIniA::TNamesDepend lRevRecords;
  lIni.GetAllKeys(cRevealingRecord, lRevRecords);
  for (lEntry = lRevRecords.begin(); lEntry != lRevRecords.end(); lEntry++) {
    auto lIsRevealing = lIni.GetBoolValue(cRevealingRecord, lEntry->pItem);
    const std::string lArmorRecord(lEntry->pItem);
    if (lIsRevealing) UpdateRevealing(lArmorRecord);
  }
  if (lIni.KeyExists(cControls, cINTCtrl)) fINTCtrl->value = lIni.GetBoolValue(cControls, cINTCtrl) ? 2.0f : 0.0f;
  if (lIni.KeyExists(cControls, cNPCCtrl)) fNPCCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cNPCCtrl));
  if (lIni.KeyExists(cControls, cUPGCtrl)) fUPGCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cUPGCtrl));
  if (lIni.KeyExists(cControls, cDOWCtrl)) fDOWCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cDOWCtrl));
  if (lIni.KeyExists(cControls, cREVCtrl)) fREVCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cREVCtrl));
  if (lIni.KeyExists(cGentleWomen, cGentleWomenChance)) fWomenChance->value = static_cast<float>(lIni.GetDoubleValue(cGentleWomen, cGentleWomenChance));
  lIni.SaveFile(cSettings);
}

void TngInis::LoadTngInis() noexcept {
  if (!std::filesystem::exists(cTngInisPath)) return;
  Tng::gLogger::info("Loading ini files:");
  for (const auto& entry : std::filesystem::directory_iterator(cTngInisPath)) {
    std::string lFileName = entry.path().filename().string();
    if (IsTngIni(lFileName)) {
      CSimpleIniA lIni;
      lIni.SetUnicode();
      lIni.SetMultiKey();
      lIni.LoadFile(entry.path().string().c_str());
      if (lIni.SectionExists(cExcludeSection)) {
        CSimpleIniA::TNamesDepend lExRecords;
        CSimpleIniA::TNamesDepend::const_iterator lEntry;
        lIni.GetAllKeys(cExcludeSection, lExRecords);
        for (lEntry = lExRecords.begin(); lEntry != lExRecords.end(); lEntry++) {
          auto lRecord = StrToRecord(lEntry->pItem);
          fHardExcluded.insert_or_assign(lRecord.first, lRecord.second);
        }
        Tng::gLogger::info("\t- Found [{}] excluded NPCs in ini file [{}].", lExRecords.size(), lFileName);
      }
      if (lIni.KeyExists(cSkeleton, cValidModel)) {
        CSimpleIniA::TNamesDepend lSkeletons;
        lIni.GetAllValues(cSkeleton, cValidModel, lSkeletons);
        CSimpleIniA::TNamesDepend::const_iterator lSkeleton;
        for (lSkeleton = lSkeletons.begin(); lSkeleton != lSkeletons.end(); lSkeleton++) {
          const std::string lModel(lSkeleton->pItem);
          fValidSkeletons.insert(lModel);
        }
        Tng::gLogger::info("\t- Found [{}] skeleton models in ini file [{}].", lSkeletons.size(), lFileName);
      }
      if (lIni.SectionExists(cSkinSection)) {
        if (lIni.KeyExists(cSkinSection, cSkinMod)) {
          CSimpleIniA::TNamesDepend lMods;
          lIni.GetAllValues(cSkinSection, cSkinMod, lMods);
          CSimpleIniA::TNamesDepend::const_iterator lMod;
          for (lMod = lMods.begin(); lMod != lMods.end(); lMod++) {
            const std::string lModName(lMod->pItem);
            fSkinMods.insert(lModName);
          }
          Tng::gLogger::info("\t- Found [{}] skin mods in ini file [{}].", lMods.size(), lFileName);
        }
        if (lIni.KeyExists(cSkinSection, cSkinRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          lIni.GetAllValues(cSkinSection, cSkinRecord, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleSkinIDs);
          Tng::gLogger::info("\t- Found [{}] skin records in ini file [{}].", lModRecords.size(), lFileName);
        }
      }
      if (lIni.SectionExists(cArmorSection)) {
        if (lIni.KeyExists(cArmorSection, cRevealingMod)) {
          CSimpleIniA::TNamesDepend lMods;
          lIni.GetAllValues(cArmorSection, cRevealingMod, lMods);
          for (const auto& lMod : lMods) {
            const std::string lModName(lMod.pItem);
            fRevealingMods.insert(lModName);
          }
          Tng::gLogger::info("\t- Found [{}] revealing mods in ini file [{}].", lMods.size(), lFileName);
        }
        if (lIni.KeyExists(cArmorSection, cRevealingRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          lIni.GetAllValues(cArmorSection, cRevealingRecord, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleRevealingIDs);
          Tng::gLogger::info("\t- Found [{}] revealing records in ini file [{}].", lModRecords.size(), lFileName);
        }
        if (lIni.KeyExists(cArmorSection, cCoveringRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          lIni.GetAllValues(cArmorSection, cCoveringRecord, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleCoveringIDs);
          Tng::gLogger::info("\t- Found [{}] covering records in ini file [{}].", lModRecords.size(), lFileName);
        }
      }
    } else {
      Tng::gLogger::warn("The file {} in TNG ini folder is not named correctly or is not a TNG ini file.", lFileName);
    }
  }
}

bool TngInis::GetAutoReveal(const bool aIsFemale) noexcept { return (aIsFemale ? fAutoReveal[1] : fAutoReveal[0]); }

bool TngInis::GetClipCheck() noexcept { return fClipCheck; }

bool TngInis::GetExcludePlayer() noexcept { return fExlPC; }

void TngInis::SaveRaceMult(const std::size_t aRaceIdx, const float aRaceMult) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  auto lRace = TngSizeShape::GetRaceByIdx(aRaceIdx);
  auto lRaceIDStr = RecordToStr(lRace);
  if (lRaceIDStr == "") {
    Tng::gLogger::critical("Failed to save the size multiplier for race [{:x}: {}]!", lRace->GetFormID(), lRace->GetFormEditorID());
    return;
  }
  if (aRaceMult < 0) {
    lIni.Delete(cRacialSize, lRaceIDStr.c_str());
  } else {
    lIni.SetDoubleValue(cRacialSize, lRaceIDStr.c_str(), static_cast<double>(aRaceMult));
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveRaceAddn(const std::size_t aRaceIdx, int aChoice) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  auto lRace = TngSizeShape::GetRaceByIdx(aRaceIdx);
  auto lRaceIDStr = RecordToStr(lRace);
  if (lRaceIDStr == "") {
    Tng::gLogger::critical("Failed to save the selected addon for race [{:x}: {}]!", lRace->GetFormID(), lRace->GetFormEditorID());
    return;
  }
  if (aChoice == -1) {
    lIni.Delete(cRacialGenital, lRaceIDStr.c_str());
  } else {
    auto lAddon = TngSizeShape::GetAddonAt(false, aChoice);
    auto lGenIDStr = RecordToStr(lAddon);
    lIni.SetValue(cRacialGenital, lRaceIDStr.c_str(), lGenIDStr.c_str());
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveNPCAddn(RE::TESNPC* aNPC, int aChoice) noexcept {
  auto lNPCIDStr = RecordToStr(aNPC);
  if (lNPCIDStr.empty()) return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  switch (aChoice) {
    case -2:
      lIni.Delete(cNPCAddnSection, lNPCIDStr.c_str());
      break;
    case -3:
      lIni.Delete(cNPCAddnSection, lNPCIDStr.c_str());
      lIni.SetBoolValue(cExcludeSection, lNPCIDStr.c_str(), true);
      break;
    default:
      auto lAddon = TngSizeShape::GetAddonAt(aNPC->IsFemale(), aChoice);
      auto lGenIDStr = RecordToStr(lAddon);
      lIni.Delete(cExcludeSection, lNPCIDStr.c_str());
      lIni.SetValue(cNPCAddnSection, lNPCIDStr.c_str(), lGenIDStr.c_str());
      break;
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveActorSize(RE::TESNPC* aNPC, int aGenSize) noexcept {
  auto lNPCIDStr = RecordToStr(aNPC);
  if (lNPCIDStr.empty()) return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.SetLongValue(cNPCSizeSection, lNPCIDStr.c_str(), aGenSize);
  if (aGenSize == -2) {
    lIni.Delete(cNPCSizeSection, lNPCIDStr.c_str());
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveRevealingArmor(RE::TESObjectARMO* aArmor) noexcept {
  auto lArmoIDStr = RecordToStr(aArmor);
  if (lArmoIDStr.empty()) return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.SetBoolValue(cRevealingRecord, lArmoIDStr.c_str(), true);
  lIni.SaveFile(cSettings);
}

void TngInis::RemoveRevealingArmor(RE::TESObjectARMO* aArmor) noexcept {
  auto lArmoIDStr = RecordToStr(aArmor);
  if (lArmoIDStr.empty()) return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.Delete(cRevealingRecord, lArmoIDStr.c_str());
  lIni.SaveFile(cSettings);
}

void TngInis::SaveBool(int aID, bool aValue) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  switch (aID) {
    case 1:
      lIni.SetBoolValue(cGeneral, cFAutoReveal, aValue);
      fAutoReveal[1] = aValue;
      break;
    case 2:
      lIni.SetBoolValue(cGeneral, cMAutoReveal, aValue);
      fAutoReveal[0] = aValue;
      break;
    case 3:
      lIni.SetBoolValue(cGeneral, cDoubleCheck, aValue);
      fClipCheck = aValue;
      break;
    case 4:
      lIni.SetBoolValue(cGeneral, cExlPC, aValue);
      fExlPC = aValue;
      break;
    default:
      break;
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveGlobals() noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  for (std::size_t i = 0; i < Tng::cSizeCategories; i++) lIni.SetDoubleValue(cGlobalSize, cSizeNames[i], TngSizeShape::GetGlobalSize(i));
  lIni.SetBoolValue(cControls, cINTCtrl, fINTCtrl->value > 1.0f);
  lIni.SetLongValue(cControls, cNPCCtrl, static_cast<int>(fNPCCtrl->value));
  lIni.SetLongValue(cControls, cUPGCtrl, static_cast<int>(fUPGCtrl->value));
  lIni.SetLongValue(cControls, cDOWCtrl, static_cast<int>(fDOWCtrl->value));
  lIni.SetLongValue(cControls, cREVCtrl, static_cast<int>(fREVCtrl->value));
  lIni.SetDoubleValue(cGentleWomen, cGentleWomenChance, fWomenChance->value);
  lIni.SaveFile(cSettings);
}

void TngInis::UpdateValidSkeletons(std::set<std::string> aValidSkeletons) noexcept {
  for (const auto& lModel : aValidSkeletons) fValidSkeletons.insert(lModel);
}

bool TngInis::IsValidSkeleton(std::string aModel) noexcept { return fValidSkeletons.find(aModel) != fValidSkeletons.end(); }

void TngInis::UpdateIniVersion() noexcept {
  CSimpleIniA lIni;
  CSimpleIniA::TNamesDepend lSections;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  int lIniVersion = lIni.GetLongValue(cIniVersion, cVersion, 1);
  switch (lIniVersion) {
    case 1:
      lIni.GetAllSections(lSections);
      for (CSimpleIniA::TNamesDepend::const_iterator lSection = lSections.begin(); lSection != lSections.end(); lSection++) lIni.Delete(lSection->pItem, NULL);
      break;
    case 2:
      if (lIni.KeyExists("AutoReveal", "Male")) {
        lIni.SetBoolValue(cGeneral, cMAutoReveal, lIni.GetBoolValue("AutoReveal", "Male", false));
      }
      if (lIni.KeyExists("AutoReveal", "Female")) {
        lIni.SetBoolValue(cGeneral, cMAutoReveal, lIni.GetBoolValue("AutoReveal", "Female", true));
      }
      lIni.Delete("AutoReveal", NULL);
    default:
      break;
  }
  lIni.SetLongValue(cIniVersion, cVersion, cCurrVersion);
  lIni.SaveFile(cSettings);
}

void TngInis::LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>>& aField) noexcept {
  CSimpleIniA::TNamesDepend::const_iterator lEntry;
  for (lEntry = aModRecords.begin(); lEntry != aModRecords.end(); lEntry++) {
    const std::string lModRecord(lEntry->pItem);
    aField.insert(StrToRecord(lModRecord));
  }
}

bool TngInis::IsTngIni(const std::string_view aFileName) noexcept {
  if (aFileName.size() < cTngIniEnding.size()) return false;
  return std::equal(cTngIniEnding.rbegin(), cTngIniEnding.rend(), aFileName.rbegin());
}

bool TngInis::UpdateRevealing(const std::string aArmorRecod) noexcept {
  auto lArmor = fDH->LookupForm<RE::TESObjectARMO>(StrToRecord(aArmorRecod).second, StrToRecord(aArmorRecod).first);
  if (!lArmor) {
    Tng::gLogger::info("Previously marked revealing armor from mod {} does not exist anymore!", StrToRecord(aArmorRecod).first);
    return false;
  }
  fRunTimeRevealingIDs.insert(std::make_pair<std::string, RE::FormID>(StrToRecord(aArmorRecod).first, StrToRecord(aArmorRecod).second));
  return true;
}
