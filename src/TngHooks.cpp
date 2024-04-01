#include "TngHooks.h"

#include "TngCoreBase.h"
#include "dav/TESObjectARMA.h"

void TngHooks::Install() {
  auto lHook = MakeHook(DAV::TESNPC::InitWornFormOffset, 0x2F0);
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

void TngHooks::InitWornArmor(RE::TESObjectARMO* aArmor, RE::Actor* aActor, RE::BSTSmartPointer<RE::BipedAnim>* aBiped) {
  auto race = aActor->GetRace();
  auto sex = aActor->GetActorBase()->GetSex();
  for (auto& armorAddon : aArmor->armorAddons) {
    if (DAV::TESObjectARMA::HasRace(armorAddon, race)) {
      auto visitor = std::bind(DAV::TESObjectARMA::InitWornArmorAddon, std::placeholders::_1, aArmor, aBiped, sex);
      TngCoreBase::VisitArmorAddons(aActor, armorAddon, visitor);
    }
  }
}
