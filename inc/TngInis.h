#pragma once

class TngInis : public Singleton<TngInis> {
  private:
    // TNG setting
    inline static constexpr const char* cSettings{R"(.\Data\SKSE\Plugins\TheNewGentleman.ini)"};
    inline static constexpr const char* cAutoReveal{"AutoReveal"};
    inline static constexpr const char* cFAutoReveal{"Female"};
    inline static constexpr const char* cMAutoReveal{"Male"};
    inline static constexpr const char* cRacialGenital{"RaceGenital"};
    inline static constexpr const char* cRacialSize{"RaceSizeMultplier"};
    inline static constexpr const char* cRaceNames[Tng::cRaceTypes + 3]{"Nord",      "Redguard", "Breton",      "Imperial",      "Altmer",      "Bosmer",
                                                                        "Dunmer",    "Orsimer",  "Saxhleel",    "Khajiit",       "Dremora",     "Elder",
                                                                        "Afflicted", "SnowElf",  "OtherManMer", "OtherSaxhleel", "OtherKhajiit"};

    inline static constexpr const char* cGlobalSize{"GlobalSizes"};
    inline static constexpr const char* cSizeNames[Tng::cSizeCategories]{"Size_XS", "Size__S", "Size__M", "Size__L", "Size_XL"};
    inline static constexpr const char* cNPCSizeSection{"NPCGenitalSize"};
    inline static constexpr const char* cNPCShapeSection{"NPCGenitalShape"};

    
    inline static constexpr RE::FormID cINTCtrlID{0xC00};
    inline static constexpr RE::FormID cNPCCtrlID{0xCB0};
    inline static constexpr RE::FormID cUPGCtrlID{0xCB1};
    inline static constexpr RE::FormID cDOWCtrlID{0xCB2};
    inline static constexpr RE::FormID cREVCtrlID{0xCB3};
    
    inline static constexpr const char* cControls{"Controls"};
    inline static constexpr const char* cNPCCtrl{"NPCEdit"};
    inline static constexpr const char* cUPGCtrl{"GenitalUp"};
    inline static constexpr const char* cDOWCtrl{"GenitalDown"};
    inline static constexpr const char* cREVCtrl{"Revealing"};
    inline static constexpr const char* cINTCtrl{"DAK_Integration"};

    inline static constexpr const char* cGentleWomen{"GentleWomen"};
    inline static constexpr const char* cGentleWomenChance{"Chance"};

    // Ini files
    inline static constexpr const char* cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};

    inline static constexpr std::string cTngIniEnding{"TNG.ini"};
    inline static constexpr const char* cSkinSection{"Skin"};
    inline static constexpr const char* cSkinMod{"SkinMod"};
    inline static constexpr const char* cSkinRecord{"SkinRecord"};

    inline static constexpr const char* cArmorSection{"Armor"};
    inline static constexpr const char* cRevealingMod{"RevealingMod"};
    inline static constexpr const char* cRevealingRecord{"RevealingRecord"};
    inline static constexpr const char* cCoveringRecord{"CoveringRecord"};


    static void LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>>& aField) noexcept;

    static bool IsTngIni(const std::string_view aFileName) noexcept;
    static void UpdateRevealing(const std::string aArmorRecod) noexcept;

  public:
    inline static bool FAutoReveal{true};
    inline static bool MAutoReveal{false};

    inline static std::set<std::string> fSkinMods;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleSkinIDs;
    inline static std::set<std::string> fRevealingMods;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleRevealingIDs;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleCoveringIDs;

    static void LoadTngInis() noexcept;
    static bool LoadMainIni() noexcept;
    static void SaveSize(const int aIdx) noexcept;
    static void UpdateRace(const int aRaceIdx, const int aRaceShape, const float aRaceMult) noexcept;
    static void AddActorShape(RE::FormID aFormID, std::string aModName, int aGenShape) noexcept;
    static void AddActorSize(RE::FormID aFormID, std::string aModName, int aGenSize) noexcept;
    static void AddRevealingArmor(RE::TESObjectARMO* aArmor) noexcept;
    static void SaveGlobals() noexcept;
};
