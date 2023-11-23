#pragma once

class TngInis : public Singleton<TngInis> {
  private:
    // TNG stuff
    inline static std::string cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};
    inline static constexpr const char cDelimChar{'~'};
    inline static constexpr const char cColonChar{':'};

    inline static constexpr std::string cTngIniEnding{"TNG.ini"};
    inline static constexpr const char* cSkinSection{"Skin"};
    inline static constexpr const char* cSkinMod{"SkinMod"};
    inline static constexpr const char* cSkinRecord{"SkinRecord"};

    inline static constexpr const char* cArmorSection{"Armor"};
    inline static constexpr const char* cRevealingMod{"RevealingMod"};
    inline static constexpr const char* cRevealingRecord{"RevealingRecord"};
    inline static constexpr const char* cCoveringRecord{"CoveringRecord"};

    static void LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>> aField);

    static bool IsTngIni(const std::string aFileName) noexcept;

  public:
    inline static std::set<std::string> fSkinMods;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleSkinIDs;
    inline static std::set<std::string> fRevealingMods;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleRevealingIDs;
    inline static std::set<std::pair<std::string, RE::FormID>> fSingleCoveringIDs;

    static void LoadTngInis() noexcept;
};
