#pragma once

class Inis : public Singleton<Inis> {
    // User Inis
  public:
    static void LoadTngInis();

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

    inline static std::set<std::string> excludedRaceMods;
    inline static std::set<SEFormLoc> excludedRaces;

    inline static std::set<SEFormLoc> excludedNPCs;

    inline static std::set<std::string> skinMods;
    inline static std::set<SEFormLoc> skinRecords;

    inline static std::set<std::string> revealingMods;
    inline static std::set<std::string> femRevMods;
    inline static std::set<std::string> malRevMods;
    inline static std::set<SEFormLoc> coveringRecords;
    inline static std::set<SEFormLoc> revealingRecords;
    inline static std::set<SEFormLoc> femRevRecords;
    inline static std::set<SEFormLoc> malRevRecords;

    static void LoadSingleIni(const char* path, const std::string_view fileName);

  public:
    static void LoadMainIni();
    static void LoadRgInfo();
    static void LoadNpcInfo();

    static void CleanIniLists();

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
    inline static constexpr const char* cBoolSettings[Tng::BoolSettingCount]{"ExcludePlayerSize",  "CheckPlayerRegularly", "CheckNPCsAfterLoad", "ForceChecks", "Slot52ModsAreRevealing",
                                                                             "Slot52ModsAreMixed", "RandomizeMaleAddons",  "UIExtensions",       "ShowAllRaces"};

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

    inline static std::set<std::pair<SEFormLoc, SEFormLoc>> racialAddons;
    inline static std::set<std::pair<SEFormLoc, float>> racialMults;

    inline static std::set<std::pair<SEFormLoc, SEFormLoc>> npcAddons;
    inline static std::set<std::pair<SEFormLoc, int>> npcSizeCats;

    inline static std::set<SEFormLoc> runTimeRevealingRecords;
    inline static std::set<SEFormLoc> runtimeCoveringRecords;
    inline static std::set<SEFormLoc> runTimeFemRevRecords;
    inline static std::set<SEFormLoc> runTimeMalRevRecords;

  public:
    static spdlog::level::level_enum GetLogLvl();
    static void SetLogLvl(int logLevel);
    static void SaveAddonStatus(const bool isFemale, const int addnIdx, const bool status);
    static void SaveRgMult(const size_t rg, const float mult);
    static void SaveRgAddon(const size_t rg, const int choice);
    static void SaveNPCAddon(RE::TESNPC* npc, const int choice);
    static void SaveNPCSize(RE::TESNPC* npc, int genSize);
    static void SaveRevealingArmor(RE::TESObjectARMO* armor, int revMode);
    static void LoadHoteKeys();
    static void SaveSettingBool(Tng::BoolSetting settingID, bool value);
    static void SaveGlobals();
    static std::vector<std::string> Slot52Mods();
    static bool Slot52ModBehavior(const std::string modName, const int behavior);

  private:
    static void UpdateIniVersion();
    static void LoadModRecodPairs(CSimpleIniA::TNamesDepend records, std::set<SEFormLoc>& fieldToFill);
    static void UpdateRevealing(const std::string armorRecod, const int revealingMode);

  public:
    static bool IsRaceExcluded(const RE::TESRace* race);

    static bool IsNPCExcluded(const RE::TESNPC* npc);

    static bool IsSkin(const RE::TESObjectARMO* armor, const std::string modName);
    static bool IsCovering(const RE::TESObjectARMO* armor, const std::string modName);
    static int IsRevealing(const RE::TESObjectARMO* armor, const std::string modName);
    static bool IsRTCovering(const RE::TESObjectARMO* armor, const std::string modName);
    static int IsRTRevealing(const RE::TESObjectARMO* armor, const std::string modName);
    static bool IsExtraRevealing(const std::string modName);

    static bool IsUnhandled(const std::string modName);
    static void HandleModWithSlot52(const std::string modName, const bool defRevealing);

  private:
    inline static std::set<std::string> slot52Mods;
    inline static std::set<std::string> extraRevealingMods;
};
