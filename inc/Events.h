#pragma once

class Events : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent>, public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
  public:
    static void RegisterEvents();
    static void SetPlayerInfo(RE::Actor* aPlayer, const int addnIdx);

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* aEvent, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override;

  private:
    static void CheckForAddons(RE::Actor* actor);
    static int GetNPCAutoAddn(RE::TESNPC* npc);
    static void CheckCovering(RE::Actor* actor, RE::TESObjectARMO* armor = nullptr, bool isUnequipped = false);
    static RE::TESObjectARMO* GetCoveringItem(RE::Actor* actor, RE::TESObjectARMO* armor);
    static RE::TESBoundObject* ForceTngCover(RE::Actor* actor, bool ifUpdate);
    inline static bool showErrMessage;
    

    inline static std::map<RE::FormID, RE::TESObjectARMO*> oldSkins;
    inline static std::vector<RE::BGSKeyword*> coverKeys;

    struct PlayerInfo {
        inline static bool isFemale;
        inline static RE::TESRace* race;
        inline static bool isInfoSet;
    };
    inline static PlayerInfo playerInfo{};

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
