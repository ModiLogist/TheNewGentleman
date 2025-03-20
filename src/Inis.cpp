#include <Base.h>
#include <Inis.h>

Inis *inis = Inis::GetSingleton();

void Inis::LoadTngInis() {
  SKSE::log::info("Loading ini files...");
  if (std::filesystem::exists(cTngInisPath)) {
    for (const auto &entry : std::filesystem::directory_iterator(cTngInisPath)) {
      const std::string fileName = entry.path().filename().string();
      if (fileName.ends_with(cTngIniEnding)) {
        LoadSingleIni(entry.path().string().c_str(), fileName);
      } else {
        SKSE::log::warn("The file {} in TNG ini folder is not named correctly or is not a TNG ini file.", fileName);
      }
    }
  }
}

void Inis::LoadSingleIni(const char *path, const std::string_view fileName) {
  CSimpleIniA ini;
  CSimpleIniA::TNamesDepend values;
  ini.SetUnicode();
  ini.SetMultiKey();
  ini.LoadFile(path);
  if (ini.SectionExists(cExcludeSection)) {
    if (ini.GetAllValues(cExcludeSection, cExcludeNPC, values)) {
      SKSE::log::info("\t- Found [{}] excluded NPCs in [{}].", values.size(), fileName);
      LoadModRecordPairs(values, excludedNPCs);
    }
    if (ini.GetAllValues(cExcludeSection, cExcModRaces, values)) {
      SKSE::log::info("\t- Found [{}] excluded mods for their races in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        excludedRaceMods.insert(modName);
      }
    }
    if (ini.GetAllValues(cExcludeSection, cExcRace, values)) {
      SKSE::log::info("\t- Found [{}] excluded races in [{}].", values.size(), fileName);
      LoadModRecordPairs(values, excludedRaces);
    }
  }
  if (ini.SectionExists(cSkinSection)) {
    if (ini.GetAllValues(cSkinSection, cSkinMod, values)) {
      SKSE::log::info("\t- Found [{}] skin mods in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        if (ut->SEDH()->LookupModByName(modName)) {
          SKSE::log::info("\t\tTheNewGentleman keeps an eye for [{}] as a skin mod.", modName);
          skinMods.insert(modName);
        }
      }
    }
    if (ini.GetAllValues(cSkinSection, cSkinRecord, values)) {
      SKSE::log::info("\t- Found [{}] skin records in [{}].", values.size(), fileName);
      LoadModRecordPairs(values, skinRecords);
    }
  }
  if (ini.SectionExists(cArmorSection)) {
    if (ini.GetAllValues(cArmorSection, cRevealingMod, values)) {
      SKSE::log::info("\t- Found [{}] revealing mods in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        if (ut->SEDH()->LookupModByName(modName)) {
          revealingMods.insert(modName);
          SKSE::log::info("\t\tTheNewGentleman keeps an eye for [{}] as a revealing armor mod.", modName);
        }
      }
    }
    if (ini.GetAllValues(cArmorSection, cFemRevMod, values)) {
      SKSE::log::info("\t- Found [{}] female revealing mods in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        if (ut->SEDH()->LookupModByName(modName)) {
          femRevMods.insert(modName);
          SKSE::log::info("\t\tTheNewGentleman keeps an eye for [{}] as a female revealing armor mod.", modName);
        }
      }
    }
    if (ini.GetAllValues(cArmorSection, cMalRevMod, values)) {
      SKSE::log::info("\t- Found [{}] male revealing mods in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        if (ut->SEDH()->LookupModByName(modName)) {
          malRevMods.insert(modName);
          SKSE::log::info("\t\tTheNewGentleman keeps an eye for [{}] as a male revealing armor mod.", modName);
        }
      }
    }
    if (ini.GetAllValues(cArmorSection, cCoveringRecord, values)) {
      SKSE::log::info("\t- Found [{}] covering records in ini file [{}].", values.size(), fileName);
      LoadModRecordPairs(values, coveringRecords);
    }
    if (ini.GetAllValues(cArmorSection, cRevealingRecord, values)) {
      SKSE::log::info("\t- Found [{}] revealing records in ini file [{}].", values.size(), fileName);
      LoadModRecordPairs(values, revealingRecords);
    }
    if (ini.GetAllValues(cArmorSection, cFemRevRecord, values)) {
      SKSE::log::info("\t- Found [{}] revealing records for women in ini file [{}].", values.size(), fileName);
      LoadModRecordPairs(values, femRevRecords);
    }
    if (ini.GetAllValues(cArmorSection, cMalRevRecord, values)) {
      SKSE::log::info("\t- Found [{}] revealing records for men in ini file [{}].", values.size(), fileName);
      LoadModRecordPairs(values, malRevRecords);
    }
  }
}

void Inis::LoadMainIni() {
  if (!std::filesystem::exists(SettingFile())) {
    std::ofstream newSetting(SettingFile());
    newSetting << ";TNG Settings File" << std::endl;
    newSetting.close();
  }
  SKSE::log::info("Loading TNG settings...");
  UpdateIniVersion();
  CSimpleIniA ini;
  CSimpleIniA::TNamesDepend values;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());

  for (size_t i = 0; i < base->GetAddonCount(false, false); i++) {
    auto addon = base->AddonByIdx(false, i, false);
    base->SetAddonStatus(false, i, ini.GetBoolValue(cActiveMalAddons, ut->FormToStr(addon).c_str(), true));
  }
  for (size_t i = 0; i < base->GetAddonCount(true, false); i++) {
    auto addon = base->AddonByIdx(true, i, false);
    base->SetAddonStatus(true, i, ini.GetBoolValue(cActiveFemAddons, ut->FormToStr(addon).c_str(), false));
  }
  SKSE::log::debug("\tRestored all addon status to previous selections");
  ini.GetAllKeys(cRacialAddon, values);
  for (const auto &entry : values) {
    std::string addonStr = ini.GetValue(cRacialAddon, entry.pItem);
    auto raceRecord = ut->StrToLoc(std::string((entry.pItem)));
    auto addonRecord = ut->StrToLoc(addonStr, true);
    if (!raceRecord.second.empty() && !addonRecord.second.empty()) racialAddons.insert({raceRecord, addonRecord});
  }
  ini.GetAllKeys(cRacialSize, values);
  for (const auto &entry : values) {
    auto mult = ini.GetDoubleValue(cRacialSize, entry.pItem);
    auto raceRecord = ut->StrToLoc(std::string((entry.pItem)));
    if (!raceRecord.second.empty()) {
      racialSizes.insert({raceRecord, static_cast<float>(mult)});
    }
  }
  ini.GetAllKeys(cNPCAddonSection, values);
  for (const auto &entry : values) {
    auto addonStr = ini.GetValue(cNPCAddonSection, entry.pItem);
    auto npcRecord = ut->StrToLoc(entry.pItem);
    auto addonRecord = ut->StrToLoc(addonStr, true);
    if (!npcRecord.second.empty() && !addonRecord.second.empty()) npcAddons.insert({npcRecord, addonRecord});
  }
  ini.GetAllKeys(cNPCSizeSection, values);
  for (const auto &entry : values) {
    auto sizeCat = ini.GetLongValue(cNPCSizeSection, entry.pItem);
    auto npcRecord = ut->StrToLoc(entry.pItem);
    if (!npcRecord.second.empty()) npcSizeCats.insert({npcRecord, sizeCat});
  }
  ini.GetAllKeys(cExcludeNPCSection, values);
  for (const auto &entry : values) {
    auto isExcluded = ini.GetBoolValue(cExcludeNPCSection, entry.pItem);
    const std::string npcRecord(entry.pItem);
    if (isExcluded) {
      base->ExcludeNPC(npcRecord);
      SKSE::log::debug("\tThe npc [{}] was excluded, since it was excluded by user previously.", npcRecord);
    }
  }
  ini.GetAllKeys(cRevealingSection, values);
  for (const auto &entry : values) {
    auto isRevealing = ini.GetBoolValue(cRevealingSection, entry.pItem);
    const std::string armorRecord(entry.pItem);
    UpdateRevealing(armorRecord, isRevealing ? Util::kyRevealing : Util::kyCovering);
    SKSE::log::debug("\tThe amor [{}] was marked revealing, due to user choice in the past.", armorRecord);
  }
  ini.GetAllKeys(cFemRevRecordSection, values);
  for (const auto &entry : values) {
    auto isRevealing = ini.GetBoolValue(cFemRevRecordSection, entry.pItem);
    const std::string armorRecord(entry.pItem);
    UpdateRevealing(armorRecord, isRevealing ? Util::kyRevealingF : Util::nan);
    SKSE::log::debug("\tThe amor [{}] was marked revealing for women, due to user choice in the past.", armorRecord);
  }
  ini.GetAllKeys(cMalRevRecordSection, values);
  for (const auto &entry : values) {
    auto isRevealing = ini.GetBoolValue(cMalRevRecordSection, entry.pItem);
    const std::string armorRecord(entry.pItem);
    UpdateRevealing(armorRecord, isRevealing ? Util::kyRevealingM : Util::nan);
    SKSE::log::debug("\tThe amor [{}] was marked revealing for men, due to user choice in the past.", armorRecord);
  }

  for (size_t i = 0; i < Util::boolSettingCount; i++) {
    base->SetBoolSetting(i, ini.GetBoolValue(cGeneral, cBoolSettings[i], cDefBoolSettings[i]));
    SKSE::log::debug("\tThe boolean setting [{}] was restored to [{}({})].", cBoolSettings[i], base->GetBoolSetting(i),
                     cDefBoolSettings[i] == base->GetBoolSetting(i) ? "default" : "user");
  }
  if (ut->SEDH()->LookupModByName("Racial Skin Variance - SPID.esp")) {
    base->SetBoolSetting(Util::bsCheckPlayerAddon, true);
    base->SetBoolSetting(Util::bsCheckNPCsAddons, true);
    base->SetBoolSetting(Util::bsForceRechecks, true);
    SKSE::log::info("\tTNG detected Racial Skin Variance and would force the player and NPCs to be reloaded");
  }
  if (!ut->SEDH()->LookupModByName("UIExtensions.esp")) {
    base->SetBoolSetting(Util::bsUIExtensions, false);
    SKSE::log::warn("\tTNG could not detected UIExtensions. You may want to check if it is installed.");
  }
  for (size_t i = 0; i < Util::floatSettingCount; i++) base->SetFloatSetting(i, static_cast<float>(ini.GetDoubleValue(cFLoatSections[i], cFloatNames[i], cDefFloatSettings[i])));
  SKSE::log::debug("\tGlobal size settings loaded.");
  SKSE::log::debug("\tGentlewomen chance value loaded.");
  if (ini.KeyExists(cControls, cCtrlNames[Util::ctrlDAK])) {
    base->SetIntSetting(Util::ctrlDAK, ini.GetBoolValue(cControls, cCtrlNames[Util::ctrlDAK]) ? 1 : 0);
  }
  for (size_t i = Util::ctrlSetupNPC; i < Util::ctrlCount; i++) {
    if (ini.KeyExists(cControls, cCtrlNames[i])) {
      base->SetIntSetting(i, ini.GetLongValue(cControls, cCtrlNames[i]));
    }
  }
  SKSE::log::debug("\tInput settings loaded.");
}

void Inis::LoadRgInfo() {
  for (auto &rgInfo : racialAddons) {
    auto rgIdx = base->GetRaceRgIdx(ut->SEDH()->LookupForm<RE::TESRace>(rgInfo.first.first, rgInfo.first.second));
    auto addonIdx = base->AddonIdxByLoc(false, rgInfo.second);
    if ((rgIdx < 0) || (addonIdx < 0 && addonIdx != Util::nul)) continue;
    if (base->SetRgAddon(rgIdx, addonIdx, false))
      SKSE::log::debug("\tRestored the addon of race [xx{:x}] from file [{}] to addon [xx{:x}] from file [{}]", rgInfo.first.first, rgInfo.first.second, rgInfo.second.first,
                       rgInfo.second.second);
  }
  for (auto &rgInfo : racialSizes) {
    auto rgIdx = base->GetRaceRgIdx(ut->SEDH()->LookupForm<RE::TESRace>(rgInfo.first.first, rgInfo.first.second));
    if (rgIdx < 0) continue;
    if (base->SetRgMult(rgIdx, rgInfo.second, false))
      SKSE::log::debug("\tRestored the size multiplier of race [xx{:x}] from file [{}] to [{}]", rgInfo.first.first, rgInfo.first.second, rgInfo.second);
  }
  racialAddons.clear();
  racialSizes.clear();
}

void Inis::LoadNpcInfo() {
  for (auto &npcInfo : npcAddons) {
    auto npc = ut->SEDH()->LookupForm<RE::TESNPC>(npcInfo.first.first, npcInfo.first.second);
    if (!npc) continue;
    auto addonIdx = base->AddonIdxByLoc(npc->IsFemale(), npcInfo.second);
    if (addonIdx < 0 && addonIdx != Util::nul) continue;
    if (base->SetNPCAddon(npc, addonIdx, true) >= 0)
      SKSE::log::debug("\tRestored the addon of npc [xx{:x}] from file [{}] to addon [xx{:x}] from file [{}]", npcInfo.first.first, npcInfo.first.second, npcInfo.second.first,
                       npcInfo.second.second);
  }
  for (auto &npcInfo : npcSizeCats) {
    auto npc = ut->SEDH()->LookupForm<RE::TESNPC>(npcInfo.first.first, npcInfo.first.second);
    auto sizeCat = npcInfo.second;
    if (!npc || sizeCat < 0 || sizeCat >= Util::sizeKeyCount) continue;
    npc->RemoveKeywords(ut->SizeKeys());
    npc->AddKeyword(ut->SizeKey(sizeCat));
    SKSE::log::debug("\tRestored the size category of npc [xx{:x}] from file [{}] to [{}]", npcInfo.first.first, npcInfo.first.second, npcInfo.second);
  }
  npcAddons.clear();
  npcSizeCats.clear();
}

void Inis::CleanIniLists() {
  excludedRaceMods.clear();
  excludedRaces.clear();

  skinMods.clear();
  skinRecords.clear();

  coveringRecords.clear();
  revealingMods.clear();
  femRevMods.clear();
  malRevMods.clear();
  revealingRecords.clear();
  femRevRecords.clear();
  malRevRecords.clear();

  runtimeCoveringRecords.clear();
  runTimeRevealingRecords.clear();
  runTimeFemRevRecords.clear();
  runTimeMalRevRecords.clear();
}

const char *Inis::SettingFile(const int version) const { return fmt::format(cSettings, version).c_str(); }

spdlog::level::level_enum Inis::GetLogLvl() {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  auto res = ini.GetLongValue(cGeneral, cLogLvl, static_cast<int>(spdlog::level::info));
  return res > 0 && res < static_cast<int>(spdlog::level::n_levels) ? static_cast<spdlog::level::level_enum>(res) : spdlog::level::info;
}

void Inis::SetLogLvl(int logLevel) {
  if (logLevel < 1 || logLevel >= static_cast<int>(spdlog::level::n_levels)) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  ini.SetLongValue(cGeneral, cLogLvl, logLevel);
  ini.SaveFile(SettingFile());
}

void Inis::SetAddonStatus(const bool isFemale, const int addnIdx, const bool status) {
  auto addonStr = ut->FormToStr(base->AddonByIdx(isFemale, addnIdx, false));
  if (addonStr.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  status == isFemale ? ini.SetBoolValue(isFemale ? cActiveFemAddons : cActiveMalAddons, addonStr.c_str(), status)
                     : ini.Delete(isFemale ? cActiveFemAddons : cActiveMalAddons, addonStr.c_str(), true);
  ini.SaveFile(SettingFile());
}

void Inis::SetRgMult(const size_t rg, const float mult) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  auto race = base->GetRgRace0(rg, true);
  auto raceRecord = ut->FormToStr(race);
  if (raceRecord.empty()) {
    SKSE::log::critical("Failed to save the size multiplier for race [0x{:x}: {}]!", race->GetFormID(), race->GetFormEditorID());
    return;
  }
  if (mult < 1.0001f && mult > 0.9999f) {
    ini.Delete(cRacialSize, raceRecord.c_str(), true);
  } else {
    ini.SetDoubleValue(cRacialSize, raceRecord.c_str(), static_cast<double>(mult));
  }
  ini.SaveFile(SettingFile());
}

void Inis::SaveRgAddon(const size_t rg, const int choice) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  auto race = base->GetRgRace0(rg, true);
  auto raceRecord = ut->FormToStr(race);
  if (raceRecord == "") {
    if (race) {
      SKSE::log::critical("Failed to save the selected addon for race [0x{:x}: {}]!", race->GetFormID(), race->GetFormEditorID());
    } else {
      SKSE::log::critical("Failed to save the selected addon for a race!");
    }
    return;
  }
  switch (choice) {
    case Util::def:
      ini.Delete(cRacialAddon, raceRecord.c_str(), true);
      break;
    case Util::nul:
      ini.SetValue(cRacialAddon, raceRecord.c_str(), Util::nulStr.c_str());
      break;
    default:
      auto addon = base->AddonByIdx(false, choice, false);
      auto addonRecord = ut->FormToStr(addon);
      if (addonRecord == "") {
        if (addon) {
          SKSE::log::critical("Failed to save the addon [0x{:x}: {}] for a race !", addon->GetFormID(), addon->GetFormEditorID());
        } else {
          SKSE::log::critical("Failed to save an addon for a race !");
        }
        return;
      }
      ini.SetValue(cRacialAddon, raceRecord.c_str(), addonRecord.c_str());
      break;
  }
  ini.SaveFile(SettingFile());
}

void Inis::SaveNPCAddon(RE::TESNPC *npc, const int choice) {
  auto npcRecord = ut->FormToStr(npc);
  if (npcRecord.empty()) {
    return;
  }
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  switch (choice) {
    case Util::def:
      ini.Delete(cNPCAddonSection, npcRecord.c_str(), true);
      break;
    case Util::nul:
      ini.Delete(cNPCAddonSection, npcRecord.c_str(), true);
      ini.SetBoolValue(cExcludeNPCSection, npcRecord.c_str(), true);
      break;
    default:
      auto addon = base->AddonByIdx(npc->IsFemale(), choice, false);
      auto addonRecord = ut->FormToStr(addon);
      ini.Delete(cExcludeNPCSection, npcRecord.c_str(), true);
      ini.SetValue(cNPCAddonSection, npcRecord.c_str(), addonRecord.c_str());
      break;
  }
  ini.SaveFile(SettingFile());
}

void Inis::SaveNPCSize(RE::TESNPC *npc, int genSize) {
  if (genSize == Util::nul) return;
  auto npcRecord = ut->FormToStr(npc);
  if (npcRecord.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  if (genSize == Util::def) {
    ini.Delete(cNPCSizeSection, npcRecord.c_str(), true);
  } else {
    ini.SetLongValue(cNPCSizeSection, npcRecord.c_str(), genSize);
  }
  ini.SaveFile(SettingFile());
}

void Inis::SaveRevealingArmor(RE::TESObjectARMO *armor, int revMode) {
  auto armoRecord = ut->FormToStr(armor);
  if (armoRecord.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  ini.Delete(cRevealingSection, armoRecord.c_str(), true);
  ini.Delete(cFemRevRecordSection, armoRecord.c_str(), true);
  ini.Delete(cMalRevRecordSection, armoRecord.c_str(), true);
  switch (revMode) {
    case Util::kyCovering:
      ini.SetBoolValue(cRevealingSection, armoRecord.c_str(), false);
      break;
    case Util::kyRevealing:
      ini.SetBoolValue(cRevealingSection, armoRecord.c_str(), true);
      break;
    case Util::kyRevealingF:
      ini.SetBoolValue(cFemRevRecordSection, armoRecord.c_str(), true);
      break;
    case Util::kyRevealingM:
      ini.SetBoolValue(cMalRevRecordSection, armoRecord.c_str(), true);
      break;
    default:
      ini.SetBoolValue(cRevealingSection, armoRecord.c_str(), true);
      break;
  }
  ini.SaveFile(SettingFile());
}

void Inis::SetBoolSetting(Util::eBoolSetting settingID, bool value) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  if (cDefBoolSettings[settingID] == value) {
    ini.Delete(cGeneral, cBoolSettings[settingID], true);
  } else {
    ini.SetBoolValue(cGeneral, cBoolSettings[settingID], value);
  }
  base->SetBoolSetting(settingID, value);
  ini.SaveFile(SettingFile());
}

void Inis::SetIntSetting(Util::eCtrlSetting ctrl, int value) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  if (value == -1) {
    ini.Delete(cControls, cCtrlNames[ctrl], true);
  } else if (ctrl == Util::ctrlDAK) {
    ini.SetBoolValue(cControls, cCtrlNames[ctrl], value > 0);
  } else {
    ini.SetLongValue(cControls, cCtrlNames[ctrl], value);
  }
  base->SetIntSetting(ctrl, value);
  ini.SaveFile(SettingFile());
}

void Inis::SetFloatSetting(Util::eFloatSetting settingID, const float value) {
  if (settingID < Util::floatSettingCount) {
    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile(SettingFile());
    if (cDefFloatSettings[settingID] == value) {
      ini.Delete(cFLoatSections[settingID], cFloatNames[settingID], true);
    } else {
      ini.SetDoubleValue(cFLoatSections[settingID], cFloatNames[settingID], value);
    }
    base->SetFloatSetting(settingID, value);
    ini.SaveFile(SettingFile());
  }
}

void Inis::SaveGlobals() {}

std::vector<std::string> Inis::Slot52Mods() { return std::vector<std::string>(slot52Mods.begin(), slot52Mods.end()); }

bool Inis::Slot52ModBehavior(const std::string modName, const int behavior) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.SetMultiKey();
  ini.LoadFile(SettingFile());
  switch (behavior) {
    case 1:
      ini.SetBoolValue(cRevealingModSection, modName.c_str(), true);
      ini.SaveFile(SettingFile());
      extraRevealingMods.insert(modName);
      return true;
    case 0:
      ini.Delete(cRevealingModSection, modName.c_str(), true);
      ini.SaveFile(SettingFile());
      extraRevealingMods.erase(modName);
      return false;
    default:
      CSimpleIniA::TNamesDepend values;
      ini.GetAllKeys(cRevealingModSection, values);
      auto it = std::find_if(values.begin(), values.end(), [&](const auto &entry) { return std::string(entry.pItem) == modName; });
      return it != values.end();
  }
}

void Inis::UpdateIniVersion() {
  CSimpleIniA ini;
  CSimpleIniA::TNamesDepend sections;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  int lIniVersion = ini.GetLongValue(cIniVersion, cVersion, 1);
  if (lIniVersion > cCurrVersion) {
    ut->ShowSkyrimMessage("You downgraded TNG after upgrading! This can cause issues in your game. Check mod description for more info.");
  }
  if (lIniVersion < 2) {
    ini.GetAllSections(sections);
    for (const auto &section : sections) ini.Delete(section.pItem, nullptr);
  }
  if (lIniVersion < 3) {
    ini.Delete("AutoReveal", nullptr);
    if (ini.SectionExists(cNPCAddonSection)) ini.Delete(cNPCAddonSection, nullptr);
  }
  if (lIniVersion < 4) {
    if (std::filesystem::exists(R"(.\Data\SKSE\Plugins\Defaults_TNG.ini)")) SKSE::log::warn("The [Defaults_TNG.ini] file is not used anymore by TNG, feel free to delete it.");
  }
  if (lIniVersion < 5) {
    ini.GetAllSections(sections);
    for (const auto &section : sections) {
      auto sectionName = std::string(section.pItem);
      if (sectionName.contains("RaceSize")) {
        CSimpleIniA::TNamesDepend keys;
        ini.GetAllKeys(section.pItem, keys);
        for (auto &key : keys) {
          auto mult = ini.GetDoubleValue(section.pItem, key.pItem);
          ini.SetDoubleValue(cRacialSize, key.pItem, mult);
        }
        ini.Delete(section.pItem, nullptr);
      }
    }
  }
  ini.SetLongValue(cIniVersion, cVersion, cCurrVersion);
  ini.SaveFile(SettingFile());
}

void Inis::LoadModRecordPairs(CSimpleIniA::TNamesDepend records, std::set<SEFormLoc> &fieldToFill) {
  for (const auto &entry : records) {
    const std::string modRecord(entry.pItem);
    fieldToFill.insert(ut->StrToLoc(modRecord));
  }
}

void Inis::UpdateRevealing(const std::string armorRecord, const int revealingMode) {
  auto armor = ut->SEDH()->LookupForm<RE::TESObjectARMO>(ut->StrToLoc(armorRecord).first, ut->StrToLoc(armorRecord).second);
  if (!armor) {
    SKSE::log::info("Previously save armor from mod {} does not exist anymore!", ut->StrToLoc(armorRecord).first);
    return;
  }
  if (revealingMode < 0) return;
  auto &list = [&]() -> std::set<SEFormLoc> & {
    switch (revealingMode) {
      case Util::kyCovering:
        return runtimeCoveringRecords;
      case Util::kyRevealing:
        return runTimeRevealingRecords;
      case Util::kyRevealingF:
        return runTimeFemRevRecords;
      case Util::kyRevealingM:
        return runTimeMalRevRecords;
      default:
        return runtimeCoveringRecords;
    }
  }();
  list.insert(ut->StrToLoc(armorRecord));
}

bool Inis::IsRaceExcluded(const RE::TESRace *race) {
  if (!race->GetFile(0)) return false;
  std::string modName{race->GetFile(0)->GetFilename()};
  if (excludedRaceMods.find(modName) != excludedRaceMods.end()) return true;
  if (excludedRaces.find({race->GetLocalFormID(), modName}) != excludedRaces.end()) return true;
  return false;
}

bool Inis::IsNPCExcluded(const RE::TESNPC *npc) {
  if (!npc->GetFile(0)) return false;
  if (excludedNPCs.find(ut->FormToLoc(npc)) != excludedNPCs.end()) return true;
  return false;
}

bool Inis::IsSkin(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (skinMods.find(modName) != skinMods.end()) return true;
  if (skinRecords.find({armor->GetLocalFormID(), modName}) != skinRecords.end()) return true;
  return false;
}

bool Inis::IsCovering(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (coveringRecords.find({armor->GetLocalFormID(), modName}) != coveringRecords.end()) return true;
  return false;
}

int Inis::IsRevealing(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return Util::nan;
  if (revealingMods.find(modName) != revealingMods.end()) return Util::kyRevealing;
  if (femRevMods.find(modName) != femRevMods.end()) return Util::kyRevealingF;
  if (malRevMods.find(modName) != malRevMods.end()) return Util::kyRevealingM;
  if (revealingRecords.find({armor->GetLocalFormID(), modName}) != revealingRecords.end()) return Util::kyRevealing;
  if (femRevRecords.find({armor->GetLocalFormID(), modName}) != femRevRecords.end()) return Util::kyRevealingF;
  if (malRevRecords.find({armor->GetLocalFormID(), modName}) != malRevRecords.end()) return Util::kyRevealingM;
  if (armor->HasKeywordString(Util::sosRevealing)) return Util::kyRevealing;
  return Util::nan;
}

bool Inis::IsRTCovering(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (runtimeCoveringRecords.find({armor->GetLocalFormID(), modName}) != runtimeCoveringRecords.end()) return true;
  return false;
}

int Inis::IsRTRevealing(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return Util::nan;
  if (runTimeRevealingRecords.find({armor->GetLocalFormID(), modName}) != runTimeRevealingRecords.end()) return Util::kyRevealing;
  if (runTimeFemRevRecords.find({armor->GetLocalFormID(), modName}) != runTimeFemRevRecords.end()) return Util::kyRevealingF;
  if (runTimeMalRevRecords.find({armor->GetLocalFormID(), modName}) != runTimeMalRevRecords.end()) return Util::kyRevealingM;
  return Util::nan;
}

bool Inis::IsExtraRevealing(const std::string modName) {
  if (modName == "") return false;
  if (extraRevealingMods.find(modName) != extraRevealingMods.end()) return true;
  return false;
}

bool Inis::IsUnhandled(const std::string modName) { return slot52Mods.find(modName) == slot52Mods.end(); }

void Inis::HandleModWithSlot52(const std::string modName, const bool defRevealing) {
  slot52Mods.insert(modName);
  if (defRevealing) {
    extraRevealingMods.insert(modName);
    Slot52ModBehavior(modName, 1);
  }
}
