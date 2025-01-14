Scriptname TNG_MCM extends SKI_ConfigBase

;Property
GlobalVariable[] Property GlobalSizes auto
GlobalVariable Property DAKIntegration auto
GlobalVariable Property NPCKey auto
GlobalVariable Property GenUpKey auto
GlobalVariable Property GenDownKey auto
GlobalVariable Property RevealKey auto
GlobalVariable Property WomenChance auto
Keyword Property TngCovering auto
FormList Property Gentified auto
Bool Property Notifs auto
GlobalVariable Property PlayerSkin auto
Int Property PlayerSize auto
Actor Property PlayerRef auto

;Constants
Int ciExPCSize
Int ci52DefOot
Int ci52Custom
Int ciMalRandA
Int ciLKey

;Kinda constant
Float[] cFSizeDefaults
String[] cSLogOptions
String[] cS52Options

;Local variable
String[] fSRaces
String[] fSSizeGlobals
String[] fSAllS52Mods
Actor fkLastActor = None
GlobalVariable fkDAK = None
Int fiPos = 0
Int fiLastActor = -1
Int fiL = 0

;Pointer to UI
Int[] fIRaceTypeHdls
Int[] fIRaceSizeHdls
Int[] fIGlblSizeHdls
Int[] fIFemAddonHdls
Int[] fIMalAddonHdls
Int[] fI52ModHdls 

Int fiNotifsHdl
Int fiPCSizeHdl

Int fiDAKHdl
Int fiNPCKeyHdl
Int fiRevealKeyHdl
Int fiUpKeyHdl
Int fiDownKeyHdl
Int fiWomenChanceHdl
Int fiRandMalAddnHdl
Int fiLogLvlHdl
Int fiLogDirHdl

Int fi52DefBehaviorHdl

Int Function GetVersion()
  Return 7
EndFunction

Event OnConfigInit()
  Pages = new String[4]
  Pages[0] = "$TNG_Gnl"
  Pages[1] = "$TNG_Rac"
  Pages[2] = "$TNG_Add"  
  Pages[3] = "$TNG_S52"
  
  fIRaceTypeHdls = new Int[120]
  fIRaceSizeHdls = new Int[120]
  fI52ModHdls = new Int[126]
  fIGlblSizeHdls = new Int[5]
  fIFemAddonHdls = new Int[120]  
  fIMalAddonHdls = new Int[120]  
  
  fSSizeGlobals = new String[5]
  fSSizeGlobals[0] = "$TNG_SXS"
  fSSizeGlobals[1] = "$TNG_S_S"
  fSSizeGlobals[2] = "$TNG_S_M"
  fSSizeGlobals[3] = "$TNG_S_L"
  fSSizeGlobals[4] = "$TNG_SXL"  
  
  cFSizeDefaults = new Float[5]
  cFSizeDefaults[0] = 0.8
  cFSizeDefaults[1] = 0.9
  cFSizeDefaults[2] = 1.0
  cFSizeDefaults[3] = 1.2
  cFSizeDefaults[4] = 1.4
  
  cSLogOptions = new String[5]
  cSLogOptions[0] = "$TNG_LLA"
  cSLogOptions[1] = "$TNG_LLW"
  cSLogOptions[2] = "$TNG_LLE"
  cSLogOptions[3] = "$TNG_LLC"
  cSLogOptions[4] = "$TNG_LLO"
  
  cS52Options = new String[3]
  cS52Options[0] = "$TNG_52C"
  cS52Options[1] = "$TNG_52R"
  cS52Options[2] = "$TNG_52U"
	
	ciExPCSize = 0
  ci52DefOot = 1
  ciMalRandA = 2
	ciLKey = 56
EndEvent

Event OnVersionUpdate(Int aiVersion)
  If (aiVersion > CurrentVersion)
    OnConfigInit()
  EndIf
	If (CurrentVersion < 7)
    If (PlayerSkin.GetValueInt() == -1)
			PlayerSkin.SetValueInt(-2)
			Return
		EndIf
	EndIf
EndEvent

Event OnConfigOpen()    
  fSRaces = TNG_PapyrusUtil.GetRGNames()
  fSAllS52Mods = TNG_PapyrusUtil.GetSlot52Mods();
EndEvent

Event OnConfigClose()
  TNG_PapyrusUtil.UpdateSettings()
EndEvent

Event OnGameReload()
  Parent.OnGameReload()
  
  Gentified.Revert()
  
  If Game.GetModByName("Dynamic Activation Key.esp")
    fkDAK = Game.GetFormFromFile(0x801, "Dynamic Activation Key.esp") As GlobalVariable
  EndIf
  If NPCKey.GetValueInt() > 0
    RegisterForKey(NPCKey.GetValueInt())
  EndIf
  If RevealKey.GetValueInt() > 0
    RegisterForKey(RevealKey.GetValueInt())
  EndIf
  If GenUpKey.GetValueInt() > 0
    RegisterForKey(GenUpKey.GetValueInt())
  EndIf
  If GenDownKey.GetValueInt() > 0
    RegisterForKey(GenDownKey.GetValueInt())
  EndIf
  Int res = TNG_PapyrusUtil.SetActorAddn(PlayerRef, PlayerSkin.GetValueInt())
  PlayerRef.QueueNiNodeUpdate()
  If res < 0 
    Debug.Notification("$TNG_WPT")
    PlayerSkin.SetValueInt(-2)
    TNG_PapyrusUtil.SetActorAddn(PlayerRef, PlayerSkin.GetValueInt())
    HandleWarnings(res)
  EndIf  
  res = TNG_PapyrusUtil.SetActorSize(PlayerRef, PlayerSize)
  If res < 0     
    Debug.Notification("$TNG_WPS")
    PlayerSize = -2
    TNG_PapyrusUtil.SetActorSize(PlayerRef, PlayerSize)
    HandleWarnings(res)
  EndIf
EndEvent

Event OnOptionHighlight(Int aiOption)

  If CurrentPage == Pages[0]
    If aiOption == fiDAKHdl
      SetInfoText("$TNG_HGD")
      Return
    EndIf
    If aiOption == fiNotifsHdl
      SetInfoText("$TNG_HGN")
      Return
    EndIf
    If aiOption == fiPCSizeHdl
      SetInfoText("$TNG_HGE")
      Return
    EndIf
    If (aiOption == fiUpKeyHdl) || (aiOption == fiDownKeyHdl)
      SetInfoText("$TNG_HKU")
      Return
    EndIf
    If aiOption == fiNPCKeyHdl
      SetInfoText("$TNG_HKN")
      Return
    EndIf    
    If aiOption == fiRevealKeyHdl
      SetInfoText("$TNG_HKS")
      Return
    EndIf
    If aiOption == fiLogLvlHdl
      SetInfoText("$TNG_HLL")
      Return
    EndIf
    Int liCurr = fSSizeGlobals.Length
    While liCurr
      liCurr -= 1
      If aiOption == fIGlblSizeHdls[liCurr]
        SetInfoText("$TNG_H_S")
        Return
      EndIf
    EndWhile
    Return
  EndIf  
  
  If CurrentPage == Pages[1]
    Int liCurr = fSRaces.Length
    While liCurr
      liCurr -= 1
      If aiOption == fIRaceTypeHdls[liCurr]
        SetInfoText("$TNG_HOA")
        Return
      EndIf
      If aiOption == fIRaceSizeHdls[liCurr]
        SetInfoText("$TNG_HOS")
        Return
      EndIf
    EndWhile
    Return
  EndIf
  
  If CurrentPage == Pages[2]
    If aiOption == fiWomenChanceHdl
      SetInfoText("$TNG_HAC")
      Return
    EndIf
    If aiOption == fiRandMalAddnHdl
      SetInfoText("$TNG_HRM")
      Return
    EndIf
    Int liCurr = 0		
		String[] lSMalOptions = TNG_PapyrusUtil.GetAllPossibleAddons(False)
		String[] lSFemOptions = TNG_PapyrusUtil.GetAllPossibleAddons(True)
    While liCurr < lSFemOptions.Length
      If aiOption == fIFemAddonHdls[liCurr]
        SetInfoText("$TNG_HAD")
        Return
      EndIf
      liCurr += 1
    EndWhile
    While liCurr < lSMalOptions.Length
      If aiOption == fIMalAddonHdls[liCurr]
        SetInfoText("$TNG_HAD")
        Return
      EndIf
      liCurr += 1
    EndWhile
    Return
  EndIf
  
  If CurrentPage == Pages[3]
    If aiOption == fi52DefBehaviorHdl
      SetInfoText("$TNG_H52")
      Return
    Else
      SetInfoText("$TNG_HMR")
      Return
    EndIf
  EndIf
  
EndEvent

Event OnPageReset(String asPage)
  If asPage == Pages[0]
    fiNotifsHdl = AddToggleOption("$TNG_GNT", Notifs)
    fiPCSizeHdl = AddToggleOption("$TNG_GEP", TNG_PapyrusUtil.GetBoolValue(ciExPCSize))
    AddHeaderOption("$TNG_KyH")
    AddHeaderOption("")
    fkDAK = None
    If Game.GetModByName("Dynamic Activation Key.esp")
      fkDAK = Game.GetFormFromFile(0x801, "Dynamic Activation Key.esp") As GlobalVariable
      If fkDAK
        fiDAKHdl = AddToggleOption("$TNG_DAK", DAKIntegration.GetValue()>1)
        AddEmptyOption()
      EndIf
    EndIf
    If !fkDAK
      DAKIntegration.SetValue(0)
    EndIf
    fiNPCKeyHdl = AddKeyMapOption("$TNG_K_N", NPCKey.GetValueInt())
    fiRevealKeyHdl = AddKeyMapOption("$TNG_K_R", RevealKey.GetValueInt())
    fiUpKeyHdl = AddKeyMapOption("$TNG_K_U", GenUpKey.GetValueInt())
    fiDownKeyHdl = AddKeyMapOption("$TNG_K_D", GenDownKey.GetValueInt())
    AddHeaderOption("$TNG_SOH")
    AddHeaderOption("$TNG_L_H")
    Int liGlbSize = 0
    While liGlbSize < fSSizeGlobals.Length
      fIGlblSizeHdls[liGlbSize] = AddSliderOption(fSSizeGlobals[liGlbSize], GlobalSizes[liGlbSize].GetValue(), "{2}")
      If liGlbSize == 0
        Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(-1)
        fiLogLvlHdl = AddMenuOption("$TNG_L_T", cSLogOptions[liLvl])
      EndIf
      If liGlbSize == 1
        fiLogDirHdl = AddToggleOption("$TNG_L_D", False)
      EndIf
      If liGlbSize > 1
        AddEmptyOption()
      EndIf
      liGlbSize += 1
    EndWhile 
    Return
  EndIf
  
  If asPage == Pages[1]
    AddHeaderOption("$TNG_OGS")
    AddHeaderOption("$TNG_OGT")
    Int liRace = 0
    While liRace < fSRaces.Length      
      fIRaceSizeHdls[liRace] = AddSliderOption(fSRaces[liRace], TNG_PapyrusUtil.GetRGMult(liRace), "{2}")
			String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRace)
			Int liAddnIdx = TNG_PapyrusUtil.GetRGAddn(liRace) + 2
			If (lSOptions.Length == 2) || (liAddnIdx < 0)
			  liAddnIdx = 1
			EndIf
      fIRaceTypeHdls[liRace] = AddMenuOption("", lSOptions[liAddnIdx])
      liRace += 1
    EndWhile
    Return
  EndIf
  
  If asPage == Pages[2]
    AddHeaderOption("$TNG_A_W")
    AddHeaderOption("$TNG_A_M")
		String[] lSMalOptions = TNG_PapyrusUtil.GetAllPossibleAddons(False)
		String[] lSFemOptions = TNG_PapyrusUtil.GetAllPossibleAddons(True)
    If lSFemOptions.Length == 0
      AddTextOption("$TNG_ANW", "")
    Else
      fiWomenChanceHdl = AddSliderOption("$TNG_AWC", WomenChance.GetValue(), "{0}%");
    EndIf
    If lSMalOptions.Length == 0
      AddTextOption("$TNG_ANM", "")
    Else
      fiRandMalAddnHdl = AddToggleOption("$TNG_AMR", TNG_PapyrusUtil.GetBoolValue(ciMalRandA))
    EndIf
    If (lSFemOptions.Length == 0) && (lSMalOptions.Length == 0)
      Return
    EndIf
    Int liCurr = 0
    While liCurr < lSFemOptions.Length || liCurr < lSMalOptions.Length
      If liCurr < lSFemOptions.Length
        fIFemAddonHdls[liCurr] = AddToggleOption(lSFemOptions[liCurr], TNG_PapyrusUtil.GetAddonStatus(True, liCurr))
      Else
        AddEmptyOption()
      EndIf
      If liCurr < lSMalOptions.Length
        If liCurr == 0
          AddToggleOption(lSMalOptions[liCurr], False, OPTION_FLAG_DISABLED)
        Else
          fIMalAddonHdls[liCurr] = AddToggleOption(lSMalOptions[liCurr], TNG_PapyrusUtil.GetAddonStatus(False, liCurr))
        EndIf
      Else
        AddEmptyOption()
      EndIf
      liCurr += 1
    EndWhile
    Return
  EndIf  
  
  If asPage == Pages[3]
    If fSAllS52Mods.Length == 0
      AddTextOption("$TNG_52N","")
      Return
    EndIf
    bool lb52DefR = TNG_PapyrusUtil.GetBoolValue(ci52DefOot)
    bool lb52Cust = TNG_PapyrusUtil.GetBoolValue(ci52Custom)
    Int li52Option = 0
    If lb52Cust
      li52Option = 2
    ElseIf lb52DefR
      li52Option = 1
    EndIf
    fi52DefBehaviorHdl = AddMenuOption("$TNG_52D", cS52Options[li52Option])
    AddEmptyOption()
    Int li52ModsFlag = OPTION_FLAG_DISABLED
    If li52Option == 2
      li52ModsFlag = OPTION_FLAG_NONE
    EndIf
    Int liModCount = fSAllS52Mods.Length
    While liModCount
      liModCount -= 1
      fI52ModHdls[liModCount] = AddToggleOption(fSAllS52Mods[liModCount], TNG_PapyrusUtil.Slot52ModBehavior(fSAllS52Mods[liModCount], -1), li52ModsFlag)
    EndWhile
    Return
  EndIf
  
EndEvent

Event OnOptionDefault(Int aiOption)
  Int liOpLoop = fSRaces.Length
  While liOpLoop
    liOpLoop -= 1
    If aiOption == fIRaceSizeHdls[liOpLoop]
      TNG_PapyrusUtil.SetRGMult(liOpLoop, 1.0)
      SetSliderOptionValue(fIRaceSizeHdls[liOpLoop], TNG_PapyrusUtil.GetRGMult(liOpLoop), "{2}")
      TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
      Return
    EndIf
    If aiOption == fIRaceTypeHdls[liOpLoop]
      TNG_PapyrusUtil.SetRGAddn(liOpLoop, -2)
			String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liOpLoop)
      SetMenuOptionValue(fIRaceTypeHdls[liOpLoop], lSOptions[TNG_PapyrusUtil.GetRGAddn(liOpLoop) + 2])
      Return
    EndIf
  EndWhile
  liOpLoop = fSSizeGlobals.Length
  While liOpLoop
    liOpLoop -= 1
    If aiOption == fIGlblSizeHdls[liOpLoop]      
      GlobalSizes[liOpLoop].SetValue(cFSizeDefaults[liOpLoop])
      SetSliderOptionValue(fIGlblSizeHdls[liOpLoop], GlobalSizes[liOpLoop].GetValue(), "{2}")
      TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
      Return
    EndIf
  EndWhile  
  liOpLoop = 0
	String[] lSFemOptions = TNG_PapyrusUtil.GetAllPossibleAddons(False)
  While liOpLoop < lSFemOptions.Length
    If aiOption == fIFemAddonHdls[liOpLoop]
      TNG_PapyrusUtil.SetAddonStatus(True, liOpLoop, False)
      SetToggleOptionValue(aiOption, False)
      Return
    EndIf
    liOpLoop += 1
  EndWhile
  liOpLoop = 0	
	String[] lSMalOptions = TNG_PapyrusUtil.GetAllPossibleAddons(False)
  While liOpLoop < lSMalOptions.Length
    If aiOption == fIMalAddonHdls[liOpLoop]
      TNG_PapyrusUtil.SetAddonStatus(False, liOpLoop, True)
      SetToggleOptionValue(aiOption, False)
      Return
    EndIf
    liOpLoop += 1
  EndWhile
  If aiOption == fiPCSizeHdl
    TNG_PapyrusUtil.SetBoolValue(ciExPCSize, False)
    SetToggleOptionValue(ciExPCSize, TNG_PapyrusUtil.GetBoolValue(ciExPCSize))
    Return
  EndIf
  If aiOption == fiNotifsHdl
    Notifs = True
    SetToggleOptionValue(fiNotifsHdl, True)
    Return
  EndIf
  If aiOption == fiDAKHdl
    DAKIntegration.SetValue(0)
    SetToggleOptionValue(fiDAKHdl, False)
    Bool lbResetAll = ShowMessage("$TNG_DKN", true, "$TNG_Yes", "$TNG__No")
    If lbResetAll
      OnOptionDefault(fiNPCKeyHdl)
      OnOptionDefault(fiRevealKeyHdl)
      OnOptionDefault(fiUpKeyHdl)
      OnOptionDefault(fiDownKeyHdl)
    EndIf
    Return
  EndIf
  If aiOption == fiNPCKeyHdl
    If NPCKey.GetValueInt() > 0
      UnregisterForKey(NPCKey.GetValueInt())
      NPCKey.SetValueInt(-1)
      SetKeymapOptionValue(fiNPCKeyHdl, -1)
    EndIf
    Return
  EndIf
  If aiOption == fiRevealKeyHdl
    If RevealKey.GetValueInt() > 0
      UnregisterForKey(RevealKey.GetValueInt())
      RevealKey.SetValueInt(-1)
      SetKeymapOptionValue(fiRevealKeyHdl, -1)
    EndIf
    Return
  EndIf
  If aiOption == fiUpKeyHdl
    If GenUpKey.GetValueInt() > 0
      UnregisterForKey(GenUpKey.GetValueInt())
      GenUpKey.SetValueInt(-1)
      SetKeymapOptionValue(fiUpKeyHdl, -1)
    EndIf
    Return
  EndIf
  If aiOption == fiDownKeyHdl
    If GenDownKey.GetValueInt() > 0
      UnregisterForKey(GenDownKey.GetValueInt())
      GenDownKey.SetValueInt(-1)
      SetKeymapOptionValue(fiDownKeyHdl, -1)
    EndIf
    Return
  EndIf
  If aiOption == fiWomenChanceHdl    
    WomenChance.SetValue(20)
    SetSliderOptionValue(fiWomenChanceHdl, WomenChance.GetValue(), "{0}%")
    Return
  EndIf
  If aiOption == fiRandMalAddnHdl
    TNG_PapyrusUtil.SetBoolValue(ciMalRandA,False)
    SetToggleOptionValue(fiRandMalAddnHdl,False)
    Return
  EndIf
  If aiOption == fiLogLvlHdl
    Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(0)
    SetMenuOptionValue(fiLogLvlHdl, cSLogOptions[liLvl])
  EndIf
  If aiOption == fi52DefBehaviorHdl
    TNG_PapyrusUtil.SetBoolValue(ci52DefOot, False)
    TNG_PapyrusUtil.SetBoolValue(ci52Custom, False)    
    SetMenuOptionValue(fi52DefBehaviorHdl, 0)
    Update52Behaviors(0)
  EndIf
EndEvent

Event OnOptionMenuOpen(Int aiOption)
  If aiOption == fiLogLvlHdl
    SetMenuDialogOptions(cSLogOptions)
    Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(-1)
    SetMenuDialogStartIndex(liLvl)
    Return
  EndIf
  If aiOption == fi52DefBehaviorHdl
    bool lb52DefR = TNG_PapyrusUtil.GetBoolValue(ci52DefOot)
    bool lb52Cust = TNG_PapyrusUtil.GetBoolValue(ci52Custom)
    Int li52Option = 0
    If lb52Cust
      li52Option = 2
    ElseIf lb52DefR
      li52Option = 1
    EndIf
    SetMenuDialogOptions(cS52Options)
    SetMenuDialogStartIndex(li52Option)
    Return
  EndIf
  Int liRace = fSRaces.Length
  While liRace
    liRace -= 1
    If aiOption == fIRaceTypeHdls[liRace]
		  String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRace)
      SetMenuDialogOptions(lSOptions)
      SetMenuDialogStartIndex(TNG_PapyrusUtil.GetRGAddn(liRace) + 2)
      Return
    EndIf
  EndWhile  
EndEvent

Event OnOptionMenuAccept(Int aiOption, Int aiChoice)  
  If aiOption == fiLogLvlHdl   
    Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(aiChoice) 
    SetMenuOptionValue(fiLogLvlHdl, cSLogOptions[liLvl])
    Return
  EndIf
  If aiOption == fi52DefBehaviorHdl
    Update52Behaviors(aiChoice)
    SetMenuOptionValue(fi52DefBehaviorHdl, cS52Options[aiChoice])
    Return
  EndIf
  Int liRace = fSRaces.Length
  While liRace
    liRace -= 1
    If aiOption == fIRaceTypeHdls[liRace]
      TNG_PapyrusUtil.SetRGAddn(liRace, aiChoice - 2)
			String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRace)
      SetMenuOptionValue(fIRaceTypeHdls[liRace], lSOptions[aiChoice])
      Return
    EndIf
  EndWhile
EndEvent

Event OnOptionSliderOpen(Int aiOption)
  If CurrentPage == Pages[0]
    Int liSize = fSSizeGlobals.Length
    While liSize
      liSize -= 1
      If aiOption == fIGlblSizeHdls[liSize]
        SetSliderDialogStartValue(GlobalSizes[liSize].GetValue())
        SetSliderDialogDefaultValue(cFSizeDefaults[liSize])
        SetSliderDialogRange(0.1, 4.0)
        SetSliderDialogInterval(0.01)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  If CurrentPage == Pages[1]
    Int liRace = fSRaces.Length
    While liRace
      liRace -= 1
      If aiOption == fIRaceSizeHdls[liRace]
        SetSliderDialogStartValue(TNG_PapyrusUtil.GetRGMult(liRace))
        SetSliderDialogDefaultValue(1.0)
        SetSliderDialogRange(0.1, 2.0)
        SetSliderDialogInterval(0.01)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  If aiOption == fiWomenChanceHdl
    SetSliderDialogStartValue(WomenChance.GetValue())
    SetSliderDialogDefaultValue(20)
    SetSliderDialogRange(0, 100)
    SetSliderDialogInterval(1)
    Return
  EndIf
EndEvent

Event OnOptionSliderAccept(Int aiOption, Float afValue)
  If CurrentPage == Pages[0]
    Int liSize = fSSizeGlobals.Length
    While liSize
      liSize -= 1
      If aiOption == fIGlblSizeHdls[liSize]
        GlobalSizes[liSize].SetValue(afValue)
        SetSliderOptionValue(fIGlblSizeHdls[liSize], GlobalSizes[liSize].GetValue(), "{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  If CurrentPage == Pages[1]
    Int liRace = fSRaces.Length
    While liRace
      liRace -= 1
      If aiOption == fIRaceSizeHdls[liRace]
        TNG_PapyrusUtil.SetRGMult(liRace, afValue)
        SetSliderOptionValue(fIRaceSizeHdls[liRace], TNG_PapyrusUtil.GetRGMult(liRace), "{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  If aiOption == fiWomenChanceHdl
    WomenChance.SetValue(afValue)
    SetSliderOptionValue(fiWomenChanceHdl, WomenChance.GetValue(), "{0}%")
    Return
  EndIf
EndEvent

Event OnOptionSelect(Int aiOption)
  If CurrentPage == Pages[0]
    If aiOption == fiPCSizeHdl
      TNG_PapyrusUtil.SetBoolValue(ciExPCSize, !TNG_PapyrusUtil.GetBoolValue(ciExPCSize))    
      SetToggleOptionValue(fiPCSizeHdl, TNG_PapyrusUtil.GetBoolValue(ciExPCSize))
      If TNG_PapyrusUtil.GetBoolValue(ciExPCSize) 
        PlayerSize = -1
      EndIf
      Return
    EndIf
    If aiOption == fiNotifsHdl
      Notifs = !Notifs
      SetToggleOptionValue(fiNotifsHdl, Notifs)
      Return
    EndIf
    If aiOption == fiDAKHdl
      DAKIntegration.SetValue(2.0 - DAKIntegration.GetValue())
      SetToggleOptionValue(fiDAKHdl, DAKIntegration.GetValue() > 1)
      If DAKIntegration.GetValue() < 1
        Bool lbResetAll = ShowMessage("$TNG_DKN", true, "$TNG_Yes", "$TNG__No")
        If lbResetAll
          OnOptionDefault(fiNPCKeyHdl)
          OnOptionDefault(fiRevealKeyHdl)
          OnOptionDefault(fiUpKeyHdl)
          OnOptionDefault(fiDownKeyHdl)
        EndIf
      EndIf
      Return
    EndIf
    If aiOption == fiLogDirHdl
      ShowMessage(TNG_PapyrusUtil.ShowLogLocation(), true, "Ok")
      Return
    EndIf
    Return
  EndIf
  If CurrentPage == Pages[2]
    If aiOption == fiRandMalAddnHdl
      TNG_PapyrusUtil.SetBoolValue(ciMalRandA, !TNG_PapyrusUtil.GetBoolValue(ciMalRandA))
      SetToggleOptionValue(fiRandMalAddnHdl, TNG_PapyrusUtil.GetBoolValue(ciMalRandA))
      Return
    EndIf
    Int liCurr = 0
		String[] lSMalOptions = TNG_PapyrusUtil.GetAllPossibleAddons(False)
		String[] lSFemOptions = TNG_PapyrusUtil.GetAllPossibleAddons(True)
    While liCurr < lSFemOptions.Length
      If aiOption == fIFemAddonHdls[liCurr]
        TNG_PapyrusUtil.SetAddonStatus(True, liCurr, !TNG_PapyrusUtil.GetAddonStatus(True, liCurr))
        SetToggleOptionValue(fIFemAddonHdls[liCurr], TNG_PapyrusUtil.GetAddonStatus(True, liCurr))
        Return
      EndIf
      liCurr += 1
    EndWhile
    liCurr = 0
    While liCurr < lSMalOptions.Length
      If aiOption == fIMalAddonHdls[liCurr]
        TNG_PapyrusUtil.SetAddonStatus(False, liCurr, !TNG_PapyrusUtil.GetAddonStatus(False, liCurr))
        SetToggleOptionValue(fIMalAddonHdls[liCurr], TNG_PapyrusUtil.GetAddonStatus(False, liCurr))
        Return
      EndIf
      liCurr += 1
    EndWhile
    Return
  EndIf
  If CurrentPage == Pages[3]
    Int liModCount = fSAllS52Mods.Length   
    While liModCount
      liModCount -= 1
      If aiOption == fI52ModHdls[liModCount]
        Int liNew = -1
        If TNG_PapyrusUtil.Slot52ModBehavior(fSAllS52Mods[liModCount], -1)
          liNew = 0
        Else
          liNew = 1
        EndIf
        TNG_PapyrusUtil.Slot52ModBehavior(fSAllS52Mods[liModCount], liNew)
        SetToggleOptionValue(fI52ModHdls[liModCount], liNew==1)
        Return
      EndIf
    EndWhile  
    Return
  EndIf
EndEvent

Event OnOptionKeyMapChange(Int aiOption, Int aiKeyCode, String asConflictControl, String asConflictName)
  If !fkDAK || (DAKIntegration.GetValue() < 1)
    Bool lbContinue = True
    String lsNotif
    If asConflictControl != ""
      If (asConflictName != "")
        lsNotif = "This key is already mapped to:\n\"" + asConflictControl + "\"\n(" + asConflictName + ")\n\nAre you sure you want to continue?"
      Else
        lsNotif = "This key is already mapped to:\n\"" + asConflictControl + "\"\n\nAre you sure you want to continue?"
      EndIf
      lbContinue = ShowMessage(lsNotif, true, "$TNG_Yes", "$TNG__No")  
      If (!lbContinue) 
        Return
      EndIf
    EndIf
  EndIf
  If (aiOption == fiNPCKeyHdl) || (aiOption == fiRevealKeyHdl) || (aiOption == fiUpKeyHdl) || (aiOption == fiDownKeyHdl)
    UpdateKey(aiOption, aiKeyCode)
    SetKeymapOptionValue(aiOption, aiKeyCode)
  EndIf
EndEvent

Event OnKeyDown(Int aiKey)
  If (DAKIntegration.GetValue() > 1) && !fkDAK
    Debug.Notification("$TNG_W_3")
    Return
  EndIf
  If (DAKIntegration.GetValue() > 1)
    If fkDAK.GetValueInt() != 1
      Return
    EndIf
  EndIf
  If aiKey == NPCKey.GetValueInt()
    If Utility.IsInMenuMode()
      Return
    EndIf
    ShowTNGMenu(TargetOrPlayer(False))
    Return
  EndIf
  If aiKey  == GenUpKey.GetValueInt()
	  If Input.IsKeyPressed(ciLKey)
		  LockOnNext()
		Else
			RiseAndDrop(True)
		EndIf
    Return
  EndIf
  If aiKey == GenDownKey.GetValueInt()
		If Input.IsKeyPressed(ciLKey)
		  LockOnNext()
		Else
			RiseAndDrop(False)
		EndIf
    Return
  EndIf
  If aiKey == RevealKey.GetValueInt()
    If Utility.IsInMenuMode()
      Return
    EndIf
    Actor lkActor = TargetOrPlayer(False)
    If !lkActor
      Return
    EndIf
    String[] lSItems = TNG_PapyrusUtil.ActorItemsInfo(lkActor)
		If (lSItems.Length == 0)
			If Notifs
				Debug.Notification("$TNG_N_1")
			EndIf
			Return
		EndIf
		UIListMenu lkListMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
		lkListMenu.ResetMenu()
    lkListMenu.AddEntryItem("$TNG_ASA")
    Int iTot = lSItems.Length
    Int i = 0
    While i < iTot
      lkListMenu.AddEntryItem(lSItems[i])
      i += 1
    EndWhile
    lkListMenu.OpenMenu(lkActor)
    Int liArmor = lkListMenu.GetResultInt()
		If liArmor < 1
			Return
		EndIf
		Bool lbSuccess = TNG_PapyrusUtil.SwapRevealing(lkActor, liArmor - 1)
    If Notifs
      If lbSuccess
        Debug.Notification("$TNG_N_2")
      Else
        Debug.Notification("$TNG_N_3")
      EndIf
    EndIf
    Return
  EndIf
EndEvent

Event OnUpdate()
  fiLastActor = -1
	Debug.Notification("$TNG_KAR")
EndEvent

Function UpdateKey(Int aHdl, Int aiNewKey)
  Int liCurrKey = -1
  If (aHdl == fiNPCKeyHdl)
    liCurrKey = NPCKey.GetValueInt()
    NPCKey.SetValueInt(aiNewKey)
  ElseIf (aHdl == fiRevealKeyHdl)
    liCurrKey = RevealKey.GetValueInt()
    RevealKey.SetValueInt(aiNewKey)  
  ElseIf (aHdl == fiUpKeyHdl)
    liCurrKey = GenUpKey.GetValueInt()
    GenUpKey.SetValueInt(aiNewKey)    
  ElseIf (aHdl == fiDownKeyHdl)
    liCurrKey = GenDownKey.GetValueInt()
    GenDownKey.SetValueInt(aiNewKey)
  EndIf
  If liCurrKey > 0
    UnregisterForKey(liCurrKey)
  EndIf
  RegisterForKey(aiNewKey)
EndFunction 

Function ShowTNGMenu(Actor akActor)
  If !akActor
    Debug.Notification("$TNG_W_1")
    Return
  EndIf
  Int liShape = -1
  Int liSize = 2
  Int liModifyRes = TNG_PapyrusUtil.CanModifyActor(akActor)
  If liModifyRes < 0
    HandleWarnings(liModifyRes)
    Return
  EndIf
  Bool lbShowSize = False
  If liModifyRes == 2
    String[] lSAddons = TNG_PapyrusUtil.GetActorAddons(akActor)    
    If lSAddons.Length > 0      
		  UIListMenu lkListMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
			lkListMenu.ResetMenu()
      lkListMenu.AddEntryItem("$TNG_AST")
      lkListMenu.AddEntryItem(akActor.GetLeveledActorBase().GetName())
      Int iTot = lSAddons.Length
      Int i = 0
      While i < iTot
        lkListMenu.AddEntryItem(lSAddons[i])
        i += 1
      EndWhile
      lkListMenu.OpenMenu(akActor)
      liShape = lkListMenu.GetResultInt()
      If liShape < 2
        Return
      EndIf
      If liShape >= 2
        Int liShapeRes = TNG_PapyrusUtil.SetActorAddn(akActor, liShape - 4)
        If liShapeRes < 0
          HandleWarnings(liShapeRes)
          Return
        EndIf        
        lbShowSize = (liShapeRes > 0)
        If !akActor.WornHasKeyword(TngCovering)
          akActor.QueueNiNodeUpdate()
        EndIf
      EndIf
    EndIf
  Else
    Debug.Notification("$TNG_N_4")
  EndIf    
  If lbShowSize      
    UIListMenu lkSizeMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
    lkSizeMenu.ResetMenu()
    Int i = 0
    lkSizeMenu.AddEntryItem("$TNG_ASS")
    lkSizeMenu.AddEntryItem(akActor.GetLeveledActorBase().GetName())
		lkSizeMenu.AddEntryItem("$TNG_SRS")
		lkSizeMenu.AddEntryItem("$TNG_SDC")
    While i < fSSizeGlobals.Length
      lkSizeMenu.AddEntryItem(fSSizeGlobals[i])
      i += 1
    EndWhile
    lkSizeMenu.OpenMenu(akActor)
    liSize = lkSizeMenu.GetResultInt()
    If liSize < 0
      Return
    EndIf
    If liSize > 1
      Int res = TNG_PapyrusUtil.SetActorSize(akActor, liSize - 4)
			If res < 0
				HandleWarnings(res)
			Else
				If akActor == PlayerRef
				  If liSize == 3
					  PlayerSize == -2
					Else
						PlayerSize = liSize - 4
					EndIf
				EndIf
			EndIf
    EndIf
  EndIf
EndFunction

Function RiseAndDrop(Bool aIfRise)
  Actor lkActor = TargetOrPlayer(True)
  If !lkActor
    Return
  EndIf
  Int aDir = -1
  If aIfRise
    aDir = 1
  EndIf
  If lkActor != fkLastActor
    fkLastActor = lkActor
    fiPos = -10
  EndIf
  fiPos += aDir
  If (fiPos*aDir) < 10    
    Debug.SendAnimationEvent(lkActor, "SOSBend" + fiPos)
  ElseIf fiPos == -10
    Debug.SendAnimationEvent(lkActor, "SOSFlaccid")
  Else
    fiPos -= aDir
  EndIf  
EndFunction  

Function HandleWarnings(Int aRes)
  If aRes > 0
    Return
  EndIf
  Debug.Notification(TNG_PapyrusUtil.GetErrDscr(aRes))
EndFunction


Function Update52Behaviors(Int aiChoice)  
  If aiChoice < 2
    Int liModCount = fSAllS52Mods.Length
    While liModCount
      liModCount -= 1
      TNG_PapyrusUtil.Slot52ModBehavior(fSAllS52Mods[liModCount], aiChoice)        
    EndWhile    
    TNG_PapyrusUtil.SetBoolValue(ci52DefOot, aiChoice==1)
    TNG_PapyrusUtil.SetBoolValue(ci52Custom, False)
  Else
    TNG_PapyrusUtil.SetBoolValue(ci52Custom, True)  
  EndIf  
  ForcePageReset()
EndFunction

Actor Function TargetOrPlayer(Bool abCheckLock)
  Actor lkActor = Game.GetCurrentCrosshairRef() as Actor
	If abCheckLock && (fiLastActor >= 0)
	  lkActor = GetLockedActor()
		If (!lkActor)
		  fiLastActor = -1
			Return TargetOrPlayer(False)
		EndIf
	EndIf
  If !lkActor
    lkActor = PlayerRef
  EndIf
  Return lkActor
EndFunction

Actor Function LockOnNext()
	fiLastActor += 1
  Actor lkActor = TargetOrPlayer(True)  
	If Notifs
		String lsName = lkActor.GetLeveledActorBase().GetName()
	  Debug.Notification("$TNG_KAL{" + lsName + "}")
	EndIf
	RegisterForSingleUpdate(60.0)
EndFunction

Actor Function GetLockedActor()  
	fiL += 1
  Cell lkCell = PlayerRef.GetParentCell()
	Int liCount = lkCell.GetNumRefs(43)	
	If (fiL > liCount)
		fiLastActor = -1
	  Debug.Notification("$TNG_KAR")
		Return PlayerRef
	EndIf
	If fiLastActor >= liCount
	  fiLastActor = 0
	EndIf
	Actor lkActor = lkCell.GetNthRef(fiLastActor, 43) as Actor
	ActorBase lkNPC = lkActor.GetLeveledActorBase()
  If !lkNPC
    fiLastActor += 1
    Return GetLockedActor()
  EndIf
  If (lkNPC.GetSex() == 1) && (!Gentified.HasForm(lkNPC))    
    fiLastActor += 1
    Return GetLockedActor()
  EndIf
	If (PlayerRef.GetDistance(lkActor) > 300)
	  fiLastActor += 1
    Return GetLockedActor()
	EndIf
	fiL = 0
	Return lkActor
EndFunction

