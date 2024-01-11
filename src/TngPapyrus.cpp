#include <TngInis.h>
#include <TngPapyrus.h>
#include <TngSizeShape.h>
#include <TngUtil.h>

bool TngPapyrus::tngLoaded;

bool TngPapyrus::TngLoaded(RE::StaticFunctionTag*) { return tngLoaded; }

void TngPapyrus::UpdateSize(RE::StaticFunctionTag*, int aIdx) { TngInis::SaveSize(aIdx); }

void TngPapyrus::UpdateRace(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption, float aGenMult) {
  if (aGenOption == -1) {
    TngSizeShape::GetSingleton()->genitalChoices[aRaceIdx] = TngSizeShape::cGenitalDefaults[aRaceIdx];
  } else {
    TngSizeShape::GetSingleton()->genitalChoices[aRaceIdx] = aGenOption;
  }
  TngSizeShape::GetSingleton()->genitalSizes[aRaceIdx] = aGenMult;
  TngUtil::UpdateRace(aRaceIdx, aGenOption);
  TngInis::UpdateRace(aRaceIdx, aGenOption, aGenMult);
}

void TngPapyrus::SetAutoRevealing(RE::StaticFunctionTag*, bool aFemaleArmor, bool aMaleArmor) {
  TngInis::GetSingleton()->FAutoReveal = aFemaleArmor;
  TngInis::GetSingleton()->MAutoReveal = aMaleArmor;
}

int TngPapyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor) { return TngSizeShape::CanModifyActor(aActor); }

void TngPapyrus::UpdateActor(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption, int aGenSize) {
  if (!aActor) return;
  if (aGenSize == -1) {
    TngSizeShape::RandomizeScale(aActor);
    return;
  }
  TngSizeShape::SetActorSize(aActor, aGenSize);
  TngSizeShape::SetActorSkin(aActor, aGenOption);
  if (aActor->IsPlayerRef()) return;
  std::string lModName{aActor->GetFile(0)->GetFilename()};
  TngInis::AddActor(aActor->GetLocalFormID(), lModName, aGenOption, aGenSize);
}

void TngPapyrus::UpdateMessage(RE::StaticFunctionTag*, bool aIsFemale) { TngSizeShape::UpdateMessage(aIsFemale); }

void TngPapyrus::ResetMessage(RE::StaticFunctionTag*, bool aIsFemale) { TngSizeShape::ResetMessage(aIsFemale); }

bool TngPapyrus::GetFAutoReveal(RE::StaticFunctionTag*) { return TngInis::GetSingleton()->FAutoReveal; }

bool TngPapyrus::GetMAutoReveal(RE::StaticFunctionTag*) { return TngInis::GetSingleton()->MAutoReveal; }

int TngPapyrus::GetGenType(RE::StaticFunctionTag*, int aRaceIdx) { return TngSizeShape::GetSingleton()->genitalChoices[aRaceIdx]; }

float TngPapyrus::GetGenSize(RE::StaticFunctionTag*, int aRaceIdx) { return TngSizeShape::GetSingleton()->genitalSizes[aRaceIdx]; }

bool TngPapyrus::MakeRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor) {
  if (!aArmor) return false;
  RE::BGSKeyword* lAutoCover = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cAutoCoverKeyID, Tng::cName);
  RE::BGSKeyword* lRevealing = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(Tng::cRevealingKeyID, Tng::cName);
  if (!lAutoCover || !lRevealing) return false;
  if (!aArmor->HasKeyword(lAutoCover)) return false;
  for (const auto& lAA : aArmor->armorAddons)
    if (lAA->HasPartOf(Tng::cSlotGenital) && lAA->HasPartOf(Tng::cSlotBody)) lAA->RemoveSlotFromMask(Tng::cSlotGenital);
  aArmor->RemoveKeyword(lAutoCover);
  aArmor->AddKeyword(lRevealing);
  TngInis::AddRevealingArmor(aArmor);
  return true;
}

void TngPapyrus::SaveGlobals(RE::StaticFunctionTag*) { TngInis::SaveGlobals(); }

bool TngPapyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept {
  aVM->RegisterFunction("TngLoaded", "TNG_PapyrusUtil", TngLoaded);
  aVM->RegisterFunction("UpdateSize", "TNG_PapyrusUtil", UpdateSize);
  aVM->RegisterFunction("UpdateRace", "TNG_PapyrusUtil", UpdateRace);
  aVM->RegisterFunction("SetAutoRevealing", "TNG_PapyrusUtil", SetAutoRevealing);
  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("UpdateActor", "TNG_PapyrusUtil", UpdateActor);
  aVM->RegisterFunction("UpdateMessage", "TNG_PapyrusUtil", UpdateMessage);
  aVM->RegisterFunction("UpdateActor", "TNG_PapyrusUtil", ResetMessage);
  aVM->RegisterFunction("GetFAutoReveal", "TNG_PapyrusUtil", GetFAutoReveal);
  aVM->RegisterFunction("GetMAutoReveal", "TNG_PapyrusUtil", GetMAutoReveal);
  aVM->RegisterFunction("GetGenType", "TNG_PapyrusUtil", GetGenType);
  aVM->RegisterFunction("GetGenSize", "TNG_PapyrusUtil", GetGenSize);
  aVM->RegisterFunction("MakeRevealing", "TNG_PapyrusUtil", MakeRevealing);
  aVM->RegisterFunction("SaveGlobals", "TNG_PapyrusUtil", SaveGlobals);
  return true;
}
