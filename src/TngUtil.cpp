#include <TngUtil.h>

namespace TngUtil {
    void Initialize() noexcept {
        gLogger::info("Initializing TheNewGentleman!");
        fDataHandler = RE::TESDataHandler::GetSingleton();
        if (!fDataHandler->LookupModByName(cTNGName)) {
            gLogger::error("Mod {} was not found!", cTNGName);
            return;
        }
        fBeastKey = RE::TESForm::LookupByID(cBstKeywID)->As<RE::BGSKeyword>();
        fRevealingKey = fDataHandler->LookupForm<RE::BGSKeyword>(cRevealingKeyID, cTNGName);
        fUnderwearKey = fDataHandler->LookupForm<RE::BGSKeyword>(cUnderwearKeyID, cTNGName);
        if (!(fRevealingKey && fUnderwearKey)) {
            gLogger::error("The original TNG keywords could not be found!");
        }
        fAllRaces.insert(RE::TESForm::LookupByID(cDefRaceID)->As<RE::TESRace>());
        fAstridRace = RE::TESForm::LookupByID(cAstRaceID)->As<RE::TESRace>();
        fManakinRace = RE::TESForm::LookupByID(cMnkRaceID)->As<RE::TESRace>();
        fTestRace = fDataHandler->LookupForm<RE::TESRace>(cTstRaceID, "Dawnguard.esm");
        fNPCKey = RE::TESForm::LookupByID(cNPCKeywID)->As<RE::BGSKeyword>();
        fCreatureKey = RE::TESForm::LookupByID(cCrtKeywID)->As<RE::BGSKeyword>();
        auto& lAllRacesArray = fDataHandler->GetFormArray<RE::TESRace>();
        for (const auto lRace : lAllRacesArray) {
            if (lRace->HasKeyword(fNPCKey) && !lRace->HasKeyword(fCreatureKey) && lRace->HasPartOf(cSlotBody) && !lRace->IsChildRace() &&
                !(lRace == fManakinRace) && !(lRace == fAstridRace) && !(lRace == fTestRace)) {
                fPotentialRaces.insert(lRace);
            }
        }
    }

    void ObtainGenitas() noexcept {
        gLogger::info("{} is now obtaining the genitals.", cTNGName);
        for (const auto lGenitalID : cManMerGenitalIDs) {
            const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(lGenitalID, cTNGName);
            if (!lGenital) {
                gLogger::error("Original genital [{:x}] cannot be found!", lGenitalID);
                return;
            }
            fManMerGenitals.emplace_back(lGenital);
            fAllRaces.insert(lGenital->race);
            if (fPotentialRaces.find(lGenital->race) != fPotentialRaces.end()) {
                fPotentialRaces.erase(lGenital->race);
            }
            for (const auto lAddRace : lGenital->additionalRaces) {
                fAllRaces.insert(lAddRace);
                if (fPotentialRaces.find(lAddRace) != fPotentialRaces.end()) {
                    fPotentialRaces.erase(lAddRace);
                }
            }
        }

        for (const auto lGenitalID : cBeastGenitalIDs) {
            const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(lGenitalID, cTNGName);
            if (!lGenital) {
                gLogger::error("Original genital [{:x}] cannot be found!", lGenitalID);
                return;
            }
            fBeastGenitals.emplace_back(lGenital);
            fAllRaces.insert(lGenital->race);
            if (fPotentialRaces.find(lGenital->race) != fPotentialRaces.end()) {
                fPotentialRaces.erase(lGenital->race);
            }
            for (const auto lAddRace : lGenital->additionalRaces) {
                fAllRaces.insert(lAddRace);
                if (fPotentialRaces.find(lAddRace) != fPotentialRaces.end()) {
                    fPotentialRaces.erase(lAddRace);
                }
            }
        }
        fDefSaxGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalSaxID, cTNGName);
        fDefKhaGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalKhaID, cTNGName);
        fDefMnmGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalMnmID, cTNGName);
        if (!fDefMnmGenital || !fDefKhaGenital || !fDefSaxGenital) {
            gLogger::error("The original TNG Default-genitals cannot be found!");
            return;
        }
        fManMerGenitals.emplace_back(fDefMnmGenital);
        fBeastGenitals.emplace_back(fDefSaxGenital);
        fBeastGenitals.emplace_back(fDefKhaGenital);
        for (const auto lRace : fPotentialRaces) {
            if (lRace->HasPartOf(cSlotGenital)) {
                fPotentialRaces.erase(lRace);
                gLogger::info("The race {} seems to be ready for TNG. It won't be modified.", lRace->GetFormEditorID());
            } else {
                if (lRace->HasKeyword(fBeastKey)) {
                    const auto lRaceName = std::string(lRace->GetFormEditorID()) + std::string(lRace->GetName());
                    if ((lRaceName.find("Khajiit") != std::string::npos) || (lRaceName.find("Rhat") != std::string::npos)) {
                        fDefKhaGenital->additionalRaces.emplace_back(lRace);
                        gLogger::info("The race [0x{:x}]{} from file {} was recognized as Khajiit. If this is wrong, a patch is required.", lRace->GetFormID(),
                                      lRace->GetFormEditorID(), lRace->GetFile()->GetFilename());
                    } else {
                        if ((lRaceName.find("Argonian") != std::string::npos) || (lRaceName.find("Saxhleel") != std::string::npos)) {
                            fDefSaxGenital->additionalRaces.emplace_back(lRace);
                            gLogger::info("The race [0x{:x}]{} from file {} was recognized as Saxhleel(Argonian). If this is wrong, a patch is required.",
                                          lRace->GetFormID(), lRace->GetFormEditorID(), lRace->GetFile()->GetFilename());
                        } else {
                            gLogger::warn("The race [0x{:x}]{} from file {} did not receive any genital. If they should, a patch is required.",
                                          lRace->GetFormID(), lRace->GetFormEditorID(), lRace->GetFile()->GetFilename());
                        }
                    }

                } else {
                    fDefMnmGenital->additionalRaces.emplace_back(lRace);
                }
            }
        }
    }

    void GenitalizeRaces() noexcept {
        gLogger::info(
            "Checking races: There are {} races with assigned genital and {} recognized races without assigned genital(custom races). Starting to add genital "
            "to them.",
            fAllRaces.size(), fPotentialRaces.size());
        int lGenitalized = 0;
        for (const auto lRace : fAllRaces) {
            lRace->AddSlotToMask(cSlotGenital);
            const auto lSkin = lRace->skin;
            if (fHandledSkins.find(lSkin) != fHandledSkins.end()) {
                lGenitalized++;
                continue;
            };
            if (!lSkin->HasPartOf(cSlotGenital)) {
                lSkin->AddSlotToMask(cSlotGenital);
                if (lRace->HasKeyword(fBeastKey)) {
                    for (const auto lGenital : fBeastGenitals) {
                        lSkin->armorAddons.push_back(lGenital);
                    }
                    lGenitalized++;
                } else {
                    for (const auto lGenital : fManMerGenitals) {
                        lSkin->armorAddons.push_back(lGenital);
                    }
                    lGenitalized++;
                }
                for (const auto lSkinAA : lSkin->armorAddons) {
                    fSkinAAs.insert(lSkinAA);
                }
                fHandledSkins.insert(lSkin);
            } else {
                gLogger::info("The skin [{}] belonging to race {} already has something on slot 52!", lSkin->GetFormID(), lRace->GetFormEditorID());
            }
        }
        lGenitalized += HandleCustomRaces(fDefMnmGenital);
        lGenitalized += HandleCustomRaces(fDefKhaGenital);
        lGenitalized += HandleCustomRaces(fDefSaxGenital);
        gLogger::info("Genital added to the body of {} out of {}.", lGenitalized, fAllRaces.size() + fPotentialRaces.size());
        gLogger::info("Custom races:");
        for (const auto lRace : fPotentialRaces) {
            gLogger::info("\t{}", lRace->GetFormEditorID());
        }
    }

    void GenitalizeSkins() noexcept {
        auto& lAllNPCs = fDataHandler->GetFormArray<RE::TESNPC>();
        gLogger::info("Checking NPCs for custom skins: There are {} NPCs.", lAllNPCs.size());
        int lIrr = lAllNPCs.size();
        int lHdr = lAllNPCs.size();
        int lNob = lAllNPCs.size();
        int lC = 0;
        for (const auto lNPC : lAllNPCs) {
            const auto lSkin = lNPC->skin;
            if (!lSkin) continue;
            const auto lNPCRace = lNPC->race;
            if (!lNPCRace->HasKeyword(fNPCKey) || lNPCRace->HasKeyword(fCreatureKey) || lNPCRace->IsChildRace() || (lNPCRace == fManakinRace) ||
                (lNPCRace == fAstridRace) || (lNPCRace == fTestRace))
                continue;
            lIrr--;
            if (fHandledSkins.find(lSkin) != fHandledSkins.end()) continue;
            lHdr--;
            if (!lSkin->HasPartOf(cSlotBody)) continue;
            if (lSkin->HasPartOf(cSlotGenital)) {
                gLogger::info("The skin [{}] used in NPC {} from file {} cannot have a male genital. If this is wrong, a patch is required.",
                              lSkin->GetFormID(), lNPC->GetName(), lNPC->GetFile()->GetFilename());
                continue;
            }
            lNob--;
            if (lNPC->HasKeyword(fBeastKey)) {
                for (const auto lGenital : fBeastGenitals) lSkin->armorAddons.push_back(lGenital);

            } else {
                for (const auto lGenital : fManMerGenitals) lSkin->armorAddons.push_back(lGenital);
            }
            for (const auto lSkinAA : lSkin->armorAddons) fSkinAAs.insert(lSkinAA);
            fHandledSkins.insert(lSkin);
            lC++;
        }
        gLogger::info("Out of the {} NPCs:", lAllNPCs.size());
        gLogger::info("\t{} NPCs are not adult humanoids,", lIrr);
        gLogger::info("\t{} NPCs' skins are already handled with races,", lHdr - lIrr);
        gLogger::info("\t{} NPCs's skins don't have body slot or already have genital slot.", lNob - (lIrr + lHdr));
        gLogger::info("\t{} handled {} custom skins.", cTNGName, lC);
    }

    void MakeArmorCovering() noexcept {
        auto& lAllArmorArray = fDataHandler->GetFormArray<RE::TESObjectARMO>();
        int lRCount = 0;
        int lCCount = 0;
        int lQCount = 0;
        for (const auto lArmor : lAllArmorArray) {
            if (fHandledSkins.find(lArmor) != fHandledSkins.end()) continue;
            if (fAllRaces.find(lArmor->race) == fAllRaces.end()) continue;

            if (lArmor->HasPartOf(cSlotGenital) && !lArmor->HasKeyword(fUnderwearKey)) {
                const auto lID = (std::string(lArmor->GetName()).empty()) ? lArmor->GetFormEditorID() : lArmor->GetName();
                gLogger::warn("The armor [{:x}]{} from file {} has a conflict with {}. If it is an underwear add {} to it.", lArmor->GetFormID(),
                              lID, lArmor->GetFile()->GetFilename(), cTNGName, fUnderwearKey->GetFormEditorID());
            }
            if (lArmor->HasPartOf(cSlotBody)) {
                if (lArmor->HasKeyword(fRevealingKey)) {
                    lRCount++;
                    continue;
                }
                bool lCoversGenital = false;
                bool lShouldReveal = false;
                for (const auto lAA : lArmor->armorAddons) {
                    if (fSkinAAs.find(lAA) != fSkinAAs.end()) {
                        lShouldReveal = true;
                        break;
                    }
                    if (lAA->HasPartOf(cSlotGenital)) {
                        if (fHandledArma.find(lAA) != fHandledArma.end()) {
                            lCCount++;
                        } else {
                            lQCount++;
                        }
                        lCoversGenital = true;
                        break;
                    }
                }
                if (lShouldReveal) {
                    lArmor->AddKeyword(fRevealingKey);
                    gLogger::warn("The armor 0x{:x}:{} from file {} is markerd revealing. If this is a mistake, a patch is required!", lArmor->GetFormID(),
                                  lArmor->GetFormEditorID(), lArmor->GetFile()->GetFilename());
                    lRCount++;
                    continue;
                }
                if (!lCoversGenital) {
                    lArmor->armorAddons[0]->AddSlotToMask(cSlotGenital);
                    fHandledArma.insert(lArmor->armorAddons[0]);
                    lCCount++;
                }
            }
        }
        gLogger::info("Processed [{}] body armor pieces:", lCCount + lRCount + lQCount);
        gLogger::info("\t[{}]: already covering genitals", lQCount);
        gLogger::info("\t[{}]: revealing", lRCount);
        gLogger::info("\t[{}]: updated to cover genitals", lCCount);
    }    

    int HandleCustomRaces(RE::TESObjectARMA* aGenital) {
        int res = 0;
        for (const auto lRace : aGenital->additionalRaces) {
            lRace->AddSlotToMask(cSlotGenital);
            const auto lSkin = lRace->skin;
            if (fHandledSkins.find(lSkin) != fHandledSkins.end()) {
                res++;
                continue;
            };
            if (!lSkin->HasPartOf(cSlotGenital)) {
                lSkin->AddSlotToMask(cSlotGenital);
                lSkin->armorAddons.push_back(aGenital);
                res++;
                for (const auto lSkinAA : lSkin->armorAddons) {
                    fSkinAAs.insert(lSkinAA);
                }
                fHandledSkins.insert(lSkin);
            } else {
                gLogger::info("The skin [{}] belonging to race {} already has something on slot 52!", lSkin->GetFormID(), lRace->GetFormEditorID());
            }
        }
    }

}
