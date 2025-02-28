ScriptName TNG_Reload extends ActiveMagicEffect

Event OnEffectStart(Actor akTarget, Actor akCaster)
  RegisterForSingleUpdate(5)
EndEvent

Event OnCellLoad()
  RegisterForSingleUpdate(3)
EndEvent

Event OnCellAttach()
  CheckNPCs()
EndEvent

Event OnUpdate()
  CheckNPCs()
EndEvent

Function CheckNPCs()
  Actor[] lKActors = TNG_PapyrusUtil.CheckActors()
  Int liCount = lKActors.Length
  While liCount
    liCount -= 1
    Actor lkActor = lKActors[liCount]
    If !lkActor.IsOnMount()
      lkActor.QueueNiNodeUpdate()
    EndIf
  EndWhile
EndFunction
