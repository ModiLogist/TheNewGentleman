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

    inline static constexpr const char* cSwapSection{"SwapSlot"};
    inline static constexpr const char* cSwapMod{"SwapSlotMod"};

    static void LoadModRecodPairs(std::list<CSimpleIniA::Entry> aModRecords, std::set<std::pair<std::string_view, RE::FormID>> aField);

    static bool IsTngIni(const std::string aFileName) noexcept;
    static void LoadSkinMods(CSimpleIniA* aFile) noexcept;
    static void LoadSkinRecords(CSimpleIniA* aFile) noexcept;

    static void LoadRevealingMods(CSimpleIniA* aFile) noexcept;
    static void LoadRevealingRecords(CSimpleIniA* aFile) noexcept;
    static void LoadCoveringRecords(CSimpleIniA* aFile) noexcept;

    static void LoadSwapMods(CSimpleIniA* aFile) noexcept;

  public:
    inline static std::set<std::string_view> fSkinMods;
    inline static std::set<std::pair<std::string_view, RE::FormID>> fSingleSkinIDs;
    inline static std::set<std::string_view> fRevealingMods;
    inline static std::set<std::pair<std::string_view, RE::FormID>> fSingleRevealingIDs;
    inline static std::set<std::pair<std::string_view, RE::FormID>> fSingleCoveringIDs;
    inline static std::set<std::pair<std::string_view, RE::BGSBipedObjectForm::BipedObjectSlot>> fSwapMods;

    static void LoadTngInis() noexcept;
};
