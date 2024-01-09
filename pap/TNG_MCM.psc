Scriptname TNG_MCM extends SKI_ConfigBase

;Property
GlobalVariable[] Property GlobalSizes auto
GlobalVariable Property NPCKey auto
Message Property TypeMenu auto
Message Property SizeMenu auto

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

;Pointer to UI
Int[] fIRaceTypeHdls
Int[] fIRaceSizeHdls
Int[] fIGlblSizeHdls
Int fiAutoRevealF
Int fiAutoRevealM
Int fiNPCKey

;Local
Bool fbTNGMenuOpen = False


Int Function GetVersion()
	Return 1
EndFunction

Event OnConfigInit()
EndEvent

Event OnVersionUpdate(Int aiVersion)	

EndEvent

Event OnGameReload()
	Parent.OnGameReload()
EndEvent

Event OnConfigOpen()
	Pages = new string[3]
	Pages[0] = "$TNG_Gnl"
	Pages[1] = "$TNG_Rac"
	Pages[2] = "$TNG_Add"
  
  ciRaces = 17
  ciTypes = 3
  ciSizes = 5
  
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
  
  fSSizeGlobals = new String[20]
  fSSizeGlobals[0] = "$TNG_SXS"
  fSSizeGlobals[1] = "$TNG_S_S"
  fSSizeGlobals[2] = "$TNG_S_M"
  fSSizeGlobals[3] = "$TNG_S_L"
  fSSizeGlobals[4] = "$TNG_SXL"
  fFSizeDefaults = new Float[20]
  fFSizeDefaults[0] = 0.8
  fFSizeDefaults[0] = 0.9
  fFSizeDefaults[0] = 1.0
  fFSizeDefaults[0] = 1.2
  fFSizeDefaults[0] = 1.4
  fIGlblSizeHdls = new Int[20]
  
	fSFemAddons = new String[20]
	fSMalAddons = new String[20]
EndEvent

Event OnPageReset(string asPage)	
	If asPage == Pages[0]
		AddHeaderOption("$TNG_ARH")
		AddHeaderOption("")
    bool lbARF = TNG_PapyrusUtil.GetFAutoReveal()
		fiAutoRevealF = AddToggleOption("$TNG_ARF",lbARF)
    bool lbARM = TNG_PapyrusUtil.GetMAutoReveal()
    fiAutoRevealM = AddToggleOption("$TNG_ARM",lbARM)
    AddHeaderOption("$TNG_KyH")
    AddHeaderOption("")
    fiNPCKey = AddKeyMapOption("$TNG_Key", 0)
    AddEmptyOption()
    AddHeaderOption("$TNG_SOH")
    AddHeaderOption("")
    Int liGlbSize = 0
    While liGlbSize < ciSizes
      fIGlblSizeHdls[liGlbSize] = AddSliderOption(fSSizeGlobals[liGlbSize],GlobalSizes[liGlbSize].GetValue(),"{2}")
      liGlbSize += 1
    EndWhile
    AddEmptyOption()    
    Return
  EndIf
  
  If asPage == Pages[1]
    Int liRace = 0
    While liRace < ciRaces     
      AddHeaderOption(fSRaces[liRace])
      AddHeaderOption("")
      fIRaceTypeHdls[liRace] = AddMenuOption("$TNG_OGT",fSTypeOptions[TNG_PapyrusUtil.GetGenType(liRace)])
      fIRaceSizeHdls[liRace] = AddSliderOption("$TNG_OGS",TNG_PapyrusUtil.GetGenSize(liRace),"{2}")
      liRace += 1
    EndWhile
    Return
  EndIf
  
  If asPage == Pages[2]
    AddTextOption("$TNG_ATI", "$TNG_ACS")
    Return
  EndIf
  
EndEvent

Event OnOptionHighlight(Int aiOption)
	
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
        SetSliderDialogRange(0.01,4.0)
        SetSliderDialogInterval(0.01)
        Return
      EndIf
    EndWhile
  EndIf
  If CurrentPage == Pages[1]
    Int liRace = ciRaces
    While liRace
      liRace -= 1
      If aiOption == fIRaceSizeHdls[liRace]
        SetSliderDialogStartValue(TNG_PapyrusUtil.GetGenSize(liRace))
        SetSliderDialogDefaultValue(1.0)
        SetSliderDialogRange(0.01,4.0)
        SetSliderDialogInterval(0.01)
        Return
      EndIf
    EndWhile
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
        Return
      EndIf
    EndWhile
  EndIf
  If CurrentPage == Pages[1]
    Int liRace = ciRaces
    While liRace
      liRace -= 1
      If aiOption == fIRaceSizeHdls[liRace]
        TNG_PapyrusUtil.UpdateRace(liRace,TNG_PapyrusUtil.GetGenType(liRace),afValue)
        SetSliderOptionValue(fIRaceSizeHdls[liRace],TNG_PapyrusUtil.GetGenSize(liRace),"{2}")
        Return
      EndIf
    EndWhile
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
EndEvent

Event OnOptionKeyMapChange(int aiOption, int aiKeyCode, String asConflictControl, String asConflictName)
	If (aiOption == fiNPCKey)
    Bool abContinue = True
		If (asConflictControl != "")
			String aNotif
			If (asConflictName != "")
				aNotif = "This key is already mapped to:\n\"" + asConflictControl + "\"\n(" + asConflictName + ")\n\nAre you sure you want to continue?"
			Else
				aNotif = "This key is already mapped to:\n\"" + asConflictControl + "\"\n\nAre you sure you want to continue?"
			EndIf
			abContinue = ShowMessage(aNotif, true, "$TNG_Yes", "$TNG__No")
		EndIf

		If (abContinue)
			UpdateNPCKey(NPCKey.GetValueInt(), aiKeyCode)
			SetKeymapOptionValue(fiNPCKey, aiKeyCode)
		EndIf
	EndIf
EndEvent

Function UpdateNPCKey(Int aiCurrKey, Int aiNewKey)
	UnregisterForKey(aiCurrKey)
	NPCKey.SetValueInt(aiNewKey)
	RegisterForKey(aiNewKey)
EndFunction

Event OnKeyDown(Int aiKey)
	If fbTNGMenuOpen
		Return
	EndIf
  fbTNGMenuOpen = True
	If aiKey == NPCKey.GetValueInt()
		ShowTNGMenu(TargetOrPlayer)
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
  If TNG_PapyrusUtil.CanModifyActor(akActor)
    liSkin = TypeMenu.Show()
  EndIf
  liSize = SizeMenu.Show()
  TNG_PapyrusUtil.UpdateActor(akActor,liSkin,liSize)
 fbTNGMenuOpen = False
EndFunction