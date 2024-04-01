#pragma once

class TngHooks : public Singleton<TngHooks> {
  public:
    static void Install();

  private:
    static void InitWornArmor(RE::TESObjectARMO* aArmor, RE::Actor* aActor, RE::BSTSmartPointer<RE::BipedAnim>* aBiped);
};
