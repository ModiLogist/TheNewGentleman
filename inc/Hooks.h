#pragma once

class Hooks {
  public:
    static void Install();

  private:
    struct Load3D {
        using Target = RE::Character;
        inline static constexpr size_t tableIdx{0};
        static constexpr std::string_view name{"Load3D"};
        inline static constexpr size_t index{0x6a};

        static RE::NiAVObject* thunk(Target* actor, bool backgroundLoading);

        inline static REL::Relocation<decltype(thunk)> func;
    };

    template <class T>
    static constexpr auto InstallHook() {
      REL::Relocation<std::uintptr_t> vt{T::Target::VTABLE[T::tableIdx]};
      T::func = vt.write_vfunc(T::index, T::thunk);
      SKSE::log::info("Installed {} hook.", T::name);
    }
};
