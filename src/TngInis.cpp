#include <TngInis.h>
#include <TngSizeShape.h>

void TngInis::LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>>& aField) noexcept {
  CSimpleIniA::TNamesDepend::const_iterator lEntry;
  for (lEntry = aModRecords.begin(); lEntry != aModRecords.end(); lEntry++) {
    const std::string lModRecord(lEntry->pItem);
    const size_t lSepLoc = lModRecord.find(Tng::cDelimChar);
    const RE::FormID lID = std::strtol(lModRecord.substr(0, lSepLoc).data(), nullptr, 0);
    const std::string lModName = lModRecord.substr(lSepLoc + 1);
    aField.insert(std::make_pair(lModName, lID));
  }
}

bool TngInis::IsTngIni(const std::string_view aFileName) noexcept {
  if (aFileName.size() < cTngIniEnding.size()) return false;
  return std::equal(cTngIniEnding.rbegin(), cTngIniEnding.rend(), aFileName.rbegin());
}

void TngInis::UpdateRevealing(const std::string aArmorRecod) noexcept {
  const size_t lSepLoc = aArmorRecod.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aArmorRecod.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aArmorRecod.substr(lSepLoc + 1);
  auto lArmor = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectARMO>(lFormID, lModName);
  if (!lArmor) {
    Tng::gLogger::info("Previously marked revealing armor from mod {} does not exist anymore!", lModName);
    return;
  }
  fSingleRevealingIDs.insert(std::make_pair(lModName, lFormID));
}

void TngInis::LoadTngInis() noexcept {
  if (!std::filesystem::exists(cTngInisPath)) return;
  Tng::gLogger::info("Loading ini files:");
  for (const auto& entry : std::filesystem::directory_iterator(cTngInisPath)) {
    std::string lFileName = entry.path().filename().string();
    if (IsTngIni(lFileName)) {
      Tng::gLogger::info("\tFound ini file {}:", lFileName);
      CSimpleIniA lIni;
      lIni.SetUnicode();
      lIni.SetMultiKey();
      lIni.LoadFile(entry.path().string().c_str());
      if (lIni.SectionExists(cSkinSection)) {
        if (lIni.KeyExists(cSkinSection, cSkinMod)) {
          CSimpleIniA::TNamesDepend lMods;
          lIni.GetAllValues(cSkinSection, cSkinMod, lMods);

          CSimpleIniA::TNamesDepend::const_iterator lMod;
          for (lMod = lMods.begin(); lMod != lMods.end(); lMod++) {
            const std::string lModName(lMod->pItem);
            fSkinMods.insert(lModName);
          }
        }
        if (lIni.KeyExists(cSkinSection, cSkinRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          lIni.GetAllValues(cSkinSection, cSkinRecord, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleSkinIDs);
          Tng::gLogger::info("\t\t- Found skin records in ini file [{}].", lFileName);
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
        }
        if (lIni.KeyExists(cArmorSection, cRevealingRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          lIni.GetAllValues(cArmorSection, cRevealingMod, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleRevealingIDs);
          Tng::gLogger::info("\t\t- Found revealing records in ini file [{}].", lFileName);
        }
        if (lIni.KeyExists(cArmorSection, cCoveringRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          lIni.GetAllValues(cArmorSection, cCoveringRecord, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleCoveringIDs);
          Tng::gLogger::info("\t\t- Found covering records in ini file [{}].", lFileName);
        }
      }
    } else {
      Tng::gLogger::warn("The file {} in TNG ini folder is not named correctly or is not a TNG ini file.", lFileName);
    }
  }
}

bool TngInis::LoadMainIni() noexcept {
  if (!std::filesystem::exists(cSettings)) {
    std::ofstream lTngSettings(cSettings);
    lTngSettings << ";TNG Settings File" << std::endl;
    lTngSettings.close();
  }
  Tng::gLogger::info("Loading TNG settings...");
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  if (!lIni.KeyExists(cAutoReveal, cFAutoReveal)) lIni.SetBoolValue(cAutoReveal, cFAutoReveal, FAutoReveal);
  if (!lIni.KeyExists(cAutoReveal, cMAutoReveal)) lIni.SetBoolValue(cAutoReveal, cMAutoReveal, MAutoReveal);
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    if (!lIni.KeyExists(cGlobalSize, cSizeNames[i])) lIni.SetDoubleValue(cGlobalSize, cSizeNames[i], TngSizeShape::fSizeGlobs[i]->value);
  }
  for (int i = 0; i < Tng::cRaceTypes + 3; i++) {
    auto lRaceName = cRaceNames[i];
    if (!lIni.KeyExists(cRacialGenital, lRaceName)) lIni.SetLongValue(cRacialGenital, lRaceName, TngSizeShape::genitalChoices[i]);
    if (!lIni.KeyExists(cRacialSize, lRaceName)) lIni.SetDoubleValue(cRacialSize, lRaceName, TngSizeShape::genitalSizes[i]);
  }
  FAutoReveal = lIni.GetBoolValue(cAutoReveal, cFAutoReveal, true);
  MAutoReveal = lIni.GetBoolValue(cAutoReveal, cMAutoReveal, false);
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    TngSizeShape::fSizeGlobs[i]->value = static_cast<float>(lIni.GetDoubleValue(cGlobalSize, cSizeNames[i]));
  }
  for (int i = 0; i < Tng::cRaceTypes + 3; i++) {
    TngSizeShape::genitalChoices[i] = lIni.GetLongValue(cRacialGenital, cRaceNames[i], TngSizeShape::genitalChoices[i]);
    TngSizeShape::genitalSizes[i] = static_cast<float>(lIni.GetDoubleValue(cRacialSize, cRaceNames[i], TngSizeShape::genitalSizes[i]));
  }
  if (lIni.SectionExists(cNPCSizeSection)) {
    CSimpleIniA::TNamesDepend lSizeRecords;
    CSimpleIniA::TNamesDepend::const_iterator lEntry;
    lIni.GetAllKeys(cNPCSizeSection, lSizeRecords);
    for (lEntry = lSizeRecords.begin(); lEntry != lSizeRecords.end(); lEntry++) {
      auto lSize = lIni.GetLongValue(cNPCSizeSection, lEntry->pItem);
      const std::string lNPCRecord(lEntry->pItem);
      TngSizeShape::UpdateSavedSize(lNPCRecord, lSize);
    }
  }
  if (lIni.SectionExists(cNPCShapeSection)) {
    CSimpleIniA::TNamesDepend lShapeRecords;
    CSimpleIniA::TNamesDepend::const_iterator lEntry;
    lIni.GetAllKeys(cNPCShapeSection, lShapeRecords);
    for (lEntry = lShapeRecords.begin(); lEntry != lShapeRecords.end(); lEntry++) {
      auto lShape = lIni.GetLongValue(cNPCShapeSection, lEntry->pItem);
      const std::string lNPCRecord(lEntry->pItem);
      TngSizeShape::UpdateSavedShape(lNPCRecord, lShape);
    }
  }
  if (lIni.SectionExists(cRevealingRecord)) {
    CSimpleIniA::TNamesDepend lRevRecords;
    CSimpleIniA::TNamesDepend::const_iterator lEntry;
    lIni.GetAllKeys(cRevealingRecord, lRevRecords);
    for (lEntry = lRevRecords.begin(); lEntry != lRevRecords.end(); lEntry++) {
      auto lIsRevealing = lIni.GetBoolValue(cRevealingRecord, lEntry->pItem);
      const std::string lArmorRecord(lEntry->pItem);
      if (lIsRevealing) UpdateRevealing(lArmorRecord);
    }
  }
  if (lIni.SectionExists(cControls)) {
    auto lNPCCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cNPCCtrlID, Tng::cName);
    auto lUPGCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cUPGCtrlID, Tng::cName);
    auto lDOWCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cDOWCtrlID, Tng::cName);
    auto lREVCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cREVCtrlID, Tng::cName);
    auto lINTCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cINTCtrlID, Tng::cName);
    auto lWomenChance = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
    if (!(lNPCCtrl && lUPGCtrl && lDOWCtrl && lREVCtrl && lINTCtrl && lWomenChance)) {
      Tng::gLogger::info("There seems to be an issue for saving some settings.");
      lIni.SaveFile(cSettings);
      return true;
    }
    if (lIni.KeyExists(cControls, cINTCtrl)) lINTCtrl->value = lIni.GetBoolValue(cControls, cINTCtrl) ? 2.0f : 0.0f;
    if (lIni.KeyExists(cControls, cNPCCtrl)) lNPCCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cNPCCtrl));
    if (lIni.KeyExists(cControls, cUPGCtrl)) lUPGCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cUPGCtrl));
    if (lIni.KeyExists(cControls, cDOWCtrl)) lDOWCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cDOWCtrl));
    if (lIni.KeyExists(cControls, cREVCtrl)) lREVCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cREVCtrl));
    if (lIni.KeyExists(cGentleWomen, cGentleWomenChance)) lWomenChance->value = static_cast<float>(lIni.GetDoubleValue(cGentleWomen, cGentleWomenChance));
  }
  lIni.SaveFile(cSettings);
  return true;
}

void TngInis::SaveSize(const int aIdx) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.SetDoubleValue(cGlobalSize, cSizeNames[aIdx], TngSizeShape::fSizeGlobs[aIdx]->value);
  lIni.SaveFile(cSettings);
}

void TngInis::UpdateRace(const int aRaceIdx, const int aRaceShape, const float aRaceMult) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  if (aRaceShape == -1) {
    lIni.SetLongValue(cRacialGenital, cRaceNames[aRaceIdx], TngSizeShape::cGenitalDefaults[aRaceIdx]);
  } else {
    lIni.SetLongValue(cRacialGenital, cRaceNames[aRaceIdx], aRaceShape);
  }
  lIni.SetDoubleValue(cRacialSize, cRaceNames[aRaceIdx], aRaceMult);
  lIni.SaveFile(cSettings);
}

void TngInis::AddActor(RE::FormID aFormID, std::string aModName, int aGenShape, int aGenSize) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  std::ostringstream oss;
  oss << std::hex << aFormID;
  std::string lIDStr = "0x" + oss.str() + Tng::cDelimChar + aModName;
  if (aGenShape > -1) lIni.SetLongValue(cNPCShapeSection, lIDStr.c_str(), aGenShape);
  lIni.SetLongValue(cNPCSizeSection, lIDStr.c_str(), aGenSize);
  lIni.SaveFile(cSettings);
}

void TngInis::AddRevealingArmor(RE::TESObjectARMO* aArmor) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  std::ostringstream oss;
  oss << std::hex << aArmor->GetLocalFormID();
  std::string lModName{aArmor->GetFile(0)->GetFilename()};
  std::string lIDStr = "0x" + oss.str() + Tng::cDelimChar + lModName;
  lIni.SetBoolValue(cRevealingRecord, lIDStr.c_str(), true);
  lIni.SaveFile(cSettings);
}

void TngInis::SaveGlobals() noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  auto lNPCCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cNPCCtrlID, Tng::cName);
  auto lUPGCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cUPGCtrlID, Tng::cName);
  auto lDOWCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cDOWCtrlID, Tng::cName);
  auto lREVCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cREVCtrlID, Tng::cName);
  auto lINTCtrl = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(cINTCtrlID, Tng::cName);
  auto lWomenChance = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
  if (!(lNPCCtrl && lUPGCtrl && lDOWCtrl && lREVCtrl && lINTCtrl && lWomenChance)) {
    Tng::gLogger::info("There seems to be an issue for saving some settings.");
    return;
  }
  lIni.SetBoolValue(cControls, cINTCtrl, lINTCtrl->value > 1);
  lIni.SetLongValue(cControls, cNPCCtrl, static_cast<int>(lNPCCtrl->value));
  lIni.SetLongValue(cControls, cUPGCtrl, static_cast<int>(lUPGCtrl->value));
  lIni.SetLongValue(cControls, cDOWCtrl, static_cast<int>(lDOWCtrl->value));
  lIni.SetLongValue(cControls, cREVCtrl, static_cast<int>(lREVCtrl->value));  
  lIni.SetDoubleValue(cGentleWomen, cGentleWomenChance, lWomenChance->value);

  lIni.SaveFile(cSettings);
}
