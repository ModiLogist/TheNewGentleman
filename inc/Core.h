#pragma once

#include <Inis.h>
#include <Util.h>

class Core : public Singleton<Core>, public Inis {
  public:
    void Init();

  private:
    std::vector<std::pair<RE::TESObjectARMO* const, bool>> malAddons;
    std::vector<std::pair<RE::TESObjectARMO* const, bool>> femAddons;
    std::set<RE::TESObjectARMO* const, Common::FormComparator> preSkins;
    void LoadAddons();
    // int AddonIdxByLoc(bool isFemale, SEFormLocView addonLoc);

  public:
    RE::TESObjectARMO* GetAddonByIdx(const bool isFemale, const size_t choice, const bool onlyActive) const;
    size_t GetAddonCount(const bool isFemale, const bool onlyActive) const;
    bool GetAddonStatus(const bool isFemale, const size_t addonIdx) const;
    void SetAddonStatus(const bool isFemale, const size_t addonIdx, const bool status);
    const std::string GetAddonName(const bool isFemale, const size_t addonIdx, const bool onlyActive) const;

  private:
    int GetAddonIdxByLoc(const bool isFemale, const SEFormLocView addonLoc) const;

  public:
    void ProcessRaces();
    void SetRgAddon(const size_t rgChoice, const int addonIdx, const bool onlyMCM);
    void SetRgMult(const size_t rgChoice, const float mult, const bool onlyMCM);
    const std::string GetRgInfo(const size_t rgChoice, const bool onlyMCM);
    std::vector<size_t> GetRgAddonList(const size_t rgChoice, bool isFemale, bool onlyMCM, bool onlyActive);
    std::vector<size_t> GetRgAddonList(RE::TESRace* const race, bool isFemale, bool onlyActive);

  private:
    std::vector<Common::RaceGroupInfo> rgInfoList;
    std::map<RE::TESRace* const, Common::RaceGroupInfo* const> raceRgs;
    std::map<RE::TESObjectARMO* const, RE::TESObjectARMO* const> ogSkinMap;
    Common::RaceGroupInfo* GetRg(const size_t rgIdx, const bool onlyMCM);
    void IgnoreRace(RE::TESRace* const race, bool ready);
    Common::RaceGroupInfo* ProcessRace(RE::TESRace* const race);
    Common::eRes CheckRace(RE::TESRace* const race);
    Common::RaceGroupInfo* AddRace(RE::TESRace* const race, const bool isProcessed);
    int GetRgDefAddon(Common::RaceGroupInfo& rg);
    void ProcessRgAddons(Common::RaceGroupInfo& rg, const std::vector<std::pair<RE::TESObjectARMO* const, bool>>& addons, const bool isFemale);
    void ApplyUserSettings(Common::RaceGroupInfo& rg);
    bool ReevaluateRace(RE::TESRace* const race, RE::Actor* const actor);
    RE::TESObjectARMO* GetSkinWithAddonForRg(Common::RaceGroupInfo* const rg, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);
    RE::TESObjectARMO* GetOgSkin(RE::TESObjectARMO* skin) const;

  public:
    void ProcessNPCs();
    Common::eRes CanModifyNPC(RE::TESNPC* const npc);
    Common::eRes GetActorAddon(RE::Actor* actor, RE::TESObjectARMO*& addon);
    Common::eRes SetActorAddon(RE::Actor* const actor, const int choice, const bool isUser, const bool shouldSave = true);
    Common::eRes GetActorSize(RE::Actor* const actor, int& sizeCat);
    Common::eRes SetActorSize(RE::Actor* const actor, int sizeCat, bool shouldSave = true);
    void UpdateFormLists(RE::Actor* const actor, RE::TESNPC* const npc);
    std::vector<RE::TESObjectARMO*> GetActorWornArmor(RE::Actor* const actor);

  private:
    RE::TESObjectARMO* FixSkin(RE::TESObjectARMO* const skin, RE::TESRace* const race, const char* const name);
    void ApplyUserSettings(RE::TESNPC* npc);
    Common::eRes SetNPCAddon(RE::TESNPC* const npc, const int addonIdx, const bool isUser);
    void OrganizeNPCAddonKeywords(RE::TESNPC* const npc, int addonIdx, bool isUser);
    Common::eRes UpdatePlayer(RE::Actor* const actor);

  public:
    void CheckArmorPieces();
    bool SwapRevealing(RE::Actor* const actor, RE::TESObjectARMO* const armor);

  private:
    inline static constexpr size_t hardCodedCoveringCount{1};
    inline static constexpr SEFormLocView hardCodedCovering[hardCodedCoveringCount]{{0x3D306, "Dragonborn.esm"}};
    inline static constexpr size_t hardCodedRacesCount{1};
    inline static constexpr SEFormLocView hardCodedRaces[hardCodedRacesCount]{{0x3CA97, "Dragonborn.esm"}};
    void RevisitRevealingArmor();
};

extern Core* core;