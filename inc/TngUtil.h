#pragma once

class TngUtil : public Singleton<TngUtil> {
  private:
    inline static constexpr std::string_view cTNGName = "TheNewGentleman.esp";
    inline static RE::TESDataHandler* fDataHandler{nullptr};
    // Const
    inline static constexpr int cRaceTypes{14};

    inline static constexpr RE::FormID cDefRaceID = 0x19;
    inline static constexpr RE::FormID cBaseRaceIDs[cRaceTypes] = {
        0x13746,    // Nord
        0x13748,    // Redguard
        0x13741,    // Breton
        0x13744,    // Imperial
        0x13743,    // Altmer
        0x13749,    // Bosmer
        0x13742,    // Dunmer
        0x13747,    // Orsimer
        0x13740,    // Saxhleel
        0x13745,    // Khajiit
        0x131F0,    // Dremora
        0x67CD8,    // Elder
        0x97A3D,    // Afflicted
        0x200377D,  // SnowElf
    };
    inline static constexpr RE::FormID cExclRaceIDs[3] = {
        0x07EAF3,   // AstridRace
        0x10760A,   // ManakinRace
        0x2004D31,  // TestRace
    };
    inline static constexpr std::pair<RE::FormID, int> cEquiRaceIDs[13] = {
        {0x88794, 0},     // Nord Vampire
        {0x88846, 1},     // Redguard Vampire
        {0x8883C, 2},     // Breton Vampire
        {0x88844, 3},     // Imperial Vampire
        {0x88840, 4},     // Altmer Vampire
        {0x88884, 5},     // Bosmer Vampire
        {0x8883D, 6},     // Dunmer Vampire
        {0xA82B9, 7},     // Orsimer Vampire
        {0x8883A, 8},     // Saxhleel Vampire
        {0x88845, 9},     // Khajiit Vampire
        {0x4035538, 10},  // DLC2 Dremora
        {0xA82BA, 11},    // Elder Vampire
        {0x200E88A, 0},   // DLC1 Nord
    };

    inline static constexpr RE::FormID cGenitalIDs[cRaceTypes] = {
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
    };
    inline static constexpr RE::FormID cDefGenitalMnmID = 0x8ff;  // TNG_GenitalDefaultManMer
    inline static constexpr RE::FormID cDefGenitalSaxID = 0x8fd;  // TNG_GenitalDefaultSaxhleel
    inline static constexpr RE::FormID cDefGenitalKhaID = 0x8fe;  // TNG_GenitalDefaultKhajiit
    inline static constexpr RE::FormID cTngCoverID = 0x8f0;  // TNG_GenitalCover
    

    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};
    inline static constexpr RE::FormID cNPCKeywID{0x13794};
    inline static constexpr RE::FormID cCrtKeywID{0x13795};
    inline static constexpr RE::FormID cBstKeywID{0xD61D1};
    inline static constexpr RE::FormID cRevealingKeyID{0xFFF};
    inline static constexpr RE::FormID cUnderwearKeyID{0xFFE};
    inline static constexpr RE::FormID cCoveringKeyID{0xFFD};

    // Rquires Load

    inline static RE::BGSKeyword* fBeastKey{nullptr};
    inline static RE::BGSKeyword* fNPCKey{nullptr};
    inline static RE::BGSKeyword* fCreatureKey{nullptr};
    inline static RE::BGSKeyword* fRevealingKey{nullptr};
    inline static RE::BGSKeyword* fUnderwearKey{nullptr};
    inline static RE::BGSKeyword* fCoveringKey{nullptr};

    inline static RE::TESObjectARMA* fDefSaxGenital{nullptr};
    inline static RE::TESObjectARMA* fDefKhaGenital{nullptr};
    inline static RE::TESObjectARMA* fDefMnmGenital{nullptr};
    inline static RE::TESObjectARMA* fTNGCover{nullptr};
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
    inline static std::set<RE::TESObjectARMA*> fHandledArma;
    inline static std::set<RE::TESObjectARMA*> fRevealAAs;

    inline static RE::BSTArray<RE::TESNPC*> fAllNPCs;

    // Local
    static int fRCount;
    static int fCCount;
    static int fQCount;

    // Methods
    static void AddGenitalToSkin(RE::TESObjectARMO* aSkin, RE::TESObjectARMA* aGenital) noexcept;
    static bool CheckRace(RE::TESRace* aRace);
    static void AddRace(RE::TESRace* aRace, RE::TESObjectARMA* aGenital) noexcept;
    static void HandleArmor(RE::TESObjectARMO* aArmor) noexcept;
    static void CoverByArmor(RE::TESObjectARMO* aArmor) noexcept;

  public:
    static bool Initialize() noexcept;
    static void GenitalizeRaces() noexcept;
    static void GenitalizeNPCSkins() noexcept;
    static void CheckArmorPieces() noexcept;
};