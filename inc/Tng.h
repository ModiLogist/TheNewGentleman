#pragma once

namespace Tng {
  namespace gLogger = SKSE::log;
  inline static constexpr std::string_view cName{"TheNewGentleman.esp"};
  inline static constexpr std::string_view cSkyrim{"Skyrim.esm"};
  inline static constexpr const char cDelimChar{'~'};
  inline static constexpr const char cColonChar{':'};
  inline static constexpr std::string_view cSOSR{"SOS_Revealing"};
  inline static constexpr std::size_t cSizeCategories{5};
  inline static constexpr RE::FormID cDefRaceID = 0x19;
  inline static constexpr RE::FormID cBstRaceID = 0x13745;
  inline static constexpr int cVanillaRaceTypes{14};
  inline static constexpr int cEqRaceTypes{13};
  enum RaceType { raceManMer, raceBeast, raceElder, raceDremora, raceAfflicted, raceSnowElf, cRaceTypeCount };

  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
  inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};
  inline static constexpr RE::FormID cNPCKeywID{0x13794};
  inline static constexpr RE::FormID cCrtKeywID{0x13795};
  inline static constexpr RE::FormID cBstKeywID{0xD61D1};

  inline static constexpr RE::FormID cProcessedRaceKeyID{0xFF0};
  inline static constexpr RE::FormID cReadyRaceKeyID{0xFF1};
  inline static constexpr RE::FormID cIgnoredRaceKeyID{0xFF2};
  inline static constexpr RE::FormID cIgnoredArmoKeyID{0xFF3};
  inline static constexpr RE::FormID cProblemArmoKeyID{0xFF4};
  inline static constexpr RE::FormID cExcludeKeyID{0xFF5};
  inline static constexpr RE::FormID cCustomSkinID{0xFF6};
  inline static constexpr RE::FormID cSkinWithPenisKeyID{0xFF7};
  inline static constexpr RE::FormID cGentleWomanKeyID{0xFF8};
  inline static constexpr RE::FormID cMalAddKeyID{0xFF9};
  inline static constexpr RE::FormID cFemAddKeyID{0xFFA};
  inline static constexpr RE::FormID cAutoRvealKeyID{0xFFB};
  inline static constexpr RE::FormID cAutoCoverKeyID{0xFFC};
  inline static constexpr RE::FormID cCoveringKeyID{0xFFD};
  inline static constexpr RE::FormID cUnderwearKeyID{0xFFE};
  inline static constexpr RE::FormID cRevealingKeyID{0xFFF};

  inline static constexpr RE::FormID cWomenChanceID{0xCA0};
  inline static constexpr RE::FormID cGentifiedID{0xE00};

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
}

static std::pair<std::string, RE::FormID> StrToRecord(const std::string aRecord) noexcept {
  const size_t lSepLoc = aRecord.find(Tng::cDelimChar);
  const RE::FormID lFormID = std::strtol(aRecord.substr(0, lSepLoc).data(), nullptr, 0);
  const std::string lModName = aRecord.substr(lSepLoc + 1);
  return std::make_pair(lModName, lFormID);
}

static std::string RecordToStr(RE::TESForm* aForm) noexcept {
  if (!aForm->GetFile(0)) return "";
  std::ostringstream oss;
  oss << std::hex << aForm->GetLocalFormID();
  return "0x" + oss.str() + Tng::cDelimChar + std::string(aForm->GetFile(0)->GetFilename());
}

template <typename FormType>
static constexpr FormType* LoadForm(std::string aFormRecord) {
  auto lRecod = StrToRecord(aFormRecord);
  return RE::TESDataHandler::GetSingleton()->LookupForm<FormType>(lRecod.second,lRecod.first);
};
