#include <TngInis.h>
#include <TngPapyrus.h>
#include <TngSizeShape.h>
#include <TngCore.h>

void TngPapyrus::UpdateSize(RE::StaticFunctionTag*, int aIdx) { TngInis::SaveSize(aIdx); }

void TngPapyrus::UpdateRace(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption, float aGenMult) {
  if (aGenOption == -1) {
    TngSizeShape::GetSingleton()->genitalChoices[aRaceIdx] = TngSizeShape::cGenitalDefaults[aRaceIdx];
  } else {
    TngSizeShape::GetSingleton()->genitalChoices[aRaceIdx] = aGenOption;
  }
  TngSizeShape::GetSingleton()->genitalSizes[aRaceIdx] = aGenMult;
  TngCore::UpdateRace(aRaceIdx, aGenOption);
  TngInis::UpdateRace(aRaceIdx, aGenOption, aGenMult);
}

bool TngPapyrus::AllowSkinOverwrite(RE::StaticFunctionTag*) { return TngInis::GetAllowSkinOverwrite(); }

bool TngPapyrus::GetClipCheck(RE::StaticFunctionTag*) { return TngInis::GetClipCheck(); }

int TngPapyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor, bool aAllowSkinOverwrite) { return TngSizeShape::CanModifyActor(aActor, aAllowSkinOverwrite); }

bool TngPapyrus::SetActorShape(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption) {
  if (!aActor) return false;
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  bool res = TngSizeShape::SetActorSkin(aActor, aGenOption);
  if (!aActor->IsPlayerRef() && lNPC->GetFile(0)) {
    std::string lModName{aActor->GetFile(0)->GetFilename()};
    int lFM = aActor->GetActorBase()->IsFemale() ? 1 : 0;
    TngInis::AddActorShape(aActor->GetActorBase()->GetLocalFormID(), lModName, aGenOption);
  }
  return res;
}

void TngPapyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize) {
  if (!aActor) return;
  if (aGenSize == -1) {
    TngSizeShape::RandomizeScale(aActor);
  } else {
    TngSizeShape::SetActorSize(aActor, aGenSize);
  }
  if (!aActor->IsPlayerRef() && aActor->GetFile(0)) {
    std::string lModName{aActor->GetFile(0)->GetFilename()};
    TngInis::AddActorSize(aActor->GetActorBase()->GetLocalFormID(), lModName, aGenSize);
  }
}

bool TngPapyrus::LoadAddons(RE::StaticFunctionTag*) { return TngSizeShape::LoadAddons(); }

std::vector<std::string> TngPapyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, RE::Actor* aActor) { return std::vector<std::string>(); }

bool TngPapyrus::GetAutoReveal(RE::StaticFunctionTag*, bool aIsFemale) { return TngInis::GetAutoReveal(aIsFemale); }

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

void TngPapyrus::SaveBoolValues(RE::StaticFunctionTag*, int aID, bool aValue) { TngInis::SaveBool(aID, aValue); }

void TngPapyrus::SaveGlobals(RE::StaticFunctionTag*) { TngInis::SaveGlobals(); }

bool TngPapyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept {
  aVM->RegisterFunction("UpdateSize", "TNG_PapyrusUtil", UpdateSize);
  aVM->RegisterFunction("UpdateRace", "TNG_PapyrusUtil", UpdateRace);
  aVM->RegisterFunction("AllowSkinOverwrite", "TNG_PapyrusUtil", AllowSkinOverwrite);
  aVM->RegisterFunction("GetClipCheck", "TNG_PapyrusUtil", GetClipCheck);
  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("SetActorShape", "TNG_PapyrusUtil", SetActorShape);
  aVM->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);
  aVM->RegisterFunction("LoadAddons", "TNG_PapyrusUtil", LoadAddons);
  aVM->RegisterFunction("GetAllPossibleAddons", "TNG_PapyrusUtil", GetAllPossibleAddons);
  aVM->RegisterFunction("GetAutoReveal", "TNG_PapyrusUtil", GetAutoReveal);
  aVM->RegisterFunction("GetGenType", "TNG_PapyrusUtil", GetGenType);
  aVM->RegisterFunction("GetGenSize", "TNG_PapyrusUtil", GetGenSize);
  aVM->RegisterFunction("MakeRevealing", "TNG_PapyrusUtil", MakeRevealing);
  aVM->RegisterFunction("SaveBoolValues", "TNG_PapyrusUtil", SaveBoolValues);
  aVM->RegisterFunction("SaveGlobals", "TNG_PapyrusUtil", SaveGlobals);
  return true;
}
