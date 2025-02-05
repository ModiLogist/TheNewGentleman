#pragma once

class Papyrus : public Singleton<Papyrus> {
  public:
    static bool BindPapyrus(RE::BSScript::IVirtualMachine* vm);

  private:
    static bool GetBoolValue(RE::StaticFunctionTag*, int settingID);
    static void SetBoolValue(RE::StaticFunctionTag*, int settingID, bool value);

    static std::vector<std::string> GetAllPossibleAddons(RE::StaticFunctionTag*, bool isFemale);
    static bool GetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int addonIdx);
    static void SetAddonStatus(RE::StaticFunctionTag*, bool isFemale, int addonIdx, bool status);

    static std::vector<std::string> GetRgNames(RE::StaticFunctionTag*);
    static std::string GetRgInfo(RE::StaticFunctionTag*, int rgIdx);
    static std::vector<std::string> GetRgAddons(RE::StaticFunctionTag*, int rgIdx);
    static int GetRgAddon(RE::StaticFunctionTag*, int rgIdx);
    static void SetRgAddon(RE::StaticFunctionTag*, int rgIdx, int choice);
    static float GetRgMult(RE::StaticFunctionTag*, int rgIdx);
    static void SetRgMult(RE::StaticFunctionTag*, int rgIdx, float mult);

    static int CanModifyActor(RE::StaticFunctionTag*, RE::Actor* actor);
    static std::vector<std::string> GetActorAddons(RE::StaticFunctionTag*, RE::Actor* actor);
    static RE::TESObjectARMO* GetActorAddon(RE::StaticFunctionTag*, RE::Actor* actor);
    static int SetActorAddon(RE::StaticFunctionTag*, RE::Actor* actor, int choice);
    static int GetActorSize(RE::StaticFunctionTag*, RE::Actor* actor);
    static int SetActorSize(RE::StaticFunctionTag*, RE::Actor* actor, int sizeCat);
    static std::vector<std::string> ActorItemsInfo(RE::StaticFunctionTag*, RE::Actor* actor);
    static bool SwapRevealing(RE::StaticFunctionTag*, RE::Actor* actor, int choice);


    static std::vector<std::string> GetSlot52Mods(RE::StaticFunctionTag*);
    static bool Slot52ModBehavior(RE::StaticFunctionTag*, std::string modName, int behavior);

    static void UpdateSettings(RE::StaticFunctionTag*);

    static int UpdateLogLvl(RE::StaticFunctionTag*, int logLevel);
    static std::string ShowLogLocation(RE::StaticFunctionTag*);
    static std::string GetErrDscr(RE::StaticFunctionTag*, int errCode);

    static std::string WhyProblem(RE::StaticFunctionTag* tag, RE::Actor* actor, int issueID);

    enum eIssueID { iidNone, iidCanSee, iidCannotSee, iidCanSeeRep };
};
