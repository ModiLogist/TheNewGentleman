#pragma once

class Events : public Singleton<Events>,
               public RE::BSTEventSink<RE::TESObjectLoadedEvent>,
               public RE::BSTEventSink<RE::TESEquipEvent>,
               public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
  public:
    void RegisterEvents();

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override;

  public:
    void DoChecks(RE::Actor* actor, RE::TESObjectARMO* armor = nullptr, bool isEquipped = false);
    RE::TESObjectARMO* GetCoveringItem(RE::Actor* actor, RE::TESObjectARMO* exception);

  private:
    void CheckForAddons(RE::Actor* actor);
    void CheckCovering(RE::Actor* actor, RE::TESObjectARMO* armor, bool isEquipped);
    void CheckDF(RE::Actor* actor);
    std::pair<int, bool> GetNPCAutoAddon(RE::TESNPC* npc);
    bool NeedsCover(RE::Actor* actor);

    bool showErrMessage;
    std::map<RE::FormID, RE::TESObjectARMO*> oldSkins;
};
