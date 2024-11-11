#pragma once

class Events : public RE::BSTEventSink<RE::TESObjectLoadedEvent>,
                  public RE::BSTEventSink<RE::TESEquipEvent>,
                  public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
  public:
    static void RegisterEvents() noexcept;
    static void SetPlayerInfo(RE::Actor* aPlayer, const int aAddon) noexcept;

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* aEvent, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override;

  private:
    static void CheckForAddons(RE::Actor* aActor) noexcept;
    static int GetNPCAutoAddn(RE::TESNPC* aNPC) noexcept;
    static void CheckActorArmor(RE::Actor* aActor, RE::TESObjectARMO* aArmor = nullptr) noexcept;

    RE::TESDataHandler* Tng::SEDH();
    inline static RE::BGSKeyword* fPRaceKey;
    inline static RE::BGSKeyword* fCCKey;
    inline static RE::BGSKeyword* fACKey;
    inline static RE::BGSKeyword* fARKey;
    inline static RE::BGSKeyword* fRRKey;
    inline static RE::BGSKeyword* fUAKey;
    inline static RE::BGSKeyword* fPSKey;
    inline static RE::BGSKeyword* Tng::NexKey();
    inline static RE::BGSKeyword* fGenSkinKey;
    inline static RE::TESGlobal* fGWChance;
    inline static RE::TESGlobal* fPCAddon;    

    inline static std::map<RE::FormID, RE::TESObjectARMO*> fOldSkins;    

    inline static bool fIsPlayerFemale;
    inline static RE::TESRace* fPlayerRace;
    inline static bool fPlayerInfoSet;

    Events() = default;
    Events(const Events&) = delete;
    Events(Events&&) = delete;

    ~Events() override = default;

    Events& operator=(const Events&) = delete;
    Events& operator=(Events&&) = delete;

    static Events* GetSingleton() {
      static Events aSingleton;
      return &aSingleton;
    }
};
