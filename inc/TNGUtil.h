#pragma once

#include <BaseUtil.h>

namespace TNG {
  using namespace BaseUtil;

  constexpr std::string_view mainFile{"TheNewGentleman.esp"};

  constexpr RE::BGSBipedObjectForm::BipedObjectSlot bodySlot{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
  constexpr RE::BGSBipedObjectForm::BipedObjectSlot genitalSlot{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

  constexpr std::string_view cNPCUserAddon{"TNG_ActorAddnUser:"};
  constexpr std::string_view sosRevealing{"SOS_Revealing"};

  constexpr std::tuple<const char*, const char*, bool, RE::FormID> cVanillaDefaults[14]{
      {"default", "nord", false, 0xA01},        {"redguard", "yokudan", false, 0xA02}, {"breton", "reachmen", false, 0xA03}, {"cyrodi", "imperial", false, 0xA04},
      {"altmer", "highelf", false, 0xA03},      {"bosmer", "woodelf", false, 0xA01},   {"dunmer", "darkelf", false, 0xA04},  {"orsimer", "orc", false, 0xA02},
      {"saxhleel", "argonian", true, 0xA01},    {"khajiit", "rhat", true, 0xA05},      {"dremora", "dremora", false, 0xA04}, {"elder", "old", false, 0xA05},
      {"afflicted", "afflicted", false, 0xA03}, {"snowelf", "falmer", false, 0xA05}};

  enum eGenBones { egbBase, egbScrot, egbScrotL, egbScrotR, egbGen01, egbGen02, egbGen03, egbGen04, egbGen05, egbGen06, GenBonesCount };
  constexpr const char* genBoneNames[GenBonesCount]{"NPC GenitalsBase [GenBase]",
                                                    "NPC GenitalsScrotum [GenScrot]",
                                                    "NPC L GenitalsScrotum [LGenScrot]",
                                                    "NPC R GenitalsScrotum [RGenScrot]",
                                                    "NPC Genitals01 [Gen01]",
                                                    "NPC Genitals02 [Gen02]",
                                                    "NPC Genitals03 [Gen03]",
                                                    "NPC Genitals04 [Gen04]",
                                                    "NPC Genitals05 [Gen05]",
                                                    "NPC Genitals06 [Gen06]"};

  enum eRes {
    err40 = errInt,
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
    resOkRacePP = 10,
    resOkRaceR = 14,
    resOkRaceP = 15,
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
    kyGenderChanger,
    kyGenderSwapped,
    kyAddonM,
    kyAddonF,
    kyAddonSec,
    kyRevealingF,
    kyRevealingM,
    kyCovering,
    kyUnderwear,
    kyRevealing,
    kyManMer,
    kyBeast,
    kyCreature,
    kyVampire,
    keywordsCount
  };

  enum eTngFormLists { flmGentleWomen, flmGentlerMen, flCount };

  enum eBoolSetting { bsExcludePlayerSize, bsRevealSlot52Mods, bsAllowMixSlot52Mods, bsRandomizeMaleAddon, bsShowAllRaces, bsDAK, boolSettingCount };
  enum eIntSetting { isSetupNPC, isRiseGen, isFallGen, isSwapRevealing, isWhyProblem, intSettingCount };
  enum eFloatSetting { fsXS, fsS, fsM, fsL, fsXL, fsMalRndChance, fsFemRndChance, floatSettingCount };
  inline static constexpr int sizeCatCount{fsXL + 1};

  class TNGUtil : public Singleton<TNGUtil> {
    public:
      RE::TESRace* Race(const size_t idx);
      RE::BGSKeyword* Key(const size_t idx);
      std::vector<RE::BGSKeyword*> Keys(const size_t first, const size_t last);
      RE::BGSKeyword* SizeKey(const size_t idx);
      std::vector<RE::BGSKeyword*> SizeKeys(const size_t last = sizeCatCount);
      RE::BGSListForm* FormList(const size_t idx);

      bool IsCovering(const RE::Actor* const actor, const RE::TESObjectARMO* const armor);
      std::vector<RE::TESObjectARMO*> GetWornAmor(RE::Actor* const actor) const;
      bool HasCovering(RE::Actor* const actor, RE::TESObjectARMO* const exception);

    private:
      inline static constexpr SEFormLocView raceIDs[racesCount]{{0x19, skyrimFile}, {0x13745, skyrimFile}};
      // TODO: Update it before release!
      inline static constexpr SEFormLocView keyIDs[keywordsCount] = {
          // {0xFF0, mainFile}, {0xFF1, mainFile},     {0xFF2, mainFile},     {0xFF3, mainFile},     {0xFF4, mainFile},
          //                                                              {0xFF6, mainFile}, {0xFF7, mainFile},     {0xFF8, mainFile},     {0xFF9, mainFile},     {0xFFA, mainFile},
          //                                                              {0xFFB, mainFile}, {0xFFC, mainFile},     {0xFFD, mainFile},     {0xFFE, mainFile},     {0xFFF, mainFile},
          //                                                              {0xFE0, mainFile}, {0x13794, skyrimFile}, {0xD61D1, skyrimFile}, {0x13795, skyrimFile}, {0xA82BB,
          //                                                              skyrimFile}
      };
      inline static constexpr RE::FormID sizeKeyIDs[sizeCatCount]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};

      inline static constexpr SEFormLocView formListIDs[flCount] = {{0xE00, mainFile}, {0xE01, mainFile}};

      RE::TESDataHandler* sedh = nullptr;
      RE::TESRace* races[racesCount] = {};
      RE::BGSKeyword* keywords[keywordsCount] = {};
      RE::BGSKeyword* sizeKey[sizeCatCount] = {};
  };
}

extern TNG::TNGUtil* ut;
