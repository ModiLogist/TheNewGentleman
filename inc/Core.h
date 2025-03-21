#pragma once

#include <Util.h>

class Core : public Singleton<Core> {
  public:
    void ProcessRaces();
    bool SetRgAddon(const size_t rgChoice, const int addnIdx, const bool onlyMCM);

  private:
    void IgnoreRace(RE::TESRace* race, bool ready);
    Util::eRes AddPotentialRace(RE::TESRace* race, const std::set<std::string>& validSkeletons);

  public:
    void ProcessSkins();
    Util::eRes CanModifyNPC(RE::TESNPC* npc);
    Util::eRes SetActorSize(RE::Actor* actor, int genSize);
    Util::eRes SetNPCAddon(RE::TESNPC* npc, const int addnIdx, const bool isUser, const bool shouldSave = true);
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
    void SetBoolSetting(Util::eBoolSetting settingID, bool value);
    void SetIntSetting(Util::eIntSetting settingID, int value);
    void SetFloatSetting(Util::eFloatSetting settingID, float value);
    void SetAddonStatus(const bool isFemale, const int addnIdx, const bool status);
    void SetRgMult(const size_t rgChoice, const float mult, bool onlyMCM);
};
