#pragma once

class TngInis : public Singleton<TngInis> {
  private:
    inline static constexpr const char cDelimChar{'~'};
    inline static constexpr const char cColonChar{':'};

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
    inline static constexpr const char* cNPCSizeSection{"NPCGenitalSize"};
    inline static constexpr const char* cNPCShapeSection{"NPCGenitalShape"};

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
};
