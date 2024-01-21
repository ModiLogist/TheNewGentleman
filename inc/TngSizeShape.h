#pragma once

class TngSizeShape : public Singleton<TngSizeShape> {
  private:
    inline static RE::TESDataHandler* fDH;
    inline static RE::BGSKeyword* fFemAddKey;
    inline static RE::BGSKeyword* fMalAddKey;
    inline static RE::BGSKeyword* fPRaceKey;
    inline static RE::BGSKeyword* fRRaceKey;
    inline static RE::BGSKeyword* fIAKey;
    inline static RE::BGSKeyword* fSkinWithPenisKey;
    inline static RE::BGSKeyword* fCustomSkin;
    inline static RE::BGSListForm* fFemAddLst;
    inline static RE::BGSListForm* fMalAddLst;
    inline static RE::BGSListForm* fGentified;
    inline static RE::BGSKeyword* fSizeKws[Tng::cSizeCategories];
    inline static RE::TESGlobal* fSizeGlbs[Tng::cSizeCategories];
    inline static std::set<RE::TESObjectARMO*> fSkinFactory;

  public:
    static bool Init() noexcept;
    static void LoadAddons() noexcept;
    static int GetAddonCount(bool aIsFemale) noexcept;
    static std::set<std::string> GetAddonNames(bool aIsFemale) noexcept;
    static bool LoadNPCSize(const std::string aNPCRecord, const int aSize) noexcept;
    static bool LoadNPCShape(const std::string aNPCRecord, const std::string aShapeRecord) noexcept;
    static bool LoadRaceMult(const std::string aRaceRecord, const int aSize100) noexcept;
    static bool LoadRaceShape(const std::string aRaceRecord, const std::string aShapeRecord) noexcept;
    static float GetRaceMult(RE::TESRace* aRace) noexcept;
    static bool SetRaceMult(RE::TESRace* aRace, const float aMult) noexcept;
    static int GetRaceShape(RE::TESRace* aRace) noexcept;
    static void SetRaceShape(RE::TESRace* aRace, int aRaceShape) noexcept;
    static float GetRaceMult(const std::size_t aRaceIdx) noexcept;
    static bool SetRaceMult(const std::size_t aRaceIdx, const float aMult) noexcept;
    static int GetRaceShape(const std::size_t aRaceIdx) noexcept;
    static bool SetRaceShape(const std::size_t aRaceIdx, int aRaceShape) noexcept;
    static std::set<RE::TESRace*> GetRacesByIdx(const std::size_t aRaceIdx) noexcept;
    static std::set<std::string> GetRaceNames() noexcept;
    static int CanModifyActor(RE::Actor* aActor, bool aAllowOverwrite) noexcept;
    static float GetGlobalSize(int aIdx) noexcept;
    static void SetGlobalSize(int aIdx, float aSize) noexcept;
    static void RandomizeScale(RE::Actor* aActor) noexcept;
    static Tng::TNGRes SetActorSkin(RE::Actor* aActor, int aGenOption) noexcept;
    static Tng::TNGRes SetActorSize(RE::Actor* aActor, int aGenSize) noexcept;
    static std::vector<std::string> GetAllPossibleAddons(RE::Actor* aActor) noexcept;
    static RE::TESObjectARMO* GetAddonAt(bool aIsFemale, int aIdx) noexcept;

  private:
    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};
    inline static const char* cRaceMult{"TNG_RaceMult:"};
    inline static const char* cRaceShape{"TNG_RaceShape:"};
    inline static const char* cActorShape{"TNG_ActorShape:"};
    inline static const char* cShapeSkin{"ShapeSkin_"};

    inline static constexpr RE::FormID cFemAddLstID{0xE03};
    inline static constexpr RE::FormID cMalAddLstID{0xE02};
    inline static constexpr RE::FormID cNoGenSkinIDs[4]{0xAFF, 0xAFE, 0xAFD, 0xAFC};
    inline static constexpr RE::FormID cSizeKeyWIDs[Tng::cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};
    inline static constexpr RE::FormID cSizeGlobIDs[Tng::cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};

    inline static std::vector<std::pair<std::string, std::set<RE::TESRace*>>> fRaceInfo;

    static void ScaleGenital(RE::Actor* aActor, RE::TESGlobal* aGlobal) noexcept;
    static bool ResetGenital(RE::TESNPC* aNPC) noexcept;
    static bool SetNPCGenital(RE::TESNPC* aNPC, RE::TESObjectARMO* aGenital, int aChoice) noexcept;
    static int FindInFormList(RE::TESForm* aForm, RE::BGSListForm* aList) noexcept;
};
