#pragma once

class Base : public Singleton<Base> {
  public:
    void Init();

  private:
    enum eGenBones { egbBase, egbScrot, egbScrotL, egbScrotR, egbGen01, egbGen02, egbGen03, egbGen04, egbGen05, egbGen06, GenBonesCount };
    inline static const char* genBoneNames[GenBonesCount]{"NPC GenitalsBase [GenBase]",
                                                          "NPC GenitalsScrotum [GenScrot]",
                                                          "NPC L GenitalsScrotum [LGenScrot]",
                                                          "NPC R GenitalsScrotum [RGenScrot]",
                                                          "NPC Genitals01 [Gen01]",
                                                          "NPC Genitals02 [Gen02]",
                                                          "NPC Genitals03 [Gen03]",
                                                          "NPC Genitals04 [Gen04]",
                                                          "NPC Genitals05 [Gen05]",
                                                          "NPC Genitals06 [Gen06]"};
    inline static const char* cNPCAutoAddon{"TNG_ActorAddnAuto:"};
    inline static const char* cNPCUserAddon{"TNG_ActorAddnUser:"};

    inline static constexpr int cVanillaRaceTypes{14};
    inline static constexpr RE::FormID cRaceDefaults[cVanillaRaceTypes]{0xA01, 0xA02, 0xA03, 0xA04, 0xA03, 0xA01, 0xA04, 0xA02, 0xA01, 0xA05, 0xA04, 0xA05, 0xA03, 0xA05};
    inline static constexpr std::pair<std::array<const char*, 2>, bool> cRaceNames[cVanillaRaceTypes]{
        {{"default", "nord"}, false},    {{"redguard", "yokudan"}, false}, {{"breton", "reachmen"}, false},     {{"cyrodi", "imperial"}, false},  {{"altmer", "highelf"}, false},
        {{"bosmer", "woodelf"}, false},  {{"dunmer", "darkelf"}, false},   {{"orsimer", "orc"}, false},         {{"saxhleel", "argonian"}, true}, {{"khajiit", "rhat"}, true},
        {{"dremora", "dremora"}, false}, {{"elder", "old"}, false},        {{"afflicted", "afflicted"}, false}, {{"snowelf", "falmer"}, false},
    };

  public:
    int AddonIdxByLoc(bool isFemale, SEFormLocView addonLoc);
    RE::TESObjectARMO* AddonByIdx(const bool isFemale, const size_t choice, const bool onlyActive);
    size_t GetAddonCount(const bool isFemale, const bool onlyActive);
    bool GetAddonStatus(const bool isFemale, const size_t addnIdx);
    void SetAddonStatus(const bool isFemale, const size_t addnIdx, const bool aIsActive);

  private:
    std::vector<std::pair<RE::TESObjectARMO*, bool>> malAddons;
    std::vector<std::pair<RE::TESObjectARMO*, bool>> femAddons;
    void LoadAddons();

  public:
  public:
    void AddRace(RE::TESRace* race, bool isProcessed);
    bool ReevaluateRace(RE::TESRace* race, RE::Actor* actor);
    void TryUnhideRace(RE::TESRace* race);
    int GetRaceRgIdx(RE::TESRace* race);
    void UpdateRgSkins();
    RE::TESRace* GetRgRace0(const size_t rgChoice, const bool onlyMCM);
    std::vector<std::string> GetRgNames(const bool onlyMCM);
    std::string GetRgInfo(const size_t rgChoice, bool onlyMCM);
    int GetRgAddon(const size_t rgChoice, bool onlyMCM);
    int GetRgAddon(RE::TESRace* race);
    bool SetRgAddon(const size_t rgChoice, const int addnChoice, bool onlyMCM);
    float GetRgMult(const size_t rgChoice, bool onlyMCM);
    float GetRgMult(RE::TESRace* race);
    bool SetRgMult(const size_t rgChoice, const float mult, bool onlyMCM);
    std::vector<size_t> GetRgAddonList(const size_t rgChoice, bool isFemale, bool onlyMCM, bool onlyActive);
    std::vector<size_t> GetRgAddonList(RE::TESRace* race, bool isFemale, bool onlyActive);
    int IsAddonDedicatedToRg(const size_t rgChoice, bool isFemale, bool onlyMCM, size_t addnChoice);
    RE::TESObjectARMO* GetSkinWithAddonForRg(const size_t rgIdx, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);
    void ReportHiddenRgs();

  private:
    struct RaceGroupInfo {
        size_t idx{0};
        std::string name{""};
        std::string file{""};
        RE::TESRace* armorRace = nullptr;
        RE::TESObjectARMO* ogSkin = nullptr;
        bool isMain{false};
        std::vector<RE::TESRace*> races{};
        bool noMCM{false};
        float mult = {1.0f};
        int defAddonIdx{Util::nan};
        int addonIdx{Util::nan};
        std::map<size_t, std::pair<bool, RE::TESObjectARMA*>> malAddons{};
        std::map<size_t, std::pair<bool, RE::TESObjectARMA*>> femAddons{};
        std::map<RE::TESObjectARMO*, std::map<size_t, RE::TESObjectARMO*>> malSkins{};
        std::map<RE::TESObjectARMO*, std::map<size_t, RE::TESObjectARMO*>> femSkins{};
    };
    std::vector<RaceGroupInfo> rgInfoList;
    std::map<RE::TESRace*, size_t> raceRgs;
    std::map<RE::TESObjectARMO*, RE::TESObjectARMO*> ogSkinMap;
    RaceGroupInfo* GetRg(const size_t rgIdx, const bool onlyMCM);
    RaceGroupInfo* GetRg(RE::TESRace* race, const bool allowAdd);
    int GetRgDefAddon(RaceGroupInfo& rg);
    void UpdateRgAddons(RaceGroupInfo& rg);
    bool RgHasAddon(RaceGroupInfo& rg, bool isFemale, int addonIdx);
    bool AddonHasRace(const RE::TESObjectARMA* addnIdx, const RE::TESRace* race);
    RE::TESObjectARMO* GetSkinWithAddonForRg(RaceGroupInfo* rg, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);
    RE::TESObjectARMO* GetOgSkin(RE::TESObjectARMO* skin);

  public:
    Util::eRes CanModifyNPC(RE::TESNPC* npc);
    Util::eRes GetActorSizeCat(RE::Actor* actor, int& sizeCat);
    Util::eRes SetActorSize(RE::Actor* actor, const int sizeCat);
    void ExcludeNPC(const std::string npcRecordStr);
    std::pair<int, bool> GetNPCAddon(RE::TESNPC* npc);
    Util::eRes SetNPCAddon(RE::TESNPC* npc, const int addnIdx, const bool isUser);
    void SetPlayerInfo(RE::Actor* actor, const int addnIdx);
    void UnsetPlayerInfo();
    bool HasPlayerChanged(RE::Actor* actor);
    bool GetBoolSetting(const size_t idx);
    void SetBoolSetting(const size_t idx, const bool value);
    int GetIntSetting(const size_t idx);
    void SetIntSetting(const size_t idx, const int value);
    float GetFloatSetting(const size_t idx);
    void SetFloatSetting(const size_t idx, float value);

  private:
    bool boolSettings[Util::boolSettingCount];
    int intSettings[Util::ctrlCount];
    float floatSettings[Util::floatSettingCount];

    void OrganizeNPCAddonKeywords(RE::TESNPC* npc, int addnIdx, bool isUser);
    struct PlayerInfo {
        inline static bool isFemale;
        inline static RE::TESRace* race;
        inline static bool isInfoSet;
    };
    PlayerInfo playerInfo{};
};
