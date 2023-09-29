#include <TngUtil.h>

namespace TngUtil {
    void Initialize() noexcept {
        gLogger::info("Initializing TheNewGentleman!");
        fDataHandler = RE::TESDataHandler::GetSingleton();
        if (!fDataHandler->LookupModByName(fTNGName)) {
            gLogger::error("Mod {} was not found!", fTNGName);
            return;
        }
        fManakinRace = RE::TESForm::LookupByID(fMnkRaceID)->As<RE::TESRace>();
        fNPCKey = RE::TESForm::LookupByID(fNPCKeywID)->As<RE::BGSKeyword>();
        fBeastKey = RE::TESForm::LookupByID(fBstKeywID)->As<RE::BGSKeyword>();
    }

    void ObtainGenitas() noexcept {
        gLogger::info("{} is now obtaining the genitals.", TngUtil::fTNGName);
        for (const auto lGenitalID : fManMerGenitalIDs) {
            const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(lGenitalID, fTNGName);
            if (!lGenital) {
                gLogger::error("Original genital [{}] cannot be found!", lGenitalID);
                return;
            }
            fManMerGenitals.emplace_back(lGenital);
            fAllRaces.emplace_back(lGenital->race);
            for (const auto lAddRace : lGenital->additionalRaces) {
                fAllRaces.emplace_back(lAddRace);
            }
        }

        for (const auto lGenitalID : fBeastGenitalIDs) {
            const auto lGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(lGenitalID, fTNGName);
            if (!lGenital) {
                gLogger::error("Original genital [{}] cannot be found!", lGenitalID);
                return;
            }
            fBeastGenitals.emplace_back(lGenital);
            fAllRaces.emplace_back(lGenital->race);
            for (const auto lAddRace : lGenital->additionalRaces) {
                fAllRaces.emplace_back(lAddRace);
            }
        }

        const auto lDefGenital = fDataHandler->LookupForm<RE::TESObjectARMA>(fDefGenitalID, fTNGName);
        if (!lDefGenital) {
            gLogger::error("Default genital [{}] cannot be found!", fDefGenitalID);
            return;
        }
    }

    void GenitalizeRaces() noexcept {
        gLogger::info("There are {} races with assigned genital. Starting to add genital to them.", fAllRaces.size());
        int lGenitalized = 0;
        for (const auto lRace : fAllRaces) {
            const auto lSkin = lRace->skin;
            if (fHandledSkins.find(lSkin) != fHandledSkins.end()) {
                lGenitalized++;
                continue;
            };
            if (!lSkin->HasPartOf(fSlotGenital)) {
                lSkin->AddSlotToMask(fSlotGenital);
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
            };
            gLogger::info("Genital added to the body of {} out of {}.", lGenitalized, fAllRaces.size());
        }
    }

}
