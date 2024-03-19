#pragma once

class TngHooks : public Singleton<TngHooks> {
  public:
    static void Install();

  private:
    static void InitWornArmor(RE::TESObjectARMO* a_armor, RE::Actor* a_actor, RE::BSTSmartPointer<RE::BipedAnim>* a_biped);
};
