ScriptName TNG_PapyrusUtil hidden

Function SaveGlobals() Global Native

;For bool functions IDs: 1: Female Revealing, 2: Male Revealing, 3: Double Check Clipping, 4: Exclude Player
Bool Function GetBoolValue(Int aID) Global Native
Function SetBoolValue(Int aID, Bool aValue) Global Native

String[] Function GetRaceGrpNames() Global Native
Float Function GetRaceGrpMult(Int aRaceIdx) Global Native
Function SetRaceGrpAddn(Int aRaceIdx, Int aGenOption) Global Native
Int Function GetRaceGrpAddn(Int aRaceIdx) Global Native
Function SetRaceGrpMult(Int aRaceIdx, Float aGenMult) Global Native

Bool Function GetAddonStatus(Int aFemaleAddn) Global Native
Function SetAddonStatus(Int aFemaleAddn, Bool aStatus) Global Native

String[] Function GetAllPossibleAddons(Bool aIsFemale) Global Native
Int Function CanModifyActor(Actor aActor) Global Native
Int Function SetActorAddn(Actor aActor, Int aGenOption) Global Native
Int Function SetActorSize(Actor aActor, Int aGenSize) Global Native

Bool Function SwapRevealing(Armor aArmor) Global Native


