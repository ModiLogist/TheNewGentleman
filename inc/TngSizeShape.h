#pragma once

class TngSizeShape : public Singleton<TngSizeShape> {
  private:

    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};

    inline static RE::TESDataHandler* fDataHandler{nullptr};
    inline static constexpr const char cDelimChar{'~'};

    inline static RE::BGSKeyword* fSizeKeyWs[Tng::cSizeCategories]{};
    inline static RE::TESGlobal* fSizeGlobs[Tng::cSizeCategories]{};
    inline static RE::BGSListForm* fSizeLists[Tng::cSizeCategories]{};
    inline static RE::TESObjectARMO* fShapeSkins[3]{};
    inline static std::set<std::pair<RE::FormID, int>> fRaceIndices;
    
    static float GetRacialMult(RE::Actor* aActor) noexcept;
    static void ScaleGenital(RE::Actor* aActor, RE::TESGlobal* aGlobal) noexcept;

  public:
    inline static int genitalChoices[Tng::cRaceTypes + 4]{
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
    inline static float genitalSizes[Tng::cRaceTypes + 3] = {
        1.0f,  // TNG_GenitalNord
        1.0f,  // TNG_GenitalRedguard
        1.0f,  // TNG_GenitalBreton
        1.0f,  // TNG_GenitalImperial
        1.0f,  // TNG_GenitalAltmer
        1.0f,  // TNG_GenitalBosmer
        1.0f,  // TNG_GenitalDunmer
        1.0f,  // TNG_GenitalOrsimer
        1.0f,  // TNG_GenitalSaxhleel
        1.0f,  // TNG_GenitalKhajiit
        1.0f,  // TNG_GenitalDremora
        1.0f,  // TNG_GenitalElder
        1.0f,  // TNG_GenitalAfflicted
        1.0f,  // TNG_GenitalSnowElf
        1.0f,  // TNG_GenitalDefaultManMer
        1.0f,  // TNG_GenitalDefaultSaxhleel
        1.0f,  // TNG_GenitalDefaultKhajiit
    };



    static bool InitSizes() noexcept;
    static void UpdateSavedSize(const std::string aNPCRecord,const long aSize) noexcept;
    static void UpdateSavedShape(const std::string aNPCRecord,const long aShape) noexcept;    
    static void AddRaceIndex(RE::FormID aRace, int aRaceIndex) noexcept;
    static void RandomizeScale(RE::Actor* aActor) noexcept;
    static void SetActorSkin(RE::Actor* aActor, int aGenOption) noexcept;
    static void SetActorSize(RE::Actor* aActor, int aGenSize) noexcept;
    static bool CanModifyActor(RE::Actor* aActor) noexcept;
};