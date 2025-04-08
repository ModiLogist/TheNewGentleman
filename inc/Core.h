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
    size_t GetAddonCount(const bool isFemale, const bool onlyActive) const;
    bool GetAddonStatus(const bool isFemale, const size_t addonIdx) const;
    void SetAddonStatus(const bool isFemale, const size_t addonIdx, const bool status);
    const std::string GetAddonName(const bool isFemale, const size_t addonIdx) const;
    RE::TESObjectARMO* const GetAddonForActor(RE::Actor* const actor, const int addonIdx) const;

  private:
    int GetAddonIdxByLoc(const bool isFemale, const SEFormLocView addonLoc) const;

  public:
    struct RgKey {
        RE::TESRace* race = nullptr;
        int index = -1;
        bool onlyMCM = true;
        explicit RgKey(RE::TESRace* r) : race(r), index(-1) {}                     // by race
        explicit RgKey(size_t i, bool b) : race(nullptr), index(i), onlyMCM(b) {}  // by index
        Common::RaceGroupInfo* Get() const;
    };
    void ProcessRaces();
    std::vector<std::string> GetRgNames() const;
    bool RgIsMain(RgKey rgChoice) const;
    int GetRgAddon(RgKey rgChoice) const;
    void SetRgAddon(RgKey rgChoice, const int addonIdx);
    float GetRgMult(RgKey rgChoice) const;
    void SetRgMult(RgKey rgChoice, const float mult);
    const std::string GetRgInfo(RgKey rgChoice) const;
    std::vector<std::pair<size_t, bool>> GetRgAddons(RgKey rgChoice) const;
    bool ReevaluateRace(RE::TESRace* const race, RE::Actor* const actor);

  private:
    std::vector<Common::RaceGroupInfo> rgInfoList;
    std::map<RE::TESRace* const, Common::RaceGroupInfo* const> raceRgs;
    std::map<RE::TESObjectARMO* const, RE::TESObjectARMO* const> ogSkinMap;
    void IgnoreRace(RE::TESRace* const race, bool ready);
    Common::RaceGroupInfo* const ProcessRace(RE::TESRace* const race);
    Common::eRes CheckRace(RE::TESRace* const race);
    Common::RaceGroupInfo* AddRace(RE::TESRace* const race, const bool isProcessed);
    int GetRgDefAddon(Common::RaceGroupInfo& rg);
    void ProcessRgAddons(Common::RaceGroupInfo& rg, const std::vector<std::pair<RE::TESObjectARMO* const, bool>>& addons, const bool isFemale);
    void ApplyUserSettings(Common::RaceGroupInfo& rg);
    RE::TESObjectARMO* GetSkinWithAddonForRg(Common::RaceGroupInfo* const rg, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);
    RE::TESObjectARMO* GetOgSkin(RE::TESObjectARMO* skin) const;

  public:
    void ProcessNPCs();
    Common::eRes CanModifyActor(RE::Actor* const actor) const;
    void UpdateActor(RE::Actor* const actor, RE::TESObjectARMO* const armor = nullptr, const bool isEquipped = false);
    std::vector<std::pair<size_t, bool>> GetActorAddons(RE::Actor* const actor, const bool onlyActive) const;
    Common::eRes GetActorAddon(RE::Actor* actor, int& addon, bool& isAuto) const;
    Common::eRes SetActorAddon(RE::Actor* const actor, const int choice, const bool isUser, const bool shouldSave);
    Common::eRes GetActorSize(RE::Actor* const actor, int& sizeCat) const;
    Common::eRes SetActorSize(RE::Actor* const actor, int sizeCat, bool shouldSave);
    void UpdateFormLists(RE::Actor* const actor, RE::TESNPC* const npc) const;

  private:
    RE::TESObjectARMO* FixSkin(RE::TESObjectARMO* const skin, RE::TESRace* const race, const char* const name);
    void ApplyUserSettings(RE::TESNPC* npc);
    void UpdateAddon(RE::Actor* const actor);
    void UpdateCover(RE::Actor* const actor, RE::TESObjectARMO* const armor, const bool isEquipped) const;
    std::pair<int, bool> GetApplicableAddon(RE::Actor* const actor) const;
    Common::eRes SetNPCAddon(RE::TESNPC* const npc, const int addonIdx, const bool isUser);
    void OrganizeNPCAddonKeywords(RE::TESNPC* const npc, int addonIdx, bool isUser) const;
    Common::eRes UpdatePlayer(RE::Actor* const actor);

  public:
    void CheckArmorPieces();
    bool SwapRevealing(RE::Actor* const actor, RE::TESObjectARMO* const armor);
    void RevisitRevealingArmor() const;

  private:
    inline static constexpr size_t hardCodedCoveringCount{1};
    inline static constexpr SEFormLocView hardCodedCovering[hardCodedCoveringCount]{{0x3D306, "Dragonborn.esm"}};
    inline static constexpr size_t hardCodedRacesCount{1};
    inline static constexpr SEFormLocView hardCodedRaces[hardCodedRacesCount]{{0x3CA97, "Dragonborn.esm"}};
};

extern Core* core;
