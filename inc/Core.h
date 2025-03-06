#pragma once

class Core : public Singleton<Core> {
  public:
    void GenitalizeRaces();
    bool SetRgAddon(const size_t rgChoice, const int addnIdx, const bool onlyMCM);

  private:
    void IgnoreRace(RE::TESRace* race, bool ready);
    Tng::TNGRes AddPotentialRace(RE::TESRace* race, const std::set<std::string>& validSkeletons);

  public:
    void GenitalizeNPCSkins();
    Tng::TNGRes CanModifyNPC(RE::TESNPC* npc);
    Tng::TNGRes SetActorSize(RE::Actor* actor, int genSize);
    Tng::TNGRes SetNPCAddon(RE::TESNPC* npc, const int addnIdx, const bool isUser, const bool shouldSave = true);
    std::vector<RE::TESObjectARMO*> GetActorWornArmor(RE::Actor* actor);

  private:
    RE::TESObjectARMO* FixSkin(RE::TESObjectARMO* skin, RE::TESRace* race, const char* const name);

  public:
    void CheckArmorPieces();
    void RevisitRevealingArmor();
    bool SwapRevealing(RE::Actor* actor, RE::TESObjectARMO* armor);

  private:
    inline static constexpr size_t hardCodedCoveringCount{1};
    inline static constexpr SEFormLocView hardCodedCovering[hardCodedCoveringCount]{{0x3D306, "Dragonborn.esm"}};
    inline static constexpr size_t hardCodedRacesCount{1};
    inline static constexpr SEFormLocView hardCodedRaces[hardCodedRacesCount]{{0x3CA97, "Dragonborn.esm"}};

  public:
    void SetBoolSetting(Tng::BoolSetting settingID, bool value);
    void SetAddonStatus(const bool isFemale, const int addnIdx, const bool status);
    void SetRgMult(const size_t rgChoice, const float mult, bool onlyMCM);
};
