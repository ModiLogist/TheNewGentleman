#include <Base.h>
#include <Inis.h>

void Inis::LoadTngInis() {
  Tng::logger::info("Loading ini files...");
  if (std::filesystem::exists(cTngInisPath)) {
    for (const auto &entry : std::filesystem::directory_iterator(cTngInisPath)) {
      const std::string fileName = entry.path().filename().string();
      if (fileName.ends_with(cTngIniEnding)) {
        LoadSingleIni(entry.path().string().c_str(), fileName);
      } else {
        Tng::logger::warn("The file {} in TNG ini folder is not named correctly or is not a TNG ini file.", fileName);
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
        if (Tng::SEDH()->LookupModByName(modName)) {
          revealingMods.insert(modName);
          Tng::logger::info("\t\tTheNewGentleman keeps an eye for [{}] as a revealing armor mod.", modName);
        }
      }
    }
    if (ini.GetAllValues(cArmorSection, cFemRevMod, values)) {
      Tng::logger::info("\t- Found [{}] female revealing mods in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        if (Tng::SEDH()->LookupModByName(modName)) {
          femRevMods.insert(modName);
          Tng::logger::info("\t\tTheNewGentleman keeps an eye for [{}] as a female revealing armor mod.", modName);
        }
      }
    }
    if (ini.GetAllValues(cArmorSection, cMalRevMod, values)) {
      Tng::logger::info("\t- Found [{}] male revealing mods in [{}].", values.size(), fileName);
      for (const auto &entry : values) {
        const std::string modName(entry.pItem);
        if (Tng::SEDH()->LookupModByName(modName)) {
          malRevMods.insert(modName);
          Tng::logger::info("\t\tTheNewGentleman keeps an eye for [{}] as a male revealing armor mod.", modName);
        }
      }
    }    
    if (ini.GetAllValues(cArmorSection, cCoveringRecord, values)) {
      Tng::logger::info("\t- Found [{}] covering records in ini file [{}].", values.size(), fileName);
      LoadModRecodPairs(values, coveringRecords);
    }
    if (ini.GetAllValues(cArmorSection, cRevealingRecord, values)) {
      Tng::logger::info("\t- Found [{}] revealing records in ini file [{}].", values.size(), fileName);
      LoadModRecodPairs(values, revealingRecords);
    }
    if (ini.GetAllValues(cArmorSection, cFemRevRecord, values)) {
      Tng::logger::info("\t- Found [{}] revealing records for women in ini file [{}].", values.size(), fileName);
      LoadModRecodPairs(values, femRevRecords);
    }
    if (ini.GetAllValues(cArmorSection, cMalRevRecord, values)) {
      Tng::logger::info("\t- Found [{}] revealing records for men in ini file [{}].", values.size(), fileName);
      LoadModRecodPairs(values, malRevRecords);
    }
  }
}

void Inis::LoadMainIni() {
  if (!std::filesystem::exists(cSettings)) {
    std::ofstream title(cSettings);
    title << ";TNG Settings File" << std::endl;
    title.close();
  }
  Tng::logger::info("Loading TNG settings...");
  UpdateIniVersion();
  CSimpleIniA ini;
  CSimpleIniA::TNamesDepend values;
  ini.SetUnicode();
  ini.LoadFile(cSettings);

  for (size_t i = 0; i < Base::GetAddonCount(false, false); i++) {
    auto addon = Base::AddonByIdx(false, i, false);
    Base::SetAddonStatus(false, i, ini.GetBoolValue(cActiveMalAddons, FormToStr(addon).c_str(), true));
  }
  for (size_t i = 0; i < Base::GetAddonCount(true, false); i++) {
    auto addon = Base::AddonByIdx(true, i, false);
    Base::SetAddonStatus(true, i, ini.GetBoolValue(cActiveFemAddons, FormToStr(addon).c_str(), false));
  }
  Tng::logger::debug("\tRestored all addon status to previous selections");
  ini.GetAllKeys(cRacialAddon, values);
  for (const auto &entry : values) {
    std::string addonStr = ini.GetValue(cRacialAddon, entry.pItem);
    auto raceRecord = StrToLoc(std::string((entry.pItem)));
    auto addonRecord = StrToLoc(addonStr);
    if (!raceRecord.second.empty() && !addonRecord.second.empty()) racialAddons.insert({raceRecord, addonRecord});
  }
  ini.GetAllKeys(cRacialSize, values);
  for (const auto &entry : values) {
    auto mult = ini.GetDoubleValue(cRacialSize, entry.pItem);
    auto raceRecord = StrToLoc(std::string((entry.pItem)));
    if (!raceRecord.second.empty()) {
      racialMults.insert({raceRecord, static_cast<float>(mult)});
    }
  }
  ini.GetAllKeys(cNPCAddonSection, values);
  for (const auto &entry : values) {
    auto addonStr = ini.GetValue(cNPCAddonSection, entry.pItem);
    auto addonRecord = StrToLoc(addonStr);
    auto npcRecord = StrToLoc(entry.pItem);
    if (!npcRecord.second.empty() && !addonRecord.second.empty()) npcAddons.insert({npcRecord, addonRecord});
  }
  ini.GetAllKeys(cNPCSizeSection, values);
  for (const auto &entry : values) {
    auto sizeCat = ini.GetLongValue(cNPCSizeSection, entry.pItem);
    auto npcRecord = StrToLoc(entry.pItem);
    if (!npcRecord.second.empty()) npcSizeCats.insert({npcRecord, sizeCat});
  }
  ini.GetAllKeys(cExcludeNPCSection, values);
  for (const auto &entry : values) {
    auto isExcluded = ini.GetBoolValue(cExcludeNPCSection, entry.pItem);
    const std::string npcRecord(entry.pItem);
    if (isExcluded) {
      Base::ExcludeNPC(npcRecord);
      Tng::logger::debug("\tThe npc [{}] was excluded, since it was excluded by user previously.", npcRecord);
    }
  }
  ini.GetAllKeys(cRevealingSection, values);
  for (const auto &entry : values) {
    auto isRevealing = ini.GetBoolValue(cRevealingSection, entry.pItem);
    const std::string armorRecord(entry.pItem);
    UpdateRevealing(armorRecord, isRevealing ? Tng::akeyReveal : Tng::akeyCover);
    Tng::logger::debug("\tThe amor [{}] was marked revealing, due to user choice in the past.", armorRecord);
  }
  ini.GetAllKeys(cFemRevRecordSection, values);
  for (const auto &entry : values) {
    auto isRevealing = ini.GetBoolValue(cFemRevRecordSection, entry.pItem);
    const std::string armorRecord(entry.pItem);
    UpdateRevealing(armorRecord, isRevealing ? Tng::akeyRevFem : Tng::cNA);
    Tng::logger::debug("\tThe amor [{}] was marked revealing for women, due to user choice in the past.", armorRecord);
  }
  ini.GetAllKeys(cMalRevRecordSection, values);
  for (const auto &entry : values) {
    auto isRevealing = ini.GetBoolValue(cMalRevRecordSection, entry.pItem);
    const std::string armorRecord(entry.pItem);
    UpdateRevealing(armorRecord, isRevealing ? Tng::akeyRevMal : Tng::cNA);
    Tng::logger::debug("\tThe amor [{}] was marked revealing for men, due to user choice in the past.", armorRecord);
  }

  for (size_t i = 0; i < Tng::BoolSettingCount; i++) {
    Tng::boolSettings[i] = ini.GetBoolValue(cGeneral, cBoolSettings[i], false);
    Tng::logger::debug("\tThe boolean setting [{}] was restored to [{}].", cBoolSettings[i], Tng::boolSettings[i]);
  }
  if (ini.KeyExists(cControls, cCtrlNames[Tng::ctrlDAK])) Tng::UserCtrl(Tng::ctrlDAK)->value = ini.GetBoolValue(cControls, cCtrlNames[Tng::ctrlDAK]) ? 2.0f : 0.0f;
  for (size_t i = 0; i < Tng::UserCtrlsCount; i++)
    if (ini.KeyExists(cControls, cCtrlNames[i])) Tng::UserCtrl(i)->value = static_cast<float>(ini.GetLongValue(cControls, cCtrlNames[i]));
  Tng::logger::debug("\tInput settings loaded.");
  for (size_t i = 0; i < Tng::cSizeCategories; i++) Base::SetGlobalSize(i, static_cast<float>(ini.GetDoubleValue(cGlobalSize, cSizeNames[i], cDefSizes[i])));
  Tng::logger::debug("\tGlobal size settings loaded.");
  if (ini.KeyExists(cGentleWomen, cGentleWomenChance)) Tng::WRndGlb()->value = static_cast<float>(ini.GetDoubleValue(cGentleWomen, cGentleWomenChance));
  Tng::logger::debug("\tGentlewomen chance value loaded.");
}

void Inis::LoadRgInfo() {
  for (auto &rgInfo : racialAddons) {
    auto rgIdx = Base::GetRaceRgIdx(Tng::SEDH()->LookupForm<RE::TESRace>(rgInfo.first.first, rgInfo.first.second));
    auto addonIdx = Base::AddonIdxByLoc(false, rgInfo.second);
    if ((rgIdx < 0) || (addonIdx < 0)) continue;
    if (Base::SetRgAddon(rgIdx, addonIdx, false))
      Tng::logger::debug("\tRestored the addon of race [xx{:x}] from file [{}] to addon [xx{:x}] from file [{}]", rgInfo.first.first, rgInfo.first.second, rgInfo.second.first, rgInfo.second.second);
  }
  for (auto &rgInfo : racialMults) {
    auto rgIdx = Base::GetRaceRgIdx(Tng::SEDH()->LookupForm<RE::TESRace>(rgInfo.first.first, rgInfo.first.second));
    if (rgIdx < 0) continue;
    if (Base::SetRgMult(rgIdx, rgInfo.second, false)) Tng::logger::debug("\tRestored the size multiplier of race [xx{:x}] from file [{}] to [{}]", rgInfo.first.first, rgInfo.first.second, rgInfo.second);
  }
  racialAddons.clear();
  racialMults.clear();
}

void Inis::LoadNpcInfo() {
  for (auto &npcInfo : npcAddons) {
    auto npc = Tng::SEDH()->LookupForm<RE::TESNPC>(npcInfo.first.first, npcInfo.first.second);
    if (!npc) continue;
    auto addonIdx = Base::AddonIdxByLoc(npc->IsFemale(), npcInfo.second);
    if (addonIdx < 0) continue;
    if (Base::SetNPCAddon(npc, addonIdx, true) >= 0)
      Tng::logger::debug("\tRestored the addon of npc [xx{:x}] from file [{}] to addon [xx{:x}] from file [{}]", npcInfo.first.first, npcInfo.first.second, npcInfo.second.first, npcInfo.second.second);
  }
  for (auto &npcInfo : npcSizeCats) {
    auto npc = Tng::SEDH()->LookupForm<RE::TESNPC>(npcInfo.first.first, npcInfo.first.second);
    auto sizeCat = npcInfo.second;
    if (!npc || sizeCat < 0 || sizeCat >= Tng::cSizeCategories) continue;
    npc->RemoveKeywords(Tng::SizeKeys());
    npc->AddKeyword(Tng::SizeKey(sizeCat));
    Tng::logger::debug("\tRestored the size category of npc [xx{:x}] from file [{}] to [{}]", npcInfo.first.first, npcInfo.first.second, npcInfo.second);
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

spdlog::level::level_enum Inis::GetLogLvl() {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  auto res = ini.GetLongValue(cGeneral, cLogLvl, static_cast<int>(spdlog::level::info));
  return res > 0 && res < static_cast<int>(spdlog::level::n_levels) ? static_cast<spdlog::level::level_enum>(res) : spdlog::level::info;
}

void Inis::SetLogLvl(int logLevel) {
  if (logLevel < 1 || logLevel >= static_cast<int>(spdlog::level::n_levels)) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  ini.SetLongValue(cGeneral, cLogLvl, logLevel);
  ini.SaveFile(cSettings);
}

void Inis::SaveAddonStatus(const bool isFemale, const int addnIdx, const bool status) {
  auto addonStr = FormToStr(Base::AddonByIdx(isFemale, addnIdx, false));
  if (addonStr.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  status == isFemale ? ini.SetBoolValue(isFemale ? cActiveFemAddons : cActiveMalAddons, addonStr.c_str(), status) : ini.Delete(isFemale ? cActiveFemAddons : cActiveMalAddons, addonStr.c_str(), true);
  ini.SaveFile(cSettings);
}

void Inis::SaveRgMult(const size_t rg, const float mult) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  auto race = Base::GetRgRace0(rg, true);
  auto raceRecord = FormToStr(race);
  if (raceRecord.empty()) {
    Tng::logger::critical("Failed to save the size multiplier for race [0x{:x}: {}]!", race->GetFormID(), race->GetFormEditorID());
    return;
  }
  if (mult < 1.0001f && mult > 0.9999f) {
    ini.Delete(cRacialSize, raceRecord.c_str(), true);
  } else {
    ini.SetDoubleValue(cRacialSize, raceRecord.c_str(), static_cast<double>(mult));
  }
  ini.SaveFile(cSettings);
}

void Inis::SaveRgAddon(const size_t rg, const int choice) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  auto race = Base::GetRgRace0(rg, true);
  auto raceRecord = FormToStr(race);
  if (raceRecord == "") {
    if (race) {
      Tng::logger::critical("Failed to save the selected addon for race [0x{:x}: {}]!", race->GetFormID(), race->GetFormEditorID());
    } else {
      Tng::logger::critical("Failed to save the selected addon for a race!");
    }
    return;
  }
  switch (choice) {
    case Tng::cDef:
      ini.Delete(cRacialAddon, raceRecord.c_str(), true);
      break;
    case Tng::cNul:
      ini.SetValue(cRacialAddon, raceRecord.c_str(), "None");
      break;
    default:
      auto addon = Base::AddonByIdx(false, choice, false);
      auto addonRecord = FormToStr(addon);
      if (addonRecord == "") {
        if (addon) {
          Tng::logger::critical("Failed to save the addon [0x{:x}: {}] for a race !", addon->GetFormID(), addon->GetFormEditorID());
        } else {
          Tng::logger::critical("Failed to save an addon for a race !");
        }
        return;
      }
      ini.SetValue(cRacialAddon, raceRecord.c_str(), addonRecord.c_str());
      break;
  }
  ini.SaveFile(cSettings);
}

void Inis::SaveNPCAddon(RE::TESNPC *npc, const int choice) {
  auto npcRecord = FormToStr(npc);
  if (npcRecord.empty()) {
    return;
  }
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  switch (choice) {
    case Tng::cDef:
      ini.Delete(cNPCAddonSection, npcRecord.c_str(), true);
      break;
    case Tng::cNul:
      ini.Delete(cNPCAddonSection, npcRecord.c_str(), true);
      ini.SetBoolValue(cExcludeNPCSection, npcRecord.c_str(), true);
      break;
    default:
      auto addon = Base::AddonByIdx(npc->IsFemale(), choice, false);
      auto addonRecord = FormToStr(addon);
      ini.Delete(cExcludeNPCSection, npcRecord.c_str(), true);
      ini.SetValue(cNPCAddonSection, npcRecord.c_str(), addonRecord.c_str());
      break;
  }
  ini.SaveFile(cSettings);
}

void Inis::SaveNPCSize(RE::TESNPC *npc, int genSize) {
  if (genSize == Tng::cNul) return;
  auto npcRecord = FormToStr(npc);
  if (npcRecord.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  if (genSize == Tng::cDef) {
    ini.Delete(cNPCSizeSection, npcRecord.c_str(), true);
  } else {
    ini.SetLongValue(cNPCSizeSection, npcRecord.c_str(), genSize);
  }
  ini.SaveFile(cSettings);
}

void Inis::SaveRevealingArmor(RE::TESObjectARMO *armor, int revMode) {
  auto armoRecord = FormToStr(armor);
  if (armoRecord.empty()) return;
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  ini.Delete(cRevealingSection, armoRecord.c_str(), true);
  ini.Delete(cFemRevRecordSection, armoRecord.c_str(), true);
  ini.Delete(cMalRevRecordSection, armoRecord.c_str(), true);
  switch (revMode) {
    case Tng::akeyCover:
      ini.SetBoolValue(cRevealingSection, armoRecord.c_str(), false);
      break;
    case Tng::akeyReveal:
      ini.SetBoolValue(cRevealingSection, armoRecord.c_str(), true);
      break;
    case Tng::akeyRevFem:
      ini.SetBoolValue(cFemRevRecordSection, armoRecord.c_str(), true);
      break;
    case Tng::akeyRevMal:
      ini.SetBoolValue(cMalRevRecordSection, armoRecord.c_str(), true);
      break;
    default:
      ini.SetBoolValue(cRevealingSection, armoRecord.c_str(), true);
      break;
  }
  ini.SaveFile(cSettings);
}

void Inis::LoadHoteKeys() {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(cSettings);
  if (ini.KeyExists(cControls, cCtrlNames[Tng::ctrlDAK])) Tng::UserCtrl(Tng::ctrlDAK)->value = ini.GetBoolValue(cControls, cCtrlNames[Tng::ctrlDAK]) ? 2.0f : 0.0f;
  for (size_t i = 0; i < Tng::UserCtrlsCount; i++)
    if (ini.KeyExists(cControls, cCtrlNames[i])) Tng::UserCtrl(i)->value = static_cast<float>(ini.GetLongValue(cControls, cCtrlNames[i]));
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

std::vector<std::string> Inis::Slot52Mods() { return std::vector<std::string>(slot52Mods.begin(), slot52Mods.end()); }

bool Inis::Slot52ModBehavior(const std::string modName, const int behavior) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.SetMultiKey();
  ini.LoadFile(cSettings);
  switch (behavior) {
    case 1:
      ini.SetBoolValue(cRevealingModSection, modName.c_str(), true);
      ini.SaveFile(cSettings);
      extraRevealingMods.insert(modName);
      return true;
    case 0:
      ini.Delete(cRevealingModSection, modName.c_str(), true);
      ini.SaveFile(cSettings);
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
  ini.LoadFile(cSettings);
  int lIniVersion = ini.GetLongValue(cIniVersion, cVersion, 1);
  switch (lIniVersion) {
    case 1:
      ini.GetAllSections(sections);
      for (CSimpleIniA::TNamesDepend::const_iterator section = sections.begin(); section != sections.end(); section++) ini.Delete(section->pItem, NULL);
      break;
    case 2:
      ini.Delete("AutoReveal", NULL);
      if (ini.SectionExists(cNPCAddonSection)) ini.Delete(cNPCAddonSection, NULL);
      break;
    case 3:
      if (std::filesystem::remove(R"(.\Data\SKSE\Plugins\Defaults_TNG.ini)")) Tng::logger::info("Removed the [Defaults_TNG.ini] file since it is not used anymore");
    default:
      break;
  }
  ini.SetLongValue(cIniVersion, cVersion, cCurrVersion);
  ini.SaveFile(cSettings);
}

void Inis::LoadModRecodPairs(CSimpleIniA::TNamesDepend records, std::set<SEFormLoc> &fieldToFill) {
  for (const auto &entry : records) {
    const std::string modRecord(entry.pItem);
    fieldToFill.insert(StrToLoc(modRecord));
  }
}

void Inis::UpdateRevealing(const std::string armorRecod, const int revealingMode) {
  auto armor = Tng::SEDH()->LookupForm<RE::TESObjectARMO>(StrToLoc(armorRecod).first, StrToLoc(armorRecod).second);
  if (!armor) {
    Tng::logger::info("Previously save armor from mod {} does not exist anymore!", StrToLoc(armorRecod).first);
    return;
  }
  if (revealingMode < 0) return;
  auto &list = [&]() -> std::set<SEFormLoc> & {
    switch (revealingMode) {
      case Tng::akeyCover:
        return runtimeCoveringRecords;
      case Tng::akeyReveal:
        return runTimeRevealingRecords;
      case Tng::akeyRevFem:
        return runTimeFemRevRecords;
      case Tng::akeyRevMal:
        return runTimeMalRevRecords;
      default:
        return runtimeCoveringRecords;
    }
  }();
  list.insert(StrToLoc(armorRecod));
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
  if (excludedNPCs.find(FormToLoc(npc)) != excludedNPCs.end()) return true;
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
  if (modName == "") return Tng::cNA;
  if (revealingMods.find(modName) != revealingMods.end()) return Tng::akeyReveal;
  if (femRevMods.find(modName) != femRevMods.end()) return Tng::akeyRevFem;
  if (malRevMods.find(modName) != malRevMods.end()) return Tng::akeyRevMal;
  if (revealingRecords.find({armor->GetLocalFormID(), modName}) != revealingRecords.end()) return Tng::akeyReveal;
  if (femRevRecords.find({armor->GetLocalFormID(), modName}) != femRevRecords.end()) return Tng::akeyRevFem;
  if (malRevRecords.find({armor->GetLocalFormID(), modName}) != malRevRecords.end()) return Tng::akeyRevMal;
  if (armor->HasKeywordString(Tng::cSOSR)) return Tng::akeyReveal;
  return Tng::cNA;
}

bool Inis::IsRTCovering(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return false;
  if (runtimeCoveringRecords.find({armor->GetLocalFormID(), modName}) != runtimeCoveringRecords.end()) return true;
  return false;
}

int Inis::IsRTRevealing(const RE::TESObjectARMO *armor, const std::string modName) {
  if (modName == "") return Tng::cNA;
  if (runTimeRevealingRecords.find({armor->GetLocalFormID(), modName}) != runTimeRevealingRecords.end()) return Tng::akeyReveal;
  if (runTimeFemRevRecords.find({armor->GetLocalFormID(), modName}) != runTimeFemRevRecords.end()) return Tng::akeyRevFem;
  if (runTimeMalRevRecords.find({armor->GetLocalFormID(), modName}) != runTimeMalRevRecords.end()) return Tng::akeyRevMal;
  return Tng::cNA;
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
