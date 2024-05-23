ScriptName TNG_PapyrusUtil hidden

Int Function UpdateLogLvl(Int aLvl) Global Native
String Function ShowLogLocation() Global Native

;For bool functions IDs: 
; 1: Exclude Player,
; 2: Mark mods with slot 52 as revealing by default,
; 3: Allow a combination of revealing and covering in mods with slot 52
; 4: Randomize Male addons among the active ones
Bool Function GetBoolValue(Int aID) Global Native
Function SetBoolValue(Int aID, Bool aValue) Global Native

String[] Function GetRaceGrpNames() Global Native
Float Function GetRaceGrpMult(Int aRaceIdx) Global Native
Function SetRaceGrpAddn(Int aRaceIdx, Int aGenOption) Global Native
Int Function GetRaceGrpAddn(Int aRaceIdx) Global Native
Function SetRaceGrpMult(Int aRaceIdx, Float aGenMult) Global Native

Bool Function GetAddonStatus(Bool aIsFemale, Int aAddn) Global Native
Function SetAddonStatus(Bool aIsFemale, Int aAddn, Bool aStatus) Global Native

String[] Function GetAllPossibleAddons(Bool aIsFemale) Global Native
Int Function CanModifyActor(Actor aActor) Global Native
Int Function SetActorAddn(Actor aActor, Int aGenOption) Global Native
Int Function SetActorSize(Actor aActor, Int aGenSize) Global Native

String[] Function GetSlot52Mods() Global Native
Bool Function Slot52ModBehavior(String aModName, Int aBehavior) Global Native ;aBehavior value: -1 -> Get behavior, 0 -> Set to Normal, 1 -> Set to Revealing
Bool Function SwapRevealing(Armor aArmor) Global Native

Function SetPlayerInfo(Actor aPlayer, Int aPlayerAddn) Global Native
Int Function GetPlayerAddn() Global Native


Function UpdateSettings() Global Native


