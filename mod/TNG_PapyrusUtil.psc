ScriptName TNG_PapyrusUtil hidden


;For bool functions IDs: 
; 0: Exclude player,
; 1: Check player addon after load,
; 2: Check NPCs addon after load,
; 3: RSV compatibility (Used internally)
; 4: Mark mods with slot 52 as revealing by default,
; 5: Allow user to choose the behavior of mods with slot 52,
; 6: Randomize Male addons among the active ones
; 7: Go to bed compatibility
; 8: Show all races in the MCM
Bool Function GetBoolValue(Int aiID) Global Native
Function SetBoolValue(Int aiID, Bool abValue) Global Native

;For int functions IDs:
; 0: Dynamic Activation Key Integration 
; 1: Setup NPCs
; 2: Raise Genitals
; 3: Lower Genitals
; 4: Swap Revealing/Covering
; 5: Why Problem
Int Function GetIntValue(Int aiID) Global Native
Function SetIntValue(Int aiID, Int aiValue) Global Native

;For float functions IDs:
; 0-4: Genitals scales for XS, S, M, L, XL
; 5: Chance of women to have an addon
Float Function GetFloatValue(Int aiID) Global Native
Function SetFloatValue(Int aiID, Float afValue) Global Native

Int Function GetAllAddonsCount(Bool abIsFemale) Global Native
String[] Function GetAllPossibleAddons(Bool abIsFemale) Global Native
Bool Function GetAddonStatus(Bool abIsFemale, Int aiAddon) Global Native
Function SetAddonStatus(Bool abIsFemale, Int aiAddon, Bool abStatus) Global Native

String[] Function GetRgNames() Global Native
String Function GetRgInfo(Int aiRgIndex) Global Native
String[] Function GetRgAddons(Int aiRgIndex) Global Native
Int Function GetRgAddon(Int aiRgIndex) Global Native
Function SetRgAddon(Int aiRgIndex, Int aiChoice) Global Native
Float Function GetRgMult(Int aiRgIndex) Global Native
Function SetRgMult(Int aiRgIndex, Float afMult) Global Native

Int Function CanModifyActor(Actor akActor) Global Native
String[] Function GetActorAddons(Actor akActor) Global Native
Armor Function GetActorAddon(Actor akActor) Global Native ;Returns None if there is no addon or another error happens
Int Function SetActorAddon(Actor akActor, Int aiChoice) Global Native
Int Function GetActorSize(Actor akActor) Global Native ;Returns -1 if there is an issue
Int Function SetActorSize(Actor akActor, Int aiSizeCat) Global Native
String[] Function ActorItemsInfo(Actor akActor) Global Native
Bool Function SwapRevealing(Actor akActor, Int aiChoice) Global Native

String[] Function GetSlot52Mods() Global Native
;Behavior value: -1 -> Get behavior, 0 -> Set to Normal, 1 -> Set to Revealing
Bool Function Slot52ModBehavior(String asModName, Int aiBehavior) Global Native 

Int Function UpdateLogLvl(Int aiLogLevel) Global Native
String Function ShowLogLocation() Global Native
String Function GetErrDscr(Int aiErrCode) Global Native

String Function WhyProblem(Actor akActor, Int aiIssueID) Global Native
