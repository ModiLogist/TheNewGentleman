Scriptname TNG_MCM extends SKI_ConfigBase

;Property

GlobalVariable Property DAKIntegration auto
GlobalVariable Property GenDownKey auto
FormList Property Gentified auto
GlobalVariable Property GenUpKey auto
GlobalVariable[] Property GlobalSizes auto
Bool Property Notifs auto
GlobalVariable Property NPCKey auto
Actor Property PlayerRef auto
Int Property PlayerSize auto
GlobalVariable Property PlayerSkin auto
GlobalVariable Property RevealKey auto
GlobalVariable Property WomenChance auto


;Constants
Int ciExcludePlayer
Int ciCheckingPCGen
Int ciRevealSlot52s
Int ciLetMixSlot52s    
Int ciRandomizeMale

Int ciLockKey

;Kinda constant
Float[] cFSizeDefaults
String[] cSSizeGlobals
String[] cSLogOptions
String[] cS52Options

;Local variable

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


Int fiDAKHdl
Int fiNPCKeyHdl
Int fiRevealKeyHdl
Int fiUpKeyHdl
Int fiDownKeyHdl
Int fiWomenChanceHdl
Int fiRandMalAddonHdl
Int fiLogLvlHdl
Int fiLogDirHdl

Int fi52DefBehaviorHdl

Int fiPCEHdl
Int fiPCUHdl

Int Function GetVersion()
  Return 8
EndFunction

Event OnConfigInit()
  Pages = new String[5]
  Pages[0] = "$TNG_Gnl"
  Pages[1] = "$TNG_Rac"
  Pages[2] = "$TNG_Add"  
  Pages[3] = "$TNG_S52"
  Pages[4] = "$TNG_Adv"
  
  fIRaceTypeHdls = new Int[120]
  fIRaceSizeHdls = new Int[120]
  fI52ModHdls = new Int[126]
  fIGlblSizeHdls = new Int[5]
  fIFemAddonHdls = new Int[120]  
  fIMalAddonHdls = new Int[120]  
  
  cSSizeGlobals = new String[5]
  cSSizeGlobals[0] = "$TNG_SXS"
  cSSizeGlobals[1] = "$TNG_S_S"
  cSSizeGlobals[2] = "$TNG_S_M"
  cSSizeGlobals[3] = "$TNG_S_L"
  cSSizeGlobals[4] = "$TNG_SXL"  
  
  cFSizeDefaults = new Float[5]
  cFSizeDefaults[0] = 0.8
  cFSizeDefaults[1] = 0.9
  cFSizeDefaults[2] = 1.0
  cFSizeDefaults[3] = 1.2
  cFSizeDefaults[4] = 1.4
  
  cSLogOptions = new String[6]
  cSLogOptions[0] = "$TNG_LLD"
	cSLogOptions[1] = "$TNG_LLI"
  cSLogOptions[2] = "$TNG_LLW"
  cSLogOptions[3] = "$TNG_LLE"
  cSLogOptions[4] = "$TNG_LLC"
  cSLogOptions[5] = "$TNG_LLO"
  
  cS52Options = new String[3]
  cS52Options[0] = "$TNG_52C"
  cS52Options[1] = "$TNG_52R"
  cS52Options[2] = "$TNG_52U"
	
	ciExcludePlayer = 0
	ciCheckingPCGen = 1
  ciRevealSlot52s = 2
	ciLetMixSlot52s = 3
  ciRandomizeMale = 4

	ciLockKey = 56 ; Left Alt Key
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

Event OnConfigClose()
  TNG_PapyrusUtil.UpdateSettings()
EndEvent

Event OnGameReload()
  Parent.OnGameReload()
  
  Gentified.Revert()
  
  If TNG_PapyrusUtil.GetBoolValue(ciCheckingPCGen) 
    RegisterForSingleUpdate(1)
  EndIf
  
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
  Int res = TNG_PapyrusUtil.SetActorAddon(PlayerRef, PlayerSkin.GetValueInt())
	If (res >= 0) && !PlayerRef.IsOnMount()
		PlayerRef.QueueNiNodeUpdate()
	Else
    Debug.Notification("$TNG_WPT")
    PlayerSkin.SetValueInt(-2)
    TNG_PapyrusUtil.SetActorAddon(PlayerRef, PlayerSkin.GetValueInt())
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

Event OnPageReset(String asPage)

  If asPage == Pages[0]
    fiNotifsHdl = AddToggleOption("$TNG_GNT", Notifs)
		AddEmptyOption()
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
    fiNPCKeyHdl = AddKeyMapOption("$TNG_K_N", NPCKey.GetValueInt(), OPTION_FLAG_WITH_UNMAP)
    fiRevealKeyHdl = AddKeyMapOption("$TNG_K_R", RevealKey.GetValueInt(), OPTION_FLAG_WITH_UNMAP)
    fiUpKeyHdl = AddKeyMapOption("$TNG_K_U", GenUpKey.GetValueInt(), OPTION_FLAG_WITH_UNMAP)
    fiDownKeyHdl = AddKeyMapOption("$TNG_K_D", GenDownKey.GetValueInt(), OPTION_FLAG_WITH_UNMAP)
    AddHeaderOption("$TNG_SOH")
		AddHeaderOption("")
    Int liGlbSize = 0
    While liGlbSize < cSSizeGlobals.Length
      fIGlblSizeHdls[liGlbSize] = AddSliderOption(cSSizeGlobals[liGlbSize], GlobalSizes[liGlbSize].GetValue(), "{2}")
      liGlbSize += 1
    EndWhile
		AddEmptyOption()
    Return
  EndIf
  
  If asPage == Pages[1]
    AddHeaderOption("$TNG_OGS")
    AddHeaderOption("$TNG_OGT")
    Int liRg = 0
		String[] lSRgNames = TNG_PapyrusUtil.GetRGNames()
    While liRg < lSRgNames.Length      
      fIRaceSizeHdls[liRg] = AddSliderOption(lSRgNames[liRg], TNG_PapyrusUtil.GetRGMult(liRg), "{2}")
			String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRg)
			Int liAddonIdx = TNG_PapyrusUtil.GetRGAddon(liRg) + 2
			If (lSOptions.Length == 2) || (liAddonIdx < 0)
			  liAddonIdx = 1
			EndIf
      fIRaceTypeHdls[liRg] = AddMenuOption("", lSOptions[liAddonIdx])
      liRg += 1
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
      fiRandMalAddonHdl = AddToggleOption("$TNG_AMR", TNG_PapyrusUtil.GetBoolValue(ciRandomizeMale))
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
        fIMalAddonHdls[liCurr] = AddToggleOption(lSMalOptions[liCurr], TNG_PapyrusUtil.GetAddonStatus(False, liCurr))
      Else
        AddEmptyOption()
      EndIf
      liCurr += 1
    EndWhile
    Return
  EndIf  
  
  If asPage == Pages[3]
		String[] lSAllS52Mods = TNG_PapyrusUtil.GetSlot52Mods()
    If lSAllS52Mods.Length == 0
      AddTextOption("$TNG_52N","")
      Return
    EndIf
    bool lb52DefR = TNG_PapyrusUtil.GetBoolValue(ciRevealSlot52s)
    bool lb52Cust = TNG_PapyrusUtil.GetBoolValue(ciLetMixSlot52s)
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
    Int liModCount = lSAllS52Mods.Length
    While liModCount > 0
		  liModCount -= 1
      fI52ModHdls[liModCount] = AddToggleOption(lSAllS52Mods[liModCount], TNG_PapyrusUtil.Slot52ModBehavior(lSAllS52Mods[liModCount], -1), li52ModsFlag)
    EndWhile
    Return
  EndIf
  
  If asPage == Pages[4]
		AddHeaderOption("$TNG_GPH")
		AddHeaderOption("")
    fiPCEHdl = AddToggleOption("$TNG_GEP", TNG_PapyrusUtil.GetBoolValue(ciExcludePlayer))
    fiPCUHdl = AddToggleOption("$TNG_GPC", TNG_PapyrusUtil.GetBoolValue(ciCheckingPCGen))
				
		AddHeaderOption("$TNG_L_H")
		AddHeaderOption("")
    Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(-1)
    fiLogLvlHdl = AddMenuOption("$TNG_L_T", cSLogOptions[liLvl])
    fiLogDirHdl = AddToggleOption("$TNG_L_D", False)
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
    Int liCurr = cSSizeGlobals.Length
    While liCurr > 0
      liCurr -= 1
      If aiOption == fIGlblSizeHdls[liCurr]
        SetInfoText("$TNG_H_S")
        Return
      EndIf
    EndWhile
    Return
  EndIf  
  
  If CurrentPage == Pages[1]
    Int liCurr = TNG_PapyrusUtil.GetRGNames().Length
    While liCurr > 0
      liCurr -= 1
      If aiOption == fIRaceTypeHdls[liCurr]		
				If Input.IsKeyPressed(ciLockKey)
					SetInfoText(TNG_PapyrusUtil.GetRgInfo(liCurr))
				Else
					SetInfoText("$TNG_HOA")
				EndIf
        Return
      EndIf
      If aiOption == fIRaceSizeHdls[liCurr]
				If Input.IsKeyPressed(ciLockKey)
					SetInfoText(TNG_PapyrusUtil.GetRgInfo(liCurr))
				Else
					SetInfoText("$TNG_HOS")
				EndIf
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
    If aiOption == fiRandMalAddonHdl
      SetInfoText("$TNG_HRM")
      Return
    EndIf
		Int liCurr = TNG_PapyrusUtil.GetAllPossibleAddons(True).Length		
    While liCurr > 0
		  liCurr -= 1
      If aiOption == fIFemAddonHdls[liCurr]
        SetInfoText("$TNG_HAD")
        Return
      EndIf
    EndWhile
		liCurr = TNG_PapyrusUtil.GetAllPossibleAddons(False).Length
    While liCurr > 0
			liCurr -= 1
      If aiOption == fIMalAddonHdls[liCurr]
        SetInfoText("$TNG_HAD")
        Return
      EndIf
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
  
  If CurrentPage == Pages[4]
    If aiOption == fiPCUHdl
      SetInfoText("$TNG_HPU")
      Return
    EndIf
    If aiOption == fiPCEHdl
      SetInfoText("$TNG_HGE")
      Return
    EndIf
    If aiOption == fiLogLvlHdl
      SetInfoText("$TNG_HLL")
      Return
    EndIf
		If aiOption == fiLogDirHdl
		  SetInfoText("$TNG_HLF")
			Return
		EndIf
    Return
  EndIf
  
EndEvent

Event OnOptionDefault(Int aiOption)

  If CurrentPage == Pages[0]
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
	  Int liOpLoop = cSSizeGlobals.Length
		While liOpLoop > 0
			liOpLoop -= 1
			If aiOption == fIGlblSizeHdls[liOpLoop]      
				GlobalSizes[liOpLoop].SetValue(cFSizeDefaults[liOpLoop])
				SetSliderOptionValue(fIGlblSizeHdls[liOpLoop], GlobalSizes[liOpLoop].GetValue(), "{2}")
				TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
				Return
			EndIf
		EndWhile
		Return
	EndIf

	
  If CurrentPage == Pages[1]
		Int liOpLoop = TNG_PapyrusUtil.GetRGNames().Length
		While liOpLoop > 0
			liOpLoop -= 1
			If aiOption == fIRaceSizeHdls[liOpLoop]
				TNG_PapyrusUtil.SetRGMult(liOpLoop, 1.0)
				SetSliderOptionValue(fIRaceSizeHdls[liOpLoop], TNG_PapyrusUtil.GetRGMult(liOpLoop), "{2}")
				TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
				Return
			EndIf
			If aiOption == fIRaceTypeHdls[liOpLoop]
				TNG_PapyrusUtil.SetRGAddon(liOpLoop, -2)
				String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liOpLoop)
				Int liChoiceRes = TNG_PapyrusUtil.GetRGAddon(liOpLoop) + 2
				SetMenuOptionValue(fIRaceTypeHdls[liOpLoop], lSOptions[liChoiceRes])
				Return
			EndIf
		EndWhile
		Return
	EndIf

	If CurrentPage == Pages[2]	
		If aiOption == fiWomenChanceHdl    
			WomenChance.SetValue(20)
			SetSliderOptionValue(fiWomenChanceHdl, WomenChance.GetValue(), "{0}%")
			Return
		EndIf
		If aiOption == fiRandMalAddonHdl
			TNG_PapyrusUtil.SetBoolValue(ciRandomizeMale,False)
			SetToggleOptionValue(fiRandMalAddonHdl,False)
			Return
		EndIf
		Int liOpLoop = TNG_PapyrusUtil.GetAllPossibleAddons(True).Length
		While liOpLoop > 0
			liOpLoop -= 1
			If aiOption == fIFemAddonHdls[liOpLoop]
				TNG_PapyrusUtil.SetAddonStatus(True, liOpLoop, False)
				SetToggleOptionValue(aiOption, False)
				Return
			EndIf
		EndWhile
		liOpLoop = TNG_PapyrusUtil.GetAllPossibleAddons(False).Length
		While liOpLoop > 0
			liOpLoop -= 1
			If aiOption == fIMalAddonHdls[liOpLoop]
				TNG_PapyrusUtil.SetAddonStatus(False, liOpLoop, True)
				SetToggleOptionValue(aiOption, False)
				Return
			EndIf
		EndWhile
		Return
	EndIf
	
	If CurrentPage == Pages[3]
		If aiOption == fi52DefBehaviorHdl
			TNG_PapyrusUtil.SetBoolValue(ciRevealSlot52s, False)
			TNG_PapyrusUtil.SetBoolValue(ciLetMixSlot52s, False)    
			SetMenuOptionValue(fi52DefBehaviorHdl, cS52Options[0])
			Update52Behaviors(0)
		EndIf
		Return
	EndIf

	If CurrentPage == Pages[4]
		If aiOption == fiPCUHdl
			TNG_PapyrusUtil.SetBoolValue(ciCheckingPCGen, False)
			SetToggleOptionValue(fiPCUHdl,False)
		EndIf
		If aiOption == fiPCEHdl
			TNG_PapyrusUtil.SetBoolValue(ciExcludePlayer, False)
			SetToggleOptionValue(ciExcludePlayer, False)
			Return
		EndIf
		If aiOption == fiLogLvlHdl
			Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(1)
			SetMenuOptionValue(fiLogLvlHdl, cSLogOptions[liLvl])
		EndIf
		Return
	EndIf
EndEvent

Event OnOptionMenuOpen(Int aiOption)
  
  If CurrentPage == Pages[1]
		Int liRg = TNG_PapyrusUtil.GetRGNames().Length
		While liRg
			liRg -= 1
			If aiOption == fIRaceTypeHdls[liRg]
				String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRg)
				SetMenuDialogOptions(lSOptions)
				SetMenuDialogStartIndex(TNG_PapyrusUtil.GetRGAddon(liRg) + 2)
				SetMenuDialogDefaultIndex(0)
				Return
			EndIf
		EndWhile
		Return
	EndIf
	
	If CurrentPage == Pages[3]
		If aiOption == fi52DefBehaviorHdl
			bool lb52DefR = TNG_PapyrusUtil.GetBoolValue(ciRevealSlot52s)
			bool lb52Cust = TNG_PapyrusUtil.GetBoolValue(ciLetMixSlot52s)
			Int li52Option = 0
			If lb52Cust
				li52Option = 2
			ElseIf lb52DefR
				li52Option = 1
			EndIf
			SetMenuDialogOptions(cS52Options)
			SetMenuDialogStartIndex(li52Option)			
			SetMenuDialogDefaultIndex(0)
			Return
		EndIf
		Return
  EndIf
	
	If CurrentPage == Pages[4]
		If aiOption == fiLogLvlHdl			
			SetMenuDialogOptions(cSLogOptions)
			Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(-1)
			SetMenuDialogStartIndex(liLvl)
			SetMenuDialogDefaultIndex(1)
			Return
		EndIf
		Return
  EndIf
EndEvent

Event OnOptionMenuAccept(Int aiOption, Int aiChoice)  

  If CurrentPage == Pages[1]
		Int liRg = TNG_PapyrusUtil.GetRGNames().Length
		While liRg
			liRg -= 1
			If aiOption == fIRaceTypeHdls[liRg]
				TNG_PapyrusUtil.SetRGAddon(liRg, aiChoice - 2)
				String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRg)
				Int liChoiceRes = TNG_PapyrusUtil.GetRGAddon(liRg) + 2
				SetMenuOptionValue(fIRaceTypeHdls[liRg], lSOptions[liChoiceRes])
				Return
			EndIf
		EndWhile
    Return
  EndIf

	If CurrentPage == Pages[3]
		If aiOption == fi52DefBehaviorHdl
			SetMenuOptionValue(fi52DefBehaviorHdl, cS52Options[aiChoice])
			Update52Behaviors(aiChoice)
			Return
		EndIf
    Return
  EndIf

	If CurrentPage == Pages[4]
		If aiOption == fiLogLvlHdl   
			Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(aiChoice) 
			SetMenuOptionValue(fiLogLvlHdl, cSLogOptions[liLvl])
			Return
		EndIf
		Return
	EndIf

EndEvent

Event OnOptionSliderOpen(Int aiOption)
  If CurrentPage == Pages[0]
    Int liSize = cSSizeGlobals.Length
    While liSize > 0
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
    Int liRg = TNG_PapyrusUtil.GetRGNames().Length
    While liRg > 0
      liRg -= 1
      If aiOption == fIRaceSizeHdls[liRg]
        SetSliderDialogStartValue(TNG_PapyrusUtil.GetRGMult(liRg))
        SetSliderDialogDefaultValue(1.0)
        SetSliderDialogRange(0.1, 2.0)
        SetSliderDialogInterval(0.01)
        Return
      EndIf
    EndWhile
    Return
  EndIf

	If CurrentPage == Pages[2]
		If aiOption == fiWomenChanceHdl
			SetSliderDialogStartValue(WomenChance.GetValue())
			SetSliderDialogDefaultValue(20)
			SetSliderDialogRange(0, 100)
			SetSliderDialogInterval(1)
			Return
		EndIf
		Return
	EndIf

EndEvent

Event OnOptionSliderAccept(Int aiOption, Float afValue)

  If CurrentPage == Pages[0]
    Int liSize = cSSizeGlobals.Length
    While liSize > 0
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
    Int liRg = TNG_PapyrusUtil.GetRGNames().Length
    While liRg > 0
      liRg -= 1
      If aiOption == fIRaceSizeHdls[liRg]
        TNG_PapyrusUtil.SetRGMult(liRg, afValue)
        SetSliderOptionValue(fIRaceSizeHdls[liRg], TNG_PapyrusUtil.GetRGMult(liRg), "{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
        Return
      EndIf
    EndWhile
    Return
  EndIf

	If CurrentPage == Pages[2]
		If aiOption == fiWomenChanceHdl
			WomenChance.SetValue(afValue)
			SetSliderOptionValue(fiWomenChanceHdl, WomenChance.GetValue(), "{0}%")
			Return
		EndIf
		Return
	EndIf	

EndEvent

Event OnOptionSelect(Int aiOption)

  If CurrentPage == Pages[0]    
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
    Return
  EndIf

  If CurrentPage == Pages[2]
    If aiOption == fiRandMalAddonHdl
      TNG_PapyrusUtil.SetBoolValue(ciRandomizeMale, !TNG_PapyrusUtil.GetBoolValue(ciRandomizeMale))
      SetToggleOptionValue(fiRandMalAddonHdl, TNG_PapyrusUtil.GetBoolValue(ciRandomizeMale))
      Return
    EndIf
    Int liCurr = TNG_PapyrusUtil.GetAllPossibleAddons(True).Length
    While liCurr > 0
			liCurr -= 1
      If aiOption == fIFemAddonHdls[liCurr]
        TNG_PapyrusUtil.SetAddonStatus(True, liCurr, !TNG_PapyrusUtil.GetAddonStatus(True, liCurr))
        SetToggleOptionValue(fIFemAddonHdls[liCurr], TNG_PapyrusUtil.GetAddonStatus(True, liCurr))
        Return
      EndIf
    EndWhile
    liCurr = TNG_PapyrusUtil.GetAllPossibleAddons(False).Length
    While liCurr > 0
			liCurr -= 1
      If aiOption == fIMalAddonHdls[liCurr]
        TNG_PapyrusUtil.SetAddonStatus(False, liCurr, !TNG_PapyrusUtil.GetAddonStatus(False, liCurr))
        SetToggleOptionValue(fIMalAddonHdls[liCurr], TNG_PapyrusUtil.GetAddonStatus(False, liCurr))
        Return
      EndIf
    EndWhile
    Return
  EndIf

  If CurrentPage == Pages[3]
		String[] lSAllS52Mods = TNG_PapyrusUtil.GetSlot52Mods()
    Int liModCount = lSAllS52Mods.Length   
    While liModCount > 0
      liModCount -= 1
      If aiOption == fI52ModHdls[liModCount]
        Int liNew = -1
        If TNG_PapyrusUtil.Slot52ModBehavior(lSAllS52Mods[liModCount], -1)
          liNew = 0
        Else
          liNew = 1
        EndIf
        TNG_PapyrusUtil.Slot52ModBehavior(lSAllS52Mods[liModCount], liNew)
        SetToggleOptionValue(fI52ModHdls[liModCount], TNG_PapyrusUtil.Slot52ModBehavior(lSAllS52Mods[liModCount], -1))
        Return
      EndIf
    EndWhile  
    Return
  EndIf

	If CurrentPage == Pages[4]		
		If aiOption == fiPCEHdl
      TNG_PapyrusUtil.SetBoolValue(ciExcludePlayer, !TNG_PapyrusUtil.GetBoolValue(ciExcludePlayer))    
      SetToggleOptionValue(fiPCEHdl, TNG_PapyrusUtil.GetBoolValue(ciExcludePlayer))
      If TNG_PapyrusUtil.GetBoolValue(ciExcludePlayer) 
        PlayerSize = -1
      EndIf
      Return
    EndIf
		If aiOption == fiPCUHdl
		  TNG_PapyrusUtil.SetBoolValue(ciCheckingPCGen, !TNG_PapyrusUtil.GetBoolValue(ciCheckingPCGen))			
      SetToggleOptionValue(fiPCUHdl, TNG_PapyrusUtil.GetBoolValue(ciCheckingPCGen))
			If TNG_PapyrusUtil.GetBoolValue(ciCheckingPCGen)
				Int res = TNG_PapyrusUtil.SetActorAddon(PlayerRef, PlayerSkin.GetValueInt())
				If (res >= 0) && !PlayerRef.IsOnMount()
					PlayerRef.QueueNiNodeUpdate()
				Else
					Debug.Notification("$TNG_WPT")
					PlayerSkin.SetValueInt(-2)
					TNG_PapyrusUtil.SetActorAddon(PlayerRef, PlayerSkin.GetValueInt())
					HandleWarnings(res)
				EndIf  
			EndIf			
		EndIf		
    If aiOption == fiLogDirHdl
      ShowMessage(TNG_PapyrusUtil.ShowLogLocation(), true, "Ok")
      Return
    EndIf
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
	  If Input.IsKeyPressed(ciLockKey)
		  LockOnNext()
		Else
			RiseAndDrop(True)
		EndIf
    Return
  EndIf
  If aiKey == GenDownKey.GetValueInt()
		If Input.IsKeyPressed(ciLockKey)
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
	If TNG_PapyrusUtil.GetBoolValue(ciCheckingPCGen)
		Int res = TNG_PapyrusUtil.SetActorAddon(PlayerRef, PlayerSkin.GetValueInt())
		If (res >= 0) && !PlayerRef.IsOnMount()
			PlayerRef.QueueNiNodeUpdate()
		EndIf
	EndIf

	If fiLastActor > 0
		fiLastActor = -1
		If Notifs
			Debug.Notification("$TNG_KAR")
		EndIf
	EndIf
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
  Bool lbShowSize = True
  If liModifyRes > 0
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
        Int liShapeRes = TNG_PapyrusUtil.SetActorAddon(akActor, liShape - 4)
        If liShapeRes < 0
          HandleWarnings(liShapeRes)
          Return
        EndIf        
        lbShowSize = (liShapeRes > 0)
        If !akActor.IsOnMount()
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
    While i < cSSizeGlobals.Length
      lkSizeMenu.AddEntryItem(cSSizeGlobals[i])
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
  Int liDir = -1
  If aIfRise
    liDir = 1
  EndIf
  If lkActor != fkLastActor
    fkLastActor = lkActor
    fiPos = -10
  EndIf
  fiPos += liDir
  If (fiPos*liDir) < 10    
    Debug.SendAnimationEvent(lkActor, "SOSBend" + fiPos)
  ElseIf fiPos == -10
    Debug.SendAnimationEvent(lkActor, "SOSFlaccid")
  Else
    fiPos -= liDir
  EndIf  
EndFunction  

Function HandleWarnings(Int aiRes)
  If aiRes > 0
    Return
  EndIf
  Debug.Notification(TNG_PapyrusUtil.GetErrDscr(aiRes))
EndFunction


Function Update52Behaviors(Int aiChoice)  
  If aiChoice < 2
		String[] lSAllS52Mods = TNG_PapyrusUtil.GetSlot52Mods()
    Int liModCount = lSAllS52Mods.Length
    While liModCount
      liModCount -= 1
      TNG_PapyrusUtil.Slot52ModBehavior(lSAllS52Mods[liModCount], aiChoice)        
    EndWhile    
    TNG_PapyrusUtil.SetBoolValue(ciRevealSlot52s, aiChoice==1)
    TNG_PapyrusUtil.SetBoolValue(ciLetMixSlot52s, False)
  Else
    TNG_PapyrusUtil.SetBoolValue(ciLetMixSlot52s, True)  
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
		If fiLastActor == 0
		  Debug.Notification("TNG_KAL")
		EndIf
	  Debug.Notification(lkActor.GetLeveledActorBase().GetName())
	EndIf
	RegisterForSingleUpdate(60.0)
EndFunction

Actor Function GetLockedActor()  
	fiL += 1
  Cell lkCell = PlayerRef.GetParentCell()
	Int liCount = lkCell.GetNumRefs(43)	
	If (fiL > liCount)
		fiLastActor = -1
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

