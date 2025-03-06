#pragma once

class Events : public Singleton<Events>,
               public RE::BSTEventSink<RE::TESObjectLoadedEvent>,
               public RE::BSTEventSink<RE::TESEquipEvent>,
               public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
  public:
    void RegisterEvents();

    friend class Papyrus;

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override;

  private:
    void DoChecks(RE::Actor* actor, RE::TESObjectARMO* armor = nullptr, bool isEquipped = false);
    std::pair<int, bool> GetNPCAutoAddon(RE::TESNPC* npc);
    void CheckCovering(RE::Actor* actor, RE::TESObjectARMO* armor, bool isEquipped);
    RE::TESObjectARMO* GetCoveringItem(RE::Actor* actor, RE::TESObjectARMO* armor);
    bool NeedsCover(RE::Actor* actor);
    RE::TESBoundObject* ForceTngCover(RE::Actor* actor, bool ifUpdate);
    void CheckForAddons(RE::Actor* actor);

    bool showErrMessage;
    std::map<RE::FormID, RE::TESObjectARMO*> oldSkins;
    std::vector<RE::BGSKeyword*> coverKeys;
};
