#pragma once

namespace Common {
  inline static constexpr std::string_view skyrimFile{"Skyrim.esm"};
  inline static constexpr int nan{-99};
  inline static constexpr int defPlayer{-3};
  inline static constexpr int def{-2};
  inline static constexpr int nul{-1};
  inline static constexpr std::string nanStr{"NA"};
  inline static constexpr std::string defStr{"Default"};
  inline static constexpr std::string nulStr{"None"};
  inline static constexpr char delim{'~'};

  struct FormComparator {
      bool operator()(const RE::TESForm* lhs, const RE::TESForm* rhs) const { return lhs->formID < rhs->formID; }
  };

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

  class BaseUtil {
    public:
      RE::TESDataHandler* SEDH() const { return RE::TESDataHandler::GetSingleton(); }
      void ShowSkyrimMessage(const char* message) const { RE::DebugMessageBox(message); }
      RE::BGSKeyword* ProduceOrGetKw(const std::string& keyword);
      int HasKeywordInList(const RE::BGSKeywordForm* form, const std::vector<RE::BGSKeyword*>& keywords) const;
      SEFormLoc FormToLoc(const RE::TESForm* form, const int choice = nan) const;
      std::string LocToStr(const SEFormLoc& loc) const;
      SEFormLoc StrToLoc(const std::string& locStr) const;
      std::string FormToStr(const RE::TESForm* form) const { return LocToStr(FormToLoc(form)); }
      std::string Join(const std::vector<std::string>& strings, const std::string_view delimiter) const;
      std::vector<std::string> Split(const std::string& str, const std::string_view delimiter) const;
      std::string NameToStr(std::string name) const;
      std::string StrToName(std::string name) const;
      void QueueNiNodeUpdate(const RE::Actor* actor) const;
      void UpdateFormList(RE::BGSListForm* formList, RE::TESForm* form, const bool addRemove) const;  // Add: true, Remove: false
      void DoDelayed(std::function<void()> func, std::function<bool()> condition, const int fixedDelay, const bool enforceCond, const std::string fmsg = "") const;

    private:
      inline static constexpr size_t fixedDelayTime{5000};
      inline static constexpr size_t delayTime{500};
      inline static constexpr size_t maxDelayCount{20};
      inline static constexpr size_t newGameDelayMult{2};
      inline static constexpr std::string iniChars{"[=:]"};
      const bool try_strtoul(const std::string& str, std::uint32_t& result, int base = 0) const;
  };
}
