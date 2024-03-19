#include "dav/TESObjectARMA.h"
#include "TngHooks.h"
#include "TngCoreBase.h"



void TngHooks::Install() {
  auto lHook = MakeHook(DAV::TESNPC::InitWornFormOffset, 0x2F0);

  // Expected size: 0x12
  struct Patch : public Xbyak::CodeGenerator {
      Patch(std::uintptr_t a_funcAddr) {
        mov(rdx, r13);
        mov(rcx, rbp);
        mov(rax, a_funcAddr);
        call(rax);
      }
  };

  Patch patch{reinterpret_cast<std::uintptr_t>(&InitWornArmor)};
  patch.ready();

  if (patch.getSize() > 0x17) {
    SKSE::stl::report_and_fail("Patch was too large, failed to install"sv);
  }

  REL::safe_fill(lHook.address(), REL::NOP, 0x17);
  REL::safe_write(lHook.address(), patch.getCode(), patch.getSize());
}

void TngHooks::InitWornArmor(RE::TESObjectARMO* a_armor, RE::Actor* a_actor, RE::BSTSmartPointer<RE::BipedAnim>* a_biped) {
  auto race = a_actor->GetRace();
  auto sex = a_actor->GetActorBase()->GetSex();

  for (auto& armorAddon : a_armor->armorAddons) {
    if (DAV::TESObjectARMA::HasRace(armorAddon, race)) {
      auto visitor = std::bind(DAV::TESObjectARMA::InitWornArmorAddon, std::placeholders::_1, a_armor, a_biped, sex);
      TngCoreBase::VisitArmorAddons(a_actor, armorAddon, visitor);
    }
  }
}
