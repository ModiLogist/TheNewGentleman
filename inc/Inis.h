#pragma once

#include <Util.h>

class Inis {
  public:
    void LoadMainIni();
    void SaveMainIni();
    virtual ~Inis() = default;

    Common::TypedSetting<bool, Common::eBoolSetting, Common::boolSettingCount> boolSettings{
        std::array<bool, Common::boolSettingCount>{false, false, false, false, false, false, false, false, false},
        std::array<std::string, Common::boolSettingCount>{"General", "General", "General", "General", "General", "General", "General", "General", "Controls"},
        std::array<std::string, Common::boolSettingCount>{"ExcludePlayerSize", "CheckPlayerRegularly", "ForceChecks", "Slot52ModsAreRevealing", "Slot52ModsAreMixed",
                                                          "RandomizeMaleAddons", "UIExtensions", "ShowAllRaces", "DAK_Integration"}};
    Common::TypedSetting<int, Common::eIntSetting, Common::intSettingCount> intSettings{
        std::array<int, Common::intSettingCount>{Common::nul, Common::nul, Common::nul, Common::nul, Common::nul}, std::array<std::string, Common::intSettingCount>{"Controls"},
        std::array<std::string, Common::intSettingCount>{"NPCEdit", "GenitalUp", "GenitalDown", "Revealing", "WhyProblem"}};
    Common::TypedSetting<float, Common::eFloatSetting, Common::floatSettingCount> floatSettings{
        std::array<float, Common::floatSettingCount>{0.8f, 0.9f, 1.0f, 1.2f, 1.4f, 0.0f, 20.0f},
        std::array<std::string, Common::floatSettingCount>{"GlobalSizes", "GlobalSizes", "GlobalSizes", "GlobalSizes", "GlobalSizes", cActiveMalAddons, cActiveFemAddons},
        std::array<std::string, Common::floatSettingCount>{"Size_XS", "Size__S", "Size__M", "Size__L", "Size_XL", "RandomChance", "RandomChance"}};

    spdlog::level::level_enum GetLogLvl() const;
    void SetLogLvl(const int newLevel);

  private:
    inline static constexpr int iniVersion = 5;
    inline static constexpr const char* cSettings{R"(.\Data\SKSE\Plugins\TheNewGentleman{}.ini)"};
    inline static constexpr const char* cTngIniEnding{"TNG.ini"};
    inline static constexpr const char* cTngInisPath{R"(.\Data\SKSE\Plugins\TNG)"};

    inline static constexpr const char* versionKey{"IniVersion"};
    inline static constexpr const char* versionSection{"Version"};

    inline static constexpr const char* cGeneral{"General"};
    inline static constexpr const char* cLogLvl{"LoggingLevel"};

    inline static constexpr const char* cActiveMalAddons{"ActiveMaleAddons"};
    inline static constexpr const char* cActiveFemAddons{"ActiveFemaleAddons"};

    inline static constexpr const char* cValidSkeletons{"ValidSkeletons"};
    inline static constexpr const char* cRacialAddon{"RaceGenital"};
    inline static constexpr const char* cRacialSize{"RaceSizeMultiplier"};

    inline static constexpr const char* cNPCAddonSection{"NPCGenitalAddon"};
    inline static constexpr const char* cNPCSizeSection{"NPCGenitalSize"};
    inline static constexpr const char* cActorAddonSection{"ActorGenitalAddon"};
    inline static constexpr const char* cActorSizeSection{"ActorGenitalSize"};

    inline static constexpr const char* cRevealingModSection{"RevealingMod"};
    inline static constexpr const char* cArmorStatusSection{"ArmorStatus"};

    const char* SettingFile(const int version = iniVersion) const;
    void TransferOldIni();

  protected:
    inline static constexpr std::array<Common::eKeyword, 4> statusKeys{Common::kyCovering, Common::kyRevealing, Common::kyRevealingF, Common::kyRevealingM};
    std::map<SEFormLoc, bool> activeMalAddons;
    std::map<SEFormLoc, bool> activeFemAddons;
    std::set<std::string> validSkeletons;
    std::map<SEFormLoc, SEFormLoc> racialAddons;
    std::map<SEFormLoc, float> racialSizes;
    std::map<SEFormLoc, SEFormLoc> npcAddons;
    std::map<SEFormLoc, int> npcSizeCats;
    std::map<SEFormLoc, int> runTimeArmorStatus;
    std::map<std::string, bool> slot52Mods;

  public:
    void SetAddonStatus(const bool isFemale, const RE::TESObjectARMO* addon, const bool status);

  protected:
    void SetValidSkeleton(const std::string& skeletonModel);
    void SetRgAddon(const RE::TESRace* rg0, const RE::TESObjectARMO* addon, const int choice);
    void SetRgMult(const RE::TESRace* rgRace, const float mult);

    SEFormLoc GetActorAddon(const RE::Actor* actor) const;
    bool SetNPCAddon(const RE::TESNPC* npc, const RE::TESObjectARMO* addon, const int choice);
    void SetActorAddon(const RE::Actor* actor, const RE::TESObjectARMO* addon, const int choice);

    int GetActorSize(const RE::Actor* actor) const;
    bool SetNPCSize(const RE::TESNPC* npc, int genSize);
    void SetActorSize(const RE::Actor* actor, const int genSize);

    void SetArmorStatus(const RE::TESObjectARMO* armor, const Common::eKeyword revMode);

  private:
    std::map<SEFormLoc, SEFormLoc> actorAddons;
    std::map<SEFormLoc, int> actorSizeCats;

  public:
    void LoadPlayerInfos(const std::string& saveName);
    bool Slot52ModBehavior(const std::string& modName) const;
    bool Slot52ModBehavior(const std::string& modName, const int behavior);
    const std::vector<std::string> Slot52Mods() const;

  protected:
    Common::PlayerInfo* GetPlayerInfo(const RE::Actor* actor, const bool allowAdd);
    void SetPlayerInfo(const RE::Actor* actor, const RE::TESObjectARMO* addon, const int choice = Common::nan, const int sizeCatInp = Common::nan);

  private:
    inline static constexpr const char* cPlayerSection{"PlayerInfo"};
    inline static constexpr const char* cPlayerName{"Name"};
    inline static constexpr const char* cPlayerGender{"Gender"};
    inline static constexpr const char* cPlayerRace{"Race"};
    inline static constexpr const char* cPlayerAddon{"Addon"};
    inline static constexpr const char* cPlayerSkin{"Skin"};
    inline static constexpr const char* cPlayerSize{"Size"};
    std::vector<Common::PlayerInfo> playerInfos;

  public:
    void LoadTngInis();

  private:
    inline static constexpr const char* cExcludeSection{"Exclusions"};
    inline static constexpr const char* cExcModRaces{"ExcludeRacesInMod"};
    inline static constexpr const char* cExcRace{"ExcludeRace"};
    inline static constexpr const char* cExcludeNPC{"ExcludeNPC"};

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
    void LoadSingleIni(const char* path, const std::string_view fileName);

  protected:
    std::set<std::string> excludedRaceMods;
    std::set<SEFormLoc> excludedRaces;
    std::set<std::string> skinMods;
    std::set<SEFormLoc> skinRecords;
    std::set<std::string> revealingMods;
    std::set<std::string> femRevMods;
    std::set<std::string> malRevMods;
    std::set<SEFormLoc> coveringRecords;
    std::set<SEFormLoc> revealingRecords;
    std::set<SEFormLoc> femRevRecords;
    std::set<SEFormLoc> malRevRecords;
    bool IsRaceExcluded(const RE::TESRace* race) const;
    bool IsNPCExcluded(const RE::TESNPC* npc) const;
    bool IsSkin(const RE::TESObjectARMO* armor, const std::string& modName);
    Common::eKeyword HasStatus(const RE::TESObjectARMO* armor) const;

  private:
    std::set<SEFormLoc> excludedNPCs;

  protected:
    void ClearInis();

  private:
    void LoadModNames(const CSimpleIniA::TNamesDepend& records, std::set<std::string>& fieldToFill, std::string_view entryType, std::string_view fileName);
    void LoadModRecordPairs(const CSimpleIniA::TNamesDepend& records, std::set<SEFormLoc>& fieldToFill, std::string_view entryType, std::string_view fileName);

    template <typename T>
    T GetDefault() const {
      if constexpr (std::is_same_v<T, int>) {
        return Common::nan;
      } else if constexpr (std::is_same_v<T, bool>) {
        return false;
      } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
        return 1.0f;
      } else if constexpr (std::is_same_v<T, std::string>) {
        return "";
      } else if constexpr (std::is_same_v<T, SEFormLoc>) {
        return {0, ""};
      } else {
        static_assert(false, "Unsupported type for LoadIniPairs");
      }
    }

    template <typename T>
    void LoadIniPairs(CSimpleIniA& settingIni, const char* section, std::map<SEFormLoc, T>& fieldToFill) {
      LoadIniPairs(settingIni, section, fieldToFill, GetDefault<T>());
    }

    template <typename T>
    void LoadIniPairs(CSimpleIniA& settingIni, const char* section, std::map<SEFormLoc, T>& fieldToFill, const T defValue) {
      CSimpleIniA::TNamesDepend keys;
      if (settingIni.GetAllKeys(section, keys)) {
        for (const auto& entry : keys) {
          T value = defValue;
          auto key = entry.pItem;
          const auto keyLoc = ut->StrToLoc(std::string(key));
          if (keyLoc.second.empty()) continue;
          if constexpr (std::is_same_v<T, int>) {
            value = settingIni.GetLongValue(section, key, defValue);
          } else if constexpr (std::is_same_v<T, bool>) {
            value = settingIni.GetBoolValue(section, key, defValue);
          } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            value = static_cast<T>(settingIni.GetDoubleValue(section, key, static_cast<double>(defValue)));
          } else if constexpr (std::is_same_v<T, std::string>) {
            value = settingIni.GetValue(section, key, defValue);
          } else if constexpr (std::is_same_v<T, SEFormLoc>) {
            value = ut->StrToLoc(std::string(settingIni.GetValue(section, key)));
          } else {
            static_assert(false, "Unsupported type for LoadIniPairs");
          }
          if (value != defValue) fieldToFill[keyLoc] = value;
        }
      }
    }

    template <typename T>
    void SaveIniPairs(CSimpleIniA& settingIni, const char* section, const std::map<SEFormLoc, T>& fieldToSave) {
      SaveIniPairs(settingIni, section, fieldToSave, GetDefault<T>());
    }

    template <typename T>
    void SaveIniPairs(CSimpleIniA& settingIni, const char* section, const std::map<SEFormLoc, T>& fieldToSave, const T defValue) {
      for (const auto& [keyLoc, value] : fieldToSave) {
        const auto keyStr = ut->LocToStr(keyLoc);
        if (keyStr.empty()) continue;
        if (value == defValue) {
          settingIni.Delete(section, keyStr.c_str(), true);
        } else {
          if constexpr (std::is_same_v<T, int>) {
            settingIni.SetLongValue(section, keyStr.c_str(), value);
          } else if constexpr (std::is_same_v<T, bool>) {
            settingIni.SetBoolValue(section, keyStr.c_str(), value);
          } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            settingIni.SetDoubleValue(section, keyStr.c_str(), static_cast<double>(value));
          } else if constexpr (std::is_same_v<T, std::string>) {
            settingIni.SetValue(section, keyStr.c_str(), value.c_str());
          } else if constexpr (std::is_same_v<T, SEFormLoc>) {
            settingIni.SetValue(section, keyStr.c_str(), ut->LocToStr(value).c_str());
          } else {
            static_assert(false, "Unsupported type for SaveIniPairs");
          }
        }
      }
      settingIni.SaveFile(SettingFile());
    }
};
