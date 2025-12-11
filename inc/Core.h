#pragma once

#include <Inis.h>
#include <TNGUtil.h>
namespace TNG {
  struct AddonInfo {
      std::string name{""};
      std::vector<RE::TESObjectARMO*> records{};
      bool isActive{false};
      bool isGenderChanger{false};
  };

  struct RaceGroupInfo {
      std::string name{""};
      std::string file{""};
      RE::TESRace* armorRace = nullptr;
      bool isMain{false};
      std::vector<RE::TESRace*> races{};
      bool noMCM{false};
      float mult = {1.0f};
      int defAddonIdx{errInt};
      int addonIdx{errInt};
      std::map<size_t, RE::TESObjectARMO*> malAddons{};
      std::map<size_t, RE::TESObjectARMO*> femAddons{};
  };

  struct RgKey {
      RE::TESRace* race = nullptr;
      int index = -1;
      bool onlyMCM = true;
      explicit RgKey(RE::TESRace* r) : race(r), index(-1) {}                           // by race
      explicit RgKey(size_t i, const bool b) : race(nullptr), index(i), onlyMCM(b) {}  // by index
  };

  class Core : public Singleton<Core>, public Inis {
    public:
      void Process();

      std::vector<AddonInfo>& GenderAddons(const bool isFemale) { return isFemale ? allFemAddons : allMalAddons; };

      RaceGroupInfo* Rg(const RgKey& ky);
      const RaceGroupInfo* Rg(const RgKey& ky) const;
      bool RgIsMain(RgKey rgChoice) const {
        auto rg = Rg(rgChoice);
        return rg && rg->isMain;
      };
      int GetRgAddon(RgKey rgChoice) const;
      void SetRgAddon(RgKey rgChoice, const int addonIdx);
      float GetRgMult(RgKey rgChoice) const;
      void SetRgMult(RgKey rgChoice, const float mult);
      const std::string GetRgInfo(RgKey rgChoice) const;
      std::vector<size_t> GetRgAddons(RgKey rgChoice) const;
      bool ReevaluateRace(RE::TESRace* const race, RE::Actor* const actor);

      eRes CanModifyActor(RE::Actor* const actor) const;
      void UpdateActor(RE::Actor* const actor, RE::TESObjectARMO* const armor = nullptr, const bool isEquipped = false);
      std::vector<size_t> GetActorAddons(RE::Actor* const actor, const bool onlyActive) const;
      eRes GetActorAddon(RE::Actor* actor, int& addon, bool& isAuto) const;
      eRes SetActorAddon(RE::Actor* const actor, const int choice, const bool isUser, const bool shouldSave);
      eRes GetActorSize(RE::Actor* const actor, int& sizeCat) const;
      eRes SetActorSize(RE::Actor* const actor, int sizeCat, const bool shouldSave);
      void UpdatePlayerAfterLoad();

      bool SwapRevealing(RE::Actor* const actor, RE::TESObjectARMO* const armor);
      void RevisitRevealingArmor() const;

    private:
      inline static constexpr size_t hardCodedCoveringCount{1};
      inline static constexpr SEFormLocView hardCodedCovering[hardCodedCoveringCount]{{0x3D306, "Dragonborn.esm"}};
      inline static constexpr size_t hardCodedRacesCount{1};
      inline static constexpr SEFormLocView hardCodedRaces[hardCodedRacesCount]{{0x3CA97, "Dragonborn.esm"}};
      std::vector<RaceGroupInfo> rgInfoList;
      std::vector<AddonInfo> allMalAddons;
      std::vector<AddonInfo> allFemAddons;

      void LoadAddons();
      int AddonIdxByLoc(const bool isFemale, const SEFormLocView addonLoc) const;

      void ProcessRaces();
      void IgnoreRace(RE::TESRace* const race, const bool ready);
      RaceGroupInfo* const ProcessRace(RE::TESRace* const race);
      eRes CheckRace(RE::TESRace* const race) const;
      RaceGroupInfo* AddRace(RE::TESRace* const race, const bool isProcessed);
      int GetRgDefAddon(RaceGroupInfo& rg);
      void ProcessRgAddons(RaceGroupInfo& rg, const std::vector<std::pair<RE::TESObjectARMO*, bool>>& addons, const bool isFemale);
      void ApplyUserSettings(RaceGroupInfo& rg);
      void ProcessNPCs();
      std::pair<int, bool> GetApplicableAddon(RE::Actor* const actor) const;
      eRes SetNPCAddon(RE::TESNPC* const npc, const int addonIdx, const bool isUser);
      void OrganizeNPCKeywords(RE::TESNPC* const npc, int addonIdx, const bool isUser) const;
      void UpdateAddon(RE::Actor* const actor, const bool isRRace);
      eRes UpdatePlayer(RE::Actor* const actor, const bool isRRace);
      void UpdateFormLists(RE::Actor* const actor) const;
      void CheckArmorPieces();
  };
}
extern TNG::Core* core;
