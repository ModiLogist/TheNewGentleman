#pragma once

namespace Tng {
  inline static constexpr std::string_view cName{"TheNewGentleman.esp"};
  inline static constexpr std::string_view cSkyrim{"Skyrim.esm"};
  inline static constexpr const char cDelimChar{'~'};
  inline static constexpr const char cColonChar{':'};
  inline static constexpr std::string_view cSOSR{"SOS_Revealing"};
  inline static constexpr std::size_t cSizeCategories{5};
  inline static constexpr SEFormLoc cDefRaceID{0x19, cSkyrim};
  inline static constexpr SEFormLoc cBstRaceID{0x13745, cSkyrim};
  inline static constexpr int cVanillaRaceTypes{14};

  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};
  inline static constexpr SEFormLoc cNPCKeywID{0x13794, cSkyrim};
  inline static constexpr SEFormLoc cCrtKeywID{0x13795, cSkyrim};
  inline static constexpr SEFormLoc cBstKeywID{0xD61D1, cSkyrim};

  inline static constexpr SEFormLoc cProblemArmoKeyID{0xFF4, cName};
  inline static constexpr SEFormLoc cCustomSkinID{0xFF6, cName};
  inline static constexpr SEFormLoc cSkinWithPenisKeyID{0xFF7, cName};
  inline static constexpr SEFormLoc cGentleWomanKeyID{0xFF8, cName};
  inline static constexpr SEFormLoc cMalAddKeyID{0xFF9, cName};
  inline static constexpr std::pair<RE::FormID, std::string_view> cFemAddKeyID{0xFFA, cName};

  inline static constexpr SEFormLoc cWomenChanceID{0xCA0, cName};
  inline static constexpr SEFormLoc cPCAddon{0xCFF, cName};
  inline static constexpr SEFormLoc cGentifiedID{0xE00, cName};

  inline static constexpr int cMalDefAddnPriority{0};

  enum TNGRes {
    pgErr = -9,
    skinErr = -5,
    armoErr = -4,
    addonErr = -3,
    npcErr = -2,
    raceErr = -1,
    resOkNoGen = 0,
    resOkGen = 1,
    resOkRaceP = 2,
    resOkRaceR = 3,
    resOkAC = 4,
    resOkCC = 5,
    resOkAR = 6,
    resOkRR = 7,
    resOkIA = 8,
  };
  namespace {
    inline static constexpr SEFormLoc cProcessedRaceKeyID{0xFF0, cName};
    inline static constexpr SEFormLoc cReadyRaceKeyID{0xFF1, cName};
    inline static constexpr SEFormLoc cIgnoredRaceKeyID{0xFF2, cName};
    inline static constexpr SEFormLoc cIgnoredArmoKeyID{0xFF3, cName};
    inline static constexpr SEFormLoc cAutoCoverKeyID{0xFFC, cName};
    inline static constexpr SEFormLoc cCoveringKeyID{0xFFD, cName};
    inline static constexpr SEFormLoc cAutoRvealKeyID{0xFFB, cName};
    inline static constexpr SEFormLoc cRevealingKeyID{0xFFF, cName};
    inline static constexpr SEFormLoc cUnderwearKeyID{0xFFE, cName};
    inline static constexpr SEFormLoc cExcludeNPCKeyID{0xFF5, cName};
    inline static constexpr RE::FormID cSizeKeyIDs[cSizeCategories]{0xFE1, 0xFE2, 0xFE3, 0xFE4, 0xFE5};

    inline static constexpr RE::FormID cSizeGlbIDs[cSizeCategories]{0xC01, 0xC02, 0xC03, 0xC04, 0xC05};

    inline static RE::TESDataHandler* fSEDH;
    inline static RE::BGSKeyword* fPRaceKey;
    inline static RE::BGSKeyword* fRRaceKey;
    inline static RE::BGSKeyword* fIRaceKey;
    inline static RE::BGSKeyword* fNPCExKey;
    inline static RE::BGSKeyword* fNPCGwKey;
    inline static RE::BGSKeyword* fIAKey;
    inline static RE::BGSKeyword* fACKey;
    inline static RE::BGSKeyword* fCCKey;
    inline static RE::BGSKeyword* fARKey;
    inline static RE::BGSKeyword* fRRKey;
    inline static RE::BGSKeyword* fUWKey;
    inline static RE::BGSKeyword* fGPKey;
    inline static RE::BGSKeyword* fSizeKws[cSizeCategories];

    inline static RE::TESGlobal* fSizeGlbs[cSizeCategories];
    
    inline static RE::BGSListForm* fGentified;
  }

  static RE::TESDataHandler* SEDH() noexcept {
    if (!fSEDH) fSEDH = RE::TESDataHandler::GetSingleton();
    return Tng::SEDH();
  }

  static RE::BGSKeyword* RapKey() noexcept {
    if (!fPRaceKey) fPRaceKey = SEDH()->LookupForm<RE::BGSKeyword>(cProcessedRaceKeyID.first, cProcessedRaceKeyID.second);
    return fPRaceKey;
  }

  static RE::BGSKeyword* RarKey() noexcept {
    if (!fRRaceKey) fRRaceKey = SEDH()->LookupForm<RE::BGSKeyword>(cReadyRaceKeyID.first, cReadyRaceKeyID.second);
    return fRRaceKey;
  }

  static RE::BGSKeyword* RaiKey() noexcept {
    if (!fIRaceKey) fIRaceKey = SEDH()->LookupForm<RE::BGSKeyword>(cIgnoredRaceKeyID.first, cIgnoredRaceKeyID.second);
    return fIRaceKey;
  }

  static RE::BGSKeyword* NexKey() noexcept {
    if (!fNPCExKey) fNPCExKey = SEDH()->LookupForm<RE::BGSKeyword>(cExcludeNPCKeyID.first, cExcludeNPCKeyID.second);
    return fNPCExKey;
  }

  static RE::BGSKeyword* NgwKey() noexcept {
    if (!fNPCGwKey) fNPCGwKey = SEDH()->LookupForm<RE::BGSKeyword>(cGentleWomanKeyID.first, cGentleWomanKeyID.second);
    return fNPCGwKey;
  }

  static RE::BGSKeyword* AiaKey() noexcept {
    if (!fIAKey) fIAKey = SEDH()->LookupForm<RE::BGSKeyword>(cIgnoredArmoKeyID.first, cIgnoredArmoKeyID.second);
    return fIAKey;
  }

  static RE::BGSKeyword* AacKey() noexcept {
    if (!fACKey) fACKey = SEDH()->LookupForm<RE::BGSKeyword>(cAutoCoverKeyID.first, cAutoCoverKeyID.second);
    return fACKey;
  }

  static RE::BGSKeyword* AccKey() noexcept {
    if (!fCCKey) fCCKey = SEDH()->LookupForm<RE::BGSKeyword>(cCoveringKeyID.first, cCoveringKeyID.second);
    return fCCKey;
  }

  static RE::BGSKeyword* AccKey() noexcept {
    if (!fARKey) fARKey = SEDH()->LookupForm<RE::BGSKeyword>(cAutoRvealKeyID.first, cAutoRvealKeyID.second);
    return fARKey;
  }

  static RE::BGSKeyword* AccKey() noexcept {
    if (!fRRKey) fRRKey = SEDH()->LookupForm<RE::BGSKeyword>(cRevealingKeyID.first, cRevealingKeyID.second);
    return fRRKey;
  }

  static RE::BGSKeyword* AuwKey() noexcept {
    if (!fUWKey) fUWKey = SEDH()->LookupForm<RE::BGSKeyword>(cUnderwearKeyID.first, cUnderwearKeyID.second);
    return fUWKey;
  }

  static RE::BGSKeyword* GwpKey() noexcept {
    if (!fGPKey) fGPKey = SEDH()->LookupForm<RE::BGSKeyword>(cSkinWithPenisKeyID.first, cSkinWithPenisKeyID.second);
    return fGPKey;
  }

  static RE::BGSKeyword* SizeKey(const std::size_t aIdx) noexcept {
    if (!fSizeKws[aIdx]) fSizeKws[aIdx] = SEDH()->LookupForm<RE::BGSKeyword>(cSizeKeyIDs[aIdx], cName);
    return fSizeKws[aIdx];
  }

  static RE::TESGlobal* SizeGlb(const std::size_t aIdx) noexcept {
    if (!fSizeGlbs[aIdx]) fSizeGlbs[aIdx] = SEDH()->LookupForm<RE::TESGlobal>(cSizeGlbIDs[aIdx], cName);
    return fSizeGlbs[aIdx];
  }

  static RE::BGSListForm* GentFml() noexcept {
    if (!fGentified) fGentified = SEDH()->LookupForm<RE::BGSListForm>(cGentifiedID.first, cGentifiedID.second);
    return fGentified;
  }
}


