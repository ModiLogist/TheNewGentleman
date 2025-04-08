#pragma once

#include <BaseUtil.h>

namespace Common {

  inline static constexpr std::string_view mainFile{"TheNewGentleman.esp"};
  inline static constexpr std::string_view sosRevealing{"SOS_Revealing"};

  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot bodySlot{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot genitalSlot{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

  inline static const char* cNPCAutoAddon{"TNG_ActorAddnAuto:"};
  inline static const char* cNPCUserAddon{"TNG_ActorAddnUser:"};

  inline static constexpr std::tuple<const char*, const char*, bool, RE::FormID> cVanillaDefaults[14]{
      {"default", "nord", false, 0xA01},        {"redguard", "yokudan", false, 0xA02}, {"breton", "reachmen", false, 0xA03}, {"cyrodi", "imperial", false, 0xA04},
      {"altmer", "highelf", false, 0xA03},      {"bosmer", "woodelf", false, 0xA01},   {"dunmer", "darkelf", false, 0xA04},  {"orsimer", "orc", false, 0xA02},
      {"saxhleel", "argonian", true, 0xA01},    {"khajiit", "rhat", true, 0xA05},      {"dremora", "dremora", false, 0xA04}, {"elder", "old", false, 0xA05},
      {"afflicted", "afflicted", false, 0xA03}, {"snowelf", "falmer", false, 0xA05}};
  enum eGenBones { egbBase, egbScrot, egbScrotL, egbScrotR, egbGen01, egbGen02, egbGen03, egbGen04, egbGen05, egbGen06, GenBonesCount };
  inline static const char* genBoneNames[GenBonesCount]{"NPC GenitalsBase [GenBase]",
                                                        "NPC GenitalsScrotum [GenScrot]",
                                                        "NPC L GenitalsScrotum [LGenScrot]",
                                                        "NPC R GenitalsScrotum [RGenScrot]",
                                                        "NPC Genitals01 [Gen01]",
                                                        "NPC Genitals02 [Gen02]",
                                                        "NPC Genitals03 [Gen03]",
                                                        "NPC Genitals04 [Gen04]",
                                                        "NPC Genitals05 [Gen05]",
                                                        "NPC Genitals06 [Gen06]"};

  inline static constexpr float fEpsilon{0.0001f};
  inline static constexpr float fErr{-1.0f};

  enum eRes {
    err40 = nan,
    errRaceBase = -9,
    errRg = -8,
    errSkeleton = -7,
    errPlayer = -6,
    errSkin = -5,
    errArmo = -4,
    errAddon = -3,
    errNPC = -2,
    errRace = -1,
    resOkFixed = 0,
    resOkSizable = 1,
    resOkNoAddon = 2,
    resOkHasAddon = 3,
    resOkRaceP = 10,
    resOkRaceR = 11,
    resOkRacePP = 12,
    resOkMain = 20,
    resOkDedicated = 21,
    resOkSupported = 22,
  };

  enum eRace { raceDefault, raceDefBeast, racesCount };

  enum eKeyword {
    kyProcessed,
    kyReady,
    kyPreProcessed,
    kyIgnored,
    kyExcluded,
    kyTngSkin,
    kySkinWP,
    kyGentlewoman,
    kyAddonM,
    kyAddonF,
    kyRevealingF,
    kyRevealingM,
    kyCovering,
    kyUnderwear,
    kyRevealing,
    kyPreSkin,
    kyManMer,
    kyBeast,
    kyCreature,
    kyVampire,
    keywordsCount
  };

  enum eTngFormLists { flmGentleWomen, flmNonGentleMen, flCount };

  enum eBoolSetting {
    bsExcludePlayerSize,
    bsCheckPlayerAddon,
    bsForceRechecks,
    bsRevealSlot52Mods,
    bsAllowMixSlot52Mods,
    bsRandomizeMaleAddon,
    bsUIExtensions,
    bsShowAllRaces,
    bsDAK,
    boolSettingCount
  };
  enum eIntSetting { isSetupNPC, isRiseGen, isFallGen, isSwapRevealing, isWhyProblem, intSettingCount };
  enum eFloatSetting { fsXS, fsS, fsM, fsL, fsXL, fsMalRndChance, fsFemRndChance, floatSettingCount };
  inline static constexpr int sizeCatCount{fsXL + 1};

  struct RaceGroupInfo {
      std::string name{""};
      std::string file{""};
      RE::TESRace* armorRace = nullptr;
      RE::TESObjectARMO* ogSkin = nullptr;
      bool isMain{false};
      std::vector<RE::TESRace*> races{};
      bool noMCM{false};
      float mult = {1.0f};
      int defAddonIdx{nan};
      int addonIdx{nan};
      std::map<size_t, std::pair<bool, RE::TESObjectARMA*>> malAddons{};
      std::map<size_t, std::pair<bool, RE::TESObjectARMA*>> femAddons{};
      std::map<RE::TESObjectARMO*, std::map<size_t, RE::TESObjectARMO*>> malSkins{};
      std::map<RE::TESObjectARMO*, std::map<size_t, RE::TESObjectARMO*>> femSkins{};
  };

  struct PlayerInfo {
      std::string name;
      bool isFemale;
      SEFormLoc race;
      SEFormLoc addon;
      int sizeCat = {def};
      bool isInfoSet;
  };

  class Util : public Singleton<Util>, public BaseUtil {
    public:
      RE::TESRace* Race(const size_t idx);
      RE::BGSKeyword* Key(const size_t idx);
      std::vector<RE::BGSKeyword*> Keys(const size_t first, const size_t last);
      RE::BGSKeyword* SizeKey(const size_t idx);
      std::vector<RE::BGSKeyword*> SizeKeys(const size_t last = sizeCatCount);
      RE::BGSListForm* FormList(const size_t idx);
      RE::TESGlobal* PCAddon();
      RE::TESObjectARMO* Block();
      bool IsBlock(RE::TESForm* form) const { return form && (FormToLoc(form) == coverID); }
      bool IsCovering(const RE::TESNPC* npc, const RE::TESObjectARMO* armor);
      std::vector<RE::TESObjectARMO*> GetActorWornArmor(RE::Actor* const actor) const;
      RE::TESObjectARMO* GetCoveringItem(RE::Actor* const actor, RE::TESObjectARMO* const exception);

    private:
      inline static constexpr SEFormLocView raceIDs[racesCount]{{0x19, skyrimFile}, {0x13745, skyrimFile}};
      inline static constexpr SEFormLocView keyIDs[keywordsCount] = {{0xFF0, mainFile}, {0xFF1, mainFile},     {0xFF2, mainFile},     {0xFF3, mainFile},     {0xFF4, mainFile},
                                                                     {0xFF6, mainFile}, {0xFF7, mainFile},     {0xFF8, mainFile},     {0xFF9, mainFile},     {0xFFA, mainFile},
                                                                     {0xFFB, mainFile}, {0xFFC, mainFile},     {0xFFD, mainFile},     {0xFFE, mainFile},     {0xFFF, mainFile},
                                                                     {0xFE0, mainFile}, {0x13794, skyrimFile}, {0xD61D1, skyrimFile}, {0x13795, skyrimFile}, {0xA82BB, skyrimFile}};

      inline static constexpr SEFormLocView pcAddonID{0xCFF, mainFile};
      inline static constexpr RE::FormID sizeKeyIDs[sizeCatCount]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};

      inline static constexpr SEFormLocView formListIDs[flCount] = {{0xE00, mainFile}, {0xE01, mainFile}};

      inline static constexpr SEFormLocView coverID{0xAFF, mainFile};

      RE::TESDataHandler* sedh = nullptr;
      RE::TESRace* races[racesCount] = {};
      RE::BGSKeyword* keywords[keywordsCount] = {};
      RE::BGSKeyword* sizeKey[sizeCatCount] = {};
      RE::TESGlobal* pcAddon = nullptr;
      RE::BGSListForm* tngFormLists[flCount] = {};
      RE::TESObjectARMO* block = nullptr;
  };
}

extern Common::Util* ut;
