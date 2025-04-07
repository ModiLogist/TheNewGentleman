#pragma once

#include <Util.h>

class Inis {
  public:
    void LoadMainIni();
    void SaveMainIni() { settingIni.SaveFile(SettingFile()); };
    virtual ~Inis() = default;

    Common::TypedSetting<bool, Common::eBoolSetting> boolSettings{
        settingIni, Common::boolSettingCount, std::vector<bool>(Common::boolSettingCount, false),
        std::vector<const char*>{"General", "General", "General", "General", "General", "General", "General", "General", "General", "Controls"},
        std::vector<const char*>{"ExcludePlayerSize", "CheckPlayerRegularly", "ForceChecks", "Slot52ModsAreRevealing", "Slot52ModsAreMixed", "RandomizeMaleAddons", "UIExtensions",
                                 "ShowAllRaces", "DAK_Integration"}};

    Common::TypedSetting<int, Common::eIntSetting> intSettings{
        settingIni, Common::intSettingCount, std::vector<int>{Common::nul, Common::nul, Common::nul, Common::nul, Common::nul, Common::nul},
        std::vector<const char*>(Common::intSettingCount, "Controls"), std::vector<const char*>{"NPCEdit", "GenitalUp", "GenitalDown", "Revealing", "WhyProblem"}};

    Common::TypedSetting<float, Common::eFloatSetting> floatSettings{
        settingIni, Common::floatSettingCount, std::vector<float>{0.8, 0.9, 1.0, 1.2, 1.4, 20.0},
        std::vector<const char*>{"GlobalSizes", "GlobalSizes", "GlobalSizes", "GlobalSizes", "GlobalSizes", "GentleWomen"},
        std::vector<const char*>{"Size_XS", "Size__S", "Size__M", "Size__L", "Size_XL", "Chance"}};

    spdlog::level::level_enum GetLogLvl() const {
      auto lvl = settingIni.GetLongValue(cGeneral, cLogLvl, static_cast<int>(spdlog::level::info));
      return lvl > 0 && lvl < static_cast<int>(spdlog::level::n_levels) ? static_cast<spdlog::level::level_enum>(lvl) : spdlog::level::info;
    };
    void SetLogLvl(const int newLevel) {
      if (newLevel < 1 || newLevel >= static_cast<int>(spdlog::level::n_levels)) return;
      auto logLevel = static_cast<spdlog::level::level_enum>(newLevel);
      if (logLevel == spdlog::level::info) {
        settingIni.Delete(cGeneral, cLogLvl, true);
      } else {
        settingIni.SetLongValue(cGeneral, cLogLvl, logLevel);
      }
    };

  private:
    inline static constexpr int iniVersion = 5;
    inline static constexpr const char* cSettings{R"(.\Data\SKSE\Plugins\TheNewGentleman{}.ini)"};
    inline static constexpr const char* cTngIniEnding{"TNG.ini"};
    inline static constexpr const char* cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};

    inline static constexpr const char* versionKey{"IniVersion"};
    inline static constexpr const char* versionSection{"Version"};

    inline static constexpr const char* cGeneral{"General"};
    inline static constexpr const char* cLogLvl{"LoggingLevel"};

    inline static constexpr const char* cActiveMalAddons{"ActiveMaleAddons"};
    inline static constexpr const char* cActiveFemAddons{"ActiveFemaleAddons"};

    inline static constexpr const char* cValidSkeletons{"ValidSkeletons"};
    inline static constexpr const char* cRacialAddon{"RaceGenital"};
    inline static constexpr const char* cRacialSize{"RaceSizeMultiplier"};

    inline static constexpr const char* cNPCAddonSection{"NPCGenitalAddon"};
    inline static constexpr const char* cNPCSizeSection{"NPCGenitalSize"};
    inline static constexpr const char* cActorAddonSection{"ActorGenitalAddon"};
    inline static constexpr const char* cActorSizeSection{"ActorGenitalSize"};

    inline static constexpr const char* cRevealingModSection{"RevealingMod"};
    inline static constexpr const char* cRevealingSection{"RevealingRecord"};
    inline static constexpr const char* cFemRevRecordSection{"FemaleRevealingRecord"};
    inline static constexpr const char* cMalRevRecordSection{"MaleRevealingRecord"};

    CSimpleIniA settingIni;
    const char* SettingFile(const int version = iniVersion) const;
    void TransferOldIni();

  protected:
    std::map<SEFormLoc, bool> activeMalAddons;
    std::map<SEFormLoc, bool> activeFemAddons;
    std::set<std::string> validSkeletons;
    std::map<SEFormLoc, SEFormLoc> racialAddons;
    std::map<SEFormLoc, float> racialSizes;
    std::map<SEFormLoc, SEFormLoc> npcAddons;
    std::map<SEFormLoc, int> npcSizeCats;
    std::map<SEFormLoc, bool> runTimeArmorRecords;
    std::map<SEFormLoc, bool> runTimeMalRevRecords;
    std::map<SEFormLoc, bool> runTimeFemRevRecords;

  public:
    bool Slot52ModBehavior(const std::string& modName, const int behavior);

  protected:
    void SetAddonStatus(const bool isFemale, const RE::TESObjectARMO* addon, const bool status);

    void SetValidSkeleton(const std::string& skeletonModel) { settingIni.SetBoolValue(cValidSkeletons, skeletonModel.c_str(), true); };
    void SetRgAddon(const RE::TESRace* rg0, const RE::TESObjectARMO* addon, const int choice);
    void SetRgMult(const RE::TESRace* rgRace, const float mult);

    SEFormLoc GetActorAddon(const RE::Actor* actor) const;
    bool SetNPCAddon(const RE::TESNPC* npc, const RE::TESObjectARMO* addon, const int choice);
    void SetActorAddon(const RE::Actor* actor, const RE::TESObjectARMO* addon, const int choice);

    int GetActorSize(const RE::Actor* actor) const;
    bool SetNPCSize(const RE::TESNPC* npc, int genSize);
    void SetActorSize(const RE::Actor* actor, const int genSize);

    void SetArmorStatus(const RE::TESObjectARMO* armor, const int revMode);
    void Process52(const std::string modName);
    bool IsExtraRevealing(const std::string& modName) const;
    bool IsExtraRevealing(const RE::TESObjectARMO* armor) const;

  private:
    std::map<SEFormLoc, SEFormLoc> actorAddons;
    std::map<SEFormLoc, int> actorSizeCats;
    std::set<std::string> slot52Mods;
    std::set<std::string> extraRevealingMods;
    bool SetAddon(const std::string& record, const RE::TESObjectARMO* addon, const int choice, const char* section, const std::string& formType);

  public:
    void LoadPlayerInfos(const std::string& saveName);
    const Common::PlayerInfo* GetPlayerInfo(const RE::Actor* actor);
    void SetPlayerInfo(const RE::Actor* actor, const RE::TESObjectARMO* addon, const int choice = Common::nan, const int sizeCatInp = Common::nan);
    std::vector<std::string> Slot52Mods() { return std::vector<std::string>(slot52Mods.begin(), slot52Mods.end()); };

  private:
    inline static constexpr const char* cPlayerSection{"PlayerInfo"};
    inline static constexpr const char* cPlayerName{"Name"};
    inline static constexpr const char* cPlayerGender{"Gender"};
    inline static constexpr const char* cPlayerRace{"Race"};
    inline static constexpr const char* cPlayerAddon{"Addon"};
    inline static constexpr const char* cPlayerSkin{"Skin"};
    inline static constexpr const char* cPlayerSize{"Size"};
    std::vector<Common::PlayerInfo> playerInfos;
    int activePlayerInfoIdx{Common::nul};
    std::string playerIdx;

  public:
    void LoadTngInis();

  private:
    inline static constexpr const char* cExcludeSection{"Exclusions"};
    inline static constexpr const char* cExcModRaces{"ExcludeRacesInMod"};
    inline static constexpr const char* cExcRace{"ExcludeRace"};
    inline static constexpr const char* cExcludeNPC{"ExcludeNPC"};

    inline static constexpr const char* cSkinSection{"Skin"};
    inline static constexpr const char* cSkinMod{"SkinMod"};
    inline static constexpr const char* cSkinRecord{"SkinRecord"};

    inline static constexpr const char* cArmorSection{"Armor"};
    inline static constexpr const char* cCoveringRecord{"CoveringRecord"};
    inline static constexpr const char* cRevealingMod{"RevealingMod"};
    inline static constexpr const char* cFemRevMod{"FemaleRevealingMod"};
    inline static constexpr const char* cMalRevMod{"MaleRevealingMod"};
    inline static constexpr const char* cRevealingRecord{"RevealingRecord"};
    inline static constexpr const char* cFemRevRecord{"FemaleRevealingRecord"};
    inline static constexpr const char* cMalRevRecord{"MaleRevealingRecord"};
    void LoadSingleIni(const char* path, const std::string_view fileName);

  protected:
    std::set<std::string> excludedRaceMods;
    std::set<SEFormLoc> excludedRaces;
    std::set<std::string> skinMods;
    std::set<SEFormLoc> skinRecords;
    std::set<std::string> revealingMods;
    std::set<std::string> femRevMods;
    std::set<std::string> malRevMods;
    std::set<SEFormLoc> coveringRecords;
    std::set<SEFormLoc> revealingRecords;
    std::set<SEFormLoc> femRevRecords;
    std::set<SEFormLoc> malRevRecords;
    bool IsRaceExcluded(const RE::TESRace* race) const;
    bool IsNPCExcluded(const RE::TESNPC* npc);
    bool IsSkin(const RE::TESObjectARMO* armor, const std::string& modName);
    Common::eKeyword HasStatus(const RE::TESObjectARMO* armor) const;

  private:
    std::set<SEFormLoc> excludedNPCs;

  protected:
    void ClearInis();

  private:
    void LoadModNames(const CSimpleIniA::TNamesDepend& records, std::set<std::string>& fieldToFill, std::string_view entryType, std::string_view fileName);
    void LoadModRecordPairs(const CSimpleIniA::TNamesDepend& records, std::set<SEFormLoc>& fieldToFill, std::string_view entryType, std::string_view fileName);

    template <typename T>
    void LoadIniPairs(const char* section, std::map<SEFormLoc, T>& fieldToFill, const T defaultValue, const bool canBeNone = false) {
      CSimpleIniA::TNamesDepend keys;
      if (settingIni.GetAllKeys(section, keys)) {
        for (const auto& entry : keys) {
          auto key = entry.pItem;
          const auto keyLoc = ut->StrToLoc(std::string(key));
          if (keyLoc.second.empty()) continue;
          if constexpr (std::is_same_v<T, int>) {
            fieldToFill[keyLoc] = settingIni.GetLongValue(section, key, defaultValue);
          } else if constexpr (std::is_same_v<T, bool>) {
            fieldToFill[keyLoc] = settingIni.GetBoolValue(section, key, defaultValue);
          } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            fieldToFill[keyLoc] = static_cast<T>(settingIni.GetDoubleValue(section, key, static_cast<double>(defaultValue)));
          } else if constexpr (std::is_same_v<T, std::string>) {
            fieldToFill[keyLoc] = settingIni.GetValue(section, key, defaultValue);
          } else if constexpr (std::is_same_v<T, SEFormLoc>) {
            fieldToFill[keyLoc] = ut->StrToLoc(std::string(settingIni.GetValue(section, key)));
          } else {
            static_assert(false, "Unsupported type for LoadIniPairs");
          }
        }
      }
    }
};
