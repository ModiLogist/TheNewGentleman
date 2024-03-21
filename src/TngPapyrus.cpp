#include <TngCore.h>
#include <TngCoreBase.h>
#include <TngInis.h>
#include <TngPapyrus.h>

void TngPapyrus::SaveGlobals(RE::StaticFunctionTag*) { TngInis::SaveGlobals(); }

bool TngPapyrus::GetBoolValue(RE::StaticFunctionTag*, int aID) {
  if (TngInis::cNoneBoolID < aID && aID < TngInis::cBoolIDsCount) return TngInis::GetSettingBool(static_cast<TngInis::IniBoolIDs>(aID));
  return false;
}

void TngPapyrus::SetBoolValue(RE::StaticFunctionTag*, int aID, bool aValue) {
  if (TngInis::cNoneBoolID < aID && aID < TngInis::cBoolIDsCount) TngInis::SaveSettingBool(static_cast<TngInis::IniBoolIDs>(aID), aValue);
}

std::vector<std::string> TngPapyrus::GetRaceGrpNames(RE::StaticFunctionTag*) { return TngCoreBase::GetRaceGrpNames(); }

int TngPapyrus::GetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return Tng::pgErr;
  return TngCoreBase::GetRaceGrpAddn(static_cast<std::size_t>(aRaceIdx));
}

float TngPapyrus::GetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return -1.0f;
  return TngCoreBase::GetRaceGrpMult(static_cast<std::size_t>(aRaceIdx));
}

void TngPapyrus::SetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption) {
  if (aRaceIdx < 0) return;
  TngCore::UpdateRaceGrpAddn(static_cast<int>(aRaceIdx), aGenOption);
}

void TngPapyrus::SetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx, float aGenMult) {
  if (TngCoreBase::SetRaceGrpMult(static_cast<std::size_t>(aRaceIdx), aGenMult)) TngInis::SaveRaceMult(static_cast<std::size_t>(aRaceIdx), aGenMult);
}

bool TngPapyrus::GetAddonStatus(RE::StaticFunctionTag*, int aFemaleAddn) {
  if (aFemaleAddn < 0 || aFemaleAddn >= TngCoreBase::GetAddonCount(true)) return false;
  return TngCoreBase::GetAddonStatus(static_cast<int>(aFemaleAddn));
}

void TngPapyrus::SetAddonStatus(RE::StaticFunctionTag*, int aFemaleAddn, bool aStatus) {
  if (aFemaleAddn < 0 || aFemaleAddn >= TngCoreBase::GetAddonCount(true)) return;
  TngInis::SaveActiveAddon(aFemaleAddn, aStatus);
  TngCoreBase::SetAddonStatus(aFemaleAddn, aStatus);
}

std::vector<std::string> TngPapyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale) {
  auto lNames = TngCoreBase::GetAddonNames(aIsFemale);
  return std::vector<std::string>{lNames.begin(), lNames.end()};
}

int TngPapyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor) { return TngCore::CanModifyActor(aActor); }

int TngPapyrus::SetActorAddn(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption) {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  return TngCore::SetNPCSkin(lNPC, aGenOption);
}

int TngPapyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize) {   
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  return TngCore::SetCharSize(aActor, lNPC, aGenSize); }

bool TngPapyrus::SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor) {
  if (!aArmor) return false;
  return TngCore::SwapRevealing(aArmor);
}

bool TngPapyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept {
  aVM->RegisterFunction("SaveGlobals", "TNG_PapyrusUtil", SaveGlobals);
  aVM->RegisterFunction("GetBoolValue", "TNG_PapyrusUtil", GetBoolValue);
  aVM->RegisterFunction("SetBoolValue", "TNG_PapyrusUtil", SetBoolValue);
  aVM->RegisterFunction("GetRaceGrpNames", "TNG_PapyrusUtil", GetRaceGrpNames);
  aVM->RegisterFunction("GetRaceGrpAddn", "TNG_PapyrusUtil", GetRaceGrpAddn);
  aVM->RegisterFunction("GetRaceGrpMult", "TNG_PapyrusUtil", GetRaceGrpMult);
  aVM->RegisterFunction("SetRaceGrpAddn", "TNG_PapyrusUtil", SetRaceGrpAddn);
  aVM->RegisterFunction("SetRaceGrpMult", "TNG_PapyrusUtil", SetRaceGrpMult);
  aVM->RegisterFunction("GetAddonStatus", "TNG_PapyrusUtil", GetAddonStatus);
  aVM->RegisterFunction("SetAddonStatus", "TNG_PapyrusUtil", SetAddonStatus);
  aVM->RegisterFunction("GetAllPossibleAddons", "TNG_PapyrusUtil", GetAllPossibleAddons);
  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("SetActorAddn", "TNG_PapyrusUtil", SetActorAddn);
  aVM->RegisterFunction("SetActorSize", "TNG_PapyrusUtil", SetActorSize);
  aVM->RegisterFunction("SwapRevealing", "TNG_PapyrusUtil", SwapRevealing);
  return true;
}
