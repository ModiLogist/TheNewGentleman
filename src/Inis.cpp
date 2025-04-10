#include <Inis.h>

void Inis::LoadMainIni() {
  SKSE::log::info("Loading TNG settings...");
  if (!std::filesystem::exists(SettingFile())) {
    std::ofstream newSetting(SettingFile());
    newSetting << ";TNG Settings File" << std::endl;
    newSetting.close();
    TransferOldIni();
  }
  CSimpleIniA settingIni;
  CSimpleIniA::TNamesDepend keys;
  settingIni.SetUnicode();
  settingIni.LoadFile(SettingFile());
  boolSettings.Load(settingIni);
  intSettings.Load(settingIni);
  floatSettings.Load(settingIni);
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
  LoadIniPairs<bool>(settingIni, cActiveMalAddons, activeMalAddons, true);
  LoadIniPairs<bool>(settingIni, cActiveFemAddons, activeFemAddons, false);
  SKSE::log::debug("\tRestored all addon status to previous selections");
  settingIni.GetAllKeys(cValidSkeletons, keys);
  for (auto &key : keys) validSkeletons.emplace(std::string(key.pItem));
  LoadIniPairs<SEFormLoc>(settingIni, cRacialAddon, racialAddons);
  LoadIniPairs<float>(settingIni, cRacialSize, racialSizes);
  SKSE::log::debug("\tRestored all racial addon and size settings");
  LoadIniPairs<SEFormLoc>(settingIni, cNPCAddonSection, npcAddons);
  LoadIniPairs<int>(settingIni, cNPCSizeSection, npcSizeCats);
  LoadIniPairs<SEFormLoc>(settingIni, cActorAddonSection, actorAddons);
  LoadIniPairs<int>(settingIni, cActorSizeSection, actorSizeCats);
  SKSE::log::debug("\tRestored all NPC and actor addon and size settings");
  LoadIniPairs<int>(settingIni, cArmorStatusSection, runTimeArmorStatus);
  SKSE::log::debug("\tRestored all revealing records settings");
  settingIni.GetAllKeys(cRevealingModSection, keys);
  for (auto &key : keys) {
    auto modName = ut->StrToName(std::string(key.pItem));
    auto status = settingIni.GetBoolValue(cRevealingModSection, key.pItem, false);
    if (!modName.empty()) slot52Mods.emplace(modName, status);
  }
  SKSE::log::info("TNG settings loaded.");
}

void Inis::SaveMainIni() {
  CSimpleIniA settingIni;
  settingIni.SetUnicode();
  settingIni.LoadFile(SettingFile());
  boolSettings.Store(settingIni);
  intSettings.Store(settingIni);
  floatSettings.Store(settingIni);
  SaveIniPairs<bool>(settingIni, cActiveMalAddons, activeMalAddons, true);
  SaveIniPairs<bool>(settingIni, cActiveFemAddons, activeFemAddons);
  SKSE::log::debug("\tStored all addon status to previous selections");
  for (auto &skeleton : validSkeletons) settingIni.SetBoolValue(cValidSkeletons, skeleton.c_str(), true);
  SaveIniPairs<SEFormLoc>(settingIni, cRacialAddon, racialAddons);
  SaveIniPairs<float>(settingIni, cRacialSize, racialSizes);
  SKSE::log::debug("\tStored all racial addon and size settings");
  SaveIniPairs<SEFormLoc>(settingIni, cNPCAddonSection, npcAddons);
  SaveIniPairs<int>(settingIni, cNPCSizeSection, npcSizeCats);
  SaveIniPairs<SEFormLoc>(settingIni, cActorAddonSection, actorAddons);
  SaveIniPairs<int>(settingIni, cActorSizeSection, actorSizeCats);
  SKSE::log::debug("\tStored all NPC and actor addon and size settings");
  SaveIniPairs<int>(settingIni, cArmorStatusSection, runTimeArmorStatus);
  SKSE::log::debug("\tStored all armor records status settings");
  auto playerIdx = RE::BGSSaveLoadManager::GetSingleton()->currentCharacterID & 0xFFFFFFFF;
  auto section = fmt::format("{}{:X}", cPlayerSection, playerIdx).c_str();
  for (auto &pcInfo : playerInfos) {
    auto key = pcInfo.IdStr().c_str();
    auto value = pcInfo.InfoStr().c_str();
    SKSE::log::debug("Saving player info: section [{}], key [{}], value [{}]", section, key, value);
    settingIni.SetValue(section, key, value);
  }
  for (auto &pair : slot52Mods) {
    if (!pair.first.empty()) settingIni.SetBoolValue(cRevealingModSection, pair.first.c_str(), pair.second);
  }
  settingIni.SaveFile(SettingFile());
}

spdlog::level::level_enum Inis::GetLogLvl() const {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  auto lvl = ini.GetLongValue(cGeneral, cLogLvl, static_cast<int>(spdlog::level::info));
  return lvl > 0 && lvl < static_cast<int>(spdlog::level::n_levels) ? static_cast<spdlog::level::level_enum>(lvl) : spdlog::level::info;
}

void Inis::SetLogLvl(const int newLevel) {
  CSimpleIniA ini;
  ini.SetUnicode();
  ini.LoadFile(SettingFile());
  if (newLevel < 1 || newLevel >= static_cast<int>(spdlog::level::n_levels)) return;
  auto logLevel = static_cast<spdlog::level::level_enum>(newLevel);
  if (logLevel == spdlog::level::info) {
    ini.Delete(cGeneral, cLogLvl, true);
  } else {
    ini.SetLongValue(cGeneral, cLogLvl, logLevel);
  }
  ini.SaveFile(SettingFile());
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
  auto addonLoc = ut->FormToLoc(addon);
  if (addonLoc.second.empty()) {
    SKSE::log::critical("Failed to save the status of the addon [0x{:x}]!", addon->GetFormID());
    return;
  }
  status == isFemale ? activeFemAddons[addonLoc] = status : activeMalAddons[addonLoc] = status;
}

void Inis::SetValidSkeleton(const std::string &skeletonModel) { validSkeletons.emplace(skeletonModel); }

void Inis::SetRgAddon(const RE::TESRace *rgRace, const RE::TESObjectARMO *addon, const int choice) {
  auto raceLoc = ut->FormToLoc(rgRace);
  if (raceLoc.second.empty()) {
    if (rgRace) {
      SKSE::log::critical("Failed to save the selected addon for race [0x{:x}]!", rgRace->GetFormID());
    } else {
      SKSE::log::critical("Failed to save the selected addon for a race!");
    }
    return;
  }
  switch (choice) {
    case Common::def:
      racialAddons[raceLoc] = GetDefault<SEFormLoc>();
      break;
    case Common::nul:
      racialAddons[raceLoc] = {0, ""};
      break;
    default: {
      auto addonLoc = ut->FormToLoc(addon);
      if (addonLoc.second.empty()) {
        if (addon) {
          SKSE::log::critical("Failed to save the addon [0x{:x}] for race [0x{:x}]!", addon->GetFormID(), rgRace->GetFormID());
        } else {
          SKSE::log::critical("Failed to save an addon for a for race [0x{:x}]!", rgRace->GetFormID());
        }
        return;
      }
      racialAddons[raceLoc] = addonLoc;
    } break;
  }
}

void Inis::SetRgMult(const RE::TESRace *rgRace, const float mult) {
  auto raceLoc = ut->FormToLoc(rgRace);
  if (raceLoc.second.empty()) {
    if (rgRace) {
      SKSE::log::critical("Failed to save the size multiplier for race [0x{:x}]!", rgRace->GetFormID());
    } else {
      SKSE::log::critical("Failed to save the size multiplier a race!");
    }
    return;
  }
  if (mult < 1.0001f && mult > 0.9999f) {
    racialSizes[raceLoc] = GetDefault<float>();
  } else {
    racialSizes[raceLoc] = mult;
  }
}

SEFormLoc Inis::GetActorAddon(const RE::Actor *actor) const {
  if (!actor || actor->IsPlayerRef()) return {0, ""};
  auto actorLoc = ut->FormToLoc(actor);
  if (!actorLoc.second.empty() && actorAddons.find(actorLoc) != actorAddons.end()) return actorAddons.at(actorLoc);
  return {0, ""};
}

bool Inis::SetNPCAddon(const RE::TESNPC *npc, const RE::TESObjectARMO *addon, const int choice) {
  auto npcLoc = ut->FormToLoc(npc);
  if (npcLoc.second.empty()) return false;
  auto addonLoc = ut->FormToLoc(addon, choice);
  if (addonLoc.second.empty()) {
    if (addon) {
      SKSE::log::critical("Failed to save the addon [{}] for npc [0x{:x}~{}]!", addon->GetFormID(), npcLoc.first, npcLoc.second);
    } else {
      SKSE::log::critical("Failed to save an addon for npc [0x{:x}~{}]!", npcLoc.first, npcLoc.second);
    }
    return false;
  }
  npcAddons[npcLoc] = addonLoc;
  return true;
}

void Inis::SetActorAddon(const RE::Actor *actor, const RE::TESObjectARMO *addon, const int choice) {
  auto actorLoc = ut->FormToLoc(actor);
  auto addonLoc = ut->FormToLoc(addon, choice);
  if (actorLoc.second.empty()) {
    SKSE::log::critical("Failed to save the selected addon for actor [0x{:x}]!", actor->GetFormID());
    return;
  }
  if (addonLoc.second.empty()) {
    if (addon) {
      SKSE::log::debug("Failed to save the addon [0x{:x}] for actor [0x{:x}~{}]!", addon->GetFormID(), actorLoc.first, actorLoc.second);
    } else {
      SKSE::log::debug("Failed to save an addon for actor [0x{:x}~{}]!", actorLoc.first, actorLoc.second);
    }
    return;
  }
  actorAddons[actorLoc] = addonLoc;
}

int Inis::GetActorSize(const RE::Actor *actor) const {
  if (!actor || actor->IsPlayerRef()) return Common::nul;
  auto actorLoc = ut->FormToLoc(actor);
  if (!actorLoc.second.empty() && actorSizeCats.find(actorLoc) != actorSizeCats.end()) return actorSizeCats.at(actorLoc);
  return Common::nul;
}

bool Inis::SetNPCSize(const RE::TESNPC *npc, int genSize) {
  if (genSize == Common::nul) return true;
  auto npcLoc = ut->FormToLoc(npc);
  if (npcLoc.second.empty()) return false;
  npcSizeCats[npcLoc] = genSize == Common::def ? GetDefault<int>() : genSize;
  return true;
}

void Inis::SetActorSize(const RE::Actor *actor, const int genSize) {
  if (genSize == Common::nul) return;
  auto actorLoc = ut->FormToLoc(actor);
  if (actorLoc.second.empty()) {
    if (actor) {
      SKSE::log::debug("Failed to save the size for actor [0x{:x}].", actor->GetFormID());
    } else {
      SKSE::log::debug("Failed to save the size for an actor.");
    }
    return;
  }
  actorSizeCats[actorLoc] = genSize == Common::def ? GetDefault<int>() : genSize;
}

void Inis::SetArmorStatus(const RE::TESObjectARMO *armor, const Common::eKeyword revMode) {
  auto armoLoc = ut->FormToLoc(armor);
  if (armoLoc.second.empty()) {
    if (armor) {
      SKSE::log::debug("Failed to save the revealing status for armor [0x{:x}].", armor->GetFormID());
    } else {
      SKSE::log::debug("Failed to save the revealing status for an armor.");
    }
    return;
  }
  auto revIdx = std::ranges::find(statusKeys, revMode);
  if (revIdx == statusKeys.end()) {
    runTimeArmorStatus[armoLoc] = Common::nan;
  } else {
    runTimeArmorStatus[armoLoc] = static_cast<int>(std::distance(statusKeys.begin(), revIdx));
  }
}

void Inis::LoadPlayerInfos(const std::string &saveName) {
  playerInfos.clear();
  std::string playerStr = "";
  if (const auto save = ut->Split(saveName, "_"); save.size() == 9) playerStr = save[1];
  CSimpleIniA settingIni;
  settingIni.SetUnicode();
  settingIni.LoadFile(SettingFile());
  auto section = fmt::format("{}{}", cPlayerSection, playerStr).c_str();
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

bool Inis::Slot52ModBehavior(const std::string &modName) const { return slot52Mods.at(modName); }

bool Inis::Slot52ModBehavior(const std::string &modName, const int behavior) {
  if (behavior >= 0) slot52Mods[modName] = behavior == 1;
  return slot52Mods[modName];
}

const std::vector<std::string> Inis::Slot52Mods() const {
  std::vector<std::string> mods;
  for (auto &pair : slot52Mods) {
    if (!pair.first.empty()) mods.push_back(pair.first);
  }
  return mods;
}

Common::PlayerInfo *Inis::GetPlayerInfo(const RE::Actor *actor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race) return {};
  std::tuple<const std::string, const SEFormLoc, const bool> pcId{npc->GetName(), ut->FormToLoc(npc->race), npc->IsFemale()};
  if ((std::get<0>(pcId)).empty() || (std::get<0>(pcId)).empty()) {
    SKSE::log::critical("Failed to update the player information for current character.");
    return nullptr;
  }
  auto infoIdx = std::find_if(playerInfos.begin(), playerInfos.end(), [&pcId](const auto &pcInfo) { return pcInfo.Id() == pcId; });
  if (infoIdx != playerInfos.end()) {
    return &(*infoIdx);
  } else {
    playerInfos.push_back({});
    auto &pcInfo = playerInfos.back();
    pcInfo.name = std::get<0>(pcId);
    pcInfo.race = std::get<1>(pcId);
    pcInfo.isFemale = std::get<2>(pcId);
    return &pcInfo;
  }
}

void Inis::SetPlayerInfo(const RE::Actor *actor, const RE::TESObjectARMO *addon, const int addonChoice, const int sizeChoice) {
  auto pcInfo = GetPlayerInfo(actor);
  if (!pcInfo) return;
  if (addonChoice == Common::nan && sizeChoice == Common::nan) return;
  SEFormLoc addonLoc{0, Common::defStr};
  switch (addonChoice) {
    case Common::nan:
      addonLoc = pcInfo->addon;
      break;
    case Common::nul:
      addonLoc = {0, Common::nulStr};
      break;
    case Common::def:
      break;
    default:
      addonLoc = ut->FormToLoc(addon);
      break;
  }
  if (addonLoc.second.empty()) {
    return;
  }
  int sizeCat = sizeChoice == Common::nan ? pcInfo->sizeCat : sizeChoice;
  if (addonLoc != pcInfo->addon || sizeCat != pcInfo->sizeCat) {
    pcInfo->addon = addonLoc;
    pcInfo->sizeCat = sizeCat;
  }
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
  if (runTimeArmorStatus.find(armorLoc) != runTimeArmorStatus.end()) return statusKeys[static_cast<size_t>(runTimeArmorStatus.at(armorLoc))];
  if (coveringRecords.find(armorLoc) != coveringRecords.end()) return Common::kyCovering;
  if (!isBody) return Common::keywordsCount;
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
  runTimeArmorStatus.clear();
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
