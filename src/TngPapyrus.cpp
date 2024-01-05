#include <TngInis.h>
#include <TngPapyrus.h>
#include <TngSizeShape.h>

void TngUtil::UpdateActor(RE::StaticFunctionTag*, RE::Actor* aActor, int aGenOption, int aGenSize) noexcept {
  if (!aActor) return;
  if (aGenOption > -1) TngSizeShape::SetActorSkin(aActor, aGenOption);
  TngSizeShape::SetActorSize(aActor, aGenSize);
}

void TngUtil::UpdateRace(RE::StaticFunctionTag*, int aRaceIdx, int aGenOption, float aGenMult) noexcept {
  TngSizeShape::GetSingleton()->genitalChoices[aRaceIdx] = aGenOption;
  TngSizeShape::GetSingleton()->genitalSizes[aRaceIdx] = aGenMult;
}

void TngUtil::SetAutoRevealing(RE::StaticFunctionTag*, bool aFemaleArmor, bool aMaleArmor) noexcept {
  TngInis::GetSingleton()->FAutoReveal = aFemaleArmor;
  TngInis::GetSingleton()->MAutoReveal = aMaleArmor;
  Tng::gLogger::info("it works!");
}

bool TngUtil::CanModifyActor(RE::StaticFunctionTag*, RE::Actor* aActor) { return TngSizeShape::CanModifyActor(aActor); }

bool TngUtil::GetFAutoReveal(RE::StaticFunctionTag* ) { return TngInis::GetSingleton()->FAutoReveal; }

bool TngUtil::GetMAutoReveal(RE::StaticFunctionTag*) { return TngInis::GetSingleton()->MAutoReveal; }

int TngUtil::GetGenType(RE::StaticFunctionTag*, int aRaceIdx) noexcept { return TngSizeShape::GetSingleton()->genitalChoices[aRaceIdx]; }

float TngUtil::GetGenSize(RE::StaticFunctionTag*, int aRaceIdx) noexcept { return TngSizeShape::GetSingleton()->genitalSizes[aRaceIdx]; }

void TngUtil::BindPapyrus(RE::BSScript::IVirtualMachine* aVM) noexcept { 
  aVM->RegisterFunction("UpdateActor", "TNG_PapyrusUtil", UpdateActor);
  aVM->RegisterFunction("UpdateRace", "TNG_PapyrusUtil", UpdateRace);
  aVM->RegisterFunction("SetAutoRevealing", "TNG_PapyrusUtil", SetAutoRevealing);
  aVM->RegisterFunction("CanModifyActor", "TNG_PapyrusUtil", CanModifyActor);
  aVM->RegisterFunction("GetFAutoReveal", "TNG_PapyrusUtil", GetFAutoReveal);
  aVM->RegisterFunction("GetMAutoReveal", "TNG_PapyrusUtil", GetMAutoReveal);
  aVM->RegisterFunction("GetGenType", "TNG_PapyrusUtil", GetGenType);
  aVM->RegisterFunction("GetGenSize", "TNG_PapyrusUtil", GetGenSize);
}


