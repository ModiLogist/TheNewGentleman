#pragma once

class Tng : public Singleton<Tng> {
  public:
    inline static constexpr std::string_view cName{"TheNewGentleman.esp"};
    inline static constexpr std::string_view cSkyrim{"Skyrim.esm"};
    inline static constexpr const char cDelimChar{'~'};
    inline static constexpr const char cColonChar{':'};
    inline static constexpr std::string_view cSOSR{"SOS_Revealing"};
    inline static constexpr size_t cSizeCategories{5};

    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

    inline static constexpr SEFormLocView cCover{0xAFF, cName};

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

    enum BoolSetting {
      bsExcludePlayerSize,
      bsCheckPlayerAddon,
      bsCheckNPCsAddons,
      bsForceRechecks,
      bsRevealSlot52Mods,
      bsAllowMixSlot52Mods,
      bsRandomizeMaleAddon,
      bsUIExtensions,
      bsShowAllRaces,
      BoolSettingCount
    };

    enum Races { raceDefault, raceDefBeast, RacesCount };

    enum eKeywords {
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
      KeywordsCount
    };

    enum eTngFormLists { flmGentleWomen, flmNonGentleMen, TngFormListsCount };
    inline static constexpr eKeywords RevKeys[3]{kyRevealing, kyRevealingF, kyRevealingM};
    enum UserCtrls { ctrlDAK, ctrlSetupNPC, ctrlRiseGen, ctrlFallGen, ctrlSwapRevealing, ctrWhyProblem, UserCtrlsCount };

  private:
    inline static constexpr SEFormLocView cRaceIDs[RacesCount]{{0x19, cSkyrim}, {0x13745, cSkyrim}};
    inline static constexpr SEFormLocView cTngKeyIDs[KeywordsCount] = {{0xFF0, cName}, {0xFF1, cName},     {0xFF2, cName},     {0xFF3, cName},     {0xFF4, cName},    {0xFF6, cName}, {0xFF7, cName},
                                                                       {0xFF8, cName}, {0xFF9, cName},     {0xFFA, cName},     {0xFFB, cName},     {0xFFC, cName},    {0xFFD, cName}, {0xFFE, cName},
                                                                       {0xFFF, cName}, {0x13794, cSkyrim}, {0xD61D1, cSkyrim}, {0x13795, cSkyrim}, {0xA82BB, cSkyrim}};

    inline static constexpr SEFormLocView cPCAddon{0xCFF, cName};
    inline static constexpr SEFormLocView cWomenChanceID{0xCA0, cName};
    inline static constexpr RE::FormID cSizeKeyIDs[cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};
    inline static constexpr RE::FormID cSizeGlbIDs[cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};
    inline static constexpr SEFormLocView cUserCtrlIDs[UserCtrlsCount] = {{0xC00, Tng::cName}, {0xCB0, Tng::cName}, {0xCB1, Tng::cName}, {0xCB2, Tng::cName}, {0xCB3, Tng::cName}, {0xCB4, Tng::cName}};

    inline static constexpr SEFormLocView cTngFormListIds[TngFormListsCount] = {{0xE00, cName}, {0xE01, cName}};

    inline static RE::TESDataHandler* fSEDH;
    inline static RE::TESRace* races[RacesCount];
    inline static RE::BGSKeyword* keywords[KeywordsCount];
    inline static RE::BGSKeyword* sizeKey[cSizeCategories];
    inline static RE::TESGlobal* pcAddon;
    inline static RE::TESGlobal* gwChance;
    inline static RE::TESGlobal* sizeGlbs[cSizeCategories];
    inline static RE::TESGlobal* ctrlGlbs[UserCtrlsCount];
    inline static RE::BGSListForm* tngFormLists[TngFormListsCount];
    inline static RE::TESObjectARMO* block;

  public:
    static RE::TESDataHandler* SEDH() {
      if (!fSEDH) fSEDH = RE::TESDataHandler::GetSingleton();
      return fSEDH;
    }

    static RE::TESRace* Race(const size_t idx) {
      if (idx >= RacesCount) return nullptr;
      if (!races[idx]) races[idx] = SEDH()->LookupForm<RE::TESRace>(cRaceIDs[idx].first, cRaceIDs[idx].second);
      return races[idx];
    }

    static RE::BGSKeyword* Key(const size_t idx) {
      if (idx >= KeywordsCount) return nullptr;
      if (!keywords[idx]) keywords[idx] = SEDH()->LookupForm<RE::BGSKeyword>(cTngKeyIDs[idx].first, cTngKeyIDs[idx].second);
      return keywords[idx];
    }

    static std::vector<RE::BGSKeyword*> Keys(const size_t first, const size_t last) {
      std::vector<RE::BGSKeyword*> res = {};
      if (last >= KeywordsCount) return res;
      for (auto i = first; i <= last; i++) res.push_back(Key(i));
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

    static RE::BGSListForm* TngFml(const size_t idx) {
      if (!tngFormLists[idx]) tngFormLists[idx] = SEDH()->LookupForm<RE::BGSListForm>(cTngFormListIds[idx].first, cTngFormListIds[idx].second);
      return tngFormLists[idx];
    }

    static RE::TESObjectARMO* Block() {
      if (!block) block = Tng::SEDH()->LookupForm<RE::TESObjectARMO>(cCover.first, cCover.second);
      return block;
    }

    static RE::BGSKeyword* ProduceOrGetKw(const std::string& keyword) {
      auto& allKeywords = Tng::SEDH()->GetFormArray<RE::BGSKeyword>();
      auto it = std::find_if(allKeywords.begin(), allKeywords.end(), [&](const auto& kw) { return kw && kw->formEditorID == keyword.c_str(); });
      RE::BGSKeyword* res{nullptr};
      if (it != allKeywords.end()) {
        res = *it;
      } else {
        const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
        if (res = factory ? factory->Create() : nullptr; res) {
          res->formEditorID = keyword;
          allKeywords.push_back(res);
        }
      }
      return res;
    }
};

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
