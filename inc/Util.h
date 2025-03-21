#pragma once

class Util;
class Base;
class Inis;
class Core;
class Events;

extern Util* ut;
extern Base* base;
extern Inis* inis;
extern Core* core;
extern Events* events;

class Util : public Singleton<Util> {
  public:
    inline static constexpr std::string_view mainFile{"TheNewGentleman.esp"};
    inline static constexpr std::string_view skyrimFile{"Skyrim.esm"};
    inline static constexpr char delim{'~'};
    inline static constexpr char colon{':'};
    inline static constexpr std::string_view sosRevealing{"SOS_Revealing"};

    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot bodySlot{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot genitalSlot{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

    inline static constexpr SEFormLocView coverID{0xAFF, mainFile};

    inline static constexpr float fEpsilon{0.0001f};
    inline static constexpr size_t malRndChance{100};
    inline static constexpr int nan{-99};
    inline static constexpr int def{-2};
    inline static constexpr int nul{-1};
    inline static constexpr std::string nanStr{"NA"};
    inline static constexpr std::string defStr{"Default"};
    inline static constexpr std::string nulStr{"None"};

    enum eRes {
      err40 = nan,
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
      resOkRaceP = 10,
      resOkRaceR = 11,
      resOkRacePP = 12,
      resOkMain = 20,
      resOkDedicated = 21,
      resOkSupported = 22,
    };

    enum eBoolSetting {
      bsExcludePlayerSize,
      bsCheckPlayerAddon,
      bsCheckNPCsAddons,
      bsForceRechecks,
      bsRevealSlot52Mods,
      bsAllowMixSlot52Mods,
      bsRandomizeMaleAddon,
      bsUIExtensions,
      bsShowAllRaces,
      boolSettingCount
    };

    enum eIntSetting { isDAK, isSetupNPC, isRiseGen, isFallGen, isSwapRevealing, isWhyProblem, intSettingCount };

    enum eFloatSetting { fsXS, fsS, fsM, fsL, fsXL, fsWomenChance, floatSettingCount };
    inline static constexpr int sizeKeyCount{floatSettingCount - 1};

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
      kyManMer,
      kyBeast,
      kyCreature,
      kyVampire,
      keywordsCount
    };

    enum eTngFormLists { flmGentleWomen, flmNonGentleMen, flCount };
    inline static constexpr eKeyword RevKeys[3]{kyRevealing, kyRevealingF, kyRevealingM};

  private:
    inline static constexpr SEFormLocView raceIDs[racesCount]{{0x19, skyrimFile}, {0x13745, skyrimFile}};
    inline static constexpr SEFormLocView keyIDs[keywordsCount] = {{0xFF0, mainFile},     {0xFF1, mainFile},     {0xFF2, mainFile},     {0xFF3, mainFile},    {0xFF4, mainFile},
                                                                   {0xFF6, mainFile},     {0xFF7, mainFile},     {0xFF8, mainFile},     {0xFF9, mainFile},    {0xFFA, mainFile},
                                                                   {0xFFB, mainFile},     {0xFFC, mainFile},     {0xFFD, mainFile},     {0xFFE, mainFile},    {0xFFF, mainFile},
                                                                   {0x13794, skyrimFile}, {0xD61D1, skyrimFile}, {0x13795, skyrimFile}, {0xA82BB, skyrimFile}};

    inline static constexpr SEFormLocView pcAddonID{0xCFF, mainFile};
    inline static constexpr RE::FormID sizeKeyIDs[sizeKeyCount]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};

    inline static constexpr SEFormLocView formListIDs[flCount] = {{0xE00, mainFile}, {0xE01, mainFile}};

  public:
    RE::TESDataHandler* SEDH();
    RE::TESRace* Race(const size_t idx);
    RE::BGSKeyword* Key(const size_t idx);
    std::vector<RE::BGSKeyword*> Keys(const size_t first, const size_t last);
    RE::BGSKeyword* SizeKey(const size_t idx);
    std::vector<RE::BGSKeyword*> SizeKeys(const size_t last = sizeKeyCount);
    RE::TESGlobal* PCAddon();
    RE::BGSListForm* TngFml(const size_t idx);
    RE::TESObjectARMO* Block();
    RE::BGSKeyword* ProduceOrGetKw(const std::string& keyword);
    void ShowSkyrimMessage(const char* message);
    SEFormLoc StrToLoc(const std::string recordStr, bool canBeNone = false);
    SEFormLoc FormToLoc(const RE::TESForm* form);
    SEFormLocView FormToLocView(RE::TESForm* form);
    std::string FormToStr(RE::TESForm* form);

  private:
    RE::TESDataHandler* sedh = nullptr;
    RE::TESRace* races[racesCount] = {};
    RE::BGSKeyword* keywords[keywordsCount] = {};
    RE::BGSKeyword* sizeKey[sizeKeyCount] = {};
    RE::TESGlobal* pcAddon = nullptr;
    RE::BGSListForm* tngFormLists[flCount] = {};
    RE::TESObjectARMO* block = nullptr;
};
