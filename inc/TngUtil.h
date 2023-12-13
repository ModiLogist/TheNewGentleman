#pragma once

class TngUtil : public Singleton<TngUtil> {
  private:
    inline static RE::TESDataHandler* fDataHandler{nullptr};
    // Const
    inline static constexpr RE::FormID cDefRaceID = 0x19;
    inline static constexpr std::pair<RE::FormID, std::string_view> cBaseRaceIDs[Tng::cRaceTypes] = {
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
    inline static constexpr std::pair<std::pair<RE::FormID, std::string_view>, int> cEquiRaceIDs[13] = {
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

    inline static constexpr RE::FormID cGenitalIDs[Tng::cRaceTypes * 3] = {
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
    inline static constexpr RE::FormID cDefGenitalMnmID[3] = {0x8f0, 0x8f3, 0x8f6};  // TNG_GenitalDefaultManMer
    inline static constexpr RE::FormID cDefGenitalSaxID[3] = {0x8f1, 0x8f4, 0x8f7};  // TNG_GenitalDefaultSaxhleel
    inline static constexpr RE::FormID cDefGenitalKhaID[3] = {0x8f2, 0x8f5, 0x8f8};  // TNG_GenitalDefaultKhajiit

    // Rquires Load

    inline static RE::BGSKeyword* fBeastKey{nullptr};
    inline static RE::BGSKeyword* fNPCKey{nullptr};
    inline static RE::BGSKeyword* fCreatureKey{nullptr};
    inline static RE::BGSKeyword* fRevealingKey{nullptr};
    inline static RE::BGSKeyword* fUnderwearKey{nullptr};
    inline static RE::BGSKeyword* fAutoCoverKey{nullptr};
    inline static RE::BGSKeyword* fCoveringKey{nullptr};

    inline static int fGenitalChoices[Tng::cRaceTypes + 3]{
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
        2,  // TNG_GenitalDefaultManMer
        0,  // TNG_GenitalDefaultSaxhleel
        2,  // TNG_GenitalDefaultKhajiit
    };
    inline static RE::TESObjectARMA* fDefSaxGenital[3]{};
    inline static RE::TESObjectARMA* fDefKhaGenital[3]{};
    inline static RE::TESObjectARMA* fDefMnmGenital[3]{};
    inline static RE::TESRace* fDefRace{nullptr};

    inline static std::set<std::pair<RE::TESRace*, RE::TESObjectARMA*>> fBaseRaceGens;
    inline static std::set<std::pair<RE::TESRace*, RE::TESObjectARMA*>> fEquiRaceGens;
    inline static std::set<std::pair<RE::TESRace*, RE::TESObjectARMA*>> fExtrRaceGens;
    inline static std::set<std::pair<RE::TESRace*, RE::TESObjectARMA*>> fAllRaceGens;
    inline static std::set<RE::TESRace*> fExclRaces;
    inline static std::set<RE::TESRace*> fHandledRaces;
    inline static std::set<RE::TESRace*> fIgnoreRaces;
    inline static std::set<RE::TESObjectARMO*> fRacialSkins;
    inline static std::set<RE::TESObjectARMO*> fHandledSkins;
    inline static std::set<RE::TESObjectARMA*> fSkinAAs;
    inline static std::set<RE::TESObjectARMA*> fCoveringAAs;
    inline static std::set<RE::TESObjectARMA*> fRevealAAs;

    inline static RE::BSTArray<RE::TESNPC*> fAllNPCs;

    // Local
    static int fRCount;
    static int fCCount;
    static int fQCount;

    // Methods
    static bool FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept;
    static void AddGenitalToSkin(RE::TESObjectARMO* aSkin, RE::TESObjectARMA* aGenital, RE::TESRace* aRace = nullptr) noexcept;
    static void IgnoreRace(RE::TESRace* aRace);
    static bool CheckRace(RE::TESRace* aRace);
    static void AddRace(RE::TESRace* aRace, RE::TESObjectARMA* aGenital, RE::TESRace* aRNAM = nullptr) noexcept;
    static void HandleArmor(RE::TESObjectARMO* aArmor) noexcept;
    static void CoverByArmor(RE::TESObjectARMO* aArmor) noexcept;
    static void ReCheckArmor(RE::TESObjectARMO* aArmor) noexcept;

  public:
    static bool Initialize() noexcept;
    static void GenitalizeRaces() noexcept;
    static void GenitalizeNPCSkins() noexcept;
    static void CheckArmorPieces() noexcept;
};