#pragma once
class TngPapyrus : public Singleton<TngPapyrus> {
  private:
    static void UpdateSize(RE::StaticFunctionTag*, int aIdx);
    static void UpdateRace(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption, float aGenMult);
    static bool AllowSkinOverwrite(RE::StaticFunctionTag*);
    static bool GetClipCheck(RE::StaticFunctionTag*);
    static int CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor, bool aAllowSkinOverwrite);
    static bool SetActorShape(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption);
    static void SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize);
    static bool LoadAddons(RE::StaticFunctionTag*);
    static std::vector<std::string> GetAllPossibleAddons(RE::StaticFunctionTag*, RE::Actor* aActor);
    static bool GetAutoReveal(RE::StaticFunctionTag*, bool aIsFemale);
    static int GetGenType(RE::StaticFunctionTag*, int aRaceIdx);
    static float GetGenSize(RE::StaticFunctionTag*, int aRaceIdx);
    static bool MakeRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor);
    static void SaveBoolValues(RE::StaticFunctionTag*, int aID, bool aValue);
    static void SaveGlobals(RE::StaticFunctionTag*);

  public:
    static bool BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept;
};