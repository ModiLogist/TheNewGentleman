#include <Base.h>
#include <Inis.h>

bool Inis::Init() noexcept {
  Tng::SEDH() = RE::TESDataHandler::GetSingleton();
  fNPCCtrl = Tng::SEDH()->LookupForm<RE::TESGlobal>(cNPCCtrlID, Tng::cName);
  fUPGCtrl = Tng::SEDH()->LookupForm<RE::TESGlobal>(cUPGCtrlID, Tng::cName);
  fDOWCtrl = Tng::SEDH()->LookupForm<RE::TESGlobal>(cDOWCtrlID, Tng::cName);
  fREVCtrl = Tng::SEDH()->LookupForm<RE::TESGlobal>(cREVCtrlID, Tng::cName);
  fINTCtrl = Tng::SEDH()->LookupForm<RE::TESGlobal>(cINTCtrlID, Tng::cName);
  fWomenChance = Tng::SEDH()->LookupForm<RE::TESGlobal>(Tng::cWomenChanceID, Tng::cName);
  if (!(fNPCCtrl && fUPGCtrl && fDOWCtrl && fREVCtrl && fINTCtrl && fWomenChance)) {
    SKSE::log::critical("Information required for saving and loading ini files could not be found. Please report this issue!");
    return false;
  }
  return true;
}

void Inis::LoadMainIni() noexcept {
  if (!std::filesystem::exists(cSettings)) {
    std::ofstream lTngSettings(cSettings);
    lTngSettings << ";TNG Settings File" << std::endl;
    lTngSettings.close();
  }
  SKSE::log::info("Loading TNG settings...");
  UpdateIniVersion();
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  fRandM = lIni.GetBoolValue(cGeneral, cRandM, false);
  fExlPC = lIni.GetBoolValue(cGeneral, cExlPC, false);
  fRADef = lIni.GetBoolValue(cGeneral, cRADef, false);
  fRAUsr = lIni.GetBoolValue(cGeneral, cRAUsr, false);
  for (std::size_t i = 0; i < Tng::cSizeCategories; i++) {
    Base::SetGlobalSize(i, static_cast<float>(lIni.GetDoubleValue(cGlobalSize, cSizeNames[i], cDefSizes[i])));
  }
  CSimpleIniA::TNamesDepend::const_iterator lEntry;
  CSimpleIniA::TNamesDepend lSectionRecords;
  lIni.GetAllKeys(cRacialGenital, lSectionRecords);
  for (lEntry = lSectionRecords.begin(); lEntry != lSectionRecords.end(); lEntry++) {
    std::string addon = lIni.GetValue(cRacialGenital, lEntry->pItem);
    const std::string lRaceRecord(lEntry->pItem);
    if (!Base::LoadRaceAddn(lRaceRecord, addon)) lIni.Delete(cRacialGenital, lEntry->pItem);
  }
  lIni.GetAllKeys(cRacialSize, lSectionRecords);
  for (lEntry = lSectionRecords.begin(); lEntry != lSectionRecords.end(); lEntry++) {
    auto lMult = lIni.GetDoubleValue(cRacialSize, lEntry->pItem);
    const std::string lRaceRecord(lEntry->pItem);
    if (!Base::LoadRaceMult(lRaceRecord, static_cast<float>(lMult))) lIni.Delete(cRacialSize, lEntry->pItem);
  }
  lIni.GetAllKeys(cNPCSizeSection, lSectionRecords);
  for (lEntry = lSectionRecords.begin(); lEntry != lSectionRecords.end(); lEntry++) {
    auto lSize = lIni.GetLongValue(cNPCSizeSection, lEntry->pItem);
    const std::string lNPCRecord(lEntry->pItem);
    if (!Base::LoadNPCSize(lNPCRecord, lSize)) lIni.Delete(cNPCSizeSection, lEntry->pItem);
  }
  lIni.GetAllKeys(cNPCAddnSection, lSectionRecords);
  for (lEntry = lSectionRecords.begin(); lEntry != lSectionRecords.end(); lEntry++) {
    const std::string lNPCRecord(lEntry->pItem);
    std::string addon = lIni.GetValue(cNPCAddnSection, lEntry->pItem);
    if (!Base::LoadNPCAddn(lNPCRecord, addon)) lIni.Delete(cNPCAddnSection, lEntry->pItem);
  }
  lIni.GetAllKeys(cExcludeSection, lSectionRecords);
  for (lEntry = lSectionRecords.begin(); lEntry != lSectionRecords.end(); lEntry++) {
    const std::string lNPCRecord(lEntry->pItem);
    Base::ExcludeNPC(lNPCRecord);
  }
  lIni.GetAllKeys(cRevealingRecord, lSectionRecords);
  for (lEntry = lSectionRecords.begin(); lEntry != lSectionRecords.end(); lEntry++) {
    auto lIsRevealing = lIni.GetBoolValue(cRevealingRecord, lEntry->pItem);
    const std::string lArmorRecord(lEntry->pItem);
    UpdateRevealing(lArmorRecord, lIsRevealing);
  }
  for (std::size_t i = 0; i < Base::GetAddonCount(false); i++) {
    auto addon = Base::GetAddonAt(false, i);
    Base::SetAddonStatus(false, i, lIni.GetBoolValue(cActiveMalAddons, RecordToStr(addon).c_str(), true));
  }
  for (std::size_t i = 0; i < Base::GetAddonCount(true); i++) {
    auto addon = Base::GetAddonAt(true, i);
    Base::SetAddonStatus(true, i, lIni.GetBoolValue(cActiveFemAddons, RecordToStr(addon).c_str(), false));
  }
  if (lIni.KeyExists(cControls, cINTCtrl)) fINTCtrl->value = lIni.GetBoolValue(cControls, cINTCtrl) ? 2.0f : 0.0f;
  if (lIni.KeyExists(cControls, cNPCCtrl)) fNPCCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cNPCCtrl));
  if (lIni.KeyExists(cControls, cUPGCtrl)) fUPGCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cUPGCtrl));
  if (lIni.KeyExists(cControls, cDOWCtrl)) fDOWCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cDOWCtrl));
  if (lIni.KeyExists(cControls, cREVCtrl)) fREVCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cREVCtrl));
  if (lIni.KeyExists(cGentleWomen, cGentleWomenChance)) fWomenChance->value = static_cast<float>(lIni.GetDoubleValue(cGentleWomen, cGentleWomenChance));
  lIni.SaveFile(cSettings);
}

void Inis::LoadTngInis() noexcept {
  SKSE::log::info("Loading ini files...");
  if (std::filesystem::exists(cTngInisPath)) {
    for (const auto& lEntry : std::filesystem::directory_iterator(cTngInisPath)) {
      const std::string lFileName = lEntry.path().filename().string();
      if (lFileName.ends_with(cTngIniEnding)) {
        LoadSingleIni(lEntry.path().string().c_str(), lFileName);
      } else {
        SKSE::log::warn("The file {} in TNG ini folder is not named correctly or is not a TNG ini file.", lFileName);
      }
    }
  }
  if (!std::filesystem::exists(cRevealingIni)) {
    std::ofstream lTngRevealingExtra(cRevealingIni);
    lTngRevealingExtra << ";Revealing mods with pieces on slot 52 for TNG" << std::endl;
    lTngRevealingExtra << ";If you are here, please consider sharing your ini file with me: @modilogist on discord or nexus." << std::endl
                       << ";We would share it on TNG page for others, with you being credited for it." << std::endl
                       << ";You can also upload your ini files as a separate mod on nexus if you prefer that." << std::endl
                       << ";Thanks in advance!" << std::endl;
    lTngRevealingExtra.close();
  }
  LoadSingleIni(cRevealingIni, "UserDefinedRevealing");
}

void Inis::LoadSingleIni(const char* aPath, const std::string& aFileName) {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.SetMultiKey();
  lIni.LoadFile(aPath);
  if (lIni.SectionExists(cRaceSection)) {
    CSimpleIniA::TNamesDepend lExMods;
    lIni.GetAllValues(cRaceSection, cExcRaces, lExMods);
    CSimpleIniA::TNamesDepend::const_iterator lMod;
    for (lMod = lExMods.begin(); lMod != lExMods.end(); lMod++) {
      const std::string lModName(lMod->pItem);
      fRaceExMods.insert(lModName);
    }
    SKSE::log::info("\t- Found [{}] excluded mods for their races in [{}].", lExMods.size(), aFileName);
  }
  if (lIni.SectionExists(cExcludeSection)) {
    CSimpleIniA::TNamesDepend lExRecords;
    CSimpleIniA::TNamesDepend::const_iterator lEntry;
    lIni.GetAllKeys(cExcludeSection, lExRecords);
    for (lEntry = lExRecords.begin(); lEntry != lExRecords.end(); lEntry++) {
      auto lRecord = StrToRecord(lEntry->pItem);
      fHardExcluded.insert_or_assign(lRecord.first, lRecord.second);
    }
    SKSE::log::info("\t- Found [{}] excluded NPCs in [{}].", lExRecords.size(), aFileName);
  }
  if (lIni.KeyExists(cSkeleton, cValidModel)) {
    CSimpleIniA::TNamesDepend lSkeletons;
    lIni.GetAllValues(cSkeleton, cValidModel, lSkeletons);
    CSimpleIniA::TNamesDepend::const_iterator lSkeleton;
    for (lSkeleton = lSkeletons.begin(); lSkeleton != lSkeletons.end(); lSkeleton++) {
      const std::string lModel(lSkeleton->pItem);
      fValidSkeletons.insert(lModel);
    }
    SKSE::log::info("\t- Found [{}] skeleton models in [{}].", lSkeletons.size(), aFileName);
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
      SKSE::log::info("\t- Found [{}] skin mods in [{}].", lMods.size(), aFileName);
    }
    if (lIni.KeyExists(cSkinSection, cSkinRecord)) {
      CSimpleIniA::TNamesDepend lModRecords;
      lIni.GetAllValues(cSkinSection, cSkinRecord, lModRecords);
      LoadModRecodPairs(lModRecords, fSingleSkinIDs);
      SKSE::log::info("\t- Found [{}] skin records in [{}].", lModRecords.size(), aFileName);
    }
  }
  if (lIni.SectionExists(cArmorSection)) {
    if (lIni.KeyExists(cArmorSection, cRevealingMod)) {
      CSimpleIniA::TNamesDepend lMods;
      lIni.GetAllValues(cArmorSection, cRevealingMod, lMods);
      for (const auto& lMod : lMods) {
        const std::string lModName(lMod.pItem);
        (aFileName == "UserDefinedRevealing" && fRevealingMods.find(lModName) == fRevealingMods.end()) ? fExtraRevealing.insert(lModName) : fRevealingMods.insert(lModName);
      }
      SKSE::log::info("\t- Found [{}] revealing mods in [{}].", lMods.size(), aFileName);
    }
    if (lIni.KeyExists(cArmorSection, cRevealingRecord)) {
      CSimpleIniA::TNamesDepend lModRecords;
      lIni.GetAllValues(cArmorSection, cRevealingRecord, lModRecords);
      LoadModRecodPairs(lModRecords, fSingleRevealingIDs);
      SKSE::log::info("\t- Found [{}] revealing records in ini file [{}].", lModRecords.size(), aFileName);
    }
    if (lIni.KeyExists(cArmorSection, cCoveringRecord)) {
      CSimpleIniA::TNamesDepend lModRecords;
      lIni.GetAllValues(cArmorSection, cCoveringRecord, lModRecords);
      LoadModRecodPairs(lModRecords, fSingleCoveringIDs);
      SKSE::log::info("\t- Found [{}] covering records in ini file [{}].", lModRecords.size(), aFileName);
    }
  }
}

void Inis::LoadHoteKeys() noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  if (lIni.KeyExists(cControls, cINTCtrl)) fINTCtrl->value = lIni.GetBoolValue(cControls, cINTCtrl) ? 2.0f : 0.0f;
  if (lIni.KeyExists(cControls, cNPCCtrl)) fNPCCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cNPCCtrl));
  if (lIni.KeyExists(cControls, cUPGCtrl)) fUPGCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cUPGCtrl));
  if (lIni.KeyExists(cControls, cDOWCtrl)) fDOWCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cDOWCtrl));
  if (lIni.KeyExists(cControls, cREVCtrl)) fREVCtrl->value = static_cast<float>(lIni.GetLongValue(cControls, cREVCtrl));
}

int Inis::GetLogLvl() noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  return lIni.GetLongValue(cGeneral, cLogLvl, static_cast<int>(spdlog::level::info));
}

void Inis::SetLogLvl(int aLvl) noexcept {
  if (aLvl < static_cast<int>(spdlog::level::info) || aLvl >= static_cast<int>(spdlog::level::n_levels)) return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.SetLongValue(cGeneral, cLogLvl, aLvl);
  lIni.SaveFile(cSettings);
}

bool Inis::GetSettingBool(IniBoolIDs aID) noexcept {
  switch (aID) {
    case excludePlayerSize:
      return fExlPC;
    case revealSlot52Mods:
      return fRADef;
    case revealSlot52User:
      return fRAUsr;
    case randomizeMaleAddn:
      return fRandM;
    default:
      return false;
  }
}

void Inis::SaveSettingBool(IniBoolIDs aID, bool aValue) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  switch (aID) {
    case excludePlayerSize:
      lIni.SetBoolValue(cGeneral, cExlPC, aValue);
      fExlPC = aValue;
      break;
    case revealSlot52Mods:
      lIni.SetBoolValue(cGeneral, cRADef, aValue);
      fRADef = aValue;
      break;
    case revealSlot52User:
      lIni.SetBoolValue(cGeneral, cRAUsr, aValue);
      fRAUsr = aValue;
      break;
    case randomizeMaleAddn:
      lIni.SetBoolValue(cGeneral, cRandM, aValue);
      fRandM = aValue;
      break;
    default:
      break;
  }
  lIni.SaveFile(cSettings);
}

void Inis::SaveRaceMult(const std::size_t aRgId, const float aRaceMult) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  auto race = Base::RgIdRace(aRgId);
  auto lRaceIDStr = RecordToStr(race);
  if (lRaceIDStr.empty()) {
    SKSE::log::critical("Failed to save the size multiplier for race [{:x}: {}]!", race->GetFormID(), race->GetFormEditorID());
    return;
  }
  if (aRaceMult < 0) {
    lIni.Delete(cRacialSize, lRaceIDStr.c_str());
  } else {
    lIni.SetDoubleValue(cRacialSize, lRaceIDStr.c_str(), static_cast<double>(aRaceMult));
  }
  lIni.SaveFile(cSettings);
}

void Inis::SaveRaceAddn(const std::size_t aRgId, int aChoice) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  auto race = Base::RgIdRace(aRgId);
  auto lRaceIDStr = RecordToStr(race);
  if (lRaceIDStr.empty()) {
    SKSE::log::critical("Failed to save the selected addon for race [{:x}: {}]!", race->GetFormID(), race->GetFormEditorID());
    return;
  }
  if (aChoice == -1) {
    lIni.Delete(cRacialGenital, lRaceIDStr.c_str());
  } else {
    auto addon = Base::GetAddonAt(false, aChoice);
    auto lGenIDStr = RecordToStr(addon);
    lIni.SetValue(cRacialGenital, lRaceIDStr.c_str(), lGenIDStr.c_str());
  }
  lIni.SaveFile(cSettings);
}

void Inis::SaveNPCAddn(RE::TESNPC* aNPC, int aChoice) noexcept {
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
      auto addon = Base::GetAddonAt(aNPC->IsFemale(), aChoice);
      auto lGenIDStr = RecordToStr(addon);
      lIni.Delete(cExcludeSection, lNPCIDStr.c_str());
      lIni.SetValue(cNPCAddnSection, lNPCIDStr.c_str(), lGenIDStr.c_str());
      break;
  }
  lIni.SaveFile(cSettings);
}

void Inis::SaveNPCSize(RE::TESNPC* aNPC, int aGenSize) noexcept {
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

void Inis::SaveActiveAddon(const bool aIsFemale, const int aAddon, const bool aStatus) noexcept {
  auto lAddonStr = RecordToStr(Base::GetAddonAt(aIsFemale, aAddon));
  if (lAddonStr.empty()) return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.SetBoolValue(aIsFemale ? cActiveFemAddons : cActiveMalAddons, lAddonStr.c_str(), aStatus);
  lIni.SaveFile(cSettings);
}

void Inis::SaveRevealingArmor(RE::TESObjectARMO* aArmor) noexcept {
  auto lArmoIDStr = RecordToStr(aArmor);
  if (lArmoIDStr.empty()) return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.SetBoolValue(cRevealingRecord, lArmoIDStr.c_str(), true);
  lIni.SaveFile(cSettings);
}

void Inis::SaveCoveringArmor(RE::TESObjectARMO* aArmor) noexcept {
  auto lArmoIDStr = RecordToStr(aArmor);
  if (lArmoIDStr.empty()) return;
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  lIni.SetBoolValue(cRevealingRecord, lArmoIDStr.c_str(), false);
  lIni.SaveFile(cSettings);
}

void Inis::SaveGlobals() noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.LoadFile(cSettings);
  for (std::size_t i = 0; i < Tng::cSizeCategories; i++) lIni.SetDoubleValue(cGlobalSize, cSizeNames[i], Base::GetGlobalSize(i));
  lIni.SetBoolValue(cControls, cINTCtrl, fINTCtrl->value > 1.0f);
  lIni.SetLongValue(cControls, cNPCCtrl, static_cast<int>(fNPCCtrl->value));
  lIni.SetLongValue(cControls, cUPGCtrl, static_cast<int>(fUPGCtrl->value));
  lIni.SetLongValue(cControls, cDOWCtrl, static_cast<int>(fDOWCtrl->value));
  lIni.SetLongValue(cControls, cREVCtrl, static_cast<int>(fREVCtrl->value));
  lIni.SetDoubleValue(cGentleWomen, cGentleWomenChance, fWomenChance->value);
  lIni.SaveFile(cSettings);
}

void Inis::UpdateValidSkeletons(std::set<std::string> aValidSkeletons) noexcept { fValidSkeletons.insert(aValidSkeletons.begin(), aValidSkeletons.end()); }

bool Inis::IsValidSkeleton(std::string aModel) noexcept { return fValidSkeletons.find(aModel) != fValidSkeletons.end(); }

bool Inis::Slot52ModBehavior(const std::string& aModName, const int aBehavior) noexcept {
  CSimpleIniA lIni;
  lIni.SetUnicode();
  lIni.SetMultiKey();
  lIni.LoadFile(cRevealingIni);
  switch (aBehavior) {
    case 1:
      lIni.DeleteValue(cArmorSection, cRevealingMod, aModName.c_str());
      lIni.SetValue(cArmorSection, cRevealingMod, aModName.c_str());
      lIni.SaveFile(cRevealingIni);
      fExtraRevealing.insert(aModName);
      return true;
    case 0:
      lIni.DeleteValue(cArmorSection, cRevealingMod, aModName.c_str());
      lIni.SaveFile(cRevealingIni);
      fExtraRevealing.erase(aModName);
      return false;
    default:
      CSimpleIniA::TNamesDepend lMods;
      lIni.GetAllValues(cArmorSection, cRevealingMod, lMods);
      CSimpleIniA::TNamesDepend::const_iterator lMod;
      for (lMod = lMods.begin(); lMod != lMods.end(); lMod++) {
        const std::string lModName(lMod->pItem);
        if (lModName == aModName) return true;
      }
      return false;
  }
}

void Inis::UpdateIniVersion() noexcept {
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
      lIni.Delete("AutoReveal", NULL);
      if (lIni.SectionExists(cNPCAddnSection)) lIni.Delete(cNPCAddnSection, NULL);
      break;
    default:
      break;
  }
  lIni.SetLongValue(cIniVersion, cVersion, cCurrVersion);
  lIni.SaveFile(cSettings);
}

void Inis::LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>>& aField) noexcept {
  CSimpleIniA::TNamesDepend::const_iterator lEntry;
  for (lEntry = aModRecords.begin(); lEntry != aModRecords.end(); lEntry++) {
    const std::string lModRecord(lEntry->pItem);
    aField.insert(StrToRecord(lModRecord));
  }
}

void Inis::UpdateRevealing(const std::string aArmorRecod, const bool aIsRevealing) noexcept {
  auto armor = Tng::SEDH()->LookupForm<RE::TESObjectARMO>(StrToRecord(aArmorRecod).second, StrToRecord(aArmorRecod).first);
  if (!armor) {
    SKSE::log::info("Previously marked {} armor from mod {} does not exist anymore!", aIsRevealing ? "revealing" : "covering", StrToRecord(aArmorRecod).first);
    return;
  }
  auto& list = aIsRevealing ? fRunTimeRevealingIDs : fRuntimeCoveringIDs;
  list.insert(std::make_pair<std::string, RE::FormID>(StrToRecord(aArmorRecod).first, StrToRecord(aArmorRecod).second));
}
