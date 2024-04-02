#include <Core.h>
#include <Base.h>
#include <Inis.h>
#include <Papyrus.h>

bool Papyrus::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept {
  aVM->RegisterFunction("UpdateLogLvl", "TNG_PapyrusUtil", UpdateLogLvl);
  aVM->RegisterFunction("ShowLogLocation", "TNG_PapyrusUtil", ShowLogLocation);
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

int Papyrus::UpdateLogLvl(RE::StaticFunctionTag*, int aLogLvl) {
  Inis::SetLogLvl(aLogLvl < 0 ? aLogLvl : aLogLvl + spdlog::level::info);
  return int(Inis::GetLogLvl()) - int(spdlog::level::info);
}

std::string Papyrus::ShowLogLocation(RE::StaticFunctionTag*) { 
  auto lMaybeDir{Tng::gLogger::log_directory};
  std::filesystem::path lDir = lMaybeDir().value_or("$TNG_LDN");
  return lDir.string();
}

void Papyrus::SaveGlobals(RE::StaticFunctionTag*) { Inis::SaveGlobals(); }

bool Papyrus::GetBoolValue(RE::StaticFunctionTag*, int aID) {
  if (Inis::cNoneBoolID < aID && aID < Inis::cBoolIDsCount) return Inis::GetSettingBool(static_cast<Inis::IniBoolIDs>(aID));
  return false;
}

void Papyrus::SetBoolValue(RE::StaticFunctionTag*, int aID, bool aValue) {
  if (Inis::cNoneBoolID < aID && aID < Inis::cBoolIDsCount) Inis::SaveSettingBool(static_cast<Inis::IniBoolIDs>(aID), aValue);
}

std::vector<std::string> Papyrus::GetRaceGrpNames(RE::StaticFunctionTag*) { return Base::GetRaceGrpNames(); }

int Papyrus::GetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return Tng::pgErr;
  return Base::GetRaceGrpAddn(static_cast<std::size_t>(aRaceIdx));
}

float Papyrus::GetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx) {
  if (aRaceIdx < 0) return -1.0f;
  return Base::GetRaceGrpMult(static_cast<std::size_t>(aRaceIdx));
}

void Papyrus::SetRaceGrpAddn(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption) {
  if (aRaceIdx < 0) return;
  Core::UpdateRaceGrpAddn(static_cast<int>(aRaceIdx), aGenOption);
}

void Papyrus::SetRaceGrpMult(RE::StaticFunctionTag*, int aRaceIdx, float aGenMult) {
  if (Base::SetRaceGrpMult(static_cast<std::size_t>(aRaceIdx), aGenMult)) Inis::SaveRaceMult(static_cast<std::size_t>(aRaceIdx), aGenMult);
}

bool Papyrus::GetAddonStatus(RE::StaticFunctionTag*, int aFemaleAddn) {
  if (aFemaleAddn < 0 || aFemaleAddn >= Base::GetAddonCount(true)) return false;
  return Base::GetAddonStatus(static_cast<int>(aFemaleAddn));
}

void Papyrus::SetAddonStatus(RE::StaticFunctionTag*, int aFemaleAddn, bool aStatus) {
  if (aFemaleAddn < 0 || aFemaleAddn >= Base::GetAddonCount(true)) return;
  Inis::SaveActiveAddon(aFemaleAddn, aStatus);
  Base::SetAddonStatus(aFemaleAddn, aStatus);
}

std::vector<std::string> Papyrus::GetAllPossibleAddons(RE::StaticFunctionTag*, bool aIsFemale) {
  auto lNames = Base::GetAddonNames(aIsFemale);
  return std::vector<std::string>{lNames.begin(), lNames.end()};
}

int Papyrus::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor) { return Core::CanModifyActor(aActor); }

int Papyrus::SetActorAddn(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption) {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  return Core::SetNPCSkin(lNPC, aGenOption);
}

int Papyrus::SetActorSize(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenSize) {
  const auto lNPC = aActor ? aActor->GetActorBase() : nullptr;
  if (!aActor || !lNPC) return Tng::npcErr;
  return Core::SetCharSize(aActor, lNPC, aGenSize);
}

bool Papyrus::SwapRevealing(RE::StaticFunctionTag*, RE::TESObjectARMO* aArmor) {
  if (!aArmor) return false;
  return Core::SwapRevealing(aArmor);
}
