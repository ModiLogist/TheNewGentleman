Scriptname TNG_MCM extends SKI_ConfigBase

;Property
GlobalVariable[] Property GlobalSizes auto
GlobalVariable Property NPCKey auto
GlobalVariable Property GenUpKey auto
GlobalVariable Property GenDownKey auto
GlobalVariable Property RevealKey auto
GlobalVariable Property WomenChance auto
Message Property TypeMenuF auto
Message Property TypeMenuM auto
Message Property SizeMenu auto
Keyword Property TngCovering auto
FormList Property AddonsFem auto
FormList Property AddonsMal auto
Bool Property Notifs auto

;Kinda constant
Int ciRaces
Int ciTypes
Int ciSizes

;Local initialized variable (Don't change these)
String[] fSFemAddons
String[] fSMalAddons
String[] fSRaces
String[] fSTypeOptions
String[] fSSizeGlobals
Float[] fFSizeDefaults
Actor fkLastActor = None
Int fiPos = 0

;Pointer to UI
Int[] fIRaceTypeHdls
Int[] fIRaceSizeHdls
Int[] fIGlblSizeHdls
Int fiAutoRevealF
Int fiAutoRevealM
Int fiNotifs
Int fiNPCKey
Int fiRevealKey
Int fiUpKey
Int fiDownKey
Int fiWomenChance

;Local
Bool fbTNGMenuOpen = False


Int Function GetVersion()
	Return 1
EndFunction

Event OnConfigInit()  
	Pages = new string[3]
	Pages[0] = "$TNG_Gnl"
	Pages[1] = "$TNG_Rac"
	Pages[2] = "$TNG_Add"
  
  ciRaces = 17
  ciTypes = 3
  ciSizes = 5
EndEvent

Event OnVersionUpdate(Int aiVersion)	

EndEvent

Event OnGameReload()
	Parent.OnGameReload()
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
EndEvent

Event OnConfigOpen()  
  fSRaces = new String[20]
  fSRaces[0]="$TNG_RNO"
  fSRaces[1]="$TNG_RRE"
  fSRaces[2]="$TNG_RBR"
  fSRaces[3]="$TNG_RIM"
  fSRaces[4]="$TNG_RAL"
  fSRaces[5]="$TNG_RBL"
  fSRaces[6]="$TNG_RDU"
  fSRaces[7]="$TNG_ROR"
  fSRaces[8]="$TNG_RSA"
  fSRaces[9]="$TNG_RKH"
  fSRaces[10]="$TNG_RDR"
  fSRaces[11]="$TNG_REL"
  fSRaces[12]="$TNG_RAF"
  fSRaces[13]="$TNG_RSN"
  fSRaces[14]="$TNG_ROM"
  fSRaces[15]="$TNG_ROS"
  fSRaces[16]="$TNG_ROK"
  fIRaceTypeHdls = new Int[20]
  fIRaceSizeHdls = new Int[20]
  
  fSTypeOptions = new String[20]
  fSTypeOptions[0] = "$TNG_TCT"
  fSTypeOptions[1] = "$TNG_TMT"
  fSTypeOptions[2] = "$TNG_TRT"
  Int liAddons = AddonsMal.GetSize()
  Int liAdd = 0
  If liAddons > 0
    While liAdd < liAddons    
      Armor lkAddon = AddonsMal.GetAt(liAdd) as Armor
      fSTypeOptions[3 + liAdd] = lkAddon.GetName()
      liAdd += 1
    EndWhile
  EndIf
  
  fSSizeGlobals = new String[20]
  fSSizeGlobals[0] = "$TNG_SXS"
  fSSizeGlobals[1] = "$TNG_S_S"
  fSSizeGlobals[2] = "$TNG_S_M"
  fSSizeGlobals[3] = "$TNG_S_L"
  fSSizeGlobals[4] = "$TNG_SXL"
  fFSizeDefaults = new Float[20]
  fFSizeDefaults[0] = 0.8
  fFSizeDefaults[1] = 0.9
  fFSizeDefaults[2] = 1.0
  fFSizeDefaults[3] = 1.2
  fFSizeDefaults[4] = 1.4
  fIGlblSizeHdls = new Int[20]
  
	fSFemAddons = new String[20]
	fSMalAddons = new String[20]
EndEvent

Event OnConfigClose()
  TNG_PapyrusUtil.SaveKeys()
EndEvent

Event OnPageReset(string asPage)
  If !TNG_PapyrusUtil.TngLoaded()
    AddTextOption("$TNG_E_H", "$TNG_E_0")
    Return
  EndIf
	If asPage == Pages[0]
		AddHeaderOption("$TNG_GRH")
		AddHeaderOption("")
    bool lbARF = TNG_PapyrusUtil.GetFAutoReveal()
		fiAutoRevealF = AddToggleOption("$TNG_GRF",lbARF)
    bool lbARM = TNG_PapyrusUtil.GetMAutoReveal()
    fiAutoRevealM = AddToggleOption("$TNG_GRM",lbARM)
    AddHeaderOption("$TNG_N_H")
    fiNotifs = AddToggleOption("$TNG_N_T",Notifs)
    AddHeaderOption("$TNG_KyH")
    AddHeaderOption("")
    fiNPCKey = AddKeyMapOption("$TNG_K_N", NPCKey.GetValueInt())
    fiRevealKey = AddKeyMapOption("$TNG_K_R", RevealKey.GetValueInt())
    fiUpKey = AddKeyMapOption("$TNG_K_U", GenUpKey.GetValueInt())
    fiDownKey = AddKeyMapOption("$TNG_K_D", GenDownKey.GetValueInt())
    AddHeaderOption("$TNG_SOH")
    AddHeaderOption("")
    Int liGlbSize = 0
    While liGlbSize < ciSizes
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
    While liRace < ciRaces
      fIRaceTypeHdls[liRace] = AddMenuOption(fSRaces[liRace],fSTypeOptions[TNG_PapyrusUtil.GetGenType(liRace)])
      fIRaceSizeHdls[liRace] = AddSliderOption("",TNG_PapyrusUtil.GetGenSize(liRace),"{2}")
      liRace += 1
    EndWhile
    Return
  EndIf
  
  If asPage == Pages[2]
    AddHeaderOption("$TNG_A_W")
    AddHeaderOption("$TNG_A_M")
    Int liLines = 0
    Int liFLines = AddonsFem.GetSize()
    Int liMLines = AddonsMal.GetSize()
    If liMLines > liFLines
      liLines = liMLines
    Else
      liLines = liFLines
    EndIf
    If liFLines == 0
      AddTextOption("$TNG_OOP", "$TNG_ANW")
    Else
      fiWomenChance = AddSliderOption("$TNG_AWC",WomenChance.GetValue(),"{0}%");
    EndIf
    If liMLines == 0
      AddTextOption("$TNG_OOP", "$TNG_ANM")
    Else
      AddEmptyOption()
    EndIf
    If liLines == 0
      Return
    EndIf
    Int liCurr = 0
    While liCurr < liLines
      If liCurr < liFLines
        AddTextOption(AddonsFem.GetAt(liCurr).GetFormID(), AddonsFem.GetAt(liCurr).GetName())
      Else
        AddEmptyOption()
      EndIf
      If liCurr < liMLines
        AddTextOption(AddonsMal.GetAt(liCurr).GetName(),"")
      Else
        AddEmptyOption()
      EndIf
      liCurr += 1
    EndWhile
    Return
  EndIf
  
EndEvent

Event OnOptionHighlight(Int aiOption)
	
EndEvent

Event OnOptionDefault(Int aiOption)
  Int liOpLoop = ciRaces
  While liOpLoop
    liOpLoop -= 1
    If aiOption == fIRaceSizeHdls[liOpLoop]
      TNG_PapyrusUtil.UpdateRace(liOpLoop,TNG_PapyrusUtil.GetGenType(liOpLoop),1.0)
      SetSliderOptionValue(fIRaceSizeHdls[liOpLoop],TNG_PapyrusUtil.GetGenSize(liOpLoop),"{2}")
      TNG_PapyrusUtil.UpdateActor(Game.GetPlayer(),-1,-1)
      Return
    EndIf
    If aiOption == fIRaceTypeHdls[liOpLoop]
      TNG_PapyrusUtil.UpdateRace(liOpLoop, -1,TNG_PapyrusUtil.GetGenSize(liOpLoop))
      SetMenuOptionValue(fIRaceTypeHdls[liOpLoop],TNG_PapyrusUtil.GetGenType(liOpLoop))
      Return
    EndIf
  EndWhile
  liOpLoop = ciSizes
  While liOpLoop
    liOpLoop -= 1
    If aiOption == fIGlblSizeHdls[liOpLoop]      
      GlobalSizes[liOpLoop].SetValue(fFSizeDefaults[liOpLoop])
      SetSliderOptionValue(fIGlblSizeHdls[liOpLoop],GlobalSizes[liOpLoop].GetValue(),"{2}")
      TNG_PapyrusUtil.UpdateActor(Game.GetPlayer(),-1,-1)
      Return
    EndIf
  EndWhile
  If aiOption == fiAutoRevealF
    TNG_PapyrusUtil.SetAutoRevealing(True,TNG_PapyrusUtil.GetMAutoReveal())    
		SetToggleOptionValue(fiAutoRevealF,TNG_PapyrusUtil.GetFAutoReveal())
    Return
  EndIf
  If aiOption == fiAutoRevealM
    TNG_PapyrusUtil.SetAutoRevealing(TNG_PapyrusUtil.GetFAutoReveal(),False)    
		SetToggleOptionValue(fiAutoRevealM,TNG_PapyrusUtil.GetMAutoReveal())
    Return
  EndIf
  If aiOption == fiNotifs
    Notifs = True
    SetToggleOptionValue(fiNotifs,True)
    Return
  EndIf
  If aiOption == fiNPCKey
    If NPCKey.GetValueInt() > 0
      UnregisterForKey(NPCKey.GetValueInt())
      NPCKey.SetValueInt(-1)
			SetKeymapOptionValue(fiNPCKey, -1)
    EndIf
    Return
  EndIf
  If aiOption == fiRevealKey
    If RevealKey.GetValueInt() > 0
      UnregisterForKey(RevealKey.GetValueInt())
      RevealKey.SetValueInt(-1)
			SetKeymapOptionValue(fiRevealKey, -1)
    EndIf
    Return
  EndIf
  If aiOption == fiUpKey
    If GenUpKey.GetValueInt() > 0
      UnregisterForKey(GenUpKey.GetValueInt())
      GenUpKey.SetValueInt(-1)
			SetKeymapOptionValue(fiUpKey, -1)
    EndIf
    Return
  EndIf
  If aiOption == fiDownKey
    If GenDownKey.GetValueInt() > 0
      UnregisterForKey(GenDownKey.GetValueInt())
      GenDownKey.SetValueInt(-1)
			SetKeymapOptionValue(fiDownKey, -1)
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
  Int liRace = ciRaces
  While liRace
    liRace -= 1
    If aiOption == fIRaceTypeHdls[liRace]
      SetMenuDialogOptions(fSTypeOptions)
      SetMenuDialogStartIndex(TNG_PapyrusUtil.GetGenType(liRace))
      Return
    EndIf
  EndWhile	
EndEvent

Event OnOptionMenuAccept(int aiOption,int aiChoice)	
	Int liRace = ciRaces
  While liRace
    liRace -= 1
    If aiOption == fIRaceTypeHdls[liRace]
      TNG_PapyrusUtil.UpdateRace(liRace, aiChoice,TNG_PapyrusUtil.GetGenSize(liRace))
      SetMenuOptionValue(fIRaceTypeHdls[liRace],fSTypeOptions[aiChoice])
      Return
    EndIf
  EndWhile
EndEvent

Event OnOptionSliderOpen(int aiOption)
	If CurrentPage == Pages[0]
    Int liSize = ciSizes
    While liSize
      liSize -= 1
      If aiOption == fIGlblSizeHdls[liSize]
        SetSliderDialogStartValue(GlobalSizes[liSize].GetValue())
        SetSliderDialogDefaultValue(fFSizeDefaults[liSize])
        SetSliderDialogRange(0.1,4.0)
        SetSliderDialogInterval(0.01)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  If CurrentPage == Pages[1]
    Int liRace = ciRaces
    While liRace
      liRace -= 1
      If aiOption == fIRaceSizeHdls[liRace]
        SetSliderDialogStartValue(TNG_PapyrusUtil.GetGenSize(liRace))
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

Event OnOptionSliderAccept(int aiOption,float afValue)
	If CurrentPage == Pages[0]
    Int liSize = ciSizes
    While liSize
      liSize -= 1
      If aiOption == fIGlblSizeHdls[liSize]
        GlobalSizes[liSize].SetValue(afValue)
        SetSliderOptionValue(fIGlblSizeHdls[liSize],GlobalSizes[liSize].GetValue(),"{2}")
        TNG_PapyrusUtil.UpdateSize(liSize)
        TNG_PapyrusUtil.UpdateActor(Game.GetPlayer(),-1,-1)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  If CurrentPage == Pages[1]
    Int liRace = ciRaces
    While liRace
      liRace -= 1
      If aiOption == fIRaceSizeHdls[liRace]
        TNG_PapyrusUtil.UpdateRace(liRace,TNG_PapyrusUtil.GetGenType(liRace),afValue)
        SetSliderOptionValue(fIRaceSizeHdls[liRace],TNG_PapyrusUtil.GetGenSize(liRace),"{2}")
        TNG_PapyrusUtil.UpdateActor(Game.GetPlayer(),-1,-1)
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

Event OnOptionSelect(int aiOption)	
	If aiOption == fiAutoRevealF
    TNG_PapyrusUtil.SetAutoRevealing(!TNG_PapyrusUtil.GetFAutoReveal(),TNG_PapyrusUtil.GetMAutoReveal())    
		SetToggleOptionValue(fiAutoRevealF,TNG_PapyrusUtil.GetFAutoReveal())
  EndIf
  If aiOption == fiAutoRevealM
    TNG_PapyrusUtil.SetAutoRevealing(TNG_PapyrusUtil.GetFAutoReveal(),!TNG_PapyrusUtil.GetMAutoReveal())    
		SetToggleOptionValue(fiAutoRevealM,TNG_PapyrusUtil.GetMAutoReveal())
  EndIf
  If aiOption == fiNotifs
    Notifs = !Notifs
    SetToggleOptionValue(fiNotifs,Notifs)
  EndIf
EndEvent

Event OnOptionKeyMapChange(int aiOption, int aiKeyCode, String asConflictControl, String asConflictName)
  Bool abContinue = True
  String aNotif
  If asConflictControl != ""
    If (asConflictName != "")
      aNotif = "This key is already mapped to:\n\"" + asConflictControl + "\"\n(" + asConflictName + ")\n\nAre you sure you want to continue?"
    Else
      aNotif = "This key is already mapped to:\n\"" + asConflictControl + "\"\n\nAre you sure you want to continue?"
    EndIf
    abContinue = ShowMessage(aNotif, true, "$TNG_Yes", "$TNG__No")  
    If (!abContinue) 
      Return
    EndIf
  EndIf
  
  If (aiOption == fiNPCKey) || (aiOption == fiRevealKey) || (aiOption == fiUpKey) || (aiOption == fiDownKey)
    UpdateKey(aiOption, aiKeyCode)
    SetKeymapOptionValue(aiOption, aiKeyCode)
  EndIf
EndEvent

Function UpdateKey(Int aHdl, Int aiNewKey)
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

Event OnKeyDown(Int aiKey)
	If fbTNGMenuOpen || Utility.IsInMenuMode()
		Return
	EndIf
	If aiKey == NPCKey.GetValueInt()
    fbTNGMenuOpen = True
		ShowTNGMenu(TargetOrPlayer)
    Return
	EndIf
  If aiKey  == GenUpKey.GetValueInt()
    Actor lkActor = TargetOrPlayer
    If !lkActor
      Return
    EndIf
    If lkActor != fkLastActor
      fkLastActor = lkActor
      fiPos = 0
    EndIf    
    If fiPos < 9
      fiPos += 1
      Debug.SendAnimationEvent(lkActor, "SOSBend" + fiPos)
    EndIf
    Return
  EndIf
  If aiKey == GenDownKey.GetValueInt()
    Actor lkActor = TargetOrPlayer
    If !lkActor
      Return
    EndIf
    If lkActor != fkLastActor
      fkLastActor = lkActor
      fiPos = 0
    EndIf    
    If fiPos > -9
      fiPos -= 1
      Debug.SendAnimationEvent(lkActor, "SOSBend" + fiPos)
    EndIf
    Return
  EndIf
  If aiKey == RevealKey.GetValueInt()
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
    Bool lbSuccess = TNG_PapyrusUtil.MakeRevealing(lkBodyArmor)
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

Actor Property TargetOrPlayer
	Actor Function Get()
		Actor lkActor = Game.GetCurrentCrosshairRef() as Actor

		If !lkActor
			lkActor = Game.GetPlayer()
		EndIf
    Return lkActor
	EndFunction
EndProperty

Function ShowTNGMenu(Actor akActor)
  If !akActor
    Debug.Notification("$TNG_W_1")
    Return
  EndIf
  Int liSkin = -1
  Int liSize = 2
  Int liModifyRes = TNG_PapyrusUtil.CanModifyActor(akActor)
  If liModifyRes == -1
    Debug.Notification("$TNG_W_2")
    Return
  EndIf
  If liModifyRes == 1
    If akActor.GetActorBase().GetSex()
      liSkin = TypeMenuF.Show()
    Else
      liSkin = TypeMenuM.Show()
    EndIf
  Else
    Debug.Notification("$TNG_N_4")
  EndIf
  liSize = SizeMenu.Show()
  TNG_PapyrusUtil.UpdateActor(akActor,liSkin,liSize)
  If !akActor.WornHasKeyword(TngCovering)
    akActor.QueueNiNodeUpdate()
  EndIf
  fbTNGMenuOpen = False
EndFunction