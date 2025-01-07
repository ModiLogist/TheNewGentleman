#include <Base.h>
#include <Inis.h>

void Inis::LoadMainIni() {
  if (!std::filesystem::exists(cSettings)) {
    std::ofstream lTngSettings(cSettings);
    lTngSettings << ";TNG Settings File" << std::endl;
    lTngSettings.close();
  }
  Tng::logger::info("Loading TNG settings...");
  UpdateIniVersion();
  CSimpleIniA ini;
  CSimpleIniA::TNamesDepend values;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  for (size_t i = 0; i < Tng::BoolSettingCount; i++) Tng::boolSettings[i] = ini.GetBoolValue(cGeneral, cBoolSettings[i], false);

  if (ini.KeyExists(cControls, cCtrlNames[Tng::ctrlDAK])) Tng::UserCtrl(Tng::ctrlDAK)->value = ini.GetBoolValue(cControls, cCtrlNames[Tng::ctrlDAK]) ? 2.0f : 0.0f;
  for (size_t i = 0; i < Tng::UserCtrlsCount; i++)
    if (ini.KeyExists(cControls, cCtrlNames[i])) Tng::UserCtrl(i)->value = static_cast<float>(ini.GetLongValue(cControls, cCtrlNames[i]));

  for (size_t i = 0; i < Tng::cSizeCategories; i++) Base::SetGlobalSize(i, static_cast<float>(ini.GetDoubleValue(cGlobalSize, cSizeNames[i], cDefSizes[i])));

  ini.GetAllKeys(cRacialGenital, values);
  for (const auto &entry : values) {
    std::string addon = ini.GetValue(cRacialGenital, entry.pItem);
    const std::string lRaceRecord(entry.pItem);
    if (!Base::LoadRgAddn(lRaceRecord, addon)) ini.Delete(cRacialGenital, entry.pItem);
  }
  ini.GetAllKeys(cRacialSize, values);
  for (const auto &entry : values) {
    auto lMult = ini.GetDoubleValue(cRacialSize, entry.pItem);
    const std::string lRaceRecord(entry.pItem);
    if (!Base::LoadRgMult(lRaceRecord, static_cast<float>(lMult))) ini.Delete(cRacialSize, entry.pItem);
  }
  ini.GetAllKeys(cNPCSizeSection, values);
  for (const auto &entry : values) {
    auto lSize = ini.GetLongValue(cNPCSizeSection, entry.pItem);
    const std::string lNPCRecord(entry.pItem);
    if (!Base::LoadNPCSize(lNPCRecord, lSize)) ini.Delete(cNPCSizeSection, entry.pItem);
  }
  ini.GetAllKeys(cNPCAddnSection, values);
  for (const auto &entry : values) {
    const std::string lNPCRecord(entry.pItem);
    std::string addon = ini.GetValue(cNPCAddnSection, entry.pItem);
    if (!Base::LoadNPCAddn(lNPCRecord, addon)) ini.Delete(cNPCAddnSection, entry.pItem);
  }
  for (size_t i = 0; i < Base::GetAddonCount(false, false); i++) {
    auto addon = Base::AddonByIdx(false, i, false);
    Base::SetAddonStatus(false, i, ini.GetBoolValue(cActiveMalAddons, RecordToStr(addon).c_str(), true));
  }
  for (size_t i = 0; i < Base::GetAddonCount(true, false); i++) {
    auto addon = Base::AddonByIdx(true, i, false);
    Base::SetAddonStatus(true, i, ini.GetBoolValue(cActiveFemAddons, RecordToStr(addon).c_str(), false));
  }
  ini.GetAllKeys(cExcludeNPCSection, values);
  for (const auto &entry : values) {
    auto isExcluded = ini.GetBoolValue(cExcludeNPCSection, entry.pItem);
    const std::string lNPCRecord(entry.pItem);
    if (isExcluded) Base::ExcludeNPC(lNPCRecord);
  }
  ini.GetAllKeys(cRevealingSection, values);
  for (const auto &entry : values) {
    auto lIsRevealing = ini.GetBoolValue(cRevealingSection, entry.pItem);
    const std::string lArmorRecord(entry.pItem);
    UpdateRevealing(lArmorRecord, lIsRevealing);
  }
  if (ini.KeyExists(cGentleWomen, cGentleWomenChance)) Tng::WRndGlb()->value = static_cast<float>(ini.GetDoubleValue(cGentleWomen, cGentleWomenChance));
  ini.SaveFile(cSettings);
  validSkeletons.insert(Tng::Race(Tng::raceDefault)->skeletonModels[0].model.data());
  validSkeletons.insert(Tng::Race(Tng::raceDefault)->skeletonModels[1].model.data());
  validSkeletons.insert(Tng::Race(Tng::raceBeast)->skeletonModels[0].model.data());
  validSkeletons.insert(Tng::Race(Tng::raceBeast)->skeletonModels[1].model.data());
}

void Inis::LoadTngInis() {
  Tng::logger::info("Loading ini files...");
  if (std::filesystem::exists(cTngInisPath)) {
    for (const auto &entry : std::filesystem::directory_iterator(cTngInisPath)) {
      const std::string lFileName = entry.path().filename().string();
      if (lFileName.ends_with(cTngIniEnding)) {
        LoadSingleIni(entry.path().string().c_str(), lFileName);
      } else {
        Tng::logger::warn("The file {} in TNG ini folder is not named correctly or is not a TNG ini file.", lFileName);
      }
    }
  }
  if (!std::filesystem::exists(cDefIniPath)) {
    std::ofstream lTngRevealingExtra(cDefIniPath);
    lTngRevealingExtra << ";Revealing mods with pieces on slot 52 for TNG" << std::endl;
    lTngRevealingExtra << ";If you are here, please consider sharing your ini file with me: @modilogist on discord or nexus." << std::endl
                       << ";We would share it on TNG page for others, with you being credited for it." << std::endl
                       << ";You can also upload your ini files as a separate mod on nexus if you prefer that." << std::endl
                       << ";Thanks in advance!" << std::endl;
    lTngRevealingExtra.close();
  }
  for (auto &entry : coveringRecords) handledByInis.insert(entry.first);
  for (auto &entry : revealingRecords) handledByInis.insert(entry.first);
  handledByInis.insert(revealingMods.begin(), revealingMods.end());
  LoadSingleIni(cDefIniPath, cDefIni);
}

void Inis::LoadHoteKeys() {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  if (ini.KeyExists(cControls, cCtrlNames[Tng::ctrlDAK])) Tng::UserCtrl(Tng::ctrlDAK)->value = ini.GetBoolValue(cControls, cCtrlNames[Tng::ctrlDAK]) ? 2.0f : 0.0f;
  for (size_t i = 0; i < Tng::UserCtrlsCount; i++)
    if (ini.KeyExists(cControls, cCtrlNames[i])) Tng::UserCtrl(i)->value = static_cast<float>(ini.GetLongValue(cControls, cCtrlNames[i]));
}

int Inis::GetLogLvl() {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  return ini.GetLongValue(cGeneral, cLogLvl, static_cast<int>(spdlog::level::info));
}

void Inis::SetLogLvl(int aLvl) {
  if (aLvl < static_cast<int>(spdlog::level::info) || aLvl >= static_cast<int>(spdlog::level::n_levels)) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  ini.SetLongValue(cGeneral, cLogLvl, aLvl);
  ini.SaveFile(cSettings);
}

bool Inis::GetSettingBool(Tng::BoolSetting settingID) {
  if (settingID < Tng::BoolSettingCount) {
    return Tng::boolSettings[settingID];
  } else {
    return false;
  }
}

void Inis::SaveSettingBool(Tng::BoolSetting settingID, bool value) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  ini.SetBoolValue(cGeneral, cBoolSettings[settingID], value);
  Tng::boolSettings[settingID] = value;
  ini.SaveFile(cSettings);
}

void Inis::SaveRgMult(const size_t rg, const float mult) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  auto race = Base::GetRgRace0(rg, true);
  auto raceRecord = RecordToStr(race);
  if (raceRecord.empty()) {
    Tng::logger::critical("Failed to save the size multiplier for race [{:x}: {}]!", race->GetFormID(), race->GetFormEditorID());
    return;
  }
  if (mult < 0) {
    ini.Delete(cRacialSize, raceRecord.c_str());
  } else {
    ini.SetDoubleValue(cRacialSize, raceRecord.c_str(), static_cast<double>(mult));
  }
  ini.SaveFile(cSettings);
}

void Inis::SaveRgAddn(const size_t rg, const int choice, const bool onlyMCM) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  auto race = Base::GetRgRace0(rg, onlyMCM);
  auto raceRecord = RecordToStr(race);
  if (raceRecord == "") {
    if (race) {
      Tng::logger::critical("Failed to save the selected addon for race [{:x}: {}]!", race->GetFormID(), race->GetFormEditorID());
    } else {
      Tng::logger::critical("Failed to save the selected addon for a race!");
    }
    return;
  }
  switch (choice) {
    case -1:
      ini.Delete(cRacialGenital, raceRecord.c_str());
      break;
    case -2:
      ini.SetValue(cRacialGenital, raceRecord.c_str(), "None");
      break;
    default:
      auto addon = Base::AddonByIdx(false, choice, false);
      auto addonRecord = RecordToStr(addon);
      if (addonRecord == "") {
        if (addon) {
          Tng::logger::critical("Failed to save the addon [{:x}: {}] for a race !", addon->GetFormID(), addon->GetFormEditorID());
        } else {
          Tng::logger::critical("Failed to save an addon for a race !");
        }
        return;
      }
      ini.SetValue(cRacialGenital, raceRecord.c_str(), addonRecord.c_str());
      break;
  }
  ini.SaveFile(cSettings);
}

void Inis::SaveNPCAddn(RE::TESNPC *npc, const int choice) {
  auto npcRecord = RecordToStr(npc);
  if (npcRecord.empty()) {
    return;
  }
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  switch (choice) {
    case -2:
      ini.Delete(cNPCAddnSection, npcRecord.c_str());
      break;
    case -3:
      ini.Delete(cNPCAddnSection, npcRecord.c_str());
      ini.SetBoolValue(cExcludeSection, npcRecord.c_str(), true);
      break;
    default:
      auto addon = Base::AddonByIdx(npc->IsFemale(), choice, false);
      auto addonRecord = RecordToStr(addon);
      ini.Delete(cExcludeSection, npcRecord.c_str());
      ini.SetValue(cNPCAddnSection, npcRecord.c_str(), addonRecord.c_str());
      break;
  }
  ini.SaveFile(cSettings);
}

void Inis::SaveNPCSize(RE::TESNPC *npc, int genSize) {
  auto npcRecord = RecordToStr(npc);
  if (npcRecord.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  if (genSize == -1) {
    ini.Delete(cNPCSizeSection, npcRecord.c_str());
  } else {
    ini.SetLongValue(cNPCSizeSection, npcRecord.c_str(), genSize);
  }
  ini.SaveFile(cSettings);
}

void Inis::SaveActiveAddon(const bool isFemale, const int addnIdx, const bool status) {
  auto lAddonStr = RecordToStr(Base::AddonByIdx(isFemale, addnIdx, false));
  if (lAddonStr.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  ini.SetBoolValue(isFemale ? cActiveFemAddons : cActiveMalAddons, lAddonStr.c_str(), status);
  ini.SaveFile(cSettings);
}

void Inis::SaveRevealingArmor(RE::TESObjectARMO *armor) {
  auto armoRecord = RecordToStr(armor);
  if (armoRecord.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  ini.SetBoolValue(cRevealingRecord, armoRecord.c_str(), true);
  ini.SaveFile(cSettings);
}

void Inis::SaveCoveringArmor(RE::TESObjectARMO *armor) {
  auto armoRecord = RecordToStr(armor);
  if (armoRecord.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  ini.SetBoolValue(cRevealingRecord, armoRecord.c_str(), false);
  ini.SaveFile(cSettings);
}

void Inis::SaveGlobals() {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  for (size_t i = 0; i < Tng::cSizeCategories; i++) ini.SetDoubleValue(cGlobalSize, cSizeNames[i], Base::GetGlobalSize(i));
  ini.SetBoolValue(cControls, cCtrlNames[Tng::ctrlDAK], Tng::UserCtrl(Tng::ctrlDAK)->value > 1.0f);
  for (size_t i = 0; i < Tng::UserCtrlsCount; i++) ini.SetLongValue(cControls, cCtrlNames[i], static_cast<int>(Tng::UserCtrl(i)->value));
  ini.SetDoubleValue(cGentleWomen, cGentleWomenChance, Tng::WRndGlb()->value);
  ini.SaveFile(cSettings);
}

bool Inis::Slot52ModBehavior(const std::string modName, const int behavior) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.SetMultiKey();
  ini.LoadFile(cDefIniPath);
  switch (behavior) {
    case 1:
      ini.SetValue(cArmorSection, cRevealingMod, modName.c_str());
      ini.SaveFile(cDefIniPath);
      extraRevealingMods.insert(modName);
      return true;
    case 0:
      ini.DeleteValue(cArmorSection, cRevealingMod, modName.c_str());
      ini.SaveFile(cDefIniPath);
      extraRevealingMods.erase(modName);
      return false;
    default:
      CSimpleIniA::TNamesDepend values;
      ini.GetAllValues(cArmorSection, cRevealingMod, values);
      for (const auto &entry : values) {
        const std::string savedMod(entry.pItem);
        if (savedMod == modName) return true;
      }
      return false;
  }
}

void Inis::UpdateIniVersion() {
  CSimpleIniA ini;
  CSimpleIniA::TNamesDepend lSections;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  int lIniVersion = ini.GetLongValue(cIniVersion, cVersion, 1);
  switch (lIniVersion) {
    case 1:
      ini.GetAllSections(lSections);
      for (CSimpleIniA::TNamesDepend::const_iterator lSection = lSections.begin(); lSection != lSections.end(); lSection++) ini.Delete(lSection->pItem, NULL);
      break;
    case 2:
      ini.Delete("AutoReveal", NULL);
      if (ini.SectionExists(cNPCAddnSection)) ini.Delete(cNPCAddnSection, NULL);
      break;
    default:
      break;
  }
  ini.SetLongValue(cIniVersion, cVersion, cCurrVersion);
  ini.SaveFile(cSettings);
}

void Inis::LoadModRecodPairs(CSimpleIniA::TNamesDepend records, std::set<std::pair<std::string, RE::FormID>> &fieldToFill) {
  for (const auto &entry : records) {
    const std::string lModRecord(entry.pItem);
    fieldToFill.insert(StrToRecord(lModRecord));
  }
}

void Inis::UpdateRevealing(const std::string armorRecod, const bool isRevealing) {
  auto armor = Tng::SEDH()->LookupForm<RE::TESObjectARMO>(StrToRecord(armorRecod).second, StrToRecord(armorRecod).first);
  if (!armor) {
    Tng::logger::info("Previously marked {} armor from mod {} does not exist anymore!", isRevealing ? "revealing" : "covering", StrToRecord(armorRecod).first);
    return;
  }
  auto &list = isRevealing ? runTimeRevealingRecordss : runtimeCoveringRecordss;
  list.insert(std::make_pair<std::string, RE::FormID>(StrToRecord(armorRecod).first, StrToRecord(armorRecod).second));
}

void Inis::LoadSingleIni(const char *aPath, const std::string_view fileName) {
  CSimpleIniA ini;
  CSimpleIniA::TNamesDepend values;
  ini.SetUnicode();
  ini.SetMultiKey();
  ini.LoadFile(aPath);
  if (ini.SectionExists(cExcludeSection)) {
    if (ini.GetAllValues(cExcludeSection, cExcludeNPC, values)) {
      Tng::logger::info("\t- Found [{}] excluded NPCs in [{}].", values.size(), fileName);
      LoadModRecodPairs(values, excludedNPCs);
    }
    if (ini.GetAllValues(cExcludeSection, cExcModRaces, values)) {
      Tng::logger::info("\t- Found [{}] excluded mods for their races in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        excludedRaceMods.insert(modName);
      }
    }
    if (ini.GetAllValues(cExcludeSection, cExcRace, values)) {
      Tng::logger::info("\t- Found [{}] excluded races in [{}].", values.size(), fileName);
      LoadModRecodPairs(values, excludedRaces);
    }
  }
  if (ini.SectionExists(cSkeleton)) {
    if (ini.GetAllValues(cSkeleton, cValidModel, values)) {
      Tng::logger::info("\t- Found [{}] skeleton models in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string model(entry.pItem);
        validSkeletons.insert(model);
      }
    }
    if (ini.GetAllValues(cSkeleton, cValidRace, values)) {
      Tng::logger::info("\t- Found [{}] valid races in [{}].", values.size(), fileName);
      LoadModRecodPairs(values, validRaces);
    }
  }
  if (ini.SectionExists(cSkinSection)) {
    if (ini.GetAllValues(cSkinSection, cSkinMod, values)) {
      Tng::logger::info("\t- Found [{}] skin mods in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        if (Tng::SEDH()->LookupModByName(modName)) {
          Tng::logger::info("\t\tTheNewGentleman keeps an eye for [{}] as a skin mod.", modName);
          skinMods.insert(modName);
        }
      }
    }
    if (ini.GetAllValues(cSkinSection, cSkinRecord, values)) {
      Tng::logger::info("\t- Found [{}] skin records in [{}].", values.size(), fileName);
      LoadModRecodPairs(values, skinRecords);
    }
  }
  if (ini.SectionExists(cArmorSection)) {
    if (ini.GetAllValues(cArmorSection, cRevealingMod, values)) {
      Tng::logger::info("\t- Found [{}] revealing mods in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        if (modName == cDefIni && modName == Tng::cSkyrim) {
          ini.DeleteValue(cArmorSection, cRevealingMod, entry.pItem);
          continue;
        }
        if (Tng::SEDH()->LookupModByName(modName)) {
          if (modName == cDefIni) {
            if (handledByInis.find(modName) == handledByInis.end()) {
              extraRevealingMods.insert(modName);
              slot52Mods.insert(modName);
              Tng::logger::info("\t\tTheNewGentleman keeps an eye for [{}] as a revealing armor mod since it was set to be revealing in MCM previously.", modName);
            } else {
              continue;
            }
          } else {
            revealingMods.insert(modName);
            Tng::logger::info("\t\tTheNewGentleman keeps an eye for [{}] as a revealing armor mod since it is marked as revealing in an ini.", modName);
          }
        }
      }
    }
    if (ini.GetAllValues(cArmorSection, cRevealingRecord, values)) {
      Tng::logger::info("\t- Found [{}] revealing records in ini file [{}].", values.size(), fileName);
      LoadModRecodPairs(values, revealingRecords);
    }
    if (ini.GetAllValues(cArmorSection, cCoveringRecord, values)) {
      Tng::logger::info("\t- Found [{}] covering records in ini file [{}].", values.size(), fileName);
      LoadModRecodPairs(values, coveringRecords);
    }
  }
}

bool Inis::IsValidSkeleton(const std::string model) { return validSkeletons.find(model) != validSkeletons.end(); }

bool Inis::IsRaceExcluded(const RE::TESRace *race) {
  if (!race->GetFile(0)) return false;
  std::string modName{race->GetFile(0)->GetFilename()};
  if (excludedRaceMods.find(modName) != excludedRaceMods.end()) return true;
  if (excludedRaces.find({modName, race->GetLocalFormID()}) != excludedRaces.end()) return true;
  return false;
}

bool Inis::IsNPCExcluded(const RE::TESNPC *npc) {
  if (!npc->GetFile(0)) return false;
  std::string modName{npc->GetFile(0)->GetFilename()};
  if (excludedNPCs.find({modName, npc->GetLocalFormID()}) != excludedNPCs.end()) return true;
  return false;
}

bool Inis::IsSkin(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (skinMods.find(modName) != skinMods.end()) return true;
  if (skinRecords.find({modName, armor->GetLocalFormID()}) != skinRecords.end()) return true;
  return false;
}

bool Inis::IsCovering(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (coveringRecords.find({modName, armor->GetLocalFormID()}) != coveringRecords.end()) return true;
  return false;
}

bool Inis::IsRevealing(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (revealingMods.find(modName) != revealingMods.end()) return true;
  if (revealingRecords.find({modName, armor->GetLocalFormID()}) != revealingRecords.end()) return true;
  return false;
}

bool Inis::IsExtraRevealing(const std::string modName) {
  if (modName == "") return false;
  if (extraRevealingMods.find(modName) != extraRevealingMods.end()) return true;
  return false;
}

bool Inis::IsRTCovering(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (runtimeCoveringRecordss.find({modName, armor->GetLocalFormID()}) != runtimeCoveringRecordss.end()) return true;
  return false;
}

bool Inis::IsRTRevealing(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (runTimeRevealingRecordss.find({modName, armor->GetLocalFormID()}) != runTimeRevealingRecordss.end()) return true;
  return false;
}

bool Inis::IsUnhandled(const std::string modName) { return handledByInis.find(modName) == handledByInis.end() && slot52Mods.find(modName) == slot52Mods.end(); }

void Inis::HandleModWithSlot52(const std::string modName, const bool defRevealing) {
  slot52Mods.insert(modName);
  if (defRevealing) {
    extraRevealingMods.insert(modName);
    Slot52ModBehavior(modName, 1);
  }
}
