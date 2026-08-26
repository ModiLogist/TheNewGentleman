#pragma once

namespace BaseUtil {
  constexpr std::string_view skyrimFile{"Skyrim.esm"};
  constexpr int def{-2};
  constexpr int nul{-1};
  constexpr int errInt{-99};
  constexpr float errFlt{-1.0f};
  constexpr const char* defStr{"Default"};
  constexpr const char* nulStr{"None"};
  constexpr char delim{'~'};

  // TODO: Clean up if it ends up not being used.
  //  struct FormComparator {
  //      bool operator()(const RE::TESForm* lhs, const RE::TESForm* rhs) { return lhs->formID < rhs->formID; }
  //  };

  template <typename T, typename U, size_t n>
  struct TypedSetting {
    private:
      const std::array<T, n> defValues;
      const std::array<std::string, n> sections;
      const std::array<std::string, n> keys;
      std::array<T, n> values;

      static_assert(std::is_enum_v<U>, "TypedSetting U must be an enum type");

    public:
      TypedSetting(std::array<T, n> def, std::array<std::string, n> s, std::array<std::string, n> k) : defValues(def), sections(s), keys(k) {}

      void Load(CSimpleIniA& ini) {
        for (size_t i = 0; i < n; ++i) {
          if constexpr (std::is_same_v<T, int>) {
            values[i] = ini.GetLongValue(sections[i].c_str(), keys[i].c_str(), defValues[i]);
          } else if constexpr (std::is_same_v<T, bool>) {
            values[i] = ini.GetBoolValue(sections[i].c_str(), keys[i].c_str(), defValues[i]);
          } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            values[i] = static_cast<float>(ini.GetDoubleValue(sections[i].c_str(), keys[i].c_str(), static_cast<double>(defValues[i])));
          } else if constexpr (std::is_same_v<T, std::string>) {
            values[i] = ini.GetValue(sections[i].c_str(), keys[i].c_str(), defValues[i]);
          } else {
            static_assert(false, "Unsupported type for LoadValue");
          }
          SKSE::log::debug("Loaded setting [{}] in [{}] to be [{}({})]", keys[i], sections[i], values[i], values[i] == defValues[i] ? "default" : "user");
        }
      }

      void Store(CSimpleIniA& ini) {
        for (size_t i = 0; i < n; ++i) {
          if (values[i] != defValues[i]) {
            if constexpr (std::is_same_v<T, int>) {
              ini.SetLongValue(sections[i].c_str(), keys[i].c_str(), values[i]);
            } else if constexpr (std::is_same_v<T, bool>) {
              ini.SetBoolValue(sections[i].c_str(), keys[i].c_str(), values[i]);
            } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
              ini.SetDoubleValue(sections[i].c_str(), keys[i].c_str(), static_cast<double>(values[i]));
            } else if constexpr (std::is_same_v<T, std::string>) {
              ini.SetValue(sections[i].c_str(), keys[i].c_str(), values[i].c_str());
            } else {
              static_assert(false, "Unsupported type for SaveValue");
            }
          } else {
            ini.Delete(sections[i].c_str(), keys[i].c_str(), true);
          }
        }
      }

      T Get(const U idx) const { return values[static_cast<size_t>(idx)]; }

      void Set(const U idx, const T value) {
        if (values[static_cast<size_t>(idx)] != value) values[static_cast<size_t>(idx)] = value;
      }
  };

  namespace {
    constexpr size_t fixedDelayTime{5000};
    constexpr size_t delayTime{500};
    constexpr size_t maxDelayCount{20};
    constexpr size_t newGameDelayMult{2};
    constexpr std::string_view iniChars{"[=:]"};
    constexpr std::string_view nullForm{"Null Form"};
    constexpr std::string_view unrecFile{"Unrecognized File"};
  }

  RE::TESDataHandler* SEDH() { return RE::TESDataHandler::GetSingleton(); }
  void ShowSkyrimMessage(const char* message) { RE::DebugMessageBox(message); }
  RE::BGSKeyword* ProduceOrGetKw(const std::string& keyword);
  int HasKeywordInList(const RE::BGSKeywordForm* form, const std::vector<RE::BGSKeyword*>& keywords);
  SEFormLoc FormToLoc(const RE::TESForm* form, const int choice = errInt);
  std::string LocToStr(const SEFormLoc& loc);
  SEFormLoc StrToLoc(const std::string& locStr);
  std::string FormToStr(const RE::TESForm* form) { return LocToStr(FormToLoc(form)); }
  // TODO: Clean up if it ends up not being used.
  // std::string Join(const std::vector<std::string>& strings, const std::string_view delimiter) { return fmt::format("{}", fmt::join(strings, delimiter)); }
  std::vector<std::string> Split(const std::string& str, const std::string_view delimiter);
  std::string NameToStr(std::string name);
  std::string StrToName(std::string name);
  void UpdateFormList(RE::BGSListForm* formList, RE::TESForm* form, const bool addRemove);  // Add: true, Remove: false
  // TODO: Clean up when necessary. This shouldn't be needed/used anymore, but may be a good reference for async operations.
  void DoDelayed(std::function<void()> func, std::function<bool()> condition, const int fixedDelay, const bool enforceCond, const std::string fmsg = "");
  // TODO: Clean up if it ends up not being used.
  const bool InInventory(RE::Actor* const actor, RE::TESBoundObject* const object);
  const std::vector<RE::TESObjectARMO*> InvItemsWithKey(RE::Actor* const actor, RE::BGSKeyword* const key);
  const std::string F0(const RE::TESForm* form, const bool isLogging = true) {
    auto nform = isLogging ? std::string(nullForm) : "";
    auto nfile = isLogging ? std::string(unrecFile) : "";
    return form ? form->GetFile(0) ? std::string(form->GetFile(0)->GetFilename()) : nfile : nform;
  }
  const std::string FL(const RE::TESForm* form, const bool isLogging = true) {
    auto nform = isLogging ? std::string(nullForm) : "";
    auto nfile = isLogging ? std::string(unrecFile) : "";
    return form ? form->GetFile() ? std::string(form->GetFile()->GetFilename()) : nfile : nform;
  }
  const RE::FormID I0(const RE::TESForm* form) { return form ? form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID() : 0; }
  const RE::FormID IG(const RE::TESForm* form) { return form ? form->GetFormID() : 0; }
  const std::string EI(const RE::TESForm* form, const bool isLogging = true) { return form ? std::string(form->GetFormEditorID()) : (isLogging ? std::string(nullForm) : ""); }
  const bool try_strtoul(const std::string& str, std::uint32_t& result, int base = 0);
}