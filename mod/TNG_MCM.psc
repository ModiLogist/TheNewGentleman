ScriptName TNG_MCM extends SKI_ConfigBase

;Property
Bool Property Notifs auto
Actor Property PlayerRef auto
FormList Property FlGW auto
FormList Property FlUM auto

;Constants
Int cbExcludePlayer
Int cbCheckingPCGen
Int cbForceTheCheck
Int cbRevealSlot52s
Int cbLetMixSlot52s    
Int cbRandomizeMale
Int cbGoToBed
Int cbShowEveryRace
Int cbDAK

Int ciSetupNPC 
Int ciRiseGen
Int ciFallGen
Int ciSwapRevealing
Int ciWhyProblem

Int[] cFSizes
Int cfWomenChance

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
Int fiDebugKeyHdl
Int fiWomenChanceHdl
Int fiRandMalAddonHdl
Int fiLogLvlHdl
Int fiLogDirHdl

Int fi52DefBehaviorHdl

Int fiPCEHdl
Int fiPCUHdl
Int fiPCRHdl
Int fiPZZHdl

Int Function GetVersion()
  Return 12
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
  
  cbExcludePlayer = 0
  cbCheckingPCGen = 1
  cbForceTheCheck = 2
  cbRevealSlot52s = 3
  cbLetMixSlot52s = 4
  cbRandomizeMale = 5
  cbGoToBed = 6
  cbShowEveryRace = 7
  cbDAK = 8

  ciSetupNPC = 0
  ciRiseGen = 1
  ciFallGen = 2
  ciSwapRevealing = 3
  ciWhyProblem = 4

  cFSizes = new Int[5]
  cFSizes[0] = 0
  cFSizes[1] = 1
  cFSizes[2] = 2
  cFSizes[3] = 3
  cFSizes[4] = 4
  cfWomenChance = 6

  ciLockKey = 56 ; Left Alt Key
EndEvent

Event OnVersionUpdate(Int aiVersion)
  If (aiVersion != CurrentVersion)
    OnConfigInit()
  EndIf
EndEvent

Event OnGameReload()
  Parent.OnGameReload()
  If Game.GetModByName("Dynamic Activation Key.esp")
    fkDAK = Game.GetFormFromFile(0x801, "Dynamic Activation Key.esp") As GlobalVariable
  EndIf
  Int liKey = 5
  While liKey > 0
    liKey -= 1
    If TNG_PapyrusUtil.GetIntValue(liKey) > 0
      RegisterForKey(TNG_PapyrusUtil.GetIntValue(liKey))
    EndIf
  EndWhile
EndEvent

Event OnUpdate()
  If fiLastActor > 0
    fiLastActor = -1
    ShowNotification("$TNG_KAR")
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
        fiDAKHdl = AddToggleOption("$TNG_DAK", TNG_PapyrusUtil.GetBoolValue(cbDAK))
        AddEmptyOption()
      EndIf
    EndIf
    If !fkDAK
      TNG_PapyrusUtil.SetBoolValue(cbDAK, False)
    EndIf
    Int liOption = OPTION_FLAG_WITH_UNMAP
    fiNPCKeyHdl = AddKeyMapOption("$TNG_K_N", TNG_PapyrusUtil.GetIntValue(ciSetupNPC), liOption)
    fiRevealKeyHdl = AddKeyMapOption("$TNG_K_R", TNG_PapyrusUtil.GetIntValue(ciSwapRevealing), liOption)
    fiUpKeyHdl = AddKeyMapOption("$TNG_K_U", TNG_PapyrusUtil.GetIntValue(ciRiseGen), OPTION_FLAG_WITH_UNMAP)
    fiDownKeyHdl = AddKeyMapOption("$TNG_K_D", TNG_PapyrusUtil.GetIntValue(ciFallGen), OPTION_FLAG_WITH_UNMAP)
    AddHeaderOption("$TNG_SOH")
    AddHeaderOption("")
    Int liGlbSize = 0
    While liGlbSize < cSSizeGlobals.Length
      fIGlblSizeHdls[liGlbSize] = AddSliderOption(cSSizeGlobals[liGlbSize], TNG_PapyrusUtil.GetFloatValue(cFSizes[liGlbSize]), "{2}")
      liGlbSize += 1
    EndWhile
    AddEmptyOption()
    Return
  EndIf
  
  If asPage == Pages[1]
    AddHeaderOption("$TNG_OGS")
    AddHeaderOption("$TNG_OGT")
    Int liRg = 0
    String[] lSRgNames = TNG_PapyrusUtil.GetRgNames()
    While liRg < lSRgNames.Length      
      fIRaceSizeHdls[liRg] = AddSliderOption(lSRgNames[liRg], TNG_PapyrusUtil.GetRgMult(liRg), "{2}")
      String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRg)
      Int liAddonIdx = TNG_PapyrusUtil.GetRgAddon(liRg) + 2
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
      fiWomenChanceHdl = AddSliderOption("$TNG_AWC", Tng_PapyrusUtil.GetFloatValue(cfWomenChance), "{0}%")
    EndIf
    If lSMalOptions.Length == 0
      AddTextOption("$TNG_ANM", "")
    Else
      fiRandMalAddonHdl = AddToggleOption("$TNG_AMR", TNG_PapyrusUtil.GetBoolValue(cbRandomizeMale))
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
    bool lb52DefR = TNG_PapyrusUtil.GetBoolValue(cbRevealSlot52s)
    bool lb52Cust = TNG_PapyrusUtil.GetBoolValue(cbLetMixSlot52s)
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
    AddHeaderOption("$TNG_GAD")
    fiPCEHdl = AddToggleOption("$TNG_GEP", TNG_PapyrusUtil.GetBoolValue(cbExcludePlayer))
    Int liFlag = OPTION_FLAG_NONE
    If TNG_PapyrusUtil.GetBoolValue(cbForceTheCheck)
      liFlag = OPTION_FLAG_DISABLED
    EndIf
    fiPZZHdl = AddToggleOption("$TNG_GZZ", TNG_PapyrusUtil.GetBoolValue(cbGoToBed))
    fiPCUHdl = AddToggleOption("$TNG_GPC", TNG_PapyrusUtil.GetBoolValue(cbCheckingPCGen), liFlag)
    fiPCRHdl = AddToggleOption("$TNG_GPR", TNG_PapyrusUtil.GetBoolValue(cbShowEveryRace))
            
    AddHeaderOption("$TNG_L_H")
    AddHeaderOption("")
    Int liLvl = TNG_PapyrusUtil.UpdateLogLvl(-1)
    fiLogLvlHdl = AddMenuOption("$TNG_L_T", cSLogOptions[liLvl])
    fiLogDirHdl = AddToggleOption("$TNG_L_D", False)
    Int liOption = OPTION_FLAG_WITH_UNMAP
    fiDebugKeyHdl = AddKeyMapOption("$TNG_P_K", TNG_PapyrusUtil.GetIntValue(ciWhyProblem), liOption)
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
    Int liCurr = TNG_PapyrusUtil.GetRgNames().Length
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
    If aiOption == fiPCRHdl
      SetInfoText("$TNG_HPR")
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
    If aiOption == fiDebugKeyHdl
      SetInfoText("$TNG_HLD")
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
      TNG_PapyrusUtil.SetBoolValue(cbDAK, False)
      SetToggleOptionValue(fiDAKHdl, False)
      Bool lbResetAll = ShowMessage("$TNG_DKN", true, "$TNG_Yes", "$TNG__No")
      If lbResetAll
        OnOptionDefault(fiNPCKeyHdl)
        OnOptionDefault(fiRevealKeyHdl)
        OnOptionDefault(fiUpKeyHdl)
        OnOptionDefault(fiDownKeyHdl)
        OnOptionDefault(fiDebugKeyHdl)
      EndIf
      Return
    EndIf
    If aiOption == fiNPCKeyHdl
      If TNG_PapyrusUtil.GetIntValue(ciSetupNPC) > 0
        UnregisterForKey(TNG_PapyrusUtil.GetIntValue(ciSetupNPC))
        TNG_PapyrusUtil.SetIntValue(ciSetupNPC,-1)
        SetKeymapOptionValue(fiNPCKeyHdl, -1)
      EndIf
      Return
    EndIf
    If aiOption == fiRevealKeyHdl
      If TNG_PapyrusUtil.GetIntValue(ciSwapRevealing) > 0
        UnregisterForKey(TNG_PapyrusUtil.GetIntValue(ciSwapRevealing))
        TNG_PapyrusUtil.SetIntValue(ciSwapRevealing,-1)
        SetKeymapOptionValue(fiRevealKeyHdl, -1)
      EndIf
      Return
    EndIf
    If aiOption == fiUpKeyHdl
      If TNG_PapyrusUtil.GetIntValue(ciRiseGen) > 0
        UnregisterForKey(TNG_PapyrusUtil.GetIntValue(ciRiseGen))
        TNG_PapyrusUtil.SetIntValue(ciRiseGen,-1)
        SetKeymapOptionValue(fiUpKeyHdl, -1)
      EndIf
      Return
    EndIf
    If aiOption == fiDownKeyHdl
      If TNG_PapyrusUtil.GetIntValue(ciFallGen) > 0
        UnregisterForKey(TNG_PapyrusUtil.GetIntValue(ciFallGen))
        TNG_PapyrusUtil.SetIntValue(ciFallGen,-1)
        SetKeymapOptionValue(fiDownKeyHdl, -1)
      EndIf
      Return
    EndIf
    If aiOption == fiDebugKeyHdl
      If TNG_PapyrusUtil.GetIntValue(ciWhyProblem) > 0
        UnregisterForKey(TNG_PapyrusUtil.GetIntValue(ciWhyProblem))
        TNG_PapyrusUtil.SetIntValue(ciWhyProblem,-1)
        SetKeymapOptionValue(fiDebugKeyHdl, -1)
      EndIf
      Return
    EndIf
    Int liOpLoop = cSSizeGlobals.Length
    While liOpLoop > 0
      liOpLoop -= 1
      If aiOption == fIGlblSizeHdls[liOpLoop]      
        TNG_PapyrusUtil.SetFloatValue(cFSizes[liOpLoop], cFSizeDefaults[liOpLoop])
        SetSliderOptionValue(fIGlblSizeHdls[liOpLoop], TNG_PapyrusUtil.GetFloatValue(cFSizes[liOpLoop]), "{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
        Return
      EndIf
    EndWhile
    Return
  EndIf

  
  If CurrentPage == Pages[1]
    Int liOpLoop = TNG_PapyrusUtil.GetRgNames().Length
    While liOpLoop > 0
      liOpLoop -= 1
      If aiOption == fIRaceSizeHdls[liOpLoop]
        TNG_PapyrusUtil.SetRGMult(liOpLoop, 1.0)
        SetSliderOptionValue(fIRaceSizeHdls[liOpLoop], TNG_PapyrusUtil.GetRgMult(liOpLoop), "{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
        Return
      EndIf
      If aiOption == fIRaceTypeHdls[liOpLoop]
        TNG_PapyrusUtil.SetRGAddon(liOpLoop, -2)
        String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liOpLoop)
        Int liChoiceRes = TNG_PapyrusUtil.GetRgAddon(liOpLoop) + 2
        SetMenuOptionValue(fIRaceTypeHdls[liOpLoop], lSOptions[liChoiceRes])
        Return
      EndIf
    EndWhile
    Return
  EndIf

  If CurrentPage == Pages[2]  
    If aiOption == fiWomenChanceHdl    
      TNG_PapyrusUtil.SetFloatValue(cfWomenChance, 20.0)
      SetSliderOptionValue(fiWomenChanceHdl, TNG_PapyrusUtil.GetFloatValue(cfWomenChance), "{0}%")
      Return
    EndIf
    If aiOption == fiRandMalAddonHdl
      TNG_PapyrusUtil.SetBoolValue(cbRandomizeMale,False)
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
      TNG_PapyrusUtil.SetBoolValue(cbRevealSlot52s, False)
      TNG_PapyrusUtil.SetBoolValue(cbLetMixSlot52s, False)    
      SetMenuOptionValue(fi52DefBehaviorHdl, cS52Options[0])
      Update52Behaviors(0)
    EndIf
    Return
  EndIf

  If CurrentPage == Pages[4]
    If (aiOption == fiPCUHdl) && !TNG_PapyrusUtil.GetBoolValue(cbForceTheCheck)
      TNG_PapyrusUtil.SetBoolValue(cbCheckingPCGen, False)
      SetToggleOptionValue(fiPCUHdl,False)
    EndIf
    If (aiOption == fiPCRHdl)
      TNG_PapyrusUtil.SetBoolValue(cbShowEveryRace, False)
      SetToggleOptionValue(fiPCRHdl,False)
    EndIf
    If aiOption == fiPCEHdl
      TNG_PapyrusUtil.SetBoolValue(cbExcludePlayer, False)
      SetToggleOptionValue(fiPCEHdl, False)
      Return
    EndIf
    If aiOption == fiPZZHdl
      TNG_PapyrusUtil.SetBoolValue(cbGoToBed, False)
      SetToggleOptionValue(fiPZZHdl, False)
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
    Int liRg = TNG_PapyrusUtil.GetRgNames().Length
    While liRg
      liRg -= 1
      If aiOption == fIRaceTypeHdls[liRg]
        String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRg)
        SetMenuDialogOptions(lSOptions)
        SetMenuDialogStartIndex(TNG_PapyrusUtil.GetRgAddon(liRg) + 2)
        SetMenuDialogDefaultIndex(0)
        Return
      EndIf
    EndWhile
    Return
  EndIf
  
  If CurrentPage == Pages[3]
    If aiOption == fi52DefBehaviorHdl
      bool lb52DefR = TNG_PapyrusUtil.GetBoolValue(cbRevealSlot52s)
      bool lb52Cust = TNG_PapyrusUtil.GetBoolValue(cbLetMixSlot52s)
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
    Int liRg = TNG_PapyrusUtil.GetRgNames().Length
    While liRg
      liRg -= 1
      If aiOption == fIRaceTypeHdls[liRg]
        TNG_PapyrusUtil.SetRGAddon(liRg, aiChoice - 2)
        String[] lSOptions = TNG_PapyrusUtil.GetRgAddons(liRg)
        Int liChoiceRes = TNG_PapyrusUtil.GetRgAddon(liRg) + 2
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
        SetSliderDialogStartValue(TNG_PapyrusUtil.GetFloatValue(cFSizes[liSize]))
        SetSliderDialogDefaultValue(cFSizeDefaults[liSize])
        SetSliderDialogRange(0.1, 4.0)
        SetSliderDialogInterval(0.01)
        Return
      EndIf
    EndWhile
    Return
  EndIf

  If CurrentPage == Pages[1]
    Int liRg = TNG_PapyrusUtil.GetRgNames().Length
    While liRg > 0
      liRg -= 1
      If aiOption == fIRaceSizeHdls[liRg]
        SetSliderDialogStartValue(TNG_PapyrusUtil.GetRgMult(liRg))
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
      SetSliderDialogStartValue(Tng_PapyrusUtil.GetFloatValue(cfWomenChance))
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
        TNG_PapyrusUtil.SetFloatValue(cFSizes[liSize], afValue)
        SetSliderOptionValue(fIGlblSizeHdls[liSize], TNG_PapyrusUtil.GetFloatValue(cFSizes[liSize]), "{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
        Return
      EndIf
    EndWhile
    Return
  EndIf

  If CurrentPage == Pages[1]
    Int liRg = TNG_PapyrusUtil.GetRgNames().Length
    While liRg > 0
      liRg -= 1
      If aiOption == fIRaceSizeHdls[liRg]
        TNG_PapyrusUtil.SetRGMult(liRg, afValue)
        SetSliderOptionValue(fIRaceSizeHdls[liRg], TNG_PapyrusUtil.GetRgMult(liRg), "{2}")
        TNG_PapyrusUtil.SetActorSize(PlayerRef, -1)
        Return
      EndIf
    EndWhile
    Return
  EndIf

  If CurrentPage == Pages[2]
    If aiOption == fiWomenChanceHdl
      Tng_PapyrusUtil.SetFloatValue(cfWomenChance, afValue)
      SetSliderOptionValue(fiWomenChanceHdl, Tng_PapyrusUtil.GetFloatValue(cfWomenChance), "{0}%")
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
      TNG_PapyrusUtil.SetBoolValue(cbDAK, !TNG_PapyrusUtil.GetBoolValue(cbDAK))
      SetToggleOptionValue(fiDAKHdl, TNG_PapyrusUtil.GetBoolValue(cbDAK) )
      If !TNG_PapyrusUtil.GetBoolValue(cbDAK)
        Bool lbResetAll = ShowMessage("$TNG_DKN", true, "$TNG_Yes", "$TNG__No")
        If lbResetAll
          OnOptionDefault(fiNPCKeyHdl)
          OnOptionDefault(fiRevealKeyHdl)
          OnOptionDefault(fiUpKeyHdl)
          OnOptionDefault(fiDownKeyHdl)
          OnOptionDefault(fiDebugKeyHdl)
        EndIf
      EndIf
      Return
    EndIf
    Return
  EndIf

  If CurrentPage == Pages[2]
    If aiOption == fiRandMalAddonHdl
      TNG_PapyrusUtil.SetBoolValue(cbRandomizeMale, !TNG_PapyrusUtil.GetBoolValue(cbRandomizeMale))
      SetToggleOptionValue(fiRandMalAddonHdl, TNG_PapyrusUtil.GetBoolValue(cbRandomizeMale))
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
    If !TNG_PapyrusUtil.GetBoolValue(cbLetMixSlot52s)
      Return
    EndIf
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
      TNG_PapyrusUtil.SetBoolValue(cbExcludePlayer, !TNG_PapyrusUtil.GetBoolValue(cbExcludePlayer))    
      SetToggleOptionValue(fiPCEHdl, TNG_PapyrusUtil.GetBoolValue(cbExcludePlayer))
      Return
    EndIf
    If aiOption == fiPCUHdl
      TNG_PapyrusUtil.SetBoolValue(cbCheckingPCGen, !TNG_PapyrusUtil.GetBoolValue(cbCheckingPCGen))      
      SetToggleOptionValue(fiPCUHdl, TNG_PapyrusUtil.GetBoolValue(cbCheckingPCGen))
      If TNG_PapyrusUtil.GetBoolValue(cbCheckingPCGen) && (TNG_PapyrusUtil.CanModifyActor(PlayerRef) > 0)
        Int res = TNGSetAddon(PlayerRef, -3)
      EndIf    
      Return
    EndIf
    If aiOption == fiPCRHdl
      TNG_PapyrusUtil.SetBoolValue(cbShowEveryRace, !TNG_PapyrusUtil.GetBoolValue(cbShowEveryRace))      
      SetToggleOptionValue(fiPCRHdl, TNG_PapyrusUtil.GetBoolValue(cbShowEveryRace))
      Return
    EndIf
    If aiOption == fiPZZHdl
      TNG_PapyrusUtil.SetBoolValue(cbGoToBed, !TNG_PapyrusUtil.GetBoolValue(cbGoToBed))      
      SetToggleOptionValue(fiPZZHdl, TNG_PapyrusUtil.GetBoolValue(cbGoToBed))
      Return
    EndIf
    If aiOption == fiLogDirHdl
      ShowMessage(TNG_PapyrusUtil.ShowLogLocation(), true, "Ok")
      Return
    EndIf
    Return
  EndIf

EndEvent

Event OnOptionKeyMapChange(Int aiOption, Int aiKeyCode, String asConflictControl, String asConflictName)
  If !fkDAK || !TNG_PapyrusUtil.GetBoolValue(cbDAK)
    Bool lbContinue = True
    String lsNotif
    If (asConflictControl != "") && (aiKeyCode != -1)
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
  If (aiOption == fiNPCKeyHdl) || (aiOption == fiRevealKeyHdl) || (aiOption == fiUpKeyHdl) || (aiOption == fiDownKeyHdl) || (aiOption == fiDebugKeyHdl)
    UpdateKey(aiOption, aiKeyCode)
    SetKeymapOptionValue(aiOption, aiKeyCode)
  EndIf
EndEvent

Event OnKeyDown(Int aiKey)
  If TNG_PapyrusUtil.GetBoolValue(cbDAK) && !fkDAK
    ShowNotification("$TNG_W_3", true)
    Return
  EndIf
  If TNG_PapyrusUtil.GetBoolValue(cbDAK)
    If fkDAK.GetValueInt() != 1
      Return
    EndIf
  EndIf
  If aiKey == TNG_PapyrusUtil.GetIntValue(ciSetupNPC)
    If Utility.IsInMenuMode()
      Return
    EndIf
    ShowTNGMenu(TargetOrPlayer(False))
    Return
  EndIf
  If aiKey  == TNG_PapyrusUtil.GetIntValue(ciRiseGen)
    If Input.IsKeyPressed(ciLockKey)
      LockOnNext()
    Else
      RiseAndDrop(True)
    EndIf
    Return
  EndIf
  If aiKey == TNG_PapyrusUtil.GetIntValue(ciFallGen)
    If Input.IsKeyPressed(ciLockKey)
      LockOnNext()
    Else
      RiseAndDrop(False)
    EndIf
    Return
  EndIf
  If aiKey == TNG_PapyrusUtil.GetIntValue(ciSwapRevealing)
    If Utility.IsInMenuMode()
      Return
    EndIf
    Actor lkActor = TargetOrPlayer(False)
    If !lkActor
      Return
    EndIf
    String[] lSItems = TNG_PapyrusUtil.ActorItemsInfo(lkActor)
    If (lSItems.Length == 0)      
      ShowNotification("$TNG_N_1")
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
      If lbSuccess
        ShowNotification("$TNG_N_2")
      Else
        ShowNotification("$TNG_N_3")
      EndIf
    Return
  EndIf
  If aiKey == TNG_PapyrusUtil.GetIntValue(ciWhyProblem)
    If Utility.IsInMenuMode()
      Return
    EndIf
    ShowDebugMenu(TargetOrPlayer(False))
    Return
  EndIf
EndEvent


Function UpdateKey(Int aHdl, Int aiNewKey)
  Int liCurrKey = -1
  If (aHdl == fiNPCKeyHdl)
    liCurrKey = TNG_PapyrusUtil.GetIntValue(ciSetupNPC)
    TNG_PapyrusUtil.SetIntValue(ciSetupNPC,aiNewKey)
  ElseIf (aHdl == fiRevealKeyHdl)
    liCurrKey = TNG_PapyrusUtil.GetIntValue(ciSwapRevealing)
    TNG_PapyrusUtil.SetIntValue(ciSwapRevealing,aiNewKey)  
  ElseIf (aHdl == fiUpKeyHdl)
    liCurrKey = TNG_PapyrusUtil.GetIntValue(ciRiseGen)
    TNG_PapyrusUtil.SetIntValue(ciRiseGen,aiNewKey)    
  ElseIf (aHdl == fiDownKeyHdl)
    liCurrKey = TNG_PapyrusUtil.GetIntValue(ciFallGen)
    TNG_PapyrusUtil.SetIntValue(ciFallGen,aiNewKey)
  ElseIf (aHdl == fiDebugKeyHdl)
    liCurrKey = TNG_PapyrusUtil.GetIntValue(ciWhyProblem)
    TNG_PapyrusUtil.SetIntValue(ciWhyProblem,aiNewKey)
  EndIf
  If (liCurrKey > 0) && (liCurrKey != aiNewKey)
    UnregisterForKey(liCurrKey)
  EndIf
  RegisterForKey(aiNewKey)
EndFunction 

Function ShowTNGMenu(Actor akActor)
  If !akActor
    ShowNotification("$TNG_W_1")
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
    If lSAddons.Length > 2
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
        Int liShapeRes = TNGSetAddon(akActor, liShape - 4)      
        lbShowSize = (liShapeRes > 0)
      EndIf
    Else
      lbShowSize = False
      ShowNotification("$TNG_N_4")
    EndIf
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
  ShowNotification(TNG_PapyrusUtil.GetErrDscr(aiRes))
EndFunction


Function Update52Behaviors(Int aiChoice)
  TNG_PapyrusUtil.SetBoolValue(cbLetMixSlot52s, aiChoice==2)
  If !TNG_PapyrusUtil.GetBoolValue(cbLetMixSlot52s) 
    TNG_PapyrusUtil.SetBoolValue(cbRevealSlot52s, aiChoice==1)
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
  If fiLastActor == 0
    ShowNotification("$TNG_KAL", true)
  EndIf
  ShowNotification(lkActor.GetLeveledActorBase().GetName(), true)
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
  If (lkNPC.GetSex() == 1) && (!FlGW.HasForm(lkNPC))    
    fiLastActor += 1
    Return GetLockedActor()
  EndIf
  If (FlUM.HasForm(lkNPC))
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

Function ShowDebugMenu(Actor akActor)
  UIListMenu lkDebugMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
  lkDebugMenu.ResetMenu()
  lkDebugMenu.AddEntryItem("$TNG_P_0")
  lkDebugMenu.AddEntryItem("$TNG_P_1")
  lkDebugMenu.AddEntryItem("$TNG_P_2")  
  lkDebugMenu.OpenMenu(akActor)
  Int liIssueID = lkDebugMenu.GetResultInt()  
  ShowNotification(TNG_PapyrusUtil.WhyProblem(akActor,liIssueID), true)
EndFunction

Int Function TNGSetAddon(Actor akActor, Int aiAddon)
  akActor.SendModEvent("TNGSetMyAddon", akActor.GetName(), aiAddon as Float + 0.1)
  Int liRes = TNG_PapyrusUtil.SetActorAddon(akActor, aiAddon)
  If liRes >= 0
    If !akActor.IsOnMount()
      akActor.QueueNiNodeUpdate()
    EndIf
  Else
    If akActor == PlayerRef 
      ShowNotification("$TNG_WPT")
    EndIf
    TNGSetAddon(akActor, -2)    
    HandleWarnings(liRes)
  EndIf
  Return liRes
EndFunction

Function ShowNotification(String asNotif, Bool abForce = False)
  If Notifs || abForce
    Debug.Notification(asNotif)
  EndIf
EndFunction