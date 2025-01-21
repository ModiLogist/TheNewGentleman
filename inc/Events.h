#pragma once

class Events : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent>, public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
  public:
    static void RegisterEvents();

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* event, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override;

  public:
    static void DoChecks(RE::Actor* actor, RE::TESObjectARMO* armor = nullptr, bool isEquipped = false);
    static int GetNPCAutoAddon(RE::TESNPC* npc);

  private:
    static void CheckCovering(RE::Actor* actor, RE::TESObjectARMO* armor, bool isEquipped);
    static RE::TESObjectARMO* GetCoveringItem(RE::Actor* actor, RE::TESObjectARMO* armor);
    static bool NeedsCover(RE::Actor* actor);
    static RE::TESBoundObject* ForceTngCover(RE::Actor* actor, bool ifUpdate);
    static void CheckForAddons(RE::Actor* actor);

    

    inline static bool showErrMessage;
    inline static std::map<RE::FormID, RE::TESObjectARMO*> oldSkins;
    inline static std::vector<RE::BGSKeyword*> coverKeys;

    Events() = default;
    Events(const Events&) = delete;
    Events(Events&&) = delete;

    ~Events() override = default;

    Events& operator=(const Events&) = delete;
    Events& operator=(Events&&) = delete;

    static Events* GetSingleton() {
      static Events singleton;
      return &singleton;
    }
};
