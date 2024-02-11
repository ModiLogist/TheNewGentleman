ScriptName TNG_PapyrusUtil hidden

Function SaveGlobals() Global Native

Bool Function GetClipCheck() Global Native
Bool Function GetAutoReveal(Bool aIsFemale) Global Native
Function SaveBoolValues(Int aID, Bool aValue) Global Native;IDs: 1: Female Revealing, 2: Male Revealing, 3: Double Check Clipping

String[] Function GetRaceGrpNames() Global Native
Float Function GetRaceGrpMult(Int aRaceIdx) Global Native
Function SetRaceGrpAddn(Int aRaceIdx, Int aGenOption) Global Native
Int Function GetRaceGrpAddn(Int aRaceIdx) Global Native
Function SetRaceGrpMult(Int aRaceIdx, Float aGenMult) Global Native

String[] Function GetAllPossibleAddons(Bool aIsFemale) Global Native
Int Function CanModifyActor(Actor aActor) Global Native
Int Function SetActorAddn(Actor aActor, Int aGenOption) Global Native
Int Function SetActorSize(Actor aActor, Int aGenSize) Global Native

Bool Function SwapRevealing(Armor aArmor) Global Native


