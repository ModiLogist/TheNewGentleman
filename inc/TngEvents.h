#pragma once

class TngEvents : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent> {
  private:
    TngEvents() = default;
    TngEvents(const TngEvents&) = delete;
    TngEvents(TngEvents&&) = delete;

    ~TngEvents() override = default;

    TngEvents& operator=(const TngEvents&) = delete;
    TngEvents& operator=(TngEvents&&) = delete;

    inline static constexpr RE::FormID cCoverID{0xA00};

    inline static RE::ActorEquipManager* fEquipManager{nullptr};
    inline static RE::TESObjectARMO* fCover{nullptr};
    inline static RE::BGSKeyword* fNPCKey{nullptr};

    inline static std::set<RE::Actor*> fWaitingActors;

    static bool IsCovering(const RE::TESBoundObject* aItem, const std::unique_ptr<RE::InventoryEntryData>& aEntry) noexcept;
    static void ProcessActor(RE::Actor* aActor) noexcept;

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* aEvent, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* aEvent, RE::BSTEventSource<RE::TESEquipEvent>*) override;

  public:
    static TngEvents* GetSingleton() {
      static TngEvents aSingleton;
      return &aSingleton;
    }

    static void RegisterEvents() noexcept;
};