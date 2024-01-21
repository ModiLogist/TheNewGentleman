#pragma once

class TngEvents : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent> {
  private:
    RE::TESDataHandler* fDH = RE::TESDataHandler::GetSingleton();
    inline static RE::BGSKeyword* fPRaceKey;
    inline static RE::BGSKeyword* fCCKey;
    inline static RE::BGSKeyword* fACKey;
    inline static RE::BGSKeyword* fARKey;
    inline static RE::BGSKeyword* fRRKey;
    inline static RE::BGSKeyword* fPAKey;
    inline static RE::BGSKeyword* fIAKey;
    inline static RE::BGSKeyword* fUAKey;
    inline static RE::BGSKeyword* fGWKey;
    inline static RE::BGSKeyword* fPSKey;
    inline static RE::TESGlobal* fGWChance;
    inline static RE::BGSListForm* fGentified;

  public:
    static TngEvents* GetSingleton() {
      static TngEvents aSingleton;
      return &aSingleton;
    }

    static void RegisterEvents() noexcept;

  private:
    TngEvents() = default;
    TngEvents(const TngEvents&) = delete;
    TngEvents(TngEvents&&) = delete;

    ~TngEvents() override = default;

    TngEvents& operator=(const TngEvents&) = delete;
    TngEvents& operator=(TngEvents&&) = delete;


    inline static bool fInternal;

    static void CheckForRevealing(RE::TESObjectARMO* aBodyArmor, RE::TESObjectARMO* aPelvisArmor) noexcept;
    static void CheckForClipping(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept;
    static void CheckActor(RE::Actor* aActor, RE::TESObjectARMO* aArmor = nullptr) noexcept;
    static void CheckGentlewomen(RE::Actor* aActor) noexcept;

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;
};