#pragma once

class Inis : public Singleton<Inis> {
  private:
    // TNG setting
    inline static constexpr int cCurrVersion = 3;
    inline static constexpr const char* cSettings{R"(.\Data\SKSE\Plugins\TheNewGentleman.ini)"};
    inline static constexpr const char* cIniVersion{"IniVersion"};
    inline static constexpr const char* cVersion{"Version"};

    inline static constexpr const char* cGeneral{"General"};
    inline static constexpr const char* cLogLvl{"LoggingLevel"};    
    inline static constexpr const char* cFAutoReveal{"FemaleAutoReveal"};
    inline static constexpr const char* cMAutoReveal{"MaleAutoReveal"};
    inline static constexpr const char* cExlPC{"ExcludePlayerSize"};

    inline static constexpr const char* cGlobalSize{"GlobalSizes"};
    inline static constexpr const char* cSizeNames[Tng::cSizeCategories]{"Size_XS", "Size__S", "Size__M", "Size__L", "Size_XL"};
    inline static constexpr double cDefSizes[Tng::cSizeCategories]{0.8, 0.9, 1.0, 1.2, 1.4};

    inline static constexpr const char* cControls{"Controls"};
    inline static constexpr const char* cNPCCtrl{"NPCEdit"};
    inline static constexpr const char* cUPGCtrl{"GenitalUp"};
    inline static constexpr const char* cDOWCtrl{"GenitalDown"};
    inline static constexpr const char* cREVCtrl{"Revealing"};
    inline static constexpr const char* cINTCtrl{"DAK_Integration"};
    inline static constexpr RE::FormID cINTCtrlID{0xC00};
    inline static constexpr RE::FormID cNPCCtrlID{0xCB0};
    inline static constexpr RE::FormID cUPGCtrlID{0xCB1};
    inline static constexpr RE::FormID cDOWCtrlID{0xCB2};
    inline static constexpr RE::FormID cREVCtrlID{0xCB3};

    inline static constexpr const char* cRacialGenital{"RaceGenital"};
    inline static constexpr const char* cRacialSize{"RaceSizeMultplier"};
    inline static constexpr const char* cNPCSizeSection{"NPCGenitalSize"};
    inline static constexpr const char* cNPCAddnSection{"NPCGenitalAddon"};
    inline static constexpr const char* cExcludeSection{"ExcludedNPCs"};
    inline static constexpr const char* cActiveAddons{"ActiveFemaleAddons"};

    inline static constexpr const char* cGentleWomen{"GentleWomen"};
    inline static constexpr const char* cGentleWomenChance{"Chance"};

    // Ini files
    inline static constexpr const char* cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};

    inline static constexpr const char* cTngIniEnding{"TNG.ini"};

    inline static constexpr const char* cRaceSection{"Race"};
    inline static constexpr const char* cExcRaces{"ExcludeMod"};

    inline static constexpr const char* cSkeleton{"Skeleton"};
    inline static constexpr const char* cValidModel{"ValidModel"};

    inline static constexpr const char* cSkinSection{"Skin"};
    inline static constexpr const char* cSkinMod{"SkinMod"};
    inline static constexpr const char* cSkinRecord{"SkinRecord"};

    inline static constexpr const char* cArmorSection{"Armor"};
    inline static constexpr const char* cRevealingMod{"RevealingMod"};
    inline static constexpr const char* cRevealingRecord{"RevealingRecord"};
    inline static constexpr const char* cCoveringRecord{"CoveringRecord"};

    inline static bool fExlPC;

    inline static RE::TESDataHandler* fDH;
    inline static RE::TESGlobal* fNPCCtrl;
    inline static RE::TESGlobal* fUPGCtrl;
    inline static RE::TESGlobal* fDOWCtrl;
    inline static RE::TESGlobal* fREVCtrl;
    inline static RE::TESGlobal* fINTCtrl;
    inline static RE::TESGlobal* fWomenChance;
    inline static std::set<std::string> fValidSkeletons;

  public:
    enum IniBoolIDs { cNoneBoolID, excludePlayerSize, cBoolIDsCount };
    inline static std::set<std::string> fRaceExMods;
    inline static std::set<std::string> fSkinMods;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleSkinIDs;
    inline static std::set<std::string> fRevealingMods;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleRevealingIDs;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleCoveringIDs;
    inline static std::set<std::pair<std::string, RE::FormID>> fRunTimeRevealingIDs;
    inline static std::set<std::pair<std::pair<std::string, RE::FormID>, int>> fNPCSizes;
    inline static std::set<std::pair<std::pair<std::string, RE::FormID>, int>> fNPCAddns;
    inline static std::map<std::string, RE::FormID> fHardExcluded;

    static bool Init() noexcept;
    static void LoadMainIni() noexcept;
    static void LoadTngInis() noexcept;
    static void LoadHoteKeys() noexcept;
    static int GetLogLvl() noexcept;
    static void SetLogLvl(int aLvl) noexcept;
    static bool GetExcludePlayer() noexcept;
    static bool GetSettingBool(IniBoolIDs aID) noexcept;
    static void SaveSettingBool(IniBoolIDs aID, bool aValue) noexcept;
    static void SaveRaceMult(const std::size_t aRaceIdx, const float aRaceMult) noexcept;
    static void SaveRaceAddn(const std::size_t aRaceIdx, int aChoice) noexcept;
    static void SaveNPCAddn(RE::TESNPC* aNPC, int aChoice) noexcept;
    static void SaveNPCSize(RE::TESNPC* aNPC, int aGenSize) noexcept;
    static void SaveRevealingArmor(RE::TESObjectARMO* aArmor) noexcept;
    static void SaveActiveAddon(int aFemaleAddon, bool aStatus) noexcept;
    static void RemoveRevealingArmor(RE::TESObjectARMO* aArmor) noexcept;
    static void SaveGlobals() noexcept;
    static void UpdateValidSkeletons(std::set<std::string> aValidSkeletons) noexcept;
    static bool IsValidSkeleton(std::string aModel) noexcept;

  private:
    static void UpdateIniVersion() noexcept;
    static void LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>>& aField) noexcept;
    static bool UpdateRevealing(const std::string aArmorRecod) noexcept;
};
