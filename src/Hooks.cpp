#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Hooks.h>

void Hooks::Install() {
  //
  InstallHook<Load3D>();
}

RE::NiAVObject* Hooks::Load3D::thunk(RE::Character* actor, bool backgroundLoading) {
  auto res = Load3D::func(actor, backgroundLoading);
  auto npc = actor ? actor->GetActorBase() : nullptr;
  auto canModify = core->CanModifyNPC(npc);
  if (canModify == Tng::resOkRacePP) return res;
  if (canModify < 0) return res;
  if (npc->HasKeyword(Tng::Key(Tng::kyProcessed))) return res;
  auto curSkin = actor->GetSkin();
  events->DoChecks(actor);
  if (actor->GetSkin() != curSkin) {
    SKSE::log::debug("\tHook fixed addon for actor [0x{:x}:{}].", actor->GetFormID(), npc->GetName());
  }
  return res;
}
