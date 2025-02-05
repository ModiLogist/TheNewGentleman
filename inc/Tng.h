#pragma once

namespace Tng {
  namespace logger = SKSE::log;
  inline static constexpr std::string_view cName{"TheNewGentleman.esp"};
  inline static constexpr std::string_view cSkyrim{"Skyrim.esm"};
  inline static constexpr const char cDelimChar{'~'};
  inline static constexpr const char cColonChar{':'};
  inline static constexpr std::string_view cSOSR{"SOS_Revealing"};
  inline static constexpr size_t cSizeCategories{5};

  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

  inline static constexpr SEFormLocView cProblemArmoKeyID{0xFF4, cName};
  inline static constexpr SEFormLocView cMalAddKeyID{0xFF9, cName};
  inline static constexpr SEFormLocView cFemAddKeyID{0xFFA, cName};

  inline static constexpr SEFormLocView cGentifiedID{0xE00, cName};

  inline static constexpr size_t cMalRandomPriority{100};
  inline static constexpr int cNA{-99};
  inline static constexpr int cDef{-2};
  inline static constexpr int cNul{-1};

  enum TNGRes {
    pgErr = cNA,
    rgErr = -8,
    skeletonErr = -7,
    playerErr = -6,
    skinErr = -5,
    armoErr = -4,
    addonErr = -3,
    npcErr = -2,
    raceErr = -1,
    resOkFixed = 0,
    resOkSizable = 1,
    resOkRaceP = 10,
    resOkRaceR = 11,
    resOkRacePP = 12,
    resOkMain = 20,
    resOkDedicated = 21,
    resOkSupported = 22,
  };

  enum BoolSetting { bsExcludePlayerSize, bsCheckPlayerAddon, bsRevealSlot52Mods, bsAllowMixSlot52Mods, bsRandomizeMaleAddon, BoolSettingCount };
  inline static bool boolSettings[BoolSettingCount] = {false, false, false, false, false};

  enum Races { raceDefault, raceDefBeast, RacesCount };

  enum eRaceKeys { rkeyPreprocessed, rkeyProcessed, rkeyReady, rkeyIgnore, rkeyManMer, rkeyBeast, rkeyCreature, RaceKeysCount };
  enum eNPCKeys { npckeyExclude, npckeyGentlewoman, NPCKeysCount };
  enum eArmoKeys { akeyCover, akeyRevFem, akeyRevMal, akeyReveal, akeyIgnored, akeyUnderwear, akeySkinWP, akeyGenSkin, ArmoKeysCount };
  inline static constexpr eArmoKeys RevKeys[3]{akeyReveal, akeyRevFem, akeyRevMal};

  enum UserCtrls { ctrlDAK, ctrlSetupNPC, ctrlRiseGen, ctrlFallGen, ctrlSwapRevealing, ctrWhyProblem, UserCtrlsCount };

  namespace {
    inline static constexpr SEFormLocView cRaceIDs[RacesCount]{{0x19, cSkyrim}, {0x13745, cSkyrim}};

    inline static constexpr SEFormLocView cRaceKeyIDs[RaceKeysCount] = {{0xFEF, cName}, {0xFF0, cName}, {0xFF1, cName}, {0xFF2, cName}, {0x13794, cSkyrim}, {0xD61D1, cSkyrim}, {0x13795, cSkyrim}};

    inline static constexpr SEFormLocView cNPCKeyIDs[NPCKeysCount] = {{0xFF5, cName}, {0xFF8, cName}};

    inline static constexpr SEFormLocView cArmoKeyIDs[ArmoKeysCount] = {{0xFFD, cName}, {0xFFB, cName}, {0xFFC, cName}, {0xFFF, cName}, {0xFF3, cName}, {0xFFE, cName}, {0xFF7, cName}, {0xFF6, cName}};

    inline static constexpr RE::FormID cSizeKeyIDs[cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};

    inline static constexpr SEFormLocView cPCAddon{0xCFF, cName};
    inline static constexpr SEFormLocView cWomenChanceID{0xCA0, cName};
    inline static constexpr RE::FormID cSizeGlbIDs[cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};

    inline static constexpr SEFormLocView cUserCtrlIDs[UserCtrlsCount] = {{0xC00, Tng::cName}, {0xCB0, Tng::cName}, {0xCB1, Tng::cName}, {0xCB2, Tng::cName}, {0xCB3, Tng::cName}, {0xCB4, Tng::cName}};

    inline static constexpr SEFormLocView cCover{0xAFF, cName};

    inline static RE::TESDataHandler* fSEDH;
    inline static RE::TESRace* races[RacesCount];
    inline static RE::BGSKeyword* raceKeys[RaceKeysCount];
    inline static RE::BGSKeyword* npcKeys[NPCKeysCount];
    inline static RE::BGSKeyword* armoKeys[ArmoKeysCount];
    inline static RE::BGSKeyword* skinwpKey;
    inline static RE::BGSKeyword* generatedSkinKey;
    inline static RE::BGSKeyword* sizeKey[cSizeCategories];

    inline static RE::TESGlobal* pcAddon{nullptr};
    inline static RE::TESGlobal* gwChance{nullptr};
    inline static RE::TESGlobal* sizeGlbs[cSizeCategories]{nullptr};
    inline static RE::TESGlobal* ctrlGlbs[UserCtrlsCount]{nullptr};

    inline static RE::BGSListForm* fGentified;

    inline static RE::TESObjectARMO* block;
  }

  static RE::TESDataHandler* SEDH() {
    if (!fSEDH) fSEDH = RE::TESDataHandler::GetSingleton();
    return fSEDH;
  }

  static RE::TESRace* Race(const size_t idx) {
    if (idx >= RacesCount) return nullptr;
    if (!races[idx]) races[idx] = SEDH()->LookupForm<RE::TESRace>(cRaceIDs[idx].first, cRaceIDs[idx].second);
    return races[idx];
  }

  static RE::BGSKeyword* RaceKey(const size_t idx) {
    if (idx >= RaceKeysCount) return nullptr;
    if (!raceKeys[idx]) raceKeys[idx] = SEDH()->LookupForm<RE::BGSKeyword>(cRaceKeyIDs[idx].first, cRaceKeyIDs[idx].second);
    return raceKeys[idx];
  }

  static std::vector<RE::BGSKeyword*> RaceKeys(const size_t last = RaceKeysCount) {
    std::vector<RE::BGSKeyword*> res = {};
    for (size_t i = 0; i < (last > RaceKeysCount ? RaceKeysCount : last); i++) res.push_back(RaceKey(i));
    return res;
  }

  static RE::BGSKeyword* NPCKey(const size_t idx) {
    if (idx >= NPCKeysCount) return nullptr;
    if (!npcKeys[idx]) npcKeys[idx] = SEDH()->LookupForm<RE::BGSKeyword>(cNPCKeyIDs[idx].first, cNPCKeyIDs[idx].second);
    return npcKeys[idx];
  }

  static std::vector<RE::BGSKeyword*> NPCKeys(const size_t last = NPCKeysCount) {
    std::vector<RE::BGSKeyword*> res = {};
    for (size_t i = 0; i < (last > NPCKeysCount ? NPCKeysCount : last); i++) res.push_back(NPCKey(i));
    return res;
  }

  static RE::BGSKeyword* ArmoKey(const size_t idx) {
    if (idx >= ArmoKeysCount) return nullptr;
    if (!armoKeys[idx]) armoKeys[idx] = SEDH()->LookupForm<RE::BGSKeyword>(cArmoKeyIDs[idx].first, cArmoKeyIDs[idx].second);
    return armoKeys[idx];
  }

  static std::vector<RE::BGSKeyword*> ArmoKeys(const size_t last = ArmoKeysCount) {
    std::vector<RE::BGSKeyword*> res = {};
    for (size_t i = 0; i < (last > ArmoKeysCount ? ArmoKeysCount : last); i++) res.push_back(ArmoKey(i));
    return res;
  }

  static RE::BGSKeyword* SizeKey(const size_t idx) {
    if (!sizeKey[idx]) sizeKey[idx] = SEDH()->LookupForm<RE::BGSKeyword>(cSizeKeyIDs[idx], cName);
    return sizeKey[idx];
  }

  static std::vector<RE::BGSKeyword*> SizeKeys(const size_t last = cSizeCategories) {
    std::vector<RE::BGSKeyword*> res{};
    for (size_t i = 0; i < (last > cSizeCategories ? cSizeCategories : last); i++) res.push_back(SizeKey(i));
    return res;
  }

  static RE::TESGlobal* SizeGlb(const size_t idx) {
    if (!sizeGlbs[idx]) sizeGlbs[idx] = SEDH()->LookupForm<RE::TESGlobal>(cSizeGlbIDs[idx], cName);
    return sizeGlbs[idx];
  }

  static RE::TESGlobal* UserCtrl(const size_t idx) {
    if (!ctrlGlbs[idx]) ctrlGlbs[idx] = SEDH()->LookupForm<RE::TESGlobal>(cUserCtrlIDs[idx].first, cUserCtrlIDs[idx].second);
    return ctrlGlbs[idx];
  }

  static RE::TESGlobal* PCAddon() {
    if (!pcAddon) pcAddon = SEDH()->LookupForm<RE::TESGlobal>(cPCAddon.first, cPCAddon.second);
    return pcAddon;
  }

  static RE::TESGlobal* WRndGlb() {
    if (!gwChance) gwChance = SEDH()->LookupForm<RE::TESGlobal>(cWomenChanceID.first, cWomenChanceID.second);
    return gwChance;
  }

  static RE::BGSListForm* GentFml() {
    if (!fGentified) fGentified = SEDH()->LookupForm<RE::BGSListForm>(cGentifiedID.first, cGentifiedID.second);
    return fGentified;
  }

  static RE::TESObjectARMO* Block() {
    if (!block) block = Tng::SEDH()->LookupForm<RE::TESObjectARMO>(cCover.first, cCover.second);
    return block;
  }

  static RE::BGSKeyword* ProduceOrGetKw(const std::string& keword) {
    auto& allKeywords = Tng::SEDH()->GetFormArray<RE::BGSKeyword>();
    auto it = std::find_if(allKeywords.begin(), allKeywords.end(), [&](const auto& kw) { return kw && kw->formEditorID == keword.c_str(); });
    RE::BGSKeyword* res{nullptr};
    if (it != allKeywords.end()) {
      res = *it;
    } else {
      const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
      if (res = factory ? factory->Create() : nullptr; res) {
        res->formEditorID = keword;
        allKeywords.push_back(res);
      }
    }
    return res;
  }
}

static void ShowSkyrimMessage(const char* message) { RE::DebugMessageBox(message); }

static SEFormLoc StrToLoc(const std::string recordStr) {
  const size_t sepLoc = recordStr.find(Tng::cDelimChar);
  if (sepLoc == std::string::npos) return {0, ""};
  const RE::FormID formID = std::strtol(recordStr.substr(0, sepLoc).data(), nullptr, 0);
  const std::string modName = recordStr.substr(sepLoc + 1);
  return std::make_pair(formID, modName);
}

static SEFormLoc FormToLoc(const RE::TESForm* form) {
  std::string filename = form->GetFile(0) ? std::string(form->GetFile(0)->GetFilename()) : "NoFile";
  return {form->GetLocalFormID(), filename};
}

static SEFormLocView FormToLocView(RE::TESForm* form) {
  auto filename = form->GetFile(0) ? form->GetFile(0)->GetFilename() : "NoFile";
  return {form->GetLocalFormID(), filename};
}

static std::string FormToStr(RE::TESForm* form) {
  if (!form || !form->GetFile(0)) return "";
  std::ostringstream oss;
  oss << std::hex << form->GetLocalFormID();
  return "0x" + oss.str() + Tng::cDelimChar + std::string(form->GetFile(0)->GetFilename());
}

static bool FormHasKW(const RE::BGSKeywordForm* form, const SEFormLocView formID) {
  auto rawID = Tng::SEDH()->LookupFormID(formID.first, formID.second);
  return form->HasKeywordID(rawID);
}

static bool AddonHasRace(const RE::TESObjectARMA* addnIdx, const RE::TESRace* race) {
  if (race == addnIdx->race) return true;
  for (auto& iRace : addnIdx->additionalRaces)
    if (iRace == race) return true;
  return false;
}

template <typename FormType>
static constexpr FormType* LoadForm(std::string formRecord) {
  auto record = StrToLoc(formRecord);
  return Tng::SEDH()->LookupForm<FormType>(record.first, record.second);
}
