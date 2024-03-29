#pragma once

class TngCore : public Singleton<TngCore> {
  private:
    // Const
    inline static constexpr RE::FormID cDefSkinAAID{0xD67};
    inline static constexpr RE::FormID cExSkinIDs[Tng::cRaceTypeCount] = {
        0xAFF,  // raceManMer
        0xAFE,  // raceBeast
        0xAFF,  // raceElder
        0xAFF,  // raceDremora
        0xAFD,  // raceAfflicted
        0xAFC,  // raceSnowElf
    };

    inline static constexpr std::pair<RE::FormID, std::string_view> cExclRaceIDs[4] = {
        {0x07EAF3, "Skyrim.esm"},      // AstridRace
        {0x10760A, "Skyrim.esm"},      // ManakinRace
        {0x004D31, "Dawnguard.esm"},   // TestRace
        {0x03CA97, "Dragonborn.esm"},  // Miraak
    };
    inline static constexpr std::string_view cGenSkin{"TNGSkin_"};
    inline static constexpr std::string_view cSOSR{"SOS_Revealing"};

  public:
    static bool Initialize() noexcept;

  public:
    static void GenitalizeRaces() noexcept;
    static bool UpdateRaceGrpAddn(const std::size_t aRaceIdx, int aAddon) noexcept;

  private:
    static bool IgnoreRace(RE::TESRace* aRace) noexcept;
    static bool CheckRace(RE::TESRace* aRace);
    static Tng::TNGRes AddRace(RE::TESRace* aRace) noexcept;
    static RE::TESObjectARMO* ProduceAddonSkin(RE::TESObjectARMO* aOgSkin, int aAddonChoice, bool aIsFemale) noexcept;

  public:
    static void GenitalizeNPCSkins() noexcept;
    static Tng::TNGRes CanModifyActor(RE::Actor* aActor) noexcept;
    static Tng::TNGRes SetNPCSkin(RE::TESNPC* aNPC, int aAddon, bool aIsUser = true) noexcept;
    static Tng::TNGRes SetCharSize(RE::Actor* aActor, RE::TESNPC* aNPC, int aGenSize) noexcept;
    static void RevertNPCSkin(RE::TESNPC* aNPC);

  private:
    inline static RE::TESObjectARMO* fExSkins[Tng::cRaceTypeCount];
    inline static std::set<RE::TESNPC*> fHardExcludedNPCs;
    static RE::TESObjectARMO* GetOgSkin(RE::TESNPC* aNPC) noexcept;
    static bool FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept;
    static void LoadHardExcluded() noexcept;

  public:
    static void CheckArmorPieces() noexcept;
    static Tng::TNGRes HandleArmor(RE::TESObjectARMO* aArmor, const bool aIfLog = true) noexcept;
    static bool SwapRevealing(RE::TESObjectARMO* aArmor) noexcept;
    static bool TryMakeArmorCovering(RE::TESObjectARMO* aArmor, bool aIsCC) noexcept;
    static bool TryMakeArmorRevealing(RE::TESObjectARMO* aArmor, bool aIsRR) noexcept;

  private:
    inline static std::set<RE::TESObjectARMA*> fSAAs;
    inline static std::set<RE::TESObjectARMA*> fRAAs;
    inline static std::set<RE::TESObjectARMA*> fCAAs;
    

  private:
    // Shared Variables
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
    inline static RE::BGSKeyword* fExKey;
    inline static std::vector<RE::BGSKeyword*> fArmoKeys{fARKey, fRRKey, fACKey, fCCKey, fPAKey, fIAKey, fUAKey};
    inline static RE::BGSKeyword* fNPCKey;
    inline static RE::BGSKeyword* fBstKey;
    inline static RE::BGSKeyword* fCrtKey;
    inline static RE::BGSKeyword* fSwPKey;
    inline static RE::BGSKeyword* fGenSkinKey;
    inline static RE::TESRace* fDefRace;
    inline static RE::TESRace* fBeastDef;
    inline static RE::BSTArray<RE::TESNPC*> fAllNPCs;
    inline static std::set<RE::TESRace*> fPatchedRaces;
    inline static std::map<RE::TESObjectARMO*, std::vector<RE::TESObjectARMO*>> fMalAddonSkins;
    inline static std::map<RE::TESObjectARMO*, std::vector<RE::TESObjectARMO*>> fFemAddonSkins;
};
