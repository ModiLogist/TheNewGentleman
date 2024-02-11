#pragma once

class TngEvents : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent>, public RE::BSTEventSink<RE::TESSwitchRaceCompleteEvent> {
  public:
    static void RegisterEvents() noexcept;

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESSwitchRaceCompleteEvent* aEvent, RE::BSTEventSource<RE::TESSwitchRaceCompleteEvent>*) override;

  private:
    static void CheckForRevealing(RE::TESObjectARMO* aBodyArmor, RE::TESObjectARMO* aPelvisArmor) noexcept;
    static void CheckForClipping(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept;
    static void CheckActor(RE::Actor* aActor, RE::TESObjectARMO* aArmor = nullptr) noexcept;
    static void CheckForAddons(RE::Actor* aActor) noexcept;

    RE::TESDataHandler* fDH;
    inline static RE::BGSKeyword* fPRaceKey;
    inline static RE::BGSKeyword* fCCKey;
    inline static RE::BGSKeyword* fACKey;
    inline static RE::BGSKeyword* fARKey;
    inline static RE::BGSKeyword* fRRKey;
    inline static RE::BGSKeyword* fUAKey;
    inline static RE::BGSKeyword* fGWKey;
    inline static RE::BGSKeyword* fPSKey;
    inline static RE::BGSKeyword* fExKey;
    inline static RE::BGSKeyword* fGenSkinKey;
    inline static RE::TESGlobal* fGWChance;
    inline static RE::BGSListForm* fGentified;

    inline static bool fInternal;
    inline static std::map<RE::FormID, RE::TESObjectARMO*> fOldSkins;

    TngEvents() = default;
    TngEvents(const TngEvents&) = delete;
    TngEvents(TngEvents&&) = delete;

    ~TngEvents() override = default;

    TngEvents& operator=(const TngEvents&) = delete;
    TngEvents& operator=(TngEvents&&) = delete;

    static TngEvents* GetSingleton() {
      static TngEvents aSingleton;
      return &aSingleton;
    }
};