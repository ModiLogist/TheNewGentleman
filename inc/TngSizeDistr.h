#pragma once

class TngSizeDistr : public Singleton<TngSizeDistr> {
  private:
    inline static RE::BGSKeyword* fSizeKeyWs[Tng::cSizeCategories]{};
    inline static RE::TESGlobal* fSizeGlobs[Tng::cSizeCategories]{};
    inline static RE::BGSListForm* fSizeLists[Tng::cSizeCategories]{};    

    inline static const char* cBaseBone{"NPC GenitalsBase [GenBase]"};
    inline static const char* cScrtBone{"NPC GenitalsScrotum [GenScrot]"};
    

    static void ScaleGenital(RE::Actor* aActor, RE::TESGlobal* aGlobal) noexcept;

  public:
    static bool InitSizes() noexcept;
    static void RandomizeScale(RE::Actor* aActor) noexcept;
};