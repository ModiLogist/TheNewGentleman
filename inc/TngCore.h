#pragma once

class TngCore : public Singleton<TngCore> {
  private:
    // Const
    inline static constexpr RE::FormID cDefSkinAAID{0xD67};

    inline static constexpr std::pair<RE::FormID, std::string_view> cExclRaceIDs[4] = {
        {0x07EAF3, "Skyrim.esm"},       // AstridRace
        {0x10760A, "Skyrim.esm"},       // ManakinRace
        {0x004D31, "Dawnguard.esm"},    // TestRace
        {0x03CA97, "Dragonborn.esm"},  // Miraak
    };
    inline static constexpr std::string_view cGenSkin{"TNGSkin_"};

  public:
    static bool Initialize() noexcept;

  public:
    static void GenitalizeRaces() noexcept;
    static bool UpdateRaces(const std::size_t aRaceIdx, int aAddon) noexcept;

  private:
    static bool IgnoreRace(RE::TESRace* aRace) noexcept;
    static bool CheckRace(RE::TESRace* aRace);
    static Tng::TNGRes AddRace(RE::TESRace* aRace) noexcept;
    static RE::TESObjectARMO* GentifySkin(RE::TESObjectARMO* aOgSkin, int aAddonChoice, bool aIsFemale) noexcept;

  public:
    static void GenitalizeNPCSkins() noexcept;
    static Tng::TNGRes SetActorSkin(RE::Actor* aActor, int aAddon) noexcept;
    static void RevertNPCSkin(RE::TESNPC* aNPC);

  private:
    static RE::FormID GetOgSkinID(RE::TESNPC* aNPC) noexcept;
    static RE::TESObjectARMO* GetOgSkin(RE::TESNPC* aNPC) noexcept;
    static bool FixSkin(RE::TESObjectARMO* aSkin, const char* const aName) noexcept;

  public:
    static void CheckArmorPieces() noexcept;
    static Tng::TNGRes HandleArmor(RE::TESObjectARMO* aArmor, const bool aIfLog = true) noexcept;
    static bool SwapRevealing(RE::TESObjectARMO* aArmor) noexcept;
    static void FixArmor(RE::TESObjectARMO* aArmor) noexcept;

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
    inline static std::set<RE::TESObjectARMO*> fOgSkins;
    inline static std::set<RE::TESRace*> fPatchedRaces;
    inline static std::map<RE::FormID, std::vector<RE::TESObjectARMO*>> fMalAddonSkins;
    inline static std::map<RE::FormID, std::vector<RE::TESObjectARMO*>> fFemAddonSkins;
};