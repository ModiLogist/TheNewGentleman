#pragma once

class Inis : public Singleton<Inis> {
  private:
    inline static constexpr int cCurrVersion = 3;
    inline static constexpr const char* cSettings{R"(.\Data\SKSE\Plugins\TheNewGentleman.ini)"};
    inline static constexpr const char* cDefIniPath{R"(.\Data\SKSE\Plugins\Defaults_TNG.ini)"};
    inline static constexpr std::string_view cDefIni{"Defaults_TNG"};
    inline static constexpr const char* cTngIniEnding{"TNG.ini"};
    inline static constexpr const char* cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};

    // Setting Ini
    inline static constexpr const char* cIniVersion{"IniVersion"};
    inline static constexpr const char* cVersion{"Version"};

    inline static constexpr const char* cGeneral{"General"};
    inline static constexpr const char* cLogLvl{"LoggingLevel"};
    inline static constexpr const char* cBoolSettings[Tng::BoolSettingCount]{"ExcludePlayerSize", "Slot52ModsAreRevealing", "RandomizeMaleAddons"};

    inline static constexpr const char* cGlobalSize{"GlobalSizes"};
    inline static constexpr const char* cSizeNames[Tng::cSizeCategories]{"Size_XS", "Size__S", "Size__M", "Size__L", "Size_XL"};
    inline static constexpr double cDefSizes[Tng::cSizeCategories]{0.8, 0.9, 1.0, 1.2, 1.4};

    inline static constexpr const char* cControls{"Controls"};
    inline static constexpr const char* cCtrlNames[Tng::UserCtrlsCount]{"DAK_Integration", "NPCEdit", "GenitalUp", "GenitalDown", "Revealing"};

    inline static constexpr const char* cRacialGenital{"RaceGenital"};
    inline static constexpr const char* cRacialSize{"RaceSizeMultplier"};
    inline static constexpr const char* cNPCSizeSection{"NPCGenitalSize"};
    inline static constexpr const char* cNPCAddnSection{"NPCGenitalAddon"};
    inline static constexpr const char* cActiveMalAddons{"ActiveMaleAddons"};
    inline static constexpr const char* cActiveFemAddons{"ActiveFemaleAddons"};
    inline static constexpr const char* cExcludeNPCSection{"ExcludedNPCs"};
    inline static constexpr const char* cRevealingSection{"RevealingRecord"};
    inline static constexpr const char* cGentleWomen{"GentleWomen"};
    inline static constexpr const char* cGentleWomenChance{"Chance"};

    // User Inis
    inline static constexpr const char* cExcludeSection{"Exclusions"};
    inline static constexpr const char* cExcludeNPC{"ExcludeNPC"};
    inline static constexpr const char* cExcModRaces{"ExcludeRacesInMod"};
    inline static constexpr const char* cExcRace{"ExcludeRace"};

    inline static constexpr const char* cSkeleton{"Skeleton"};
    inline static constexpr const char* cValidModel{"ValidModel"};
    inline static constexpr const char* cValidRace{"ValidRace"};

    inline static constexpr const char* cSkinSection{"Skin"};
    inline static constexpr const char* cSkinMod{"SkinMod"};
    inline static constexpr const char* cSkinRecord{"SkinRecord"};

    inline static constexpr const char* cArmorSection{"Armor"};
    inline static constexpr const char* cCoveringRecord{"CoveringRecord"};
    inline static constexpr const char* cRevealingMod{"RevealingMod"};
    inline static constexpr const char* cRevealingRecord{"RevealingRecord"};

  public:
    static void LoadMainIni();
    static void LoadTngInis();
    static void LoadHoteKeys();
    static int GetLogLvl();
    static void SetLogLvl(int aLvl);
    static bool GetSettingBool(Tng::BoolSetting settingID);
    static void SaveSettingBool(Tng::BoolSetting settingID, bool value);
    static void SaveRgMult(const size_t rg, const float mult);
    static void SaveRgAddn(const size_t rg, const int choice);
    static void SaveNPCAddn(RE::TESNPC* npc, const int choice);
    static void SaveNPCSize(RE::TESNPC* npc, int genSize);
    static void SaveAddonStatus(const bool isFemale, const int addnIdx, const bool status);
    static void SaveRevealingArmor(RE::TESObjectARMO* armor);
    static void SaveCoveringArmor(RE::TESObjectARMO* armor);
    static void SaveGlobals();
    static std::vector<std::string> Slot52Mods();
    static bool Slot52ModBehavior(const std::string modName, const int behavior);

  private:
    static void UpdateIniVersion();
    static void LoadModRecodPairs(CSimpleIniA::TNamesDepend records, std::set<std::pair<std::string, RE::FormID>>& fieldToFill);
    static void UpdateRevealing(const std::string armorRecod, const bool isRevealing);
    static void LoadSingleIni(const char* aPath, const std::string_view fileName);

  public:
    static bool IsValidSkeleton(const std::string model);
    static bool IsRaceExcluded(const RE::TESRace* race);
    static bool IsNPCExcluded(const RE::TESNPC* npc);

    static bool IsSkin(const RE::TESObjectARMO* armor, const std::string modName);
    static bool IsCovering(const RE::TESObjectARMO* armor, const std::string modName);
    static bool IsRevealing(const RE::TESObjectARMO* armor, const std::string modName);
    static bool IsExtraRevealing(const std::string modName);    
    static bool IsRTCovering(const RE::TESObjectARMO* armor, const std::string modName);
    static bool IsRTRevealing(const RE::TESObjectARMO* armor, const std::string modName);

    static bool IsUnhandled(const std::string modName);
    static void HandleModWithSlot52(const std::string modName, const bool defRevealing);

  private:
    inline static std::set<std::pair<std::string, RE::FormID>> excludedNPCs;
    inline static std::set<std::string> excludedRaceMods;
    inline static std::set<std::pair<std::string, RE::FormID>> excludedRaces;

    inline static std::set<std::string> validSkeletons;
    inline static std::set<std::pair<std::string, RE::FormID>> validRaces;

    inline static std::set<std::string> skinMods;
    inline static std::set<std::pair<std::string, RE::FormID>> skinRecords;

    inline static std::set<std::string> handledByInis;
    inline static std::set<std::string> slot52Mods;
    inline static std::set<std::pair<std::string, RE::FormID>> coveringRecords;
    inline static std::set<std::string> revealingMods;
    inline static std::set<std::pair<std::string, RE::FormID>> revealingRecords;
    inline static std::set<std::string> extraRevealingMods;

    inline static std::set<std::pair<std::string, RE::FormID>> runTimeRevealingRecords;
    inline static std::set<std::pair<std::string, RE::FormID>> runtimeCoveringRecords;

    inline static std::set<std::pair<std::pair<std::string, RE::FormID>, int>> fNPCSizes;
    inline static std::set<std::pair<std::pair<std::string, RE::FormID>, int>> fNPCAddns;
};
