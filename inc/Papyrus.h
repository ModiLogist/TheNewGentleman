#pragma once
class Papyrus : public Singleton<Papyrus> {
  public:
    static bool BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept;

  private:
    static int UpdateLogLvl(RE::StaticFunctionTag*, int aLogLvl);
    static std::string ShowLogLocation(RE::StaticFunctionTag*);

    static bool GetBoolValue(RE::StaticFunctionTag*, int aID);
    static void SetBoolValue(RE::StaticFunctionTag*, int aID, bool aValue);

    static std::vector<std::string> GetRaceGrpNames(RE::StaticFunctionTag*);
    static int GetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx);
    static float GetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx);
    static void SetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption);
    static void SetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx, float aGenMult);

    static bool GetAddonStatus(RE::StaticFunctionTag*, bool aIsFemale, int aAddn);
    static void SetAddonStatus(RE::StaticFunctionTag*, bool aIsFemale, int aAddn, bool aStatus);
    static std::vector<std::string> GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale);
    static int CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor);
    static int SetActorAddn(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption);
    static int SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize);

    static std::vector<std::string> GetSlot52Mods(RE::StaticFunctionTag*);
    static bool Slot52ModBehavior(RE::StaticFunctionTag*, std::string aModName, int aBehavior);
    static bool SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor);
    
    static void UpdateSettings(RE::StaticFunctionTag*);
};
