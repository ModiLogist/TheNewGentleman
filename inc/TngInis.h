#pragma once

class TngInis : public Singleton<TngInis> {
  private:
    // TNG stuff
    inline static std::string cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};
    inline static constexpr std::string cTngIniEnding{"TNG.ini"};
    inline static constexpr const char* cSkinSection{"Skin"};
    inline static constexpr const char* cSkinMod{"SkinMod"};
    static bool IsTngIni(const std::string aFileName) noexcept;
    static void LoadSkinMods(CSimpleIniA* aFile) noexcept;

  public:
    inline static std::set<std::string_view> fSkinMods;
    static void LoadTngInis() noexcept;
};
