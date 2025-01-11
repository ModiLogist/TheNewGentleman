#pragma once

class Base : public Singleton<Base> {
  public:
    static void Init();

  private:
    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};
    inline static const char* cNPCAutoAddn{"TNG_ActorAddnAuto:"};
    inline static const char* cNPCUserAddn{"TNG_ActorAddnUser:"};

    inline static constexpr int cVanillaRaceTypes{14};
    inline static constexpr RE::FormID cRaceDefaults[cVanillaRaceTypes]{0xA01, 0xA02, 0xA03, 0xA04, 0xA03, 0xA01, 0xA04, 0xA02, 0xA01, 0xA05, 0xA04, 0xA05, 0xA03, 0xA05};
    inline static constexpr std::pair<std::array<const char*, 2>, bool> cRaceNames[cVanillaRaceTypes]{
        {{"default", "nord"}, false},    {{"redguard", "yokudan"}, false}, {{"breton", "reachmen"}, false},     {{"cyrodi", "imperial"}, false},  {{"altmer", "highelf"}, false},
        {{"bosmer", "woodelf"}, false},  {{"dunmer", "darkelf"}, false},   {{"orsimer", "orc"}, false},         {{"saxhleel", "argonian"}, true}, {{"khajiit", "rhat"}, true},
        {{"dremora", "dremora"}, false}, {{"elder", "old"}, false},        {{"afflicted", "afflicted"}, false}, {{"snowelf", "falmer"}, false},
    };

  public:
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
    static int GetRaceRgIdx(RE::TESRace* race);
    static void UpdateRgSkins();
    static RE::TESRace* GetRgRace0(const size_t rgIdx, const bool onlyMCM);
    static std::vector<std::string> GetRgNames(const bool onlyMCM);
    static std::string GetRgRaceNames(size_t rgChoice, bool onlyMCM);
    static int GetRgAddn(const size_t rgChoice, bool onlyMCM);
    static int GetRgAddn(RE::TESRace* race);
    static bool SetRgAddn(const size_t rgIdx, const int addnChoice, bool onlyMCM);
    static float GetRgMult(const size_t rgChoice, bool onlyMCM);
    static float GetRgMult(RE::TESRace* race);
    static bool SetRgMult(const size_t rgIdx, const float aMult);
    static std::vector<size_t> GetRgAddonList(size_t rgChoice, bool isFemale, bool onlyDedicated, bool onlyMCM);
    static std::vector<size_t> GetRgAddonList(RE::TESRace* race, bool isFemale, bool onlyDedicated);
    static RE::TESObjectARMO* GetSkinWithAddonForRg(const size_t rgIdx, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);

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
    static int GetRgDefAddn(Base::RaceGroupInfo& rg);
    static void UpdateRgAddons(RaceGroupInfo& rg);
    static RE::TESObjectARMO* GetSkinWithAddonForRg(RaceGroupInfo* rg, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);

  public:
    static Tng::TNGRes CanModifyActor(RE::Actor* actor);
    static Tng::TNGRes GetActorSizeCat(RE::Actor* actor, int& sizeCat);
    static Tng::TNGRes SetActorSizeCat(RE::Actor* actor, const int sizeCat);
    static void ExcludeNPC(const std::string npcRecord);
    static std::pair<bool, int> GetNPCAddn(RE::TESNPC* npc);
    static Tng::TNGRes SetNPCAddn(RE::TESNPC* npc, int addnIdx, bool isUser);

  private:
    static void OrganizeNPCAddonKeywords(RE::TESNPC* npc, int addnIdx, bool isUser);

  public:
    static bool LoadRgMult(const std::string rgIdRaceRecord, const float size);
    static bool LoadRgAddn(const std::string rgIdRaceRecord, const std::string addonRecord);
    static bool LoadNPCSize(const std::string npcRecord, const int size);
    static bool LoadNPCAddn(const std::string npcRecord, const std::string aAddonRecord);
};
