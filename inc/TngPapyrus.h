#pragma once
class TngPapyrus : public Singleton<TngPapyrus> {
  private:
    static void SaveGlobals(RE::StaticFunctionTag*);

    static bool GetClipCheck(RE::StaticFunctionTag*);
    static bool GetAutoReveal(RE::StaticFunctionTag*, bool aIsFemale);
    static void SaveBoolValues(RE::StaticFunctionTag*, int aID, bool aValue);

    static std::vector<std::string> GetRaceNames(RE::StaticFunctionTag*);
    static int GetRaceShape(RE::StaticFunctionTag*, int aRaceIdx);
    static float GetRaceMult(RE::StaticFunctionTag*, int aRaceIdx);
    static void SetRaceShape(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption);
    static void SetRaceMult(RE::StaticFunctionTag*, int aRaceIdx, float aGenMult);

    static std::vector<std::string> GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale);
    static int CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor);
    static int SetActorShape(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption);
    static int SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize);    
    
    static bool SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor);
    

  public:
    static bool BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept;
};