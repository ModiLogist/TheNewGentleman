#pragma once

class Inis : public Singleton<Inis> {
    // User Inis
  public:
    void LoadTngInis();

  private:
    inline static constexpr const char* cExcludeSection{"Exclusions"};
    inline static constexpr const char* cExcludeNPC{"ExcludeNPC"};
    inline static constexpr const char* cExcModRaces{"ExcludeRacesInMod"};
    inline static constexpr const char* cExcRace{"ExcludeRace"};

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

    std::set<std::string> excludedRaceMods;
    std::set<SEFormLoc> excludedRaces;

    std::set<SEFormLoc> excludedNPCs;

    std::set<std::string> skinMods;
    std::set<SEFormLoc> skinRecords;

    std::set<std::string> revealingMods;
    std::set<std::string> femRevMods;
    std::set<std::string> malRevMods;
    std::set<SEFormLoc> coveringRecords;
    std::set<SEFormLoc> revealingRecords;
    std::set<SEFormLoc> femRevRecords;
    std::set<SEFormLoc> malRevRecords;

    void LoadSingleIni(const char* path, const std::string_view fileName);

  public:
    void LoadMainIni();
    void LoadRgInfo();
    void LoadNpcInfo();

    void CleanIniLists();

  private:
    inline static constexpr int cCurrVersion = 4;
    inline static constexpr const char* cSettings{R"(.\Data\SKSE\Plugins\TheNewGentleman.ini)"};
    inline static constexpr const char* cTngIniEnding{"TNG.ini"};
    inline static constexpr const char* cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};

    // Setting Ini
    inline static constexpr const char* cIniVersion{"IniVersion"};
    inline static constexpr const char* cVersion{"Version"};

    inline static constexpr const char* cGeneral{"General"};
    inline static constexpr const char* cLogLvl{"LoggingLevel"};
    inline static constexpr const char* cBoolSettings[Tng::BoolSettingCount]{"ExcludePlayerSize",   "CheckPlayerRegularly",   "CheckNPCsAfterLoad",
                                                                             "ForceChecks",         "Slot52ModsAreRevealing", "Slot52ModsAreMixed",
                                                                             "RandomizeMaleAddons", "UIExtensions",           "ShowAllRaces"};
    inline static constexpr bool cDefBoolSettings[Tng::BoolSettingCount]{false, false, true, false, false, false, false, true, false};

    inline static constexpr const char* cGlobalSize{"GlobalSizes"};
    inline static constexpr const char* cSizeNames[Tng::cSizeCategories]{"Size_XS", "Size__S", "Size__M", "Size__L", "Size_XL"};
    inline static constexpr double cDefSizes[Tng::cSizeCategories]{0.8, 0.9, 1.0, 1.2, 1.4};

    inline static constexpr const char* cControls{"Controls"};
    inline static constexpr const char* cCtrlNames[Tng::UserCtrlsCount]{"DAK_Integration", "NPCEdit", "GenitalUp", "GenitalDown", "Revealing", "WhyProblem"};

    inline static constexpr const char* cRacialAddon{"RaceGenital"};
    inline static constexpr const char* cRacialSize{"RaceSizeMultplier"};
    inline static constexpr const char* cNPCSizeSection{"NPCGenitalSize"};
    inline static constexpr const char* cNPCAddonSection{"NPCGenitalAddon"};
    inline static constexpr const char* cActiveMalAddons{"ActiveMaleAddons"};
    inline static constexpr const char* cActiveFemAddons{"ActiveFemaleAddons"};
    inline static constexpr const char* cExcludeNPCSection{"ExcludedNPCs"};
    inline static constexpr const char* cRevealingModSection{"RevealingMod"};
    inline static constexpr const char* cRevealingSection{"RevealingRecord"};
    inline static constexpr const char* cFemRevRecordSection{"FemaleRevealingRecord"};
    inline static constexpr const char* cMalRevRecordSection{"MaleRevealingRecord"};
    inline static constexpr const char* cGentleWomen{"GentleWomen"};
    inline static constexpr const char* cGentleWomenChance{"Chance"};

    std::set<std::pair<SEFormLoc, SEFormLoc>> racialAddons;
    std::set<std::pair<SEFormLoc, float>> racialMults;

    std::set<std::pair<SEFormLoc, SEFormLoc>> npcAddons;
    std::set<std::pair<SEFormLoc, int>> npcSizeCats;

    std::set<SEFormLoc> runTimeRevealingRecords;
    std::set<SEFormLoc> runtimeCoveringRecords;
    std::set<SEFormLoc> runTimeFemRevRecords;
    std::set<SEFormLoc> runTimeMalRevRecords;

  public:
    static spdlog::level::level_enum GetLogLvl();
    void SetLogLvl(int logLevel);
    void SetAddonStatus(const bool isFemale, const int addnIdx, const bool status);
    void SetRgMult(const size_t rg, const float mult);
    void SaveRgAddon(const size_t rg, const int choice);
    void SaveNPCAddon(RE::TESNPC* npc, const int choice);
    void SaveNPCSize(RE::TESNPC* npc, int genSize);
    void SaveRevealingArmor(RE::TESObjectARMO* armor, int revMode);
    void LoadHotKeys();
    void SetBoolSetting(Tng::BoolSetting settingID, bool value);
    void SaveGlobals();
    std::vector<std::string> Slot52Mods();
    bool Slot52ModBehavior(const std::string modName, const int behavior);

  private:
    void UpdateIniVersion();
    void LoadModRecordPairs(CSimpleIniA::TNamesDepend records, std::set<SEFormLoc>& fieldToFill);
    void UpdateRevealing(const std::string armorRecord, const int revealingMode);

  public:
    bool IsRaceExcluded(const RE::TESRace* race);

    bool IsNPCExcluded(const RE::TESNPC* npc);

    bool IsSkin(const RE::TESObjectARMO* armor, const std::string modName);
    bool IsCovering(const RE::TESObjectARMO* armor, const std::string modName);
    int IsRevealing(const RE::TESObjectARMO* armor, const std::string modName);
    bool IsRTCovering(const RE::TESObjectARMO* armor, const std::string modName);
    int IsRTRevealing(const RE::TESObjectARMO* armor, const std::string modName);
    bool IsExtraRevealing(const std::string modName);

    bool IsUnhandled(const std::string modName);
    void HandleModWithSlot52(const std::string modName, const bool defRevealing);

  private:
    std::set<std::string> slot52Mods;
    std::set<std::string> extraRevealingMods;
};
