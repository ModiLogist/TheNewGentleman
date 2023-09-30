#include <TngUtil.h>

namespace TngUtil {
    void Initialize() noexcept {
        gLogger::info("Initializing TheNewGentleman!");
        fDataHandler = RE::TESDataHandler::GetSingleton();
        if (!fDataHandler->LookupModByName(cTNGName)) {
            gLogger::error("Mod {} was not found!", cTNGName);
            return;
        }
        fDefRace = RE::TESForm::LookupByID(cDefRaceID)->As<RE::TESRace>();
        fAllRaces.insert(fDefRace);
        fManakinRace = RE::TESForm::LookupByID(cMnkRaceID)->As<RE::TESRace>();
        fNPCKey = RE::TESForm::LookupByID(cNPCKeywID)->As<RE::BGSKeyword>();
        fBeastKey = RE::TESForm::LookupByID(cBstKeywID)->As<RE::BGSKeyword>();
        fRevealingKey = fDataHandler->LookupForm<RE::BGSKeyword>(cRevealingKeyID, cTNGName);
        fUnderwearKey = fDataHandler->LookupForm<RE::BGSKeyword>(cUnderwearKeyID, cTNGName);
        if (!(fRevealingKey && fUnderwearKey)) {
            gLogger::error("The original TNG keywords could not be found!");
        }
    }

    void ObtainGenitas() noexcept {
        gLogger::info("{} is now obtaining the genitals.", TngUtil::cTNGName);
        for (const auto lGenitalID : cManMerGenitalIDs) {
            const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(lGenitalID, cTNGName);
            if (!lGenital) {
                gLogger::error("Original genital [{}] cannot be found!", lGenitalID);
                return;
            }
            fManMerGenitals.emplace_back(lGenital);
            fAllRaces.insert(lGenital->race);
            for (const auto lAddRace : lGenital->additionalRaces) {
                fAllRaces.insert(lAddRace);
            }
        }

        for (const auto lGenitalID : cBeastGenitalIDs) {
            const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(lGenitalID, cTNGName);
            if (!lGenital) {
                gLogger::error("Original genital [{}] cannot be found!", lGenitalID);
                return;
            }
            fBeastGenitals.emplace_back(lGenital);
            fAllRaces.insert(lGenital->race);
            for (const auto lAddRace : lGenital->additionalRaces) {
                fAllRaces.insert(lAddRace);
            }
        }

        const auto lDefGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(cDefGenitalID, cTNGName);
        if (!lDefGenital) {
            gLogger::error("Default genital [{}] cannot be found!", cDefGenitalID);
            return;
        }
    }

    void GenitalizeRaces() noexcept {
        gLogger::info("There are {} races with assigned genital. Starting to add genital to them.", fAllRaces.size());
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
                fHandledSkins.insert(lSkin);
            } else {
                gLogger::info("The skin {} belonging to race {} already has something on slot 52!", lSkin->GetFormEditorID(), lRace->GetName());
            }
        }
        gLogger::info("Genital added to the body of {} out of {}.", lGenitalized, fAllRaces.size());
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
                gLogger::warn("The armor {} from file {} has a conflict with {}. If it is an underwear add {} to it.", lArmor->GetName(), lArmor->GetFile()->GetFilename(), cTNGName,
                              fUnderwearKey->GetFormEditorID());
            }
            if (lArmor->HasPartOf(cSlotBody)) {
                if (lArmor->HasKeyword(fRevealingKey)) {
                    lRCount++;
                    continue;
                }
                bool lCoversGenital = false;
                for (const auto lAA : lArmor->armorAddons) {
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

}
