#pragma once

class TngEvents : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent> {
  private:
    TngEvents() = default;
    TngEvents(const TngEvents&) = delete;
    TngEvents(TngEvents&&) = delete;

    ~TngEvents() override = default;

    TngEvents& operator=(const TngEvents&) = delete;
    TngEvents& operator=(TngEvents&&) = delete;

    inline static RE::ActorEquipManager* fEquipManager{nullptr};
    inline static RE::BGSKeyword* fNPCKey{nullptr};
    inline static RE::BGSKeyword* fRevealingKey{nullptr};
    inline static RE::BGSKeyword* fUnderwearKey{nullptr};
    inline static RE::BGSKeyword* fCoveringKey{nullptr};
    inline static RE::TESObjectARMO* fCover{nullptr};
    inline static bool fInternal;
    

    static void MakeArmorRevealing(RE::TESObjectARMO* aArmo) noexcept;
    static void CheckActor(RE::Actor* aActor) noexcept;
    static void CoverActor(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept;

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;

  public:
    static TngEvents* GetSingleton() {
      static TngEvents aSingleton;
      return &aSingleton;
    }

    static void RegisterEvents() noexcept;
};