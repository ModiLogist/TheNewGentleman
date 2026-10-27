#include <Core.h>
#include <Hooks.h>

void Hooks::Install() { InstallHook<Load3D>(); }

RE::NiAVObject* Hooks::Load3D::thunk(Target* actor, bool backgroundLoading) {
  auto res = Load3D::func(actor, backgroundLoading);
  core->UpdateActorOnMain(actor);  // Load3D may run during background loading
  return res;
}
