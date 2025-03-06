#pragma once

class Hooks {
  public:
    static void Install();

  private:
    struct Load3D {
        static constexpr auto Target = RE::VTABLE_Character[0];
        inline static constexpr size_t index{0x6A};

        static RE::NiAVObject* thunk(RE::Character* actor, bool backgroundLoading);

        inline static REL::Relocation<decltype(thunk)> func;

        static constexpr std::string_view name{"Load3D"};
    };

    template <class T>
    static constexpr auto InstallHook() {
      REL::Relocation<std::uintptr_t> vt{T::Target};
      T::func = vt.write_vfunc(T::index, T::thunk);
      SKSE::log::info("Installed {} hook.", T::name);
    }
};
