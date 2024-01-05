#pragma once
class TngUtil : public Singleton<TngUtil> {
  private:
    static void UpdateActor(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption, int aGenSize) noexcept;
    static void UpdateRace(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption, float aGenMult) noexcept;
    static void SetAutoRevealing(RE::StaticFunctionTag*, bool aFemaleArmor, bool aMaleArmor) noexcept;
    static bool CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor);
    static bool GetFAutoReveal(RE::StaticFunctionTag*);
    static bool GetMAutoReveal(RE::StaticFunctionTag*);
    static int GetGenType(RE::StaticFunctionTag*, int aRaceIdx) noexcept;
    static float GetGenSize(RE::StaticFunctionTag*, int aRaceIdx) noexcept;

  public:
    static void BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept;
};