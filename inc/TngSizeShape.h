#pragma once

class TngSizeShape : public Singleton<TngSizeShape> {
  public:
    static bool LoadAddons() noexcept;
    static bool LoadNPCSize(const std::string aNPCRecord, const int aSize) noexcept;
    static bool LoadNPCShape(const std::string aNPCRecord, const std::string aShapeRecord) noexcept;
    static bool LoadRaceMult(const std::string aRaceRecord, const int aSize100) noexcept;
    static bool LoadRaceShape(const std::string aRaceRecord, const std::string aShapeRecord) noexcept;
    static float GetRaceMult(RE::TESRace* aRace) noexcept;
    static void SetRaceMult(RE::TESRace* aRace, const float aMult) noexcept;
    static int GetRaceShape(RE::TESRace* aRace) noexcept;
    static void SetRaceShape(RE::TESRace* aRace, int aRaceShape) noexcept;
    static int CanModifyActor(RE::Actor* aActor, bool aAllowOverwrite) noexcept;
    static float GetGlobalSize(int aIdx) noexcept;
    static void SetGlobalSize(int aIdx, float aSize) noexcept;
    static void RandomizeScale(RE::Actor* aActor) noexcept;
    static bool SetActorSkin(RE::Actor* aActor, int aGenOption) noexcept;
    static void SetActorSize(RE::Actor* aActor, int aGenSize) noexcept;
    static std::vector<std::string> GetAllPossibleAddons(RE::Actor* aActor) noexcept;
    static int GetAddonCount(bool aIsFemale) noexcept;
    static RE::TESObjectARMO* GetAddonAt(bool aIsFemale, int aIdx) noexcept;

  private:
    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};
    inline static const char* cRaceMult{"TNG_RaceMult:"};
    inline static const char* cRaceShape{"TNG_RaceShape:"};
    inline static const char* cActorShape{"TNG_ActorShape:"};

    inline static constexpr RE::FormID cSkinIDs[3]{0xA01, 0xA02, 0xA03};
    inline static constexpr RE::FormID cSizeKeyWIDs[Tng::cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};
    inline static constexpr RE::FormID cSizeGlobIDs[Tng::cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};
    inline static constexpr RE::FormID cFemAddKeyID{0xFFA};
    inline static constexpr RE::FormID cMalAddKeyID{0xFF9};
    inline static constexpr RE::FormID cFemAddLstID{0xE03};
    inline static constexpr RE::FormID cMalAddLstID{0xE02};

    static void ScaleGenital(RE::Actor* aActor, RE::TESGlobal* aGlobal) noexcept;
    static int FindInFormList(RE::TESForm* aForm, RE::BGSListForm* aList);

    template <typename FormType>
    static constexpr FormType* LoadForm(std::string aFormRecord) {
      const size_t lSepLoc = aFormRecord.find(Tng::cDelimChar);
      const RE::FormID lFormID = std::strtol(aFormRecord.substr(0, lSepLoc).data(), nullptr, 0);
      const std::string lModName = aRaceRecord.substr(lSepLoc + 1);
      return RE::TESDataHandler::GetSingleton()->LookupForm<FormType>(lFormID, lModName);
    };
};
