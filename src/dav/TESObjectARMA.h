#pragma once

namespace DAV {
  namespace TESObjectARMA {
    inline constexpr auto HasRaceOffset = MAKE_OFFSET(17757, 17359, 0x2380A0, 0x235A00);
    inline constexpr auto InitWornArmorAddonOffset = MAKE_OFFSET(17759, 17361, 0x2383A0, 0x235D10);
    bool HasRace(RE::TESObjectARMA* a_armorAddon, RE::TESRace* a_race);

    void InitWornArmorAddon(RE::TESObjectARMA* a_armorAddon, RE::TESObjectARMO* a_armor, RE::BSTSmartPointer<RE::BipedAnim>* a_biped, RE::SEX a_sex);
  }
  namespace TESNPC {
    inline constexpr auto InitWornFormOffset = MAKE_OFFSET(24736, 24232, 0x373CB0, 0x37B430);
  }
}
