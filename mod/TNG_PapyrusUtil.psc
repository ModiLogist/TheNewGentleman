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

String[] Function GetRGNames() Global Native
Float Function GetRGMult(Int rgId) Global Native
Function SetRGAddn(Int rgId, Int aGenOption) Global Native
Int Function GetRGAddn(Int rgId) Global Native
Function SetRGMult(Int rgId, Float aGenMult) Global Native

Bool Function GetAddonStatus(Bool isFemale, Int aAddn) Global Native
Function SetAddonStatus(Bool isFemale, Int aAddn, Bool aStatus) Global Native

String[] Function GetAllPossibleAddons(Bool isFemale) Global Native
Int Function CanModifyActor(Actor actor) Global Native
Int Function SetActorAddn(Actor actor, Int aGenOption) Global Native
Int Function SetActorSize(Actor actor, Int genSize) Global Native

String[] Function GetSlot52Mods() Global Native
Bool Function Slot52ModBehavior(String aModName, Int aBehavior) Global Native ;aBehavior value: -1 -> Get behavior, 0 -> Set to Normal, 1 -> Set to Revealing
Bool Function SwapRevealing(Armor aArmor) Global Native

Function UpdateSettings() Global Native


