#include "Hooks.h"

namespace Hooks {
    namespace
    {
        inline constexpr RE::FormID rcMknID{0x10760A};
        inline constexpr RE::FormID kwNPCID{0x13794};
        inline constexpr RE::BGSBipedObjectForm::BipedObjectSlot slotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
        inline constexpr RE::BGSBipedObjectForm::BipedObjectSlot slotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

        inline RE::TESRace* rcMkn{nullptr};
        inline RE::BGSKeyword* kwNPC{nullptr};
    }
    void Install() noexcept {
        stl::write_vfunc<RE::Character, AddWornItem>();
        logger::info("Installed Character::AddWornItem hook");

        stl::write_vfunc<RE::Character, Load3D>();
        logger::info("Installed Character::Load3D hook");

        rcMkn = RE::TESForm::LookupByID(rcMknID)->As<RE::TESRace>();
        logger::info("Manakin race recognized: {}", rcMkn->GetFormEditorID());
        kwNPC = RE::TESForm::LookupByID(kwNPCID)->As<RE::BGSKeyword>();
        logger::info("Actor type NPC keyword recognized: {}", kwNPC->GetFormEditorID());
    }

    bool AddWornItem::Thunk(RE::Character* a_this, RE::TESBoundObject* a_item, std::int32_t a_count, bool a_forceEquip, std::uint32_t a_arg4, std::uint32_t a_arg5) noexcept {
        const auto result{func(a_this, a_item, a_count, a_forceEquip, a_arg4, a_arg5)};
        return result;
    }

    RE::NiAVObject* Load3D::Thunk(RE::Character* a_this, bool a_arg1) noexcept {
        const auto result{func(a_this, a_arg1)};
        const auto l_npc = a_this->GetActorBase();
        if (!l_npc) return result;
        if (l_npc->GetSex() != 0) return result;
        const RE::TESRace* l_race = l_npc->GetRace();
        if (a_this->IsChild() || !a_this->HasKeyword(kwNPC) || (l_race == rcMkn)) {
            return result;
        };
        RE::TESObjectARMO* l_skin = l_race->skin;
        if (l_skin)
        {
            if (!l_skin->HasPartOf(slotGenital))
            {
                l_skin->AddSlotToMask(slotGenital);
                RE::TESObjectARMA* l_genital = RE::TESForm::LookupByID(0x166d5)->As<RE::TESObjectARMA>();
                l_skin->armorAddons.push_back(l_genital);
                logger::info("Added a genital to race {}", l_race->GetName());
            }
        }

        const RE::TESObjectARMO* armoBody{a_this->GetWornArmor(slotBody)};
        if (!armoBody) return result;
        const RE::BSTArray<RE::TESObjectARMA*> armaBody{armoBody->armorAddons};
        for (const auto armaSlot : armaBody) {
            if (armaSlot->HasPartOf(slotGenital)) {
                return result;
            }
        }
        armaBody[0]->AddSlotToMask(slotGenital);
        logger::debug("Added slot 52 to {} (0x{:x})", armoBody->GetName(), armoBody->GetFormID());
        return result;
    }

}