#pragma once

namespace Hooks {
    void Install() noexcept;

    namespace {
        inline constexpr RE::FormID rcMknID{0x10760A};
        inline constexpr RE::FormID kwNPCID{0x13794};
        inline constexpr RE::FormID kwSKRID{0xD61D1};
        inline constexpr RE::BGSBipedObjectForm::BipedObjectSlot slotBody{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
        inline constexpr RE::BGSBipedObjectForm::BipedObjectSlot slotGenital{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

        inline RE::TESRace* rcMkn{nullptr};
        inline RE::BGSKeyword* kwNPC{nullptr};
        inline RE::BGSKeyword* kwSKR{nullptr};

        static const std::string_view manmer_genitals[13] = {"TNG_GenitalNord", "TNG_GenitalRedguard", "TNG_GenitalBreton",  "TNG_GenitalImperial", "TNG_GenitalAltmer",
                                                    "TNG_GenitalBosmer", "TNG_GenitalDunmer ", "TNG_GenitalOrsimer", "TNG_GenitalDremora",  "TNG_GenitalElder",
                                                    "TNG_GenitalAfflicted", "TNG_GenitalSnowElf",  "TNG_GenitalDefault"};
        static const std::string_view beast_genitals[2] = {"TNG_GenitalSaxhleel", "TNG_GenitalKhajiit"};
    }

    class Load3D : public Singleton<Load3D> {
    public:
        static RE::NiAVObject* Thunk(RE::Character* a_this, bool a_arg1) noexcept;
        inline static REL::Relocation<decltype(Thunk)> func;
        static inline constexpr std::size_t index{0};
        static inline constexpr std::size_t size{0x6A};
    };
}