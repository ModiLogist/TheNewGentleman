#pragma once

class Core : public Singleton<Core> {
  public:
    void GenitalizeRaces();
    bool SetRgAddon(const size_t rgChoice, const int addnIdx, const bool onlyMCM);

  private:
    void IgnoreRace(RE::TESRace* race, bool ready);
    Util::TNGRes AddPotentialRace(RE::TESRace* race, const std::set<std::string>& validSkeletons);

  public:
    void GenitalizeNPCSkins();
    Util::TNGRes CanModifyNPC(RE::TESNPC* npc);
    Util::TNGRes SetActorSize(RE::Actor* actor, int genSize);
    Util::TNGRes SetNPCAddon(RE::TESNPC* npc, const int addnIdx, const bool isUser, const bool shouldSave = true);
    void UpdateFormLists(RE::Actor* actor, RE::TESNPC* npc);
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
    void SetBoolSetting(Util::BoolSetting settingID, bool value);
    void SetAddonStatus(const bool isFemale, const int addnIdx, const bool status);
    void SetRgMult(const size_t rgChoice, const float mult, bool onlyMCM);
};
