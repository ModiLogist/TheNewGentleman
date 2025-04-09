#include <Inis.h>

void Inis::LoadMainIni() {
  SKSE::log::info("Loading TNG settings...");
  if (!std::filesystem::exists(SettingFile())) {
    std::ofstream newSetting(SettingFile());
    newSetting << ";TNG Settings File" << std::endl;
    newSetting.close();
    TransferOldIni();
  }
  settingIni.SetUnicode();
  settingIni.LoadFile(SettingFile());

  if (ut->SEDH()->LookupModByName("Racial Skin Variance - SPID.esp")) {
    boolSettings.Set(Common::bsCheckPlayerAddon, true);
    boolSettings.Set(Common::bsForceRechecks, true);
    SKSE::log::info("\tTNG detected Racial Skin Variance and would force the player and NPCs to be reloaded");
  }
  if (ut->SEDH()->LookupModByName("UIExtensions.esp")) {
    boolSettings.Set(Common::bsUIExtensions, true);
  } else {
    SKSE::log::warn("\tTNG could not detected UIExtensions. You may want to check if it is installed.");
  }
  LoadIniPairs<bool>(cActiveMalAddons, activeMalAddons, true);
  LoadIniPairs<bool>(cActiveFemAddons, activeFemAddons, false);
  SKSE::log::debug("\tRestored all addon status to previous selections");
  CSimpleIniA::TNamesDepend keys;
  settingIni.GetAllKeys(cValidSkeletons, keys);
  for (auto &key : keys) validSkeletons.emplace(std::string(key.pItem));
  LoadIniPairs<SEFormLoc>(cRacialAddon, racialAddons, {0, ""});
  LoadIniPairs<float>(cRacialSize, racialSizes, 1.0f);
  SKSE::log::debug("\tRestored all racial addon and size settings");
  LoadIniPairs<SEFormLoc>(cNPCAddonSection, npcAddons, {0, ""});
  LoadIniPairs<int>(cNPCSizeSection, npcSizeCats, 0);
  LoadIniPairs<SEFormLoc>(cActorAddonSection, actorAddons, {0, ""});
  LoadIniPairs<int>(cActorSizeSection, actorSizeCats, 0);
  SKSE::log::debug("\tRestored all NPC and actor addon and size settings");
  LoadIniPairs<bool>(cRevealingSection, runTimeArmorRecords, false);
  LoadIniPairs<bool>(cMalRevRecordSection, runTimeMalRevRecords, false);
  LoadIniPairs<bool>(cFemRevRecordSection, runTimeFemRevRecords, false);
  SKSE::log::debug("\tRestored all revealing records settings");
  SKSE::log::info("TNG settings loaded.");
}

void Inis::SaveMainIni() { settingIni.SaveFile(SettingFile()); }

spdlog::level::level_enum Inis::GetLogLvl() const {
  CSimpleIniA ini;
  ini.SetUnicode();
  auto f = fmt::format(cSettings, std::to_string(iniVersion)).c_str();
  ini.LoadFile(f);
  auto lvl = ini.GetLongValue(cGeneral, cLogLvl, static_cast<int>(spdlog::level::info));
  return lvl > 0 && lvl < static_cast<int>(spdlog::level::n_levels) ? static_cast<spdlog::level::level_enum>(lvl) : spdlog::level::info;
}

void Inis::SetLogLvl(const int newLevel) {
  if (newLevel < 1 || newLevel >= static_cast<int>(spdlog::level::n_levels)) return;
  auto logLevel = static_cast<spdlog::level::level_enum>(newLevel);
  if (logLevel == spdlog::level::info) {
    settingIni.Delete(cGeneral, cLogLvl, true);
  } else {
    settingIni.SetLongValue(cGeneral, cLogLvl, logLevel);
  }
}

const char *Inis::SettingFile(const int version) const { return fmt::format(cSettings, version < 0 ? "" : std::to_string(version)).c_str(); }

void Inis::TransferOldIni() {
  auto oldV = iniVersion > 5 ? iniVersion - 1 : -1;
  while (!std::filesystem::exists(SettingFile(oldV))) {
    oldV--;
    if (oldV < 0) return;
  }
  std::ifstream oldFile(SettingFile(oldV), std::ios::binary);
  std::ofstream newFile(SettingFile(), std::ios::binary);

  if (!oldFile.is_open() || !newFile.is_open()) {
    SKSE::log::critical("\tFound an old ini file but failed transfering its content");
    return;
  }
  newFile << oldFile.rdbuf();
  oldFile.close();
  newFile.close();
  CSimpleIniA ini;
  CSimpleIniA::TNamesDepend sections;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  int lIniVersion = ini.GetLongValue(versionKey, versionSection, 1);
  SKSE::log::info("\tFound old ini file [{}:ini-ver:{}]. Transferring the settings to [{}]. Some settings might have changed!", SettingFile(oldV), lIniVersion, SettingFile());
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
    ini.Delete(cGeneral, "CheckNPCsAfterLoad", true);
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
        ini.Delete(section.pItem, nullptr, true);
      }
    }
    if (ini.KeyExists("Controls", "DAK_Integration")) {
      boolSettings.Set(Common::bsDAK, ini.GetLongValue("Controls", "DAK_Integration") > 1);
      ini.Delete("Controls", "DAK_Integration", true);
    }
    CSimpleIniA::TNamesDepend values;
    ini.GetAllKeys("ExcludedNPCs", values);
    for (const auto &entry : values) {
      auto isExcluded = ini.GetBoolValue("ExcludedNPCs", entry.pItem);
      const auto npcLoc = ut->StrToLoc(std::string(entry.pItem));
      if (isExcluded && !npcLoc.second.empty()) {
        ini.SetValue(cNPCAddonSection, entry.pItem, Common::nulStr.c_str());
      }
      ini.Delete("ExcludedNPCs", entry.pItem, true);
    }
    auto gwChance = static_cast<float>(ini.GetDoubleValue("GentleWomen", "Chance", 20.0));
    floatSettings.Set(Common::fsFemRndChance, gwChance);
  }
  ini.SetLongValue(versionKey, versionSection, iniVersion);
  ini.SaveFile(SettingFile());
  SKSE::log::info("\tThe settings were transferred.");
}

void Inis::SetAddonStatus(const bool isFemale, const RE::TESObjectARMO *addon, const bool status) {
  auto addonLocStr = ut->FormToStr(addon);
  if (addonLocStr.empty()) {
    SKSE::log::critical("Failed to save the status of the addon [0x{:x}]!", addon->GetFormID());
    return;
  }
  status == isFemale ? settingIni.SetBoolValue(isFemale ? cActiveFemAddons : cActiveMalAddons, addonLocStr.c_str(), status)
                     : settingIni.Delete(isFemale ? cActiveFemAddons : cActiveMalAddons, addonLocStr.c_str(), true);
}

void Inis::SetValidSkeleton(const std::string &skeletonModel) {
  if (skeletonModel.empty()) {
    SKSE::log::error("Failed to save the skeleton model name!");
    return;
  }
  validSkeletons.emplace(skeletonModel);
  settingIni.SetBoolValue(cValidSkeletons, skeletonModel.c_str(), true);
}

void Inis::SetRgAddon(const RE::TESRace *rgRace, const RE::TESObjectARMO *addon, const int choice) {
  auto raceRecord = ut->FormToStr(rgRace);
  if (raceRecord.empty()) {
    if (rgRace) {
      SKSE::log::critical("Failed to save the selected addon for race [0x{:x}]!", rgRace->GetFormID());
    } else {
      SKSE::log::critical("Failed to save the selected addon for a race!");
    }
    return;
  }
  switch (choice) {
    case Common::def:
      settingIni.Delete(cRacialAddon, raceRecord.c_str(), true);
      break;
    case Common::nul:
      settingIni.SetValue(cRacialAddon, raceRecord.c_str(), Common::nulStr.c_str());
      break;
    default:
      auto addonRecord = ut->FormToStr(addon);
      if (addonRecord.empty()) {
        if (addon) {
          SKSE::log::critical("Failed to save the addon [0x{:x}] for race [0x{:x}]!", addon->GetFormID(), rgRace->GetFormID());
        } else {
          SKSE::log::critical("Failed to save an addon for a for race [0x{:x}]!", rgRace->GetFormID());
        }
        return;
      }
      settingIni.SetValue(cRacialAddon, raceRecord.c_str(), addonRecord.c_str());
      break;
  }
}

void Inis::SetRgMult(const RE::TESRace *rgRace, const float mult) {
  auto raceRecord = ut->FormToStr(rgRace);
  if (raceRecord.empty()) {
    if (rgRace) {
      SKSE::log::critical("Failed to save the size multiplier for race [0x{:x}]!", rgRace->GetFormID());
    } else {
      SKSE::log::critical("Failed to save the size multiplier a race!");
    }
    return;
  }
  if (mult < 1.0001f && mult > 0.9999f) {
    settingIni.Delete(cRacialSize, raceRecord.c_str(), true);
  } else {
    settingIni.SetDoubleValue(cRacialSize, raceRecord.c_str(), static_cast<double>(mult));
  }
}

SEFormLoc Inis::GetActorAddon(const RE::Actor *actor) const {
  if (!actor || actor->IsPlayerRef()) return {0, ""};
  auto actorLoc = ut->FormToLoc(actor);
  if (!actorLoc.second.empty() && actorAddons.find(actorLoc) != actorAddons.end()) return actorAddons.at(actorLoc);
  return {0, ""};
}

bool Inis::SetNPCAddon(const RE::TESNPC *npc, const RE::TESObjectARMO *addon, const int choice) {
  auto npcRecord = ut->FormToStr(npc);
  return SetAddon(npcRecord, addon, choice, cNPCAddonSection, "NPC");
}

void Inis::SetActorAddon(const RE::Actor *actor, const RE::TESObjectARMO *addon, const int choice) {
  auto actorRecord = ut->FormToStr(actor);
  SetAddon(actorRecord, addon, choice, cActorAddonSection, "actor");
}

int Inis::GetActorSize(const RE::Actor *actor) const {
  if (!actor || actor->IsPlayerRef()) return Common::nul;
  auto actorLoc = ut->FormToLoc(actor);
  if (!actorLoc.second.empty() && actorSizeCats.find(actorLoc) != actorSizeCats.end()) return actorSizeCats.at(actorLoc);
  return Common::nul;
}

bool Inis::SetNPCSize(const RE::TESNPC *npc, int genSize) {
  if (genSize == Common::nul) return true;
  auto npcRecord = ut->FormToStr(npc);
  if (npcRecord.empty()) return false;
  if (genSize == Common::def) {
    settingIni.Delete(cNPCSizeSection, npcRecord.c_str(), true);
  } else {
    settingIni.SetLongValue(cNPCSizeSection, npcRecord.c_str(), genSize);
  }
  return true;
}

void Inis::SetActorSize(const RE::Actor *actor, const int genSize) {
  if (genSize == Common::nul) return;
  auto actorRecord = ut->FormToStr(actor);
  if (actorRecord.empty()) {
    if (actor) {
      SKSE::log::debug("Failed to save the size for actor [0x{:x}].", actor->GetFormID());
    } else {
      SKSE::log::debug("Failed to save the size for an actor.");
    }
    return;
  }
  if (genSize == Common::def) {
    settingIni.Delete(cActorSizeSection, actorRecord.c_str(), true);
  } else {
    settingIni.SetLongValue(cActorSizeSection, actorRecord.c_str(), genSize);
  }
}

void Inis::SetArmorStatus(const RE::TESObjectARMO *armor, const int revMode) {
  auto armoRecord = ut->FormToStr(armor);
  if (armoRecord.empty()) {
    if (armor) {
      SKSE::log::debug("Failed to save the revealing status for armor [0x{:x}].", armor->GetFormID());
    } else {
      SKSE::log::debug("Failed to save the revealing status for an armor.");
    }
    return;
  }
  settingIni.Delete(cRevealingSection, armoRecord.c_str(), true);
  settingIni.Delete(cFemRevRecordSection, armoRecord.c_str(), true);
  settingIni.Delete(cMalRevRecordSection, armoRecord.c_str(), true);
  switch (revMode) {
    case Common::kyCovering:
      settingIni.SetBoolValue(cRevealingSection, armoRecord.c_str(), false);
      break;
    case Common::kyRevealing:
      settingIni.SetBoolValue(cRevealingSection, armoRecord.c_str(), true);
      break;
    case Common::kyRevealingF:
      settingIni.SetBoolValue(cFemRevRecordSection, armoRecord.c_str(), true);
      break;
    case Common::kyRevealingM:
      settingIni.SetBoolValue(cMalRevRecordSection, armoRecord.c_str(), true);
      break;
    default:
      settingIni.SetBoolValue(cRevealingSection, armoRecord.c_str(), true);
      break;
  }
}

void Inis::Process52(const std::string modName) {
  slot52Mods.push_back(modName);
  if (boolSettings.Get(Common::bsRevealSlot52Mods)) {
    extraRevealingMods.insert(modName);
    Slot52ModBehavior(modName, 1);
  }
}

bool Inis::IsExtraRevealing(const std::string &modName) const {
  if (modName == "") return false;
  if (extraRevealingMods.find(modName) != extraRevealingMods.end()) return true;
  return false;
}

bool Inis::IsExtraRevealing(const RE::TESObjectARMO *armor) const {
  if (!armor || !armor->HasPartOf(Common::bodySlot)) return false;
  auto armoLoc = ut->FormToLoc(armor);
  return IsExtraRevealing(armoLoc.second);
}

bool Inis::SetAddon(const std::string &record, const RE::TESObjectARMO *addon, const int choice, const char *section, const std::string &formType) {
  if (record.empty()) {
    if (addon) {
      SKSE::log::critical("Failed to save the selected addon for {} [{}]!", formType, record);
    } else {
      SKSE::log::critical("Failed to save the selected addon for an unknown {}!", formType);
    }
    return false;
  }
  switch (choice) {
    case Common::def:
      settingIni.Delete(section, record.c_str(), true);
      break;
    case Common::nul:
      settingIni.SetValue(section, record.c_str(), Common::nulStr.c_str());
      break;
    default:
      auto addonRecord = ut->FormToStr(addon);
      if (addonRecord.empty()) {
        if (addon) {
          SKSE::log::critical("Failed to save the addon [0x{:x}] for {} [{}]!", addon->GetFormID(), formType, record);
        } else {
          SKSE::log::critical("Failed to save an addon for {} [{}]!", formType, record);
        }
        return false;
      }
      settingIni.SetValue(section, record.c_str(), addonRecord.c_str());
      break;
  }
  return true;
}

void Inis::LoadPlayerInfos(const std::string &saveName) {
  playerIdx = "";
  if (const auto save = ut->Split(saveName, "_"); save.size() == 9) playerIdx = save[1];
  auto section = fmt::format("{}{}", cPlayerSection, playerIdx).c_str();
  if (settingIni.SectionExists(section)) {
    CSimpleIniA::TNamesDepend keys;
    settingIni.GetAllKeys(section, keys);
    for (auto &key : keys) {
      std::string reqInfoStr{key.pItem};
      auto value = settingIni.GetValue(section, key.pItem);
      std::vector<std::string> pcIdTokens = ut->Split(reqInfoStr, "|");
      std::vector<std::string> pcInfoTokens = ut->Split(value, "|");
      if (pcIdTokens.size() == 3 && pcInfoTokens.size() == 2) {
        playerInfos.push_back({});
        auto &pcInfo = playerInfos.back();
        pcInfo.name = pcIdTokens[0];
        pcInfo.race = ut->StrToLoc(pcIdTokens[1]);
        pcInfo.isFemale = pcIdTokens[2] == "F";
        if (pcInfo.name.empty() || pcInfo.race.second.empty()) continue;
        pcInfo.addon = ut->StrToLoc(pcInfoTokens[0]);
        pcInfo.sizeCat = std::stoi(pcInfoTokens[1]);
        if (pcInfo.addon.second.empty()) continue;
        SKSE::log::debug("\tLoaded player info for active save: name[{}], race [{}], Female [{}], addon [{}], sizeCat [{}]", pcIdTokens[0], pcIdTokens[1], pcIdTokens[2],
                         pcInfoTokens[0], pcInfoTokens[1]);
      };
    }
  }
}

bool Inis::Slot52ModBehavior(const std::string &modName, const int behavior) {
  switch (behavior) {
    case 1:
      settingIni.SetBoolValue(cRevealingModSection, ut->NameToStr(modName).c_str(), true);
      extraRevealingMods.insert(modName);
      return true;
    case 0:
      settingIni.Delete(cRevealingModSection, ut->NameToStr(modName).c_str(), true);
      extraRevealingMods.erase(modName);
      return false;
    default:
      CSimpleIniA::TNamesDepend values;
      settingIni.GetAllKeys(cRevealingModSection, values);
      return std::ranges::find_if(values, [&](const auto &entry) { return ut->StrToName(entry.pItem) == modName; }) != values.end();
  }
}

const Common::PlayerInfo *Inis::GetPlayerInfo(const RE::Actor *actor) {
  auto pcIdTokens = UpdateActivePlayerInfoIndex(actor);
  if (pcIdTokens.empty()) return nullptr;
  if (activePlayerInfoIdx >= 0 && activePlayerInfoIdx < playerInfos.size()) {
    auto res = &playerInfos[activePlayerInfoIdx];
    return res;
  }
  return nullptr;
}

void Inis::SetPlayerInfo(const RE::Actor *actor, const RE::TESObjectARMO *addon, const int choice, const int sizeCatInp) {
  auto pcIdTokens = UpdateActivePlayerInfoIndex(actor);
  if (pcIdTokens.empty()) return;
  if (choice == Common::nan && sizeCatInp == Common::nan) return;
  SEFormLoc addonLoc{0, Common::defStr};
  switch (choice) {
    case Common::nan:
      if (activePlayerInfoIdx >= 0 && activePlayerInfoIdx < playerInfos.size()) addonLoc = playerInfos[activePlayerInfoIdx].addon;
      break;
    case Common::nul:
      addonLoc = {0, Common::nulStr};
      break;
    default:
      addonLoc = ut->FormToLoc(addon);
      break;
  }
  if (addonLoc.second.empty()) {
    if (addon) {
      SKSE::log::critical("Failed to save the addon [0x{:x}] for player!", addon->GetFormID());
    } else {
      SKSE::log::critical("Failed to save an addon for a for player!");
    }
    return;
  }
  int sizeCat = sizeCatInp == Common::nan ? Common::def : sizeCatInp;
  if (sizeCatInp == Common::nan && activePlayerInfoIdx >= 0 && activePlayerInfoIdx < playerInfos.size()) sizeCat = playerInfos[activePlayerInfoIdx].sizeCat;
  std::vector<std::string> pcInfoTokens;
  pcInfoTokens.push_back(ut->LocToStr(addonLoc));
  pcInfoTokens.push_back(std::to_string(sizeCat));
  auto section = fmt::format("{}{}", cPlayerSection, playerIdx).c_str();
  auto key = ut->Join(pcIdTokens, "|").c_str();
  auto value = ut->Join(pcInfoTokens, "|").c_str();
  settingIni.SetValue(section, key, value);
  if (activePlayerInfoIdx >= 0 && activePlayerInfoIdx < playerInfos.size()) {
    playerInfos[activePlayerInfoIdx].addon = addonLoc;
    playerInfos[activePlayerInfoIdx].sizeCat = sizeCat;
  } else {
    playerInfos.push_back({});
    auto &pcInfo = playerInfos.back();
    pcInfo.name = pcIdTokens[0];
    pcInfo.race = ut->StrToLoc(pcIdTokens[1]);
    pcInfo.isFemale = pcIdTokens[2] == "F";
    pcInfo.addon = addonLoc;
    pcInfo.sizeCat = sizeCat;
    activePlayerInfoIdx = static_cast<int>(playerInfos.size()) - 1;
  }
}

std::vector<std::string> Inis::UpdateActivePlayerInfoIndex(const RE::Actor *actor) {
  std::vector<std::string> pcIdTokens;
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race) return {};
  pcIdTokens.push_back(std::string(npc->GetName()));
  pcIdTokens.push_back(ut->FormToStr(npc->race));
  if (pcIdTokens[0].empty() || pcIdTokens[1].empty()) {
    SKSE::log::critical("Failed to update the player information for current character.");
    return {};
  }
  pcIdTokens.push_back(npc->IsFemale() ? "F" : "M");
  if (playerInfos.empty()) {
    activePlayerInfoIdx = -1;
    return pcIdTokens;
  }
  auto infoIdx = std::find_if(playerInfos.begin(), playerInfos.end(),
                              [&](const auto &pcInfo) { return pcInfo.name == pcIdTokens[0] && ut->LocToStr(pcInfo.race) == pcIdTokens[1] && pcInfo.isFemale == npc->IsFemale(); });
  if (infoIdx != playerInfos.end()) {
    activePlayerInfoIdx = static_cast<int>(std::distance(playerInfos.begin(), infoIdx));
  } else {
    activePlayerInfoIdx = -1;
  }
  return pcIdTokens;
}

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
    if (ini.GetAllValues(cExcludeSection, cExcModRaces, values)) LoadModNames(values, excludedRaceMods, cExcModRaces, fileName);
    if (ini.GetAllValues(cExcludeSection, cExcRace, values)) LoadModRecordPairs(values, excludedRaces, cExcRace, fileName);
    if (ini.GetAllValues(cExcludeSection, cExcludeNPC, values)) LoadModRecordPairs(values, excludedNPCs, cExcludeNPC, fileName);
  }
  if (ini.SectionExists(cSkinSection)) {
    if (ini.GetAllValues(cSkinSection, cSkinMod, values)) LoadModNames(values, skinMods, cSkinMod, fileName);
    if (ini.GetAllValues(cSkinSection, cSkinRecord, values)) LoadModRecordPairs(values, skinRecords, cSkinRecord, fileName);
  }
  if (ini.SectionExists(cArmorSection)) {
    if (ini.GetAllValues(cArmorSection, cRevealingMod, values)) LoadModNames(values, revealingMods, cRevealingMod, fileName);
    if (ini.GetAllValues(cArmorSection, cFemRevMod, values)) LoadModNames(values, femRevMods, cFemRevMod, fileName);
    if (ini.GetAllValues(cArmorSection, cMalRevMod, values)) LoadModNames(values, malRevMods, cMalRevMod, fileName);
    if (ini.GetAllValues(cArmorSection, cCoveringRecord, values)) LoadModRecordPairs(values, coveringRecords, cCoveringRecord, fileName);
    if (ini.GetAllValues(cArmorSection, cRevealingRecord, values)) LoadModRecordPairs(values, revealingRecords, cRevealingRecord, fileName);
    if (ini.GetAllValues(cArmorSection, cFemRevRecord, values)) LoadModRecordPairs(values, femRevRecords, cFemRevRecord, fileName);
    if (ini.GetAllValues(cArmorSection, cMalRevRecord, values)) LoadModRecordPairs(values, malRevRecords, cMalRevRecord, fileName);
  }
}

bool Inis::IsRaceExcluded(const RE::TESRace *race) const {
  if (!race->GetFile(0)) return false;
  std::string modName{race->GetFile(0)->GetFilename()};
  if (excludedRaceMods.find(modName) != excludedRaceMods.end()) return true;
  if (excludedRaces.find(ut->FormToLoc(race)) != excludedRaces.end()) return true;
  return false;
}

bool Inis::IsNPCExcluded(const RE::TESNPC *npc) const {
  if (!npc || !npc->GetFile(0)) return false;
  if (excludedNPCs.find(ut->FormToLoc(npc)) != excludedNPCs.end()) return true;
  return false;
}

bool Inis::IsSkin(const RE::TESObjectARMO *armor, const std::string &modName) {
  if (modName == "") return false;
  if (skinMods.find(modName) != skinMods.end()) return true;
  if (skinRecords.find({armor->GetLocalFormID(), modName}) != skinRecords.end()) return true;
  return false;
}

Common::eKeyword Inis::HasStatus(const RE::TESObjectARMO *armor) const {
  auto armorLoc = ut->FormToLoc(armor);
  auto isBody = armor->HasPartOf(Common::bodySlot);
  if (armorLoc.second.empty()) return Common::keywordsCount;
  if (runTimeArmorRecords.find(armorLoc) != runTimeArmorRecords.end()) return runTimeArmorRecords.at(armorLoc) ? Common::kyRevealing : Common::kyCovering;
  if (isBody && runTimeFemRevRecords.find(armorLoc) != runTimeFemRevRecords.end()) return Common::kyRevealingF;
  if (isBody && runTimeMalRevRecords.find(armorLoc) != runTimeMalRevRecords.end()) return Common::kyRevealingM;
  if (coveringRecords.find(armorLoc) != coveringRecords.end()) return Common::kyCovering;
  if (!isBody) return Common::kyCovering;
  if (revealingRecords.find(armorLoc) != revealingRecords.end()) return Common::kyRevealing;
  if (femRevRecords.find(armorLoc) != femRevRecords.end()) return Common::kyRevealingF;
  if (malRevRecords.find(armorLoc) != malRevRecords.end()) return Common::kyRevealingM;
  if (revealingMods.find(armorLoc.second) != revealingMods.end()) return Common::kyRevealing;
  if (femRevMods.find(armorLoc.second) != femRevMods.end()) return Common::kyRevealingF;
  if (malRevMods.find(armorLoc.second) != malRevMods.end()) return Common::kyRevealingM;
  if (armor->HasKeywordString(Common::sosRevealing)) return Common::kyRevealing;
  return Common::keywordsCount;
}

void Inis::ClearInis() {
  // Clearing the maps and sets to free memory, NOTE: private members need to remain alive
  activeMalAddons.clear();
  activeFemAddons.clear();
  racialAddons.clear();
  racialSizes.clear();
  npcAddons.clear();
  npcSizeCats.clear();
  runTimeArmorRecords.clear();
  runTimeFemRevRecords.clear();
  runTimeMalRevRecords.clear();
  // actorAddons|actorSizeCats|slot52Mods|extraRevealingMods should not be cleared during lifetime of the game

  excludedRaceMods.clear();
  excludedRaces.clear();
  skinMods.clear();
  skinRecords.clear();
  revealingMods.clear();
  femRevMods.clear();
  malRevMods.clear();
  coveringRecords.clear();
  revealingRecords.clear();
  femRevRecords.clear();
  malRevRecords.clear();
  // excludedNPCs should not be cleared during lifetime of the game
}

void Inis::LoadModNames(const CSimpleIniA::TNamesDepend &records, std::set<std::string> &fieldToFill, std::string_view entryType, std::string_view fileName) {
  SKSE::log::info("\t- Found [{}] [{}] entries in [{}].", records.size(), entryType, fileName);
  for (const auto &entry : records) {
    const std::string modName(ut->StrToName(entry.pItem));
    fieldToFill.insert(modName);
  }
}

void Inis::LoadModRecordPairs(const CSimpleIniA::TNamesDepend &records, std::set<SEFormLoc> &fieldToFill, std::string_view entryType, std::string_view fileName) {
  SKSE::log::info("\t- Found [{}] [{}] entries in [{}].", records.size(), entryType, fileName);
  for (const auto &entry : records) {
    const std::string modRecord(entry.pItem);
    fieldToFill.insert(ut->StrToLoc(modRecord));
  }
}
