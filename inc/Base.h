#pragma once

class Base : public Singleton<Base> {
  public:
    static void Init();

  private:
    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};
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
    static int AddonIdxByLoc(bool isFemale, SEFormLocView addonLoc);
    static RE::TESObjectARMO* AddonByIdx(bool isFemale, size_t choice, bool onlyActive);
    static size_t GetAddonCount(bool isFemale, bool onlyActive);
    static bool GetAddonStatus(const bool isFemale, const size_t addnIdx);
    static void SetAddonStatus(const bool isFemale, const size_t addnIdx, const bool aIsActive);

  private:
    inline static std::vector<std::pair<RE::TESObjectARMO*, bool>> malAddons;
    inline static std::vector<std::pair<RE::TESObjectARMO*, bool>> femAddons;
    static void LoadAddons();

  public:
    static float GetGlobalSize(size_t idx);
    static void SetGlobalSize(size_t idx, float size);

  public:
    static void AddRace(RE::TESRace* race);
    static void TryUnhideRace(RE::TESRace* race);
    static int GetRaceRgIdx(RE::TESRace* race);
    static void UpdateRgSkins();
    static RE::TESRace* GetRgRace0(const size_t rgChoice, const bool onlyMCM);
    static std::vector<std::string> GetRgNames(const bool onlyMCM);
    static std::string GetRgRaceNames(size_t rgChoice, bool onlyMCM);
    static int GetRgAddon(const size_t rgChoice, bool onlyMCM);
    static int GetRgAddon(RE::TESRace* race);
    static bool SetRgAddon(const size_t rgChoice, const int addnChoice, bool onlyMCM);
    static float GetRgMult(const size_t rgChoice, bool onlyMCM);
    static float GetRgMult(RE::TESRace* race);
    static bool SetRgMult(const size_t rgChoice, const float mult, bool onlyMCM);
    static std::vector<size_t> GetRgAddonList(size_t rgChoice, bool isFemale,  bool onlyMCM, bool onlyActive);
    static std::vector<size_t> GetRgAddonList(RE::TESRace* race, bool isFemale, bool onlyActive);
    static int IsAddonDedicatedToRg(const size_t rgChoice, bool isFemale, bool onlyMCM, size_t addnChoice);
    static RE::TESObjectARMO* GetSkinWithAddonForRg(const size_t rgIdx, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);
    static void ReportHiddenRgs();

  private:
    struct RaceGroupInfo {
        size_t idx{0};
        std::string name{""};
        RE::TESRace* armorRace = nullptr;
        RE::TESObjectARMO* ogSkin = nullptr;
        bool isMain{false};
        std::vector<RE::TESRace*> races{};
        bool noMCM{false};
        float mult = {1.0f};
        int defAddonIdx{Tng::cNA};
        int addonIdx{Tng::cNA};
        std::map<size_t, std::pair<bool, RE::TESObjectARMA*>> malAddons{};
        std::map<size_t, std::pair<bool, RE::TESObjectARMA*>> femAddons{};
        std::map<RE::TESObjectARMO*, std::map<size_t, RE::TESObjectARMO*>> malSkins{};
        std::map<RE::TESObjectARMO*, std::map<size_t, RE::TESObjectARMO*>> femSkins{};
    };
    inline static std::vector<RaceGroupInfo> rgInfoList;
    inline static std::map<RE::TESRace*, size_t> raceRgs;
    inline static std::map<RE::TESObjectARMO*, RE::TESObjectARMO*> ogSkins;
    static RaceGroupInfo* GetRg(const size_t rgIdx, const bool onlyMCM);
    static RaceGroupInfo* GetRg(RE::TESRace* race, const bool allowAdd);
    static int GetRgDefAddon(Base::RaceGroupInfo& rg);
    static void UpdateRgAddons(RaceGroupInfo& rg);
    static bool RgHasAddon(RaceGroupInfo& rg, bool isFemale, int addonIdx);
    static RE::TESObjectARMO* GetSkinWithAddonForRg(RaceGroupInfo* rg, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);

  public:
    static Tng::TNGRes CanModifyNPC(RE::TESNPC* npc);
    static Tng::TNGRes GetActorSizeCat(RE::Actor* actor, int& sizeCat);
    static Tng::TNGRes SetActorSizeCat(RE::Actor* actor, const int sizeCat);
    static void ExcludeNPC(const std::string npcRecord);
    static std::pair<bool, int> GetNPCAddon(RE::TESNPC* npc);
    static Tng::TNGRes SetNPCAddon(RE::TESNPC* npc, int addnIdx, bool isUser);
    static void SetPlayerInfo(RE::Actor* actor, const int addnIdx);
    static void UnsetPlayerInfo();
    static bool HasPlayerChanged(RE::Actor* actor);

  private:
    static void OrganizeNPCAddonKeywords(RE::TESNPC* npc, int addnIdx, bool isUser);
    struct PlayerInfo {
        inline static bool isFemale;
        inline static RE::TESRace* race;
        inline static bool isInfoSet;
    };
    inline static PlayerInfo playerInfo{};
};
