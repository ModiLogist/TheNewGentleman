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
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  for (int i = 0; i < Tng::cSizeCategories; i++) {
    TngSizeShape::SetGlobalSize(i, static_cast<float>(lIni.GetDoubleValue(cGlobalSize, cSizeNames[i], 1.0)));
  }
  if (lIni.SectionExists(cRacialGenital)) {
    CSimpleIniA::TNamesDepend lRaceRecords;
    CSimpleIniA::TNamesDepend::const_iterator lEntry;
    lIni.GetAllKeys(cRacialGenital, lRaceRecords);
    for (lEntry = lRaceRecords.begin(); lEntry != lRaceRecords.end(); lEntry++) {
      std::string lShape = lIni.GetValue(cRacialGenital, lEntry->pItem);
      const std::string lRaceRecord(lEntry->pItem);
      if (!TngSizeShape::LoadRaceShape(lRaceRecord, lShape)) lIni.Delete(cRacialGenital, lEntry->pItem);
    }
  }
  if (lIni.SectionExists(cRacialSize)) {
    CSimpleIniA::TNamesDepend lRaceRecords;
    CSimpleIniA::TNamesDepend::const_iterator lEntry;
    lIni.GetAllKeys(cRacialSize, lRaceRecords);
    for (lEntry = lRaceRecords.begin(); lEntry != lRaceRecords.end(); lEntry++) {
      auto lMult100 = lIni.GetLongValue(cRacialSize, lEntry->pItem);
      const std::string lRaceRecord(lEntry->pItem);
      if (!TngSizeShape::LoadRaceMult(lRaceRecord, lMult100)) lIni.Delete(cRacialGenital, lEntry->pItem);
    }
  }
  if (lIni.SectionExists(cNPCSizeSection)) {
    CSimpleIniA::TNamesDepend lSizeRecords;
    CSimpleIniA::TNamesDepend::const_iterator lEntry;
    lIni.GetAllKeys(cNPCSizeSection, lSizeRecords);
    for (lEntry = lSizeRecords.begin(); lEntry != lSizeRecords.end(); lEntry++) {
      auto lSize = lIni.GetLongValue(cNPCSizeSection, lEntry->pItem);
      const std::string lNPCRecord(lEntry->pItem);
      if (!TngSizeShape::LoadNPCSize(lNPCRecord, lSize)) lIni.Delete(cRacialGenital, lEntry->pItem);
    }
  }
  if (lIni.SectionExists(cNPCShapeSection)) {
    CSimpleIniA::TNamesDepend lShapeRecords;
    CSimpleIniA::TNamesDepend::const_iterator lEntry;
    lIni.GetAllKeys(cNPCShapeSection, lShapeRecords);
    for (lEntry = lShapeRecords.begin(); lEntry != lShapeRecords.end(); lEntry++) {
      std::string lShape = lIni.GetValue(cNPCShapeSection, lEntry->pItem);
      const std::string lNPCRecord(lEntry->pItem);
      if (!TngSizeShape::LoadNPCShape(lNPCRecord, lShape)) lIni.Delete(cRacialGenital, lEntry->pItem);
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
    if (lIni.KeyExists(cControls, cINTCtrl)) fINTCtrl->value = lIni.GetBoolValue(cControls, cINTCtrl) ? 2.0f : 0.0f;
    if (lIni.KeyExists(cControls, cNPCCtrl)) fNPCCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cNPCCtrl));
    if (lIni.KeyExists(cControls, cUPGCtrl)) fUPGCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cUPGCtrl));
    if (lIni.KeyExists(cControls, cDOWCtrl)) fDOWCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cDOWCtrl));
    if (lIni.KeyExists(cControls, cREVCtrl)) fREVCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cREVCtrl));
    if (lIni.KeyExists(cGentleWomen, cGentleWomenChance)) fWomenChance->value = static_cast<float>(lIni.GetDoubleValue(cGentleWomen, cGentleWomenChance));
  }
  lIni.SaveFile(cSettings);
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

bool TngInis::GetAutoReveal(const bool aIsFemale) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  return lIni.GetBoolValue(cAutoReveal, aIsFemale ? cFAutoReveal : cMAutoReveal, aIsFemale);
}

bool TngInis::GetClipCheck() noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  return lIni.GetBoolValue(cGeneral, cDoubleCheck, true);
}

void TngInis::SaveRaceMult(const std::size_t aRaceIdx, const float aRaceMult) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  for (const auto& lRace : TngSizeShape::GetRacesByIdx(aRaceIdx)) {
    auto lRaceIDStr = RecordToStr(lRace);
    if (lRaceIDStr == "") continue;
    auto lMult100 = static_cast<int>(aRaceMult * 100.0f);
    if ((lMult100 > 9) && (lMult100 < 1000)) lIni.SetLongValue(cRacialGenital, lRaceIDStr.c_str(), lMult100);
    if (lMult100 == 0) lIni.Delete(cRacialGenital, lRaceIDStr.c_str());
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveRaceShape(const std::size_t aRaceIdx, const int aRaceShape) noexcept {
  if ((aRaceShape < 0) && (aRaceShape + 1 > TngSizeShape::GetAddonCount(false))) {
    Tng::gLogger::error("Failed to set the race genital addon to {}! There are only {} addons.", aRaceShape, TngSizeShape::GetAddonCount(false));
    return;
  }
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  for (const auto& lRace : TngSizeShape::GetRacesByIdx(aRaceIdx)) {
    auto lRaceIDStr = RecordToStr(lRace);
    if (lRaceIDStr == "") continue;
    if (aRaceShape == -1) {
      lIni.Delete(cRacialGenital, lRaceIDStr.c_str());
      continue;
    }
    auto lGenIDStr = RecordToStr(TngSizeShape::GetAddonAt(false, aRaceShape));
    lIni.SetValue(cRacialGenital, lRaceIDStr.c_str(), lGenIDStr.c_str());
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveActorShape(RE::TESNPC* aNPC, int aGenShape) noexcept {
  if (aGenShape == -1) return;
  auto lNPCIDStr = RecordToStr(aNPC);
  if (lNPCIDStr == "") return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  if (aGenShape == -2) lIni.Delete(cNPCShapeSection, lNPCIDStr.c_str());
  if (aGenShape > -1) {
    auto lGenIDStr = RecordToStr(TngSizeShape::GetAddonAt(false, aGenShape));
    lIni.SetValue(cNPCShapeSection, lNPCIDStr.c_str(), lGenIDStr.c_str());
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveActorSize(RE::TESNPC* aNPC, int aGenSize) noexcept {
  auto lNPCIDStr = RecordToStr(aNPC);
  if (lNPCIDStr == "") return;
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
  if (lArmoIDStr == "") return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.SetBoolValue(cRevealingRecord, lArmoIDStr.c_str(), true);
  lIni.SaveFile(cSettings);
}

void TngInis::RemoveRevealingArmor(RE::TESObjectARMO* aArmor) noexcept {
  auto lArmoIDStr = RecordToStr(aArmor);
  if (lArmoIDStr == "") return;
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
      lIni.SetBoolValue(cAutoReveal, cFAutoReveal, aValue);
    case 2:
      lIni.SetBoolValue(cAutoReveal, cMAutoReveal, aValue);
    case 3:
      lIni.SetBoolValue(cGeneral, cDoubleCheck, aValue);
    default:
      break;
  }
  lIni.SaveFile(cSettings);
}

void TngInis::SaveGlobals() noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  for (int i = 0; i < Tng::cSizeCategories; i++) lIni.SetDoubleValue(cGlobalSize, cSizeNames[i], TngSizeShape::GetGlobalSize(i));
  lIni.SetBoolValue(cControls, cINTCtrl, fINTCtrl->value > 1);
  lIni.SetLongValue(cControls, cNPCCtrl, static_cast<int>(fNPCCtrl->value));
  lIni.SetLongValue(cControls, cUPGCtrl, static_cast<int>(fUPGCtrl->value));
  lIni.SetLongValue(cControls, cDOWCtrl, static_cast<int>(fDOWCtrl->value));
  lIni.SetLongValue(cControls, cREVCtrl, static_cast<int>(fREVCtrl->value));
  lIni.SetDoubleValue(cGentleWomen, cGentleWomenChance, fWomenChance->value);
  lIni.SaveFile(cSettings);
}

bool TngInis::IsValidSkeleton(RE::BSFixedString aModel, RE::BSFixedString aDefModels[2]) noexcept {
  static std::set<RE::BSFixedString> lValidSkeletons;
  if (lValidSkeletons.size() != 0) return lValidSkeletons.find(aModel) != lValidSkeletons.end();
  lValidSkeletons.insert(aDefModels[0]);
  lValidSkeletons.insert(aDefModels[0]);
  if (!std::filesystem::exists(cTngInisPath)) return lValidSkeletons.find(aModel) != lValidSkeletons.end();
  for (const auto& entry : std::filesystem::directory_iterator(cTngInisPath)) {
    std::string lFileName = entry.path().filename().string();
    if (IsTngIni(lFileName)) {
      Tng::gLogger::info("\tFound ini file {}:", lFileName);
      CSimpleIniA lIni;
      lIni.SetUnicode();
      lIni.SetMultiKey();
      lIni.LoadFile(entry.path().string().c_str());
      if (lIni.KeyExists(cSkeleton, cValidModel)) {
        CSimpleIniA::TNamesDepend lSkeletons;
        lIni.GetAllValues(cSkinSection, cSkinMod, lSkeletons);
        CSimpleIniA::TNamesDepend::const_iterator lSkeleton;
        for (lSkeleton = lSkeletons.begin(); lSkeleton != lSkeletons.end(); lSkeleton++) {
          const RE::BSFixedString lModel(lSkeleton->pItem);
          lValidSkeletons.insert(lModel);
        }
      }
    }
  }
  return lValidSkeletons.find(aModel) != lValidSkeletons.end();
}

void TngInis::LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>>& aField) noexcept {
  CSimpleIniA::TNamesDepend::const_iterator lEntry;
  for (lEntry = aModRecords.begin(); lEntry != aModRecords.end(); lEntry++) {
    const std::string lModRecord(lEntry->pItem);
    aField.insert(std::make_pair(StrToRecord(lModRecord).first, StrToRecord(lModRecord).second));
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
  fRunTimeRevealingIDs.insert(std::make_pair(StrToRecord(aArmorRecod).first, StrToRecord(aArmorRecod).second));
}
