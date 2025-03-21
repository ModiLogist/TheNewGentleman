#pragma once

#include <Util.h>

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
    inline static constexpr const char* cSettings{R"(.\Data\SKSE\Plugins\TheNewGentleman{}.ini)"};
    inline static constexpr const char* cTngIniEnding{"TNG.ini"};
    inline static constexpr const char* cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};

    // Setting Ini
    inline static constexpr int iniVersion = 5;
    inline static constexpr const char* versionKey{"IniVersion"};
    inline static constexpr const char* versionSection{"Version"};

    inline static constexpr const char* cGeneral{"General"};
    inline static constexpr const char* cLogLvl{"LoggingLevel"};
    inline static constexpr const char* cBoolSettings[Util::boolSettingCount]{"ExcludePlayerSize",   "CheckPlayerRegularly",   "CheckNPCsAfterLoad",
                                                                              "ForceChecks",         "Slot52ModsAreRevealing", "Slot52ModsAreMixed",
                                                                              "RandomizeMaleAddons", "UIExtensions",           "ShowAllRaces"};
    inline static constexpr bool cDefBoolSettings[Util::boolSettingCount]{false, false, true, false, false, false, false, true, false};

    inline static constexpr const char* cFLoatSections[Util::floatSettingCount]{"GlobalSizes", "GlobalSizes", "GlobalSizes", "GlobalSizes", "GlobalSizes", "GentleWomen"};
    inline static constexpr const char* cFloatNames[Util::floatSettingCount]{"Size_XS", "Size__S", "Size__M", "Size__L", "Size_XL", "Chance"};
    inline static constexpr double cDefFloatSettings[Util::floatSettingCount]{
        0.8, 0.9, 1.0, 1.2, 1.4, 20.0,
    };

    inline static constexpr const char* cControls{"Controls"};
    inline static constexpr const char* cCtrlNames[Util::intSettingCount]{"DAK_Integration", "NPCEdit", "GenitalUp", "GenitalDown", "Revealing", "WhyProblem"};

    inline static constexpr const char* cRacialAddon{"RaceGenital"};
    inline static constexpr const char* cRacialSize{"RaceSizeMultiplier"};
    inline static constexpr const char* cNPCSizeSection{"NPCGenitalSize"};
    inline static constexpr const char* cNPCAddonSection{"NPCGenitalAddon"};
    inline static constexpr const char* cActiveMalAddons{"ActiveMaleAddons"};
    inline static constexpr const char* cActiveFemAddons{"ActiveFemaleAddons"};
    inline static constexpr const char* cExcludeNPCSection{"ExcludedNPCs"};
    inline static constexpr const char* cRevealingModSection{"RevealingMod"};
    inline static constexpr const char* cRevealingSection{"RevealingRecord"};
    inline static constexpr const char* cFemRevRecordSection{"FemaleRevealingRecord"};
    inline static constexpr const char* cMalRevRecordSection{"MaleRevealingRecord"};

    std::set<std::pair<SEFormLoc, SEFormLoc>> racialAddons;
    std::set<std::pair<SEFormLoc, float>> racialSizes;

    std::set<std::pair<SEFormLoc, SEFormLoc>> npcAddons;
    std::set<std::pair<SEFormLoc, int>> npcSizeCats;

    std::set<SEFormLoc> runTimeRevealingRecords;
    std::set<SEFormLoc> runtimeCoveringRecords;
    std::set<SEFormLoc> runTimeFemRevRecords;
    std::set<SEFormLoc> runTimeMalRevRecords;

    const char* SettingFile(const int version = iniVersion) const;

  public:
    spdlog::level::level_enum GetLogLvl();
    void SetLogLvl(int logLevel);
    void SetAddonStatus(const bool isFemale, const int addnIdx, const bool status);
    void SetRgMult(const size_t rg, const float mult);
    void SaveRgAddon(const size_t rg, const int choice);
    void SaveNPCAddon(RE::TESNPC* npc, const int choice);
    void SaveNPCSize(RE::TESNPC* npc, int genSize);
    void SaveRevealingArmor(RE::TESObjectARMO* armor, int revMode);
    void SetBoolSetting(Util::eBoolSetting settingID, bool value);
    void SetIntSetting(Util::eIntSetting is, int value);
    void SetFloatSetting(Util::eFloatSetting settingID, const float value);
    void SaveGlobals();
    std::vector<std::string> Slot52Mods();
    bool Slot52ModBehavior(const std::string modName, const int behavior);

  private:
    void TransferOldIni();
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
