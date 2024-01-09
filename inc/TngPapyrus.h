#pragma once
class TngPapyrus : public Singleton<TngPapyrus> {
  private:
    static bool TngLoaded(RE::StaticFunctionTag*);
    static void UpdateSize(RE::StaticFunctionTag*, int aIdx);
    static void UpdateRace(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption, float aGenMult);
    static void SetAutoRevealing(RE::StaticFunctionTag*, bool aFemaleArmor, bool aMaleArmor);
    static int CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor);
    static void UpdateActor(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption, int aGenSize);
    static bool GetFAutoReveal(RE::StaticFunctionTag*);
    static bool GetMAutoReveal(RE::StaticFunctionTag*);
    static int GetGenType(RE::StaticFunctionTag*, int aRaceIdx);
    static float GetGenSize(RE::StaticFunctionTag*, int aRaceIdx);
    static bool MakeRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor);
    static void SaveKeys(RE::StaticFunctionTag*);

  public:
    static bool tngLoaded;
    static bool BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept;
};