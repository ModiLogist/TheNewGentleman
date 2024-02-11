#include <TngCore.h>
#include <TngInis.h>
#include <TngPapyrus.h>
#include <TngSizeShape.h>

void TngPapyrus::SaveGlobals(RE::StaticFunctionTag*) { TngInis::SaveGlobals(); }

bool TngPapyrus::GetClipCheck(RE::StaticFunctionTag*) { return TngInis::GetClipCheck(); }

bool TngPapyrus::GetAutoReveal(RE::StaticFunctionTag*, bool aIsFemale) { return TngInis::GetAutoReveal(aIsFemale); }

void TngPapyrus::SaveBoolValues(RE::StaticFunctionTag*, int aID, bool aValue) { TngInis::SaveBool(aID, aValue); }

std::vector<std::string> TngPapyrus::GetRaceGrpNames(RE::StaticFunctionTag*) { return TngSizeShape::GetRaceGrpNames(); }

int TngPapyrus::GetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return Tng::pgErr;
  return TngSizeShape::GetRaceGrpAddn(static_cast<std::size_t>(aRaceIdx));
}

float TngPapyrus::GetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return -1.0f;
  return TngSizeShape::GetRaceGrpMult(static_cast<std::size_t>(aRaceIdx));
}

void TngPapyrus::SetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption) {
  if (aRaceIdx < 0) return;
  TngCore::UpdateRaceGrpAddn(static_cast<int>(aRaceIdx), aGenOption);
}

void TngPapyrus::SetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx, float aGenMult) {
  if (TngSizeShape::SetRaceGrpMult(static_cast<std::size_t>(aRaceIdx), aGenMult)) TngInis::SaveRaceMult(static_cast<std::size_t>(aRaceIdx), aGenMult);
}

std::vector<std::string> TngPapyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale) {
  auto lNames = TngSizeShape::GetAddonNames(aIsFemale);
  return std::vector<std::string>{lNames.begin(), lNames.end()};
}

int TngPapyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor) { return TngCore::CanModifyActor(aActor); }

int TngPapyrus::SetActorAddn(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption) {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  return TngCore::SetNPCSkin(lNPC, aGenOption);
}

int TngPapyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize) { return TngCore::SetActorSize(aActor, aGenSize); }

bool TngPapyrus::SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor) {
  if (!aArmor) return false;
  return TngCore::SwapRevealing(aArmor);
}

bool TngPapyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept {
  aVM->RegisterFunction("SaveGlobals", "TNG_PapyrusUtil", SaveGlobals);
  aVM->RegisterFunction("GetClipCheck", "TNG_PapyrusUtil", GetClipCheck);
  aVM->RegisterFunction("GetAutoReveal", "TNG_PapyrusUtil", GetAutoReveal);
  aVM->RegisterFunction("SaveBoolValues", "TNG_PapyrusUtil", SaveBoolValues);
  aVM->RegisterFunction("GetRaceGrpNames", "TNG_PapyrusUtil", GetRaceGrpNames);
  aVM->RegisterFunction("GetRaceGrpAddn", "TNG_PapyrusUtil", GetRaceGrpAddn);
  aVM->RegisterFunction("GetRaceGrpMult", "TNG_PapyrusUtil", GetRaceGrpMult);
  aVM->RegisterFunction("SetRaceGrpAddn", "TNG_PapyrusUtil", SetRaceGrpAddn);
  aVM->RegisterFunction("SetRaceGrpMult", "TNG_PapyrusUtil", SetRaceGrpMult);
  aVM->RegisterFunction("GetAllPossibleAddons", "TNG_PapyrusUtil", GetAllPossibleAddons);
  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("SetActorAddn", "TNG_PapyrusUtil", SetActorAddn);
  aVM->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);
  aVM->RegisterFunction("SwapRevealing", "TNG_PapyrusUtil", SwapRevealing);
  return true;
}
