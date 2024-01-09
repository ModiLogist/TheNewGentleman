#pragma once

class TngSizeShape : public Singleton<TngSizeShape> {
  private:
    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};

    
    inline static constexpr RE::FormID cSkinIDs[3]{0xA01, 0xA02, 0xA03};
    inline static constexpr RE::FormID cSizeKeyWIDs[Tng::cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};
    inline static constexpr RE::FormID cSizeGlobIDs[Tng::cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};
    inline static constexpr RE::FormID cFemAddKeyID{0xFFA};
    inline static constexpr RE::FormID cMalAddKeyID{0xFF9};
    inline static constexpr RE::FormID cFemAddLstID{0xE03};
    inline static constexpr RE::FormID cMalAddLstID{0xE02};

    inline static RE::TESDataHandler* fDataHandler{nullptr};

    inline static RE::BGSKeyword* fTNGRaceKey{nullptr};
    inline static RE::BGSKeyword* fSizeKeyWs[Tng::cSizeCategories]{};
    inline static std::set<std::pair<RE::FormID, int>> fRaceIndices;
    
    inline static RE::BGSKeyword* fFemAddKey{nullptr};
    inline static RE::BGSKeyword* fMalAddKey{nullptr};
    inline static RE::BGSListForm* fFemAddLst{nullptr};
    inline static RE::BGSListForm* fMalAddLst{nullptr};
    static bool LoadAddons() noexcept;
    static float GetRacialMult(RE::Actor* aActor) noexcept;
    static void ScaleGenital(RE::Actor* aActor, RE::TESGlobal* aGlobal) noexcept;

  public:
    inline static constexpr int cGenitalDefaults[Tng::cRaceTypes + 3]{
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
    inline static int genitalChoices[Tng::cRaceTypes + 3]{
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
    inline static RE::TESGlobal* fSizeGlobs[Tng::cSizeCategories]{};

    inline static RE::TESObjectARMO* fFemAddons[99]{};
    inline static RE::TESObjectARMO* fMalAddons[99]{};
    inline static std::set<RE::TESObjectARMA*> fMalAddonAAs[99]{};
    static int fFemAddonCount;
    static int fMalAddonCount;



    static bool InitSizes() noexcept;
    static void UpdateSavedSize(const std::string aNPCRecord,const long aSize) noexcept;
    static void UpdateSavedShape(const std::string aNPCRecord,const long aShape) noexcept;    
    static void AddRaceIndex(RE::FormID aRace, int aRaceIndex) noexcept;
    static std::set<RE::FormID> GetRacesWithIndex(int aRaceIndex) noexcept;
    static void RandomizeScale(RE::Actor* aActor) noexcept;
    static void SetActorSkin(RE::Actor* aActor, int aGenOption) noexcept;
    static void SetActorSize(RE::Actor* aActor, int aGenSize) noexcept;
    static int CanModifyActor(RE::Actor* aActor) noexcept;
};