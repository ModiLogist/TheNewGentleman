#pragma once

class TngSizeShape : public Singleton<TngSizeShape> {
  private:
    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};
    inline static const char* cNPCAutoAddn{"TNG_ActorAddnAuto:"};
    inline static const char* cNPCUserAddn{"TNG_ActorAddnUser:"};

    inline static constexpr std::pair<RE::FormID, std::string_view> cBaseRaceIDs[Tng::cVanillaRaceTypes] = {
        {0x13746, "Skyrim.esm"},     // Nord
        {0x13748, "Skyrim.esm"},     // Redguard
        {0x13741, "Skyrim.esm"},     // Breton
        {0x13744, "Skyrim.esm"},     // Imperial
        {0x13743, "Skyrim.esm"},     // Altmer
        {0x13749, "Skyrim.esm"},     // Bosmer
        {0x13742, "Skyrim.esm"},     // Dunmer
        {0x13747, "Skyrim.esm"},     // Orsimer
        {0x13740, "Skyrim.esm"},     // Saxhleel
        {0x13745, "Skyrim.esm"},     // Khajiit
        {0x131F0, "Skyrim.esm"},     // Dremora
        {0x67CD8, "Skyrim.esm"},     // Elder
        {0x97A3D, "Skyrim.esm"},     // Afflicted
        {0x0377D, "Dawnguard.esm"},  // SnowElf
    };
    inline static constexpr std::pair<std::pair<RE::FormID, std::string_view>, int> cEquiRaceIDs[Tng::cEqRaceTypes] = {
        {{0x88794, "Skyrim.esm"}, 0},       // Nord Vampire
        {{0x88846, "Skyrim.esm"}, 1},       // Redguard Vampire
        {{0x8883C, "Skyrim.esm"}, 2},       // Breton Vampire
        {{0x88844, "Skyrim.esm"}, 3},       // Imperial Vampire
        {{0x88840, "Skyrim.esm"}, 4},       // Altmer Vampire
        {{0x88884, "Skyrim.esm"}, 5},       // Bosmer Vampire
        {{0x8883D, "Skyrim.esm"}, 6},       // Dunmer Vampire
        {{0xA82B9, "Skyrim.esm"}, 7},       // Orsimer Vampire
        {{0x8883A, "Skyrim.esm"}, 8},       // Saxhleel Vampire
        {{0x88845, "Skyrim.esm"}, 9},       // Khajiit Vampire
        {{0x35538, "Dragonborn.esm"}, 10},  // DLC2 Dremora
        {{0xA82BA, "Skyrim.esm"}, 11},      // Elder Vampire
        {{0x0E88A, "Dawnguard.esm"}, 0},    // DLC1 Nord
    };
    inline static constexpr int cVanillaRaceDefaults[Tng::cVanillaRaceTypes]{
        2,  // TNG_GenitalNord
        1,  // TNG_GenitalRedguard
        0,  // TNG_GenitalBreton
        2,  // TNG_GenitalImperial
        0,  // TNG_GenitalAltmer
        0,  // TNG_GenitalBosmer
        1,  // TNG_GenitalDunmer
        2,  // TNG_GenitalOrsimer
        0,  // TNG_GenitalSaxhleel
        2,  // TNG_GenitalKhajiit
        1,  // TNG_GenitalDremora
        2,  // TNG_GenitalElder
        0,  // TNG_GenitalAfflicted
        0,  // TNG_GenitalSnowElf
    };
    inline static constexpr std::pair<std::array<const char*, 2>, bool> cRaceNames[Tng::cVanillaRaceTypes]{
        {{"Nord", "nord"}, false},       {{"Redguard", "redguard"}, false}, {{"Breton", "Reachmen"}, false},     {{"Imperial", "imperial"}, false}, {{"Altmer", "HighElf"}, false},
        {{"Bosmer", "WoodElf"}, false},  {{"Dunmer", "DarkElf"}, false},    {{"Orsimer", "Orc"}, false},         {{"Saxhleel", "Argonian"}, true},  {{"Khajiit", "Rhat"}, true},
        {{"Dremora", "dremora"}, false}, {{"Elder", "elder"}, false},       {{"Afflicted", "afflicted"}, false}, {{"SnowElf", "Falmer"}, false}};

    inline static constexpr RE::FormID cNoGenSkinIDs[4]{0xAFF, 0xAFE, 0xAFD, 0xAFC};
    inline static constexpr RE::FormID cSizeKeyWIDs[Tng::cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};
    inline static constexpr RE::FormID cSizeGlobIDs[Tng::cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};

    struct RaceInfo {
        const char* raceName{""};
        std::vector<RE::TESRace*> races{};
        std::set<RE::TESRace*> armorRaces{};
        RE::TESObjectARMO* originalSkin = nullptr;
        int raceDefAddon = -1;
        int raceAddn = -1;
        float raceMult = 1.0f;
    };

  public:
    static bool Init() noexcept;
    static void LoadAddons() noexcept;
    static std::size_t GetAddonCount(bool aIsFemale) noexcept;
    static std::size_t GetActiveFAddnCount() noexcept;
    static int GetActualAddon(int aActiveAddon) noexcept;
    static RE::TESObjectARMO* GetAddonAt(bool aIsFemale, std::size_t aChoice) noexcept;
    static bool GetAddonStatus(std::size_t aFemaleAddon) noexcept;
    static void SetAddonStatus(std::size_t aFemaleAddon, bool aIsActive) noexcept;
    static std::vector<std::string> GetAddonNames(bool aIsFemale) noexcept;
    static std::size_t GetRaceGrp(RE::TESRace* aRace) noexcept;
    static bool LoadRaceMult(const std::string aRaceRecord, const float aSize) noexcept;
    static bool LoadRaceAddn(const std::string aRaceRecord, const std::string aAddonRecord) noexcept;
    static float GetRaceGrpMult(RE::TESRace* aRace) noexcept;
    static float GetRaceGrpMult(const std::size_t aRaceIdx) noexcept;
    static bool SetRaceGrpMult(RE::TESRace* aRace, const float aMult) noexcept;
    static bool SetRaceGrpMult(const std::size_t aRaceIdx, const float aMult) noexcept;
    static int GetRaceGrpDefAddn(RE::TESRace* aRace) noexcept;
    static int GetRaceGrpDefAddn(const std::size_t aRaceIdx) noexcept;
    static bool SetRaceGrpDefAddn(RE::TESRace* aRace, int aChoice) noexcept;
    static bool SetRaceGrpDefAddn(const std::size_t aRaceIdx, int aChoice) noexcept;
    static int GetRaceGrpAddn(RE::TESRace* aRace) noexcept;
    static int GetRaceGrpAddn(const std::size_t aRaceIdx) noexcept;
    static void UpdateRaceGrpAddn(const std::size_t aRaceIdx, const int aAddon) noexcept;
    static std::size_t GroupCount() noexcept;
    static RE::TESRace* GetRaceByIdx(const std::size_t aRaceIdx) noexcept;
    static std::string GetRaceName(const std::size_t aRaceIdx) noexcept;
    static std::vector<std::string> GetRaceGrpNames() noexcept;
    static RE::TESObjectARMO* GetRaceGrpSkin(int aRaceIdx) noexcept;
    static bool LoadNPCSize(const std::string aNPCRecord, const int aSize) noexcept;
    static bool LoadNPCAddn(const std::string aNPCRecord, const std::string aAddonRecord) noexcept;
    static void ExcludeNPC(const std::string aNPCRecord) noexcept;
    static std::pair<bool, int> GetNPCAddn(RE::TESNPC* aNPC) noexcept;
    static bool SetNPCAddn(RE::TESNPC* aNPC, int aAddon, bool aIsUser) noexcept;
    static Tng::TNGRes CanModifyActor(RE::Actor* aActor) noexcept;
    static float GetGlobalSize(std::size_t aIdx) noexcept;
    static void SetGlobalSize(std::size_t aIdx, float aSize) noexcept;
    static Tng::TNGRes SetActorSize(RE::Actor* aActor, int aGenSize) noexcept;
    static std::set<RE::TESObjectARMA*> GentifyGrpSkin(int aRaceGrp) noexcept;

  private:
    inline static RE::TESDataHandler* fDH;
    inline static RE::TESRace* fBaseRaces[Tng::cVanillaRaceTypes];
    inline static RE::TESRace* fEqRaces[Tng::cEqRaceTypes];
    inline static RE::TESRace* fDefRace;
    inline static RE::BGSKeyword* fBstKey;
    inline static RE::BGSKeyword* fFemAddKey;
    inline static RE::BGSKeyword* fMalAddKey;
    inline static RE::BGSKeyword* fPRaceKey;
    inline static RE::BGSKeyword* fRRaceKey;
    inline static RE::BGSKeyword* fIAKey;
    inline static RE::BGSKeyword* fSkinWithPenisKey;
    inline static RE::BGSKeyword* fGWKey;
    inline static RE::BGSKeyword* fExKey;
    inline static RE::BGSListForm* fGentified;
    inline static std::vector<RE::BGSKeyword*> fSizeKws;
    inline static std::vector<RE::TESGlobal*> fSizeGlbs;
    inline static std::vector<RE::TESObjectARMO*> fMalAddons;
    inline static std::vector<RE::TESObjectARMO*> fFemAddons;
    inline static std::vector<bool> fActiveFemAddons;
    inline static std::vector<RaceInfo> fRacesInfo;

    static int GetScale(RE::TESNPC* aNPC) noexcept;
    static void ScaleGenital(RE::Actor* aActor, RE::TESGlobal* aGlobal) noexcept;

  public:
    static void UpdateAddons(RE::TESRace* aRace) noexcept;
    static Tng::RaceType GetSkinType(RE::TESObjectARMO* aSkin) noexcept;
    static Tng::RaceType GetRaceType(RE::TESRace* aRace) noexcept;

  private:
    inline static std::set<RE::TESObjectARMA*> fAllMalAAs;
    inline static std::vector<std::set<RE::TESObjectARMA*>> fMalAddonAAs[6];
    inline static std::vector<std::set<RE::TESObjectARMA*>> fFemAddonAAs[6];
    static void CategorizeAddons() noexcept;
    static void CategorizeAddon(RE::TESObjectARMO* aAddon, const int aIdx, bool aIsFemale) noexcept;
    static RE::TESRace* FindEqVanilla(RE::TESRace* aRace) noexcept;

  public:
    static std::set<RE::TESObjectARMA*> GentifyMalSkin(RE::TESObjectARMO* aSkin, int aAddon = -1) noexcept;
    static std::set<RE::TESObjectARMA*> GentifyFemSkin(RE::TESObjectARMO* aSkin, int aAddon) noexcept;

  private:
    static std::map<RE::TESRace*, RE::TESObjectARMA*> GetCombinedAddons(Tng::RaceType aRaceType, RE::TESObjectARMO* aSkin) noexcept;
    static std::map<RE::TESRace*, RE::TESObjectARMA*> GetAddonAAs(Tng::RaceType aRaceType, int aAddonIdx, bool aIsFemale);
};
