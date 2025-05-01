#pragma once

#include <Inis.h>
#include <Util.h>

class Core : public Singleton<Core>, public Inis {
  public:
    void Process();
    std::vector<std::pair<RE::TESObjectARMO*, bool>>& GenderAddons(const bool isFemale) { return isFemale ? femAddons : malAddons; };
    struct RgKey {
        RE::TESRace* race = nullptr;
        int index = -1;
        bool onlyMCM = true;
        explicit RgKey(RE::TESRace* r) : race(r), index(-1) {}                           // by race
        explicit RgKey(size_t i, const bool b) : race(nullptr), index(i), onlyMCM(b) {}  // by index
    };

    Common::RaceGroupInfo* Rg(const RgKey& ky);
    const Common::RaceGroupInfo* Rg(const RgKey& ky) const;
    bool RgIsMain(RgKey rgChoice) const {
      auto rg = Rg(rgChoice);
      return rg && rg->isMain;
    };
    int GetRgAddon(RgKey rgChoice) const;
    void SetRgAddon(RgKey rgChoice, const int addonIdx);
    float GetRgMult(RgKey rgChoice) const;
    void SetRgMult(RgKey rgChoice, const float mult);
    const std::string GetRgInfo(RgKey rgChoice) const;
    std::vector<std::pair<size_t, bool>> GetRgAddons(RgKey rgChoice) const;
    bool ReevaluateRace(RE::TESRace* const race, RE::Actor* const actor);

    Common::eRes CanModifyActor(RE::Actor* const actor) const;
    void UpdateActor(RE::Actor* const actor, RE::TESObjectARMO* const armor = nullptr, const bool isEquipped = false);
    std::vector<std::pair<size_t, bool>> GetActorAddons(RE::Actor* const actor, const bool onlyActive) const;
    Common::eRes GetActorAddon(RE::Actor* actor, int& addon, bool& isAuto) const;
    Common::eRes SetActorAddon(RE::Actor* const actor, const int choice, const bool isUser, const bool shouldSave);
    Common::eRes GetActorSize(RE::Actor* const actor, int& sizeCat) const;
    Common::eRes SetActorSize(RE::Actor* const actor, int sizeCat, const bool shouldSave);
    void UpdatePlayerAfterLoad();

    bool SwapRevealing(RE::Actor* const actor, RE::TESObjectARMO* const armor);
    void RevisitRevealingArmor() const;

  private:
    inline static constexpr size_t hardCodedCoveringCount{1};
    inline static constexpr SEFormLocView hardCodedCovering[hardCodedCoveringCount]{{0x3D306, "Dragonborn.esm"}};
    inline static constexpr size_t hardCodedRacesCount{1};
    inline static constexpr SEFormLocView hardCodedRaces[hardCodedRacesCount]{{0x3CA97, "Dragonborn.esm"}};
    std::vector<Common::RaceGroupInfo> rgInfoList;
    std::map<RE::TESObjectARMO*, RE::TESObjectARMO*> ogSkinMap;
    std::vector<std::pair<RE::TESObjectARMO*, bool>> malAddons;
    std::vector<std::pair<RE::TESObjectARMO*, bool>> femAddons;
    std::set<RE::TESObjectARMO*, Common::FormComparator> preSkins;

    void LoadAddons();
    int AddonIdxByLoc(const bool isFemale, const SEFormLocView addonLoc) const;

    void ProcessRaces();
    void IgnoreRace(RE::TESRace* const race, const bool ready);
    Common::RaceGroupInfo* const ProcessRace(RE::TESRace* const race);
    Common::eRes CheckRace(RE::TESRace* const race) const;
    Common::RaceGroupInfo* AddRace(RE::TESRace* const race, const bool isProcessed);
    int GetRgDefAddon(Common::RaceGroupInfo& rg);
    void ProcessRgAddons(Common::RaceGroupInfo& rg, const std::vector<std::pair<RE::TESObjectARMO*, bool>>& addons, const bool isFemale);
    void ApplyUserSettings(Common::RaceGroupInfo& rg);
    RE::TESObjectARMO* GetSkinWithAddonForRg(Common::RaceGroupInfo* const rg, RE::TESObjectARMO* skin, const size_t addonIdx, const bool isFemale);
    RE::TESObjectARMO* GetOgSkin(RE::TESObjectARMO* skin) const;

    void ProcessNPCs();
    RE::TESObjectARMO* FixSkin(RE::TESObjectARMO* const skin, RE::TESRace* const race, const char* const name);
    std::pair<int, bool> GetApplicableAddon(RE::Actor* const actor) const;
    Common::eRes SetNPCAddon(RE::TESNPC* const npc, const int addonIdx, const bool isUser);
    void OrganizeNPCKeywords(RE::TESNPC* const npc, int addonIdx, const bool isUser) const;
    void UpdateAddon(RE::Actor* const actor, const bool isRRace);
    Common::eRes UpdatePlayer(RE::Actor* const actor, const bool isRRace);
    void UpdateFormLists(RE::Actor* const actor) const;
    void UpdateBlock(RE::Actor* const actor, RE::TESObjectARMO* const armor, const bool isEquipped) const;
    bool NeedsBlock(RE::Actor* const actor) const;
    void CheckArmorPieces();
};

extern Core* core;
