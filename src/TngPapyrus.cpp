#include <TngCore.h>
#include <TngInis.h>
#include <TngPapyrus.h>
#include <TngSizeShape.h>

void TngPapyrus::SaveGlobals(RE::StaticFunctionTag*) { TngInis::SaveGlobals(); }

bool TngPapyrus::GetClipCheck(RE::StaticFunctionTag*) { return TngInis::GetClipCheck(); }

bool TngPapyrus::GetAutoReveal(RE::StaticFunctionTag*, bool aIsFemale) { return TngInis::GetAutoReveal(aIsFemale); }

void TngPapyrus::SaveBoolValues(RE::StaticFunctionTag*, int aID, bool aValue) { TngInis::SaveBool(aID, aValue); }

std::vector<std::string> TngPapyrus::GetRaceNames(RE::StaticFunctionTag*) { return TngSizeShape::GetRaceNames(); }

int TngPapyrus::GetRaceAddn(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return Tng::pgErr;
  return TngSizeShape::GetRaceAddn(static_cast<int>(aRaceIdx));
}

float TngPapyrus::GetRaceMult(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return -1.0f;
  return TngSizeShape::GetRaceMult(static_cast<int>(aRaceIdx));
}

void TngPapyrus::SetRaceAddn(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption) {
  if (aRaceIdx < 0) return;
  TngCore::UpdateRaces(static_cast<int>(aRaceIdx), aGenOption);
}

void TngPapyrus::SetRaceMult(RE::StaticFunctionTag*, int aRaceIdx, float aGenMult) {
  if (TngSizeShape::SetRaceMult(static_cast<int>(aRaceIdx), aGenMult)) TngInis::SaveRaceMult(static_cast<int>(aRaceIdx), aGenMult);
}

std::vector<std::string> TngPapyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale) {
  auto lNames = TngSizeShape::GetAddonNames(aIsFemale);
  return std::vector<std::string>{lNames.begin(), lNames.end()};
}

int TngPapyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor) { return TngSizeShape::CanModifyActor(aActor); }

int TngPapyrus::SetActorAddn(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption) { return TngCore::SetActorSkin(aActor, aGenOption); }

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
  aVM->RegisterFunction("SaveGlobals", "TNG_PapyrusUtil", SaveGlobals);
  aVM->RegisterFunction("GetClipCheck", "TNG_PapyrusUtil", GetClipCheck);
  aVM->RegisterFunction("GetAutoReveal", "TNG_PapyrusUtil", GetAutoReveal);
  aVM->RegisterFunction("SaveBoolValues", "TNG_PapyrusUtil", SaveBoolValues);
  aVM->RegisterFunction("GetRaceNames", "TNG_PapyrusUtil", GetRaceNames);
  aVM->RegisterFunction("GetRaceAddn", "TNG_PapyrusUtil", GetRaceAddn);
  aVM->RegisterFunction("GetRaceMult", "TNG_PapyrusUtil", GetRaceMult);
  aVM->RegisterFunction("SetRaceAddn", "TNG_PapyrusUtil", SetRaceAddn);
  aVM->RegisterFunction("SetRaceMult", "TNG_PapyrusUtil", SetRaceMult);
  aVM->RegisterFunction("GetAllPossibleAddons", "TNG_PapyrusUtil", GetAllPossibleAddons);
  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("SetActorAddn", "TNG_PapyrusUtil", SetActorAddn);
  aVM->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);
  aVM->RegisterFunction("SwapRevealing", "TNG_PapyrusUtil", SwapRevealing);
  return true;
}
