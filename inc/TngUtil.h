#pragma once

namespace TngUtil {
    void Initialize() noexcept;
    void ObtainGenitas() noexcept;
    void GenitalizeRaces() noexcept;

    namespace {
        // SKSE stuff
        inline RE::TESDataHandler* fDataHandler{nullptr};
        // Skyrim fixed stuff
        inline constexpr RE::FormID fMnkRaceID{0x10760A};
        inline constexpr RE::FormID fNPCKeywID{0x13794};
        inline constexpr RE::FormID fBstKeywID{0xD61D1};
        inline constexpr RE::BGSBipedObjectForm::BipedObjectSlot fSlotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
        inline constexpr RE::BGSBipedObjectForm::BipedObjectSlot fSlotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};
        // Skyrim stuff needing load
        inline RE::TESRace* fManakinRace{nullptr};
        inline RE::BGSKeyword* fNPCKey{nullptr};
        inline RE::BGSKeyword* fBeastKey{nullptr};
        // TNG stuff
        inline constexpr RE::FormID fManMerGenitalIDs[12] = {
            0x800,  //"TNG_GenitalNord"
            0x801,  //"TNG_GenitalRedguard"
            0x802,  //"TNG_GenitalBreton"
            0x803,  //"TNG_GenitalImperial"
            0x804,  //"TNG_GenitalAltmer"
            0x805,  //"TNG_GenitalBosmer"
            0x806,  //"TNG_GenitalDunmer"
            0x807,  //"TNG_GenitalOrsimer"
            0x80A,  //"TNG_GenitalDremora"
            0x80B,  //"TNG_GenitalElder"
            0x80C,  //"TNG_GenitalAfflicted"
            0x80D,  //"TNG_GenitalSnowElf"
        };
        inline constexpr RE::FormID fBeastGenitalIDs[2] = {
            0x808,  //"TNG_GenitalSaxhleel"
            0x809,  //"TNG_GenitalKhajiit"
        };
        inline constexpr RE::FormID fDefGenitalID = 0x8ff;  //"TNG_GenitalDefault"

        // Needs loading stuff

        inline std::vector<RE::TESObjectARMA*> fManMerGenitals;
        inline std::vector<RE::TESObjectARMA*> fBeastGenitals;
        inline std::set<RE::TESObjectARMO*> fHandledSkins;
        inline std::vector<RE::TESRace*> fAllRaces;
        inline RE::TESObjectARMA* fDefGenital{nullptr};

    }

    inline constexpr std::string_view fTNGName = "TheNewGentleman.esp";

}