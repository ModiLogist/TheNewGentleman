#pragma once

class TngSizeShape : public Singleton<TngSizeShape> {
  private:
    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};
    inline static const char* cRaceMult{"TNG_RaceMult:"};
    inline static const char* cRaceAddn{"TNG_RaceAddn:"};
    inline static const char* cNPCAddn{"TNG_ActorAddn:"};

    inline static constexpr RE::FormID cNoGenSkinIDs[4]{0xAFF, 0xAFE, 0xAFD, 0xAFC};
    inline static constexpr RE::FormID cSizeKeyWIDs[Tng::cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};
    inline static constexpr RE::FormID cSizeGlobIDs[Tng::cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};

  public:
    static bool Init() noexcept;
    static void LoadAddons() noexcept;
    static std::size_t GetAddonCount(bool aIsFemale) noexcept;
    static RE::TESObjectARMO* GetAddonAt(bool aIsFemale, int aChoice) noexcept;
    static std::set<std::string> GetAddonNames(bool aIsFemale) noexcept;
    static std::vector<std::string> GetAllPossibleAddons(RE::Actor* aActor) noexcept;
    static bool LoadRaceMult(const std::string aRaceRecord, const int aSize100) noexcept;
    static bool LoadRaceAddn(const std::string aRaceRecord, const std::string aAddonRecord) noexcept;
    static float GetRaceMult(RE::TESRace* aRace) noexcept;
    static bool SetRaceMult(RE::TESRace* aRace, const float aMult) noexcept;
    static int GetRaceAddn(RE::TESRace* aRace) noexcept;
    static void SetRaceAddn(RE::TESRace* aRace, int aChoice) noexcept;
    static float GetRaceMult(const std::size_t aRaceIdx) noexcept;
    static bool SetRaceMult(const std::size_t aRaceIdx, const float aMult) noexcept;
    static int GetRaceAddn(const std::size_t aRaceIdx) noexcept;
    static bool SetRaceAddn(const std::size_t aRaceIdx, int aChoice) noexcept;
    static std::set<RE::TESRace*> GetRacesByIdx(const std::size_t aRaceIdx) noexcept;
    static std::vector<std::string> GetRaceNames() noexcept;
    static bool LoadNPCSize(const std::string aNPCRecord, const int aSize) noexcept;
    static bool LoadNPCAddn(const std::string aNPCRecord, const std::string aAddonRecord) noexcept;
    static void ExcludeNPC(const std::string aNPCRecord) noexcept; 
    static int GetNPCAddn(RE::TESNPC* aNPC) noexcept;
    static bool SetNPCAddn(RE::TESNPC* aNPC, int aAddon) noexcept;   
    static Tng::TNGRes CanModifyActor(RE::Actor* aActor) noexcept;
    static float GetGlobalSize(int aIdx) noexcept;
    static void SetGlobalSize(int aIdx, float aSize) noexcept;
    static void RandomizeScale(RE::Actor* aActor) noexcept;
    static Tng::TNGRes SetActorSize(RE::Actor* aActor, int aGenSize) noexcept;

  private:
    inline static RE::TESDataHandler* fDH;
    inline static RE::BGSKeyword* fFemAddKey;
    inline static RE::BGSKeyword* fMalAddKey;
    inline static RE::BGSKeyword* fPRaceKey;
    inline static RE::BGSKeyword* fRRaceKey;
    inline static RE::BGSKeyword* fIAKey;
    inline static RE::BGSKeyword* fSkinWithPenisKey;
    inline static RE::BGSKeyword* fGWKey;
    inline static RE::BGSKeyword* fExKey;
    inline static RE::BGSListForm* fGentified;
    inline static RE::BGSKeyword* fSizeKws[Tng::cSizeCategories];
    inline static RE::TESGlobal* fSizeGlbs[Tng::cSizeCategories];
    inline static std::vector<RE::TESObjectARMO*> fMalAddons;
    inline static std::vector<RE::TESObjectARMO*> fFemAddons;
    inline static std::vector<std::pair<std::string, std::set<RE::TESRace*>>> fRaceInfo;

    static void ScaleGenital(RE::Actor* aActor, RE::TESGlobal* aGlobal) noexcept;
};
