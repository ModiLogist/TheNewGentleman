ScriptName TNG_PapyrusUtil hidden

Int Function UpdateLogLvl(Int aLvl) Global Native

;For bool functions IDs: 
; 1: Exclude Player,
; 2: Mark mods with slot 52 as revealing by default,
; 3: Randomize Male addons among the active ones
Bool Function GetBoolValue(Int aiID) Global Native
Function SetBoolValue(Int aiID, Bool abValue) Global Native

String[] Function GetAllPossibleAddons(Bool abIsFemale) Global Native
Bool Function GetAddonStatus(Bool abIsFemale, Int aiAddon) Global Native
Function SetAddonStatus(Bool abIsFemale, Int aiAddon, Bool abStatus) Global Native

String[] Function GetRgNames() Global Native
String[] Function GetRgAddons(Int aiRgIndex) Global Native
Int Function GetRgAddn(Int aiRgIndex) Global Native
Function SetRgAddn(Int aiRgIndex, Int aiChoice) Global Native
Float Function GetRgMult(Int aiRgIndex) Global Native
Function SetRgMult(Int aiRgIndex, Float afMult) Global Native

Int Function CanModifyActor(Actor akActor) Global Native
String[] Function GetActorAddons(Actor akActor) Global Native
Int Function SetActorAddn(Actor akActor, Int aiChoice) Global Native
Int Function SetActorSize(Actor akActor, Int aiSizeCat) Global Native
String[] Function ActorItemsInfo(Actor akActor) Global Native
Bool Function SwapRevealing(Actor akActor, Int aiChoice) Global Native

String[] Function GetSlot52Mods() Global Native
;Behavior value: -1 -> Get behavior, 0 -> Set to Normal, 1 -> Set to Revealing
Bool Function Slot52ModBehavior(String asModName, Int aiBehavior) Global Native 

Function UpdateSettings() Global Native

String Function ShowLogLocation() Global Native
String Function GetErrDscr(Int aErrCode) Global Native
