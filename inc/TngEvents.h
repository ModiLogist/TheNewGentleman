#pragma once

class TngEvents : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent> {
  private:
    TngEvents() = default;
    TngEvents(const TngEvents&) = delete;
    TngEvents(TngEvents&&) = delete;

    ~TngEvents() override = default;

    TngEvents& operator=(const TngEvents&) = delete;
    TngEvents& operator=(TngEvents&&) = delete;

    inline static constexpr RE::FormID cWomenChanceID{0xCA0};
    inline static constexpr RE::FormID cGentifiedID{0xE00};
    inline static constexpr RE::FormID cSkinWithPenisKeyID{0xFF7};
    inline static constexpr RE::FormID cGentleWomanKeyID{0xFF8};

    inline static RE::ActorEquipManager* fEquipManager{nullptr};
    inline static RE::BGSKeyword* fNPCKey{nullptr};
    inline static RE::BGSKeyword* fTNGRaceKey{nullptr};
    inline static RE::BGSKeyword* fAutoRvealKey{nullptr};
    inline static RE::BGSKeyword* fRevealingKey{nullptr};
    inline static RE::BGSKeyword* fUnderwearKey{nullptr};
    inline static RE::BGSKeyword* fAutoCoverKey{nullptr};
    inline static RE::BGSKeyword* fCoveringKey{nullptr};
    inline static RE::BGSKeyword* fGentleWomanKey{nullptr};
    inline static RE::BGSKeyword* fSkinWithPenisKey{nullptr};
    inline static RE::BGSListForm* fGentified{nullptr};
    inline static RE::TESGlobal* fWomenChance{nullptr};
    inline static bool fInternal;    
    
    static void CheckForRevealing(RE::TESObjectARMO* aBodyArmor, RE::TESObjectARMO* aPelvisArmor) noexcept;
    static void CheckForClipping(RE::Actor* aActor, RE::TESObjectARMO* aArmor) noexcept;
    static void CheckActor(RE::Actor* aActor, RE::TESObjectARMO* aArmor = nullptr) noexcept;
    static void CheckGentlewomen(RE::Actor* aActor) noexcept;

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