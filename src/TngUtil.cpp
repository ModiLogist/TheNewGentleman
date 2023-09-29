#include <TngUtil.h>

namespace TngUtil {
    void Initialize() noexcept { 
        fHandler = RE::TESDataHandler::GetSingleton();
        if (!fHandler->LookupModByName(fTNGName)) {
            gLogger::error("Mod {} was not found!", fTNGName);
            return;
        }
        fManakinRace = RE::TESForm::LookupByID(fMnkRaceID)->As<RE::TESRace>();
        gLogger::info("Found {} successfully!", fManakinRace->GetFormEditorID());
        fNPCKey = RE::TESForm::LookupByID(fNPCKeywID)->As<RE::BGSKeyword>();
        gLogger::info("Found {} successfully!", fNPCKey->GetFormEditorID());
        fBeastKey = RE::TESForm::LookupByID(fBstKeywID)->As<RE::BGSKeyword>();
        gLogger::info("Found {} successfully!", fBeastKey->GetFormEditorID());
    }

    void ObtainGenitas() noexcept {        
        for (const auto lGenitalID : fManMerGenitalIDs) {
            const auto lGenital = fHandler->LookupForm<RE::TESObjectARMA>(lGenitalID, fTNGName);
            if (!lGenital) {
                gLogger::error("Original genital [{}] cannot be found!", lGenitalID);
                return;
            }
            fManMerGenitals.emplace_back(lGenital);
            const auto lRace = lGenital->race;
            gLogger::info("The genital for {} was loaded successfully.", lRace->GetFormEditorID());
            fAllRaces.emplace_back(lRace);
            for (const auto lAddRace : lGenital->additionalRaces) {
                gLogger::info("The genital for {} was loaded successfully.", lAddRace->GetFormEditorID());
                fAllRaces.emplace_back(lAddRace);
            }
        }

        for (const auto lGenitalID : fBeastGenitalIDs) {
            const auto lGenital = fHandler->LookupForm<RE::TESObjectARMA>(lGenitalID, fTNGName);
            if (!lGenital) {
                gLogger::error("Original genital [{}] cannot be found!", lGenitalID);
                return;
            }
            lGenital.
            fBeastGenitals.emplace_back(lGenital);
            const auto lRace = lGenital->race;
            gLogger::info("The genital for {} was loaded successfully.", lRace->GetFormEditorID());
            fAllRaces.emplace_back(lRace);
            for (const auto lAddRace : lGenital->additionalRaces) {
                gLogger::info("The genital for {} was loaded successfully.", lAddRace->GetFormEditorID());
                fAllRaces.emplace_back(lAddRace);
            }
        }

        const auto lDefGenital = fHandler->LookupForm<RE::TESObjectARMA>(fDefGenitalID, fTNGName);
        if (!lDefGenital) {
            gLogger::error("Default genital [{}] cannot be found!", fDefGenitalID);
            return;
        }
        gLogger::info("The default genital for custom races was loaded successfully.");
    }

    void GenitalizeRaces() noexcept {
        gLogger::info("There are {} races with assigned genital. Starting to add genital to them.", fAllRaces.size());
        for (const auto lRace : fAllRaces) {
            const auto lSkin = lRace->skin;
            if (fHandledSkins.find(lSkin) != fHandledSkins.end()) {
                gLogger::info("The race {} uses the skin {} which is already handled.", lRace->GetName(), lSkin->GetFormEditorID());
                continue;
            };

            if (!lSkin->HasPartOf(fSlotGenital)) {
                lSkin->AddSlotToMask(fSlotGenital);
                if (lRace->HasKeyword(fBeastKey)) {
                    for (const auto lGenital : fBeastGenitals) {
                        lSkin->armorAddons.push_back(lGenital);
                    }
                    gLogger::info("Added [{}]genitals to the skin {} for the beast race {}", fBeastGenitals.size(), lSkin->GetFormEditorID(), lRace->GetName());
                } else {
                    for (const auto lGenital : fManMerGenitals) {
                        lSkin->armorAddons.push_back(lGenital);
                    }
                    gLogger::info("Added [{}]genitals to the skin {} for the man/mer race {}", fManMerGenitals.size(), lSkin->GetFormEditorID(), lRace->GetName());
                }
                fHandledSkins.insert(lSkin);
            } else {
                gLogger::info("The skin {} belonging to race {} already has something on slot 52!", lSkin->GetFormEditorID(), lRace->GetName());
            };
        }
    }

}
