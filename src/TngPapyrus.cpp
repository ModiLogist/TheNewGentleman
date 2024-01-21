#include <TngCore.h>
#include <TngInis.h>
#include <TngPapyrus.h>
#include <TngSizeShape.h>

void TngPapyrus::SaveGlobals(RE::StaticFunctionTag*) { TngInis::SaveGlobals(); }

bool TngPapyrus::GetClipCheck(RE::StaticFunctionTag*) { return TngInis::GetClipCheck(); }

bool TngPapyrus::GetAutoReveal(RE::StaticFunctionTag*, bool aIsFemale) { return TngInis::GetAutoReveal(aIsFemale); }

void TngPapyrus::SaveBoolValues(RE::StaticFunctionTag*, int aID, bool aValue) { TngInis::SaveBool(aID, aValue); }

std::vector<std::string> TngPapyrus::GetRaceNames(RE::StaticFunctionTag*) {
  auto lNames = TngSizeShape::GetRaceNames();
  return std::vector<std::string>{lNames.begin(), lNames.end()};
}

int TngPapyrus::GetRaceShape(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return Tng::pgErr;
  return TngSizeShape::GetRaceShape(static_cast<int>(aRaceIdx));
}

float TngPapyrus::GetRaceMult(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return -1.0f;
  return TngSizeShape::GetRaceMult(static_cast<int>(aRaceIdx));
}

void TngPapyrus::SetRaceShape(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption) {
  if (aRaceIdx < 0) return;
  if (TngCore::UpdateRaces(static_cast<int>(aRaceIdx), aGenOption) < 0) return;
}

void TngPapyrus::SetRaceMult(RE::StaticFunctionTag*, int aRaceIdx, float aGenMult) {
  if (TngSizeShape::SetRaceMult(static_cast<int>(aRaceIdx), aGenMult)) TngInis::SaveRaceMult(static_cast<int>(aRaceIdx), aGenMult);
}

std::vector<std::string> TngPapyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale) {
  auto lNames = TngSizeShape::GetAddonNames(aIsFemale);
  return std::vector<std::string>{lNames.begin(), lNames.end()};
}

int TngPapyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor) { return TngSizeShape::CanModifyActor(aActor); }

int TngPapyrus::SetActorShape(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption) {
  auto lRes = TngSizeShape::SetActorSkin(aActor, aGenOption);
  if (lRes < 0) return lRes;
  if (!aActor->IsPlayerRef()) TngInis::SaveActorShape(aActor->GetActorBase(), aGenOption);
  return lRes;
}

int TngPapyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize) {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return false;
  auto lRes = Tng::resOkGen;
  if (aGenSize == -1) {
    TngSizeShape::RandomizeScale(aActor);
  } else {
    lRes = TngSizeShape::SetActorSize(aActor, aGenSize);
  }
  if (!aActor->IsPlayerRef()) {
    TngInis::SaveActorSize(lNPC, aGenSize);
  }
  return lRes;
}

bool TngPapyrus::SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor) {
  if (!aArmor) return false;
  return TngCore::SwapRevealing(aArmor);
}

bool TngPapyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept {
  aVM->RegisterFunction("UpdateSize", "TNG_PapyrusUtil", UpdateSize);
  aVM->RegisterFunction("UpdateRaces", "TNG_PapyrusUtil", UpdateRaces);
  aVM->RegisterFunction("AllowSkinOverwrite", "TNG_PapyrusUtil", AllowSkinOverwrite);
  aVM->RegisterFunction("GetClipCheck", "TNG_PapyrusUtil", GetClipCheck);
  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("SetActorShape", "TNG_PapyrusUtil", SetActorShape);
  aVM->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);
  aVM->RegisterFunction("GetAllPossibleAddons", "TNG_PapyrusUtil", GetAllPossibleAddons);
  aVM->RegisterFunction("GetAutoReveal", "TNG_PapyrusUtil", GetAutoReveal);
  aVM->RegisterFunction("GetGenType", "TNG_PapyrusUtil", GetGenType);
  aVM->RegisterFunction("GetGenSize", "TNG_PapyrusUtil", GetGenSize);
  aVM->RegisterFunction("MakeRevealing", "TNG_PapyrusUtil", MakeRevealing);
  aVM->RegisterFunction("SaveBoolValues", "TNG_PapyrusUtil", SaveBoolValues);
  aVM->RegisterFunction("SaveGlobals", "TNG_PapyrusUtil", SaveGlobals);
  return true;
}
