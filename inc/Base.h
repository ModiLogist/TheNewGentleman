#pragma once

class Base : public Singleton<Base> {
  private:
    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};
    inline static const char* cNPCAutoAddn{"TNG_ActorAddnAuto:"};
    inline static const char* cNPCUserAddn{"TNG_ActorAddnUser:"};

    inline static constexpr RE::FormID cRaceDefaults[Tng::cVanillaRaceTypes]{0xA01, 0xA02, 0xA03, 0xA04, 0xA03, 0xA01, 0xA04, 0xA02, 0xA01, 0xA05, 0xA04, 0xA05, 0xA03, 0xA05};
    inline static constexpr std::pair<std::array<const char*, 2>, bool> cRaceNames[Tng::cVanillaRaceTypes]{
        {{"default", "nord"}, false},    {{"redguard", "yokudan"}, false}, {{"breton", "reachmen"}, false},     {{"cyrodi", "imperial"}, false},  {{"altmer", "highelf"}, false},
        {{"bosmer", "woodelf"}, false},  {{"dunmer", "darkelf"}, false},   {{"orsimer", "orc"}, false},         {{"saxhleel", "argonian"}, true}, {{"khajiit", "rhat"}, true},
        {{"dremora", "dremora"}, false}, {{"elder", "old"}, false},        {{"afflicted", "afflicted"}, false}, {{"snowelf", "falmer"}, false},
    };

  public:
    static void LoadAddons() noexcept;
    static RE::TESObjectARMO* GetAddon(bool aIsFemale, std::size_t aChoice, bool aOnlyActive) noexcept;
    static std::size_t GetAddonCount(bool aIsFemale, bool aOnlyActive) noexcept;
    static bool GetAddonStatus(const bool aIsFemale, const std::size_t aAddon) noexcept;
    static void SetAddonStatus(const bool aIsFemale, const std::size_t aAddon, const bool aIsActive) noexcept;

  private:
    inline static std::vector<std::pair<RE::TESObjectARMO*, bool>> fMalAddons;
    inline static std::vector<std::pair<RE::TESObjectARMO*, bool>> fFemAddons;

  public:
    static float GetGlobalSize(std::size_t aIdx) noexcept;
    static void SetGlobalSize(std::size_t aIdx, float aSize) noexcept;

  public:
    static void AddRace(RE::TESRace* aRace) noexcept;
    static std::vector<std::string> GetRGNames() noexcept;
    static std::string GetRGRaceNames(std::size_t aRgId) noexcept;
    static std::vector<std::string> GetRGAddonNames(std::size_t aRgId, bool aIfFemale, bool aOnlyDedicated) noexcept;
    static int GetAddn(RE::TESRace* aRace) noexcept;
    static int GetAddn(const std::size_t aRgId) noexcept;
    static bool SetAddn(const std::size_t aRgId, const std::size_t aAddnId) noexcept;
    static float GetMult(RE::TESRace* aRace) noexcept;
    static float GetMult(const std::size_t aRgId) noexcept;
    static bool SetMult(const std::size_t aRgId, const float aMult) noexcept;
    static RE::TESRace* RgIdRace(const std::size_t aRgId) noexcept;

  private:
    struct RaceGroupInfo {
        std::string name{""};
        RE::TESRace* armorRace = nullptr;
        RE::TESObjectARMO* skin = nullptr;
        bool isMain{false};
        std::vector<RE::TESRace*> races{};
        float mult = {1.0f};
        SEFormLoc defAddon;
        RE::TESObjectARMO* addon;
        std::map<RE::TESObjectARMO*, std::pair<bool, RE::TESObjectARMA*>> malAddons{};
        std::vector<RE::TESObjectARMO*> malAddonIdList;
        std::map<RE::TESObjectARMO*, RE::TESObjectARMO*> malSkins{};
        std::map<RE::TESObjectARMO*, std::pair<bool, RE::TESObjectARMA*>> femAddons{};
        std::vector<RE::TESObjectARMO*> femAddonIdList;
        std::map<RE::TESObjectARMO*, RE::TESObjectARMO*> femSkins{};
    };
    inline static std::map<std::pair<RE::TESRace*, RE::TESObjectARMO*>, RaceGroupInfo> fRaceGroupInfoList;
    inline static std::vector<std::pair<RE::TESRace*, RE::TESObjectARMO*>> fRgIdList;

    static RaceGroupInfo& GetRG(RE::TESRace* aRace) noexcept;
    static SEFormLoc GetRGDefAddn(RE::TESRace* aIDRace) noexcept;
    static void UpdateRGAddons(RaceGroupInfo& aRG) noexcept;
    static void UpdateRgSkin(RaceGroupInfo& aRG, RE::TESObjectARMO* aOldAddon = nullptr) noexcept;

  public:
    static void ExcludeNPC(const std::string aNPCRecord) noexcept;
    static std::pair<bool, int> GetNPCAddn(RE::TESNPC* aNPC) noexcept;
    static bool SetNPCAddn(RE::TESNPC* aNPC, int aAddon, bool aIsUser) noexcept;
    static Tng::TNGRes CanModifyActor(RE::Actor* aActor) noexcept;

    static Tng::TNGRes SetCharSize(RE::Actor* aActor, RE::TESNPC* aNPC, int aGenSize) noexcept;

    static std::set<RE::TESObjectARMA*> GentifyGrpSkin(int aRG) noexcept;
    static std::set<RE::TESObjectARMA*> GentifyMalSkin(RE::TESObjectARMO* aSkin, int aAddon = -1) noexcept;
    static std::set<RE::TESObjectARMA*> GentifyFemSkin(RE::TESObjectARMO* aSkin, int aAddon) noexcept;

  private:
    static int GetScale(RE::TESNPC* aNPC) noexcept;
    static void ScaleGenital(RE::Actor* aActor, RE::TESGlobal* aGlobal) noexcept;

  public:
    static bool LoadRGMult(const std::string aRGParent, const std::string aRGSkin, const float aSize) noexcept;
    static bool LoadRGAddn(const std::string aRGParent, const std::string aRGSkin, const std::string aAddonRecord) noexcept;
    static bool LoadNPCSize(const std::string aNPCRecord, const int aSize) noexcept;
    static bool LoadNPCAddn(const std::string aNPCRecord, const std::string aAddonRecord) noexcept;

};
