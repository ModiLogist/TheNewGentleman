#pragma once

class Core : public Singleton<Core> {
  public:
    static void GenitalizeRaces();
    static bool SetRgAddon(const size_t rgChoice, const int addnIdx);

  private:
    static bool IgnoreRace(RE::TESRace* race);
    static bool CheckRace(RE::TESRace* race);
    static Tng::TNGRes AddRace(RE::TESRace* race);
    // static RE::TESObjectARMO* ProduceAddonSkin(RE::TESObjectARMO* aOgSkin, int aAddonChoice, bool isFemale) ;

  public:
    static void GenitalizeNPCSkins();
    static Tng::TNGRes CanModifyNPC(RE::TESNPC* npc);
    static Tng::TNGRes SetActorSize(RE::Actor* actor, int genSize);
    static Tng::TNGRes SetNPCAddon(RE::TESNPC* npc, int addnIdx, bool isUser);
    static std::vector<RE::TESObjectARMO*> GetActorWornArmor(RE::Actor* actor);

  private:
    static RE::TESObjectARMO* FixSkin(RE::TESObjectARMO* skin, RE::TESRace* race, const char* const aName);

  public:
    static void CheckArmorPieces();
    static void RevisitRevealingArmor();
    static bool SwapRevealing(RE::Actor* actor, RE::TESObjectARMO* armor);

  private:
    inline static constexpr size_t hardCodedCoveringCount{1};
    inline static constexpr SEFormLocView hardCodedCovering[hardCodedCoveringCount]{{0x3D306, "Dragonborn.esm"}};
    inline static constexpr size_t hardCodedRacesCount{1};
    inline static constexpr SEFormLocView hardCodedRaces[hardCodedRacesCount]{{0x67CD8, Tng::cSkyrim}};
};
