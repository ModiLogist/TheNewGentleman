#pragma once
class TngPapyrus : public Singleton<TngPapyrus> {
  private:
    static void SaveGlobals(RE::StaticFunctionTag*);

    static bool GetBoolValue(RE::StaticFunctionTag*, int aID);
    static void SetBoolValue(RE::StaticFunctionTag*, int aID, bool aValue);

    static std::vector<std::string> GetRaceGrpNames(RE::StaticFunctionTag*);
    static int GetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx);
    static float GetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx);
    static void SetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption);
    static void SetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx, float aGenMult);

    static bool GetAddonStatus(RE::StaticFunctionTag*, int aFemaleAddn);
    static void SetAddonStatus(RE::StaticFunctionTag*, int aFemaleAddn, bool aStatus);
    static std::vector<std::string> GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale);
    static int CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor);
    static int SetActorAddn(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption);
    static int SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize);

    static bool SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor);

  public:
    static bool BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept;
};