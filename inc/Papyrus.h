#pragma once
class Papyrus : public Singleton<Papyrus> {
  public:
    static bool BindPapyrus(RE::BSScript::IVirtualMachine* aVM) ;

  private:
    static int UpdateLogLvl(RE::StaticFunctionTag*, int aLogLvl);
    static std::string ShowLogLocation(RE::StaticFunctionTag*);

    static bool GetBoolValue(RE::StaticFunctionTag*, int settingID);
    static void SetBoolValue(RE::StaticFunctionTag*, int settingID, bool value);

    static std::vector<std::string> GetRgNames(RE::StaticFunctionTag*);
    static int GetRgAddn(RE::StaticFunctionTag*, int rg);
    static float GetRgMult(RE::StaticFunctionTag*, int rg);
    static void SetRgAddn(RE::StaticFunctionTag*, int rg, int aGenOption);
    static void SetRgMult(RE::StaticFunctionTag*, int rg, float aGenMult);

    static bool GetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int aAddn);
    static void SetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int aAddn, bool status);
    static std::vector<std::string> GetAllPossibleAddons(RE::StaticFunctionTag*, bool isFemale);
    static int CanModifyActor(RE::StaticFunctionTag*, RE::Actor* actor);
    static int SetActorAddn(RE::StaticFunctionTag*, RE::Actor* actor, int aGenOption);
    static int SetActorSize(RE::StaticFunctionTag*, RE::Actor* actor, int genSize);

    static std::vector<std::string> GetSlot52Mods(RE::StaticFunctionTag*);
    static bool Slot52ModBehavior(RE::StaticFunctionTag*, std::string modName, int behavior);
    static bool SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* armor);
    
    static void UpdateSettings(RE::StaticFunctionTag*);
};
