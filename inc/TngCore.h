#pragma once

class TngCore : public Singleton<TngCore> {
  private:
    // Const
    inline static constexpr RE::FormID cDefRaceID = 0x19;
    inline static constexpr RE::FormID cDefSkinAAID{0xD67};
    inline static constexpr int cVanillaRaceAddns{14};
    inline static constexpr int cEqRaceAddns{13};
    inline static constexpr std::pair<RE::FormID, std::string_view> cBaseRaceIDs[cVanillaRaceAddns] = {
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
    inline static constexpr std::pair<RE::FormID, std::string_view> cExclRaceIDs[3] = {
        {0x07EAF3, "Skyrim.esm"},     // AstridRace
        {0x10760A, "Skyrim.esm"},     // ManakinRace
        {0x004D31, "Dawnguard.esm"},  // TestRace
    };
    inline static constexpr std::pair<std::pair<RE::FormID, std::string_view>, int> cEquiRaceIDs[cEqRaceAddns] = {
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
    inline static constexpr int cVanillaRaceDefaults[cVanillaRaceAddns]{
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
    inline static constexpr std::pair<std::array<std::string_view, 2>, bool> cRaceNames[cVanillaRaceAddns]{
        {{"Nord", "nord"}, false},       {{"Redguard", "redguard"}, false}, {{"Breton", "Reachmen"}, false},     {{"Imperial", "imperial"}, false}, {{"Altmer", "HighElf"}, false},
        {{"Bosmer", "WoodElf"}, false},  {{"Dunmer", "DarkElf"}, false},    {{"Orsimer", "Orc"}, false},         {{"Saxhleel", "Argonian"}, true},  {{"Khajiit", "Rhat"}, true},
        {{"Dremora", "dremora"}, false}, {{"Elder", "elder"}, false},       {{"Afflicted", "afflicted"}, false}, {{"SnowElf", "Falmer"}, false}};

    inline static constexpr RE::FormID cGenitalIDs[cVanillaRaceAddns * 3] = {
        0x800,  // TNG_GenitalNord
        0x801,  // TNG_GenitalRedguard
        0x802,  // TNG_GenitalBreton
        0x803,  // TNG_GenitalImperial
        0x804,  // TNG_GenitalAltmer
        0x805,  // TNG_GenitalBosmer
        0x806,  // TNG_GenitalDunmer
        0x807,  // TNG_GenitalOrsimer
        0x808,  // TNG_GenitalSaxhleel
        0x809,  // TNG_GenitalKhajiit
        0x80A,  // TNG_GenitalDremora
        0x80B,  // TNG_GenitalElder
        0x80C,  // TNG_GenitalAfflicted
        0x80D,  // TNG_GenitalSnowElf
        0x810,  // TNG_GenitalNord
        0x811,  // TNG_GenitalRedguard
        0x812,  // TNG_GenitalBreton
        0x813,  // TNG_GenitalImperial
        0x814,  // TNG_GenitalAltmer
        0x815,  // TNG_GenitalBosmer
        0x816,  // TNG_GenitalDunmer
        0x817,  // TNG_GenitalOrsimer
        0x818,  // TNG_GenitalSaxhleel
        0x819,  // TNG_GenitalKhajiit
        0x81A,  // TNG_GenitalDremora
        0x81B,  // TNG_GenitalElder
        0x81C,  // TNG_GenitalAfflicted
        0x81D,  // TNG_GenitalSnowElf
        0x820,  // TNG_GenitalNord
        0x821,  // TNG_GenitalRedguard
        0x822,  // TNG_GenitalBreton
        0x823,  // TNG_GenitalImperial
        0x824,  // TNG_GenitalAltmer
        0x825,  // TNG_GenitalBosmer
        0x826,  // TNG_GenitalDunmer
        0x827,  // TNG_GenitalOrsimer
        0x828,  // TNG_GenitalSaxhleel
        0x829,  // TNG_GenitalKhajiit
        0x82A,  // TNG_GenitalDremora
        0x82B,  // TNG_GenitalElder
        0x82C,  // TNG_GenitalAfflicted
        0x82D,  // TNG_GenitalSnowElf
    };
    inline static constexpr std::string_view cGenSkin{"TNGSkin_"};

  public:
    static bool Initialize() noexcept;

  public:
    static void GenitalizeRaces() noexcept;
    static bool UpdateRaces(const std::size_t aRaceIdx, int aGenOption) noexcept;

  private:
    static bool IgnoreRace(RE::TESRace* aRace) noexcept;
    static void HandleVanillaRace(RE::TESRace* aRace, const int aDefaultChoice) noexcept;
    static bool CheckRace(RE::TESRace* aRace);
    static Tng::TNGRes AddRace(RE::TESRace* aRace) noexcept;
    static void GentifySkin(RE::TESObjectARMO* aOgSkin) noexcept;
    static RE::TESObjectARMO* GentifySkin(RE::TESRace* aRace, int aOption = -1) noexcept;
    static int FindEqVanilla(RE::TESRace* aRace) noexcept;

  public:
    static void GenitalizeNPCSkins() noexcept;
    static Tng::TNGRes SetActorSkin(RE::Actor* aActor, int aGenOption) noexcept;
    static void RevertNPCSkin(RE::TESNPC* aNPC);

  private:
    static RE::FormID GetOgSkinID(RE::TESObjectARMO* aSkin, const bool aIsFemale) noexcept;
    static RE::TESObjectARMO* GetOgSkin(RE::TESObjectARMO* aSkin, const bool aIsFemale) noexcept;
    static bool FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept;

  public:
    static void CheckArmorPieces() noexcept;
    static Tng::TNGRes HandleArmor(RE::TESObjectARMO* aArmor, const bool aIfLog = true) noexcept;
    static bool SwapRevealing(RE::TESObjectARMO* aArmor) noexcept;

  private:
    inline static std::set<RE::TESObjectARMA*> fSAAs;
    inline static std::set<RE::TESObjectARMA*> fRAAs;
    inline static std::set<RE::TESObjectARMA*> fCAAs;
    static void ProcessArmor(RE::TESObjectARMO* aArmor, bool aAcceptAR = false) noexcept;
    static void CoverByArmor(RE::TESObjectARMO* aArmor) noexcept;

  private:
    // Shard Variables
    inline static RE::TESDataHandler* fDH;
    inline static RE::BGSKeyword* fPRaceKey;
    inline static RE::BGSKeyword* fRRaceKey;
    inline static RE::BGSKeyword* fIRaceKey;
    inline static RE::BGSKeyword* fARKey;
    inline static RE::BGSKeyword* fRRKey;
    inline static RE::BGSKeyword* fACKey;
    inline static RE::BGSKeyword* fCCKey;
    inline static RE::BGSKeyword* fPAKey;
    inline static RE::BGSKeyword* fIAKey;
    inline static RE::BGSKeyword* fUAKey;
    inline static std::vector<RE::BGSKeyword*> fArmoKeys{fARKey, fRRKey, fACKey, fCCKey, fPAKey, fIAKey, fUAKey};
    inline static RE::BGSKeyword* fNPCKey;
    inline static RE::BGSKeyword* fBstKey;
    inline static RE::BGSKeyword* fCrtKey;
    inline static RE::BGSKeyword* fSwPKey;
    inline static RE::BGSKeyword* fGenSkinKey;
    inline static RE::TESRace* fDefRace;
    inline static RE::TESRace* fBeastDef;
    inline static RE::BSTArray<RE::TESNPC*> fAllNPCs;
    inline static RE::TESRace* fBaseRaces[cVanillaRaceAddns];
    inline static RE::TESRace* fEqRaces[cEqRaceAddns];
    inline static std::set<RE::TESObjectARMO*> fOgSkins;
    inline static std::set<std::pair<RE::TESRace*, int>> fRacesToPatch;
    inline static std::set<RE::TESRace*> fPatchedRaces;
    inline static std::vector<std::set<std::pair<RE::FormID, RE::TESObjectARMO*>>> fMalAddonSkins;
    inline static std::vector<std::set<std::pair<RE::FormID, RE::TESObjectARMO*>>> fFemAddonSkins;
};