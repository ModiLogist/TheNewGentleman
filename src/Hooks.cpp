#include "Hooks.h"

namespace Hooks {
    void Install() noexcept {
        

        stl::write_vfunc<RE::Character, Load3D>();
        logger::info("Installed Character::Load3D hook");

        rcMkn = RE::TESForm::LookupByID(rcMknID)->As<RE::TESRace>();
        kwNPC = RE::TESForm::LookupByID(kwNPCID)->As<RE::BGSKeyword>();
        kwSKR = RE::TESForm::LookupByID(kwSKRID)->As<RE::BGSKeyword>();
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
        if (l_skin) {
            if (!l_skin->HasPartOf(slotGenital)) {
                l_skin->AddSlotToMask(slotGenital);
                if (l_race->HasKeyword(kwSKR)) {          
                    for (const auto l_genitalID : beast_genitals) {
                        logger::info("Looking for the genital {} ", l_genitalID);     
                        RE::TESForm* l_genitalForm = RE::TESForm::LookupByEditorID(l_genitalID);
                        if (!l_genitalForm) continue;
                        logger::info("Got genital {} ", l_genitalForm->GetFormID());   
                        RE::TESObjectARMA* l_genital = RE::TESForm::LookupByEditorID(l_genitalID)->As<RE::TESObjectARMA>();                        
                        l_skin->armorAddons.push_back(l_genital);
                        logger::info("Added a genital to race {}", l_race->GetName());
                    }
                }
                else {              
                    for (const auto l_genitalID : manmer_genitals) {
                        logger::info("Looking for the genital {} ", l_genitalID);
                        RE::TESForm* l_genitalForm = RE::TESForm::LookupByEditorID(l_genitalID);
                        if (!l_genitalForm) continue;
                        logger::info("Got genital {} ", l_genitalForm->GetFormID());
                        RE::TESObjectARMA* l_genital = RE::TESForm::LookupByEditorID(l_genitalID)->As<RE::TESObjectARMA>();
                        logger::info("Got genital {} ", l_genital->GetFormID());   
                        l_skin->armorAddons.push_back(l_genital);
                        logger::info("Added a genital to race {}", l_race->GetName());
                    }
                }
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