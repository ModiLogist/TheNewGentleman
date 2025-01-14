#pragma once

class Core : public Singleton<Core> {
  public:
    static void GenitalizeRaces();
    static bool SetRgAddn(const size_t rgChoice, const int addnIdx);

  private:
    static bool IgnoreRace(RE::TESRace* race);
    static bool CheckRace(RE::TESRace* race);
    static Tng::TNGRes AddRace(RE::TESRace* race);
    // static RE::TESObjectARMO* ProduceAddonSkin(RE::TESObjectARMO* aOgSkin, int aAddonChoice, bool isFemale) ;

  public:
    static void GenitalizeNPCSkins();
    static Tng::TNGRes CanModifyActor(RE::Actor* actor);
    static Tng::TNGRes SetActorSize(RE::Actor* actor, int genSize);
    static Tng::TNGRes SetNPCAddn(RE::TESNPC* npc, int addnIdx, bool isUser);
    static std::vector<RE::TESObjectARMO*> GetActorWornArmor(RE::Actor* actor);

  private:
    static RE::TESObjectARMO* FixSkin(RE::TESObjectARMO* skin, RE::TESRace* race, const char* const aName);

  public:
    static void CheckOutfits();
    static void CheckArmorPieces();
    static void RevisitRevealingArmor();
    static bool SwapRevealing(RE::TESObjectARMO* armor);
};
