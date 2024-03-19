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
Int Property PlayerSkin auto
Int Property PlayerSize auto
Actor Property PlayerRef auto

;Constants
Int cuFemAR = 1
Int cuMalAR = 2
Int cuClipCheck = 3
Int cuExPCSize = 4

;Kinda constant
Float[] cFSizeDefaults

;Local variable
String[] fSRaces
String[] fSMalOptions
String[] fSFemOptions
String[] fSSizeGlobals
String[] fSGlobalWarnings
Actor fkLastActor = None
GlobalVariable fkDAK = None
Int fiPos = 0

;Pointer to UI
Int[] fIRaceTypeHdls
Int[] fIRaceSizeHdls
Int[] fIGlblSizeHdls
Int[] fIFemAddons

Int fiDoubleCheck
Int fiAutoRevealF
Int fiAutoRevealM
Int fiNotifs
Int fiExPCSize

Int fiDAK
Int fiNPCKey
Int fiRevealKey
Int fiUpKey
Int fiDownKey
Int fiWomenChance

Int Function GetVersion()
	Return 3
EndFunction

Event OnConfigInit()
	Pages = new String[3]
	Pages[0] = "$TNG_Gnl"
	Pages[1] = "$TNG_Rac"
	Pages[2] = "$TNG_Add"    
  
  fIRaceTypeHdls = new Int[120]
  fIRaceSizeHdls = new Int[120]
  fIGlblSizeHdls = new Int[5]
  fIFemAddons = new Int[120]  
  
  fSSizeGlobals = new String[5]
  fSSizeGlobals[0] = "$TNG_SXS"
  fSSizeGlobals[1] = "$TNG_S_S"
  fSSizeGlobals[2] = "$TNG_S_M"
  fSSizeGlobals[3] = "$TNG_S_L"
  fSSizeGlobals[4] = "$TNG_SXL"  
  
  fSGlobalWarnings = new String[100]
  fSGlobalWarnings[1] = "$TNG_WN1"
  fSGlobalWarnings[2] = "$TNG_WN2"
  fSGlobalWarnings[3] = "$TNG_WN3"
  fSGlobalWarnings[4] = "$TNG_WN4"
  fSGlobalWarnings[5] = "$TNG_WN5"
  fSGlobalWarnings[9] = "$TNG_WN9"  
  
  cFSizeDefaults = new Float[5]
  cFSizeDefaults[0] = 0.8
  cFSizeDefaults[1] = 0.9
  cFSizeDefaults[2] = 1.0
  cFSizeDefaults[3] = 1.2
  cFSizeDefaults[4] = 1.4
EndEvent

Event OnVersionUpdate(Int aiVersion)
  If (aiVersion > CurrentVersion)
    OnConfigInit()
	EndIf 
EndEvent

Event OnConfigOpen()    
  fSRaces = TNG_PapyrusUtil.GetRaceGrpNames()
  fSMalOptions = TNG_PapyrusUtil.GetAllPossibleAddons(False)
  fSFemOptions = TNG_PapyrusUtil.GetAllPossibleAddons(True)  
EndEvent

Event OnConfigClose()
  TNG_PapyrusUtil.SaveGlobals()
EndEvent

Event OnGameReload()
  Parent.OnGameReload()
  
  Gentified.Revert()
  
  
  If Game.GetModByName("Dynamic Activation Key.esp")
    fkDAK = Game.GetFormFromFile(0x801,"Dynamic Activation Key.esp") As GlobalVariable
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
  If PlayerSkin > -1
    Int lRes = TNG_PapyrusUtil.SetActorAddn(PlayerRef,PlayerSkin)
    PlayerRef.QueueNiNodeUpdate()
    If lRes < 0 
      Debug.Notification("$TNG_WPT")
      PlayerSkin = -1
      TNG_PapyrusUtil.SetActorAddn(PlayerRef,PlayerSkin)
      HandleWarnings(lRes)
    EndIf
  EndIf  
  If PlayerSize > -1
    Int lRes = TNG_PapyrusUtil.SetActorSize(PlayerRef,PlayerSize)
    If lRes < 0     
      Debug.Notification("$TNG_WPS")
      PlayerSize = -1
      TNG_PapyrusUtil.SetActorSize(PlayerRef,PlayerSize)
      HandleWarnings(lRes)
    EndIf
  EndIf  
EndEvent

Event OnOptionHighlight(Int aiOption)

  If CurrentPage == Pages[0]
    If aiOption == fiDAK
      SetInfoText("$TNG_HGD")
      Return
    EndIf 
    If aiOption == fiDoubleCheck
      SetInfoText("$TNG_HGC")
      Return
    EndIf
    If aiOption == fiAutoRevealF
      SetInfoText("$TNG_HGF")
      Return
    EndIf
    If aiOption == fiAutoRevealM
      SetInfoText("$TNG_HGM")
      Return
    EndIf
    If aiOption == fiNotifs
      SetInfoText("$TNG_HGN")
      Return
    EndIf
    If aiOption == fiExPCSize
      SetInfoText("$TNG_HGE")
      Return
    EndIf
    If (aiOption == fiUpKey) || (aiOption == fiDownKey)
      SetInfoText("$TNG_HKU")
      Return
    EndIf
    If aiOption == fiNPCKey
      SetInfoText("$TNG_HKN")
      Return
    EndIf    
    If aiOption == fiRevealKey
      SetInfoText("$TNG_HKS")
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
    If aiOption == fiWomenChance
      SetInfoText("$TNG_HAC")
      Return
    EndIf    
    Int liCurr = 0
    While liCurr < fSFemOptions.Length
      If aiOption == fIFemAddons[liCurr]
        SetInfoText("$TNG_HAD")
        Return
      EndIf
      liCurr += 1
    EndWhile
    Return
  EndIf
  
EndEvent

Event OnPageReset(String asPage)
	If asPage == Pages[0]
    fiDoubleCheck = AddToggleOption("$TNG_GRC", TNG_PapyrusUtil.GetBoolValue(cuClipCheck))    
    fiNotifs = AddToggleOption("$TNG_GNT",Notifs)
		fiAutoRevealF = AddToggleOption("$TNG_GRF",TNG_PapyrusUtil.GetBoolValue(cuFemAR))
    fiAutoRevealM = AddToggleOption("$TNG_GRM",TNG_PapyrusUtil.GetBoolValue(cuMalAR))
    fiExPCSize = AddToggleOption("$TNG_GEP", TNG_PapyrusUtil.GetBoolValue(cuExPCSize))
    AddEmptyOption()
    AddHeaderOption("$TNG_KyH")
    AddHeaderOption("")
    fkDAK = None
    If Game.GetModByName("Dynamic Activation Key.esp")
      fkDAK = Game.GetFormFromFile(0x801,"Dynamic Activation Key.esp") As GlobalVariable
      If fkDAK
        fiDAK = AddToggleOption("$TNG_DAK",DAKIntegration.GetValue()>1)
        AddEmptyOption()
      EndIf
    EndIf
    If !fkDAK
      DAKIntegration.SetValue(0)
    EndIf
    fiNPCKey = AddKeyMapOption("$TNG_K_N",NPCKey.GetValueInt())
    fiRevealKey = AddKeyMapOption("$TNG_K_R",RevealKey.GetValueInt())
    fiUpKey = AddKeyMapOption("$TNG_K_U",GenUpKey.GetValueInt())
    fiDownKey = AddKeyMapOption("$TNG_K_D",GenDownKey.GetValueInt())
    AddHeaderOption("$TNG_SOH")
    AddHeaderOption("")
    Int liGlbSize = 0
    While liGlbSize < fSSizeGlobals.Length
      fIGlblSizeHdls[liGlbSize] = AddSliderOption(fSSizeGlobals[liGlbSize],GlobalSizes[liGlbSize].GetValue(),"{2}")
      AddEmptyOption()
      liGlbSize += 1
    EndWhile 
    Return
  EndIf
  
  If asPage == Pages[1]
    AddHeaderOption("$TNG_OGT")
    AddHeaderOption("$TNG_OGS")
    Int liRace = 0
    While liRace < fSRaces.Length
      fIRaceTypeHdls[liRace] = AddMenuOption(fSRaces[liRace],fSMalOptions[TNG_PapyrusUtil.GetRaceGrpAddn(liRace)])
      fIRaceSizeHdls[liRace] = AddSliderOption("",TNG_PapyrusUtil.GetRaceGrpMult(liRace),"{2}")
      liRace += 1
    EndWhile
    Return
  EndIf
  
  If asPage == Pages[2]
    AddHeaderOption("$TNG_A_W")
    AddHeaderOption("$TNG_A_M")
    If fSFemOptions.Length == 0
      AddTextOption("$TNG_ANW","")
    Else
      fiWomenChance = AddSliderOption("$TNG_AWC",WomenChance.GetValue(),"{0}%");
    EndIf
    If fSMalOptions.Length == 0
      AddTextOption("$TNG_ANM","")
    EndIf
    If (fSFemOptions.Length == 0) && (fSMalOptions.Length == 0)
      Return
    EndIf
    Int liCurr = 0
    While liCurr < fSFemOptions.Length || liCurr < fSMalOptions.Length
      If liCurr < fSMalOptions.Length
        AddTextOption(fSMalOptions[liCurr],"")
      Else
        AddEmptyOption()
      EndIf
      If liCurr < fSFemOptions.Length
        fIFemAddons[liCurr] = AddToggleOption(fSFemOptions[liCurr],TNG_PapyrusUtil.GetAddonStatus(liCurr))
      Else
        AddEmptyOption()
      EndIf
      liCurr += 1
    EndWhile
    Return
  EndIf
  
EndEvent

Event OnOptionDefault(Int aiOption)
  Int liOpLoop = fSRaces.Length
  While liOpLoop
    liOpLoop -= 1
    If aiOption == fIRaceSizeHdls[liOpLoop]
      TNG_PapyrusUtil.SetRaceGrpMult(liOpLoop,1.0)
      SetSliderOptionValue(fIRaceSizeHdls[liOpLoop],TNG_PapyrusUtil.GetRaceGrpMult(liOpLoop),"{2}")
      TNG_PapyrusUtil.SetActorSize(PlayerRef,-1)
      Return
    EndIf
    If aiOption == fIRaceTypeHdls[liOpLoop]
      TNG_PapyrusUtil.SetRaceGrpAddn(liOpLoop,-1)
      SetMenuOptionValue(fIRaceTypeHdls[liOpLoop],fSMalOptions[TNG_PapyrusUtil.GetRaceGrpAddn(liOpLoop)])
      Return
    EndIf
  EndWhile
  liOpLoop = fSSizeGlobals.Length
  While liOpLoop
    liOpLoop -= 1
    If aiOption == fIGlblSizeHdls[liOpLoop]      
      GlobalSizes[liOpLoop].SetValue(cFSizeDefaults[liOpLoop])
      SetSliderOptionValue(fIGlblSizeHdls[liOpLoop],GlobalSizes[liOpLoop].GetValue(),"{2}")
      TNG_PapyrusUtil.SetActorSize(PlayerRef,-1)
      Return
    EndIf
  EndWhile  
  liOpLoop = 0
  While liOpLoop < fSFemOptions.Length
    If aiOption == fIFemAddons[liOpLoop]
      TNG_PapyrusUtil.SetAddonStatus(liOpLoop,False)
      SetToggleOptionValue(aiOption,False)
      Return
    EndIf
    liOpLoop += 1
  EndWhile
  If aiOption == fiAutoRevealF
    TNG_PapyrusUtil.SetBoolValue(cuFemAR,True)  
		SetToggleOptionValue(fiAutoRevealF,TNG_PapyrusUtil.GetBoolValue(cuFemAR))
    Return
  EndIf
  If aiOption == fiAutoRevealM
    TNG_PapyrusUtil.SetBoolValue(cuMalAR,False)    
		SetToggleOptionValue(fiAutoRevealM,TNG_PapyrusUtil.GetBoolValue(cuMalAR))
    Return
  EndIf
  If aiOption == fiDoubleCheck
    TNG_PapyrusUtil.SetBoolValue(cuClipCheck,True)
    SetToggleOptionValue(fiDoubleCheck,TNG_PapyrusUtil.GetBoolValue(cuClipCheck))
    Return
  EndIf
  If aiOption == fiExPCSize
    TNG_PapyrusUtil.SetBoolValue(cuExPCSize,False)
    SetToggleOptionValue(cuExPCSize,TNG_PapyrusUtil.GetBoolValue(cuExPCSize))
    Return
  EndIf
  If aiOption == fiNotifs
    Notifs = True
    SetToggleOptionValue(fiNotifs,True)
    Return
  EndIf
  If aiOption == fiDAK
    DAKIntegration.SetValue(0)
    SetToggleOptionValue(fiDAK,False)
    Bool lbResetAll = ShowMessage("$TNG_DKN",true,"$TNG_Yes","$TNG__No")
    If lbResetAll
      OnOptionDefault(fiNPCKey)
      OnOptionDefault(fiRevealKey)
      OnOptionDefault(fiUpKey)
      OnOptionDefault(fiDownKey)
    EndIf
    Return
  EndIf
  If aiOption == fiNPCKey
    If NPCKey.GetValueInt() > 0
      UnregisterForKey(NPCKey.GetValueInt())
      NPCKey.SetValueInt(-1)
			SetKeymapOptionValue(fiNPCKey,-1)
    EndIf
    Return
  EndIf
  If aiOption == fiRevealKey
    If RevealKey.GetValueInt() > 0
      UnregisterForKey(RevealKey.GetValueInt())
      RevealKey.SetValueInt(-1)
			SetKeymapOptionValue(fiRevealKey,-1)
    EndIf
    Return
  EndIf
  If aiOption == fiUpKey
    If GenUpKey.GetValueInt() > 0
      UnregisterForKey(GenUpKey.GetValueInt())
      GenUpKey.SetValueInt(-1)
			SetKeymapOptionValue(fiUpKey,-1)
    EndIf
    Return
  EndIf
  If aiOption == fiDownKey
    If GenDownKey.GetValueInt() > 0
      UnregisterForKey(GenDownKey.GetValueInt())
      GenDownKey.SetValueInt(-1)
			SetKeymapOptionValue(fiDownKey,-1)
    EndIf
    Return
  EndIf
  If aiOption == fiWomenChance    
    WomenChance.SetValue(20)
    SetSliderOptionValue(fiWomenChance,WomenChance.GetValue(),"{0}%")
    Return
  EndIf
EndEvent

Event OnOptionMenuOpen(Int aiOption)
  Int liRace = fSRaces.Length
  While liRace
    liRace -= 1
    If aiOption == fIRaceTypeHdls[liRace]
      SetMenuDialogOptions(fSMalOptions)
      SetMenuDialogStartIndex(TNG_PapyrusUtil.GetRaceGrpAddn(liRace))
      Return
    EndIf
  EndWhile	
EndEvent

Event OnOptionMenuAccept(Int aiOption,Int aiChoice)	
	Int liRace = fSRaces.Length
  While liRace
    liRace -= 1
    If aiOption == fIRaceTypeHdls[liRace]
      TNG_PapyrusUtil.SetRaceGrpAddn(liRace,aiChoice)
      SetMenuOptionValue(fIRaceTypeHdls[liRace],fSMalOptions[aiChoice])
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
        SetSliderDialogRange(0.1,4.0)
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
        SetSliderDialogStartValue(TNG_PapyrusUtil.GetRaceGrpMult(liRace))
        SetSliderDialogDefaultValue(1.0)
        SetSliderDialogRange(0.1,2.0)
        SetSliderDialogInterval(0.01)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  If aiOption == fiWomenChance
    SetSliderDialogStartValue(WomenChance.GetValue())
    SetSliderDialogDefaultValue(20)
    SetSliderDialogRange(0,100)
    SetSliderDialogInterval(1)
    Return
  EndIf
EndEvent

Event OnOptionSliderAccept(Int aiOption,Float afValue)
	If CurrentPage == Pages[0]
    Int liSize = fSSizeGlobals.Length
    While liSize
      liSize -= 1
      If aiOption == fIGlblSizeHdls[liSize]
        GlobalSizes[liSize].SetValue(afValue)
        SetSliderOptionValue(fIGlblSizeHdls[liSize],GlobalSizes[liSize].GetValue(),"{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef,-1)
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
        TNG_PapyrusUtil.SetRaceGrpMult(liRace,afValue)
        SetSliderOptionValue(fIRaceSizeHdls[liRace],TNG_PapyrusUtil.GetRaceGrpMult(liRace),"{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef,-1)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  If aiOption == fiWomenChance
    WomenChance.SetValue(afValue)
    SetSliderOptionValue(fiWomenChance,WomenChance.GetValue(),"{0}%")
    Return
  EndIf
EndEvent

Event OnOptionSelect(Int aiOption)
	If CurrentPage == Pages[0]
    If aiOption == fiAutoRevealF
      TNG_PapyrusUtil.SetBoolValue(cuFemAR,!TNG_PapyrusUtil.GetBoolValue(cuFemAR))    
      SetToggleOptionValue(fiAutoRevealF,TNG_PapyrusUtil.GetBoolValue(cuFemAR))
      Return
    EndIf
    If aiOption == fiAutoRevealM
      TNG_PapyrusUtil.SetBoolValue(cuMalAR,!TNG_PapyrusUtil.GetBoolValue(cuMalAR))    
      SetToggleOptionValue(fiAutoRevealM,TNG_PapyrusUtil.GetBoolValue(cuMalAR))
      Return
    EndIf
    If aiOption == fiDoubleCheck
      TNG_PapyrusUtil.SetBoolValue(cuClipCheck,!TNG_PapyrusUtil.GetBoolValue(cuClipCheck))    
      SetToggleOptionValue(fiDoubleCheck,TNG_PapyrusUtil.GetBoolValue(cuClipCheck))
      Return
    EndIf
    If aiOption == fiExPCSize
      TNG_PapyrusUtil.SetBoolValue(cuExPCSize,!TNG_PapyrusUtil.GetBoolValue(cuExPCSize))    
      SetToggleOptionValue(fiExPCSize,TNG_PapyrusUtil.GetBoolValue(cuExPCSize))
      If TNG_PapyrusUtil.GetBoolValue(cuExPCSize) 
        PlayerSize = -1
      EndIf
      Return
    EndIf
    If aiOption == fiNotifs
      Notifs = !Notifs
      SetToggleOptionValue(fiNotifs,Notifs)
      Return
    EndIf
    If aiOption == fiDAK
      DAKIntegration.SetValue(2.0 - DAKIntegration.GetValue())
      SetToggleOptionValue(fiDAK,DAKIntegration.GetValue() > 1)
      If DAKIntegration.GetValue() < 1
        Bool lbResetAll = ShowMessage("$TNG_DKN",true,"$TNG_Yes","$TNG__No")
        If lbResetAll
          OnOptionDefault(fiNPCKey)
          OnOptionDefault(fiRevealKey)
          OnOptionDefault(fiUpKey)
          OnOptionDefault(fiDownKey)
        EndIf
      EndIf
      Return
    EndIf
    Return
  EndIf
  If CurrentPage == Pages[2]
    Int liCurr = 0
    While liCurr < fSFemOptions.Length
      If aiOption == fIFemAddons[liCurr]
        TNG_PapyrusUtil.SetAddonStatus(liCurr,!TNG_PapyrusUtil.GetAddonStatus(liCurr))
        SetToggleOptionValue(fIFemAddons[liCurr],TNG_PapyrusUtil.GetAddonStatus(liCurr))
        Return
      EndIf
      liCurr += 1
    EndWhile
    Return
  EndIf
EndEvent

Event OnOptionKeyMapChange(Int aiOption,Int aiKeyCode,String asConflictControl,String asConflictName)
  If !fkDAK || (DAKIntegration.GetValue() < 1)
    Bool lbContinue = True
    String lsNotif
    If asConflictControl != ""
      If (asConflictName != "")
        lsNotif = "This key is already mapped to:\n\"" + asConflictControl + "\"\n(" + asConflictName + ")\n\nAre you sure you want to continue?"
      Else
        lsNotif = "This key is already mapped to:\n\"" + asConflictControl + "\"\n\nAre you sure you want to continue?"
      EndIf
      lbContinue = ShowMessage(lsNotif,true,"$TNG_Yes","$TNG__No")  
      If (!lbContinue) 
        Return
      EndIf
    EndIf
  EndIf
  If (aiOption == fiNPCKey) || (aiOption == fiRevealKey) || (aiOption == fiUpKey) || (aiOption == fiDownKey)
    UpdateKey(aiOption,aiKeyCode)
    SetKeymapOptionValue(aiOption,aiKeyCode)
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
		ShowTNGMenu(TargetOrPlayer)
    Return
	EndIf
  If aiKey  == GenUpKey.GetValueInt()
    RiseAndDrop(True)
    Return
  EndIf
  If aiKey == GenDownKey.GetValueInt()
    RiseAndDrop(False)
    Return
  EndIf
  If aiKey == RevealKey.GetValueInt()
    If Utility.IsInMenuMode()
      Return
    EndIf
    Actor lkActor = TargetOrPlayer
    If !lkActor
      Return
    EndIf
    Armor lkBodyArmor = lkActor.GetEquippedArmorInSlot(32)
    If !lkBodyArmor
      If Notifs
        Debug.Notification("$TNG_N_1")
      EndIf
      Return
    EndIf
    Bool lbSuccess = TNG_PapyrusUtil.SwapRevealing(lkBodyArmor)
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

Function UpdateKey(Int aHdl,Int aiNewKey)
	Int liCurrKey = -1
  If (aHdl == fiNPCKey)
    liCurrKey = NPCKey.GetValueInt()
    NPCKey.SetValueInt(aiNewKey)
  ElseIf (aHdl == fiRevealKey)
    liCurrKey = RevealKey.GetValueInt()
    RevealKey.SetValueInt(aiNewKey)  
  ElseIf (aHdl == fiUpKey)
    liCurrKey = GenUpKey.GetValueInt()
    GenUpKey.SetValueInt(aiNewKey)    
  ElseIf (aHdl == fiDownKey)
    liCurrKey = GenDownKey.GetValueInt()
    GenDownKey.SetValueInt(aiNewKey)
  EndIf
  If liCurrKey > 0
    UnregisterForKey(liCurrKey)
  EndIf
	RegisterForKey(aiNewKey)
EndFunction

Actor Property TargetOrPlayer
	Actor Function Get()
		Actor lkActor = Game.GetCurrentCrosshairRef() as Actor

		If !lkActor
			lkActor = PlayerRef
		EndIf
    Return lkActor
	EndFunction
EndProperty

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
  Bool lbIsFemale = akActor.GetLeveledActorBase().GetSex()
  Bool lbShowSize = !lbIsFemale
  If liModifyRes == 2
    UIListMenu lkListMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
    lkListMenu.ResetMenu()
    String[] lSAddons = TNG_PapyrusUtil.GetAllPossibleAddons(lbIsFemale)
    
    If lSAddons.Length > 0      
      lkListMenu.AddEntryItem("$TNG_AST")
      lkListMenu.AddEntryItem(akActor.GetLeveledActorBase().GetName())
      lkListMenu.AddEntryItem("$TNG_RES")
      Int iTot = lSAddons.Length
      Int i = 0
      While i < iTot
        lkListMenu.AddEntryItem(lSAddons[i])
        i += 1
      EndWhile
      lkListMenu.OpenMenu(akActor)
      liShape = lkListMenu.GetResultInt()
      If liShape < 0
        Return
      EndIf
      If liShape > 1
        If liShape == 2
          liShape = 1
        EndIf
        Int liShapeRes = TNG_PapyrusUtil.SetActorAddn(akActor,liShape - 3)
        If liShapeRes < 0
          HandleWarnings(liShapeRes)
          Return
        EndIf        
        lbShowSize = (liShapeRes > 0)        
        If akActor == PlayerRef
          PlayerSkin = liShape - 3
        EndIf
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
      TNG_PapyrusUtil.SetActorSize(akActor,liSize - 2)
    EndIf
    If akActor == PlayerRef
      PlayerSize = liSize - 2
    EndIf
  EndIf
EndFunction

Function RiseAndDrop(Bool aIfRise)
  Actor lkActor = TargetOrPlayer
  Int aDir = -1
  If aIfRise
    aDir = 1
  EndIf
  If !lkActor
    Return
  EndIf
  ActorBase lkNPC = lkActor.GetLeveledActorBase()
  If !lkNPC
    lkActor = PlayerRef
    lkNPC = lkActor.GetActorBase()
  EndIf
  If (lkNPC.GetSex() == 1) && (!Gentified.HasForm(lkNPC))
    lkActor = PlayerRef
    lkNPC = lkActor.GetActorBase()
  EndIf
  If lkActor != fkLastActor
    fkLastActor = lkActor
    fiPos = 0
  EndIf
  If (fiPos*aDir) < 9
    fiPos += aDir
    Debug.SendAnimationEvent(lkActor,"SOSBend" + fiPos)
  EndIf
EndFunction    

Function HandleWarnings(Int aRes)
  If aRes > 0
    Return
  EndIf
  Debug.Notification(fSGlobalWarnings[-aRes])
EndFunction

