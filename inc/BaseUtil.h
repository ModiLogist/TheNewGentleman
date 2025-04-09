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

  template <typename T, typename U>
  struct TypedSetting {
    private:
      static_assert(std::is_enum_v<U>, "TypedSetting U must be an enum type");
      CSimpleIniA& ini;
      const U count;
      const std::vector<T> defValues;
      const std::vector<const char*> sections;
      const std::vector<const char*> keys;

    public:
      TypedSetting(CSimpleIniA& ini, U count, std::vector<T> defValues, std::vector<const char*> sections, std::vector<const char*> keys)
          : ini(ini), count(count), defValues(std::move(defValues)), sections(std::move(sections)), keys(std::move(keys)) {}

      T Get(const U idx) const {
        auto value = defValues[idx];
        if constexpr (std::is_same_v<T, int>) {
          value = ini.GetLongValue(sections[idx], keys[idx], defValues[idx]);
        } else if constexpr (std::is_same_v<T, bool>) {
          value = ini.GetBoolValue(sections[idx], keys[idx], defValues[idx]);
        } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
          value = static_cast<T>(ini.GetDoubleValue(sections[idx], keys[idx], static_cast<double>(defValues[idx])));
        } else if constexpr (std::is_same_v<T, std::string>) {
          value = ini.GetValue(sections[idx], keys[idx], defValues[idx]);
        } else {
          static_assert(false, "Unsupported type for GetValue");
        }
        SKSE::log::debug("\tThe setting [{}] was restored to [{}({})].", keys[idx], value, value == defValues[idx] ? "default" : "user");
        return value;
      }

      void Set(const U idx, const T value) {
        if (Get(idx) != value) {
          if (defValues[idx] == value) {
            ini.Delete(sections[idx], keys[idx], true);
          } else if constexpr (std::is_same_v<T, int>) {
            ini.SetLongValue(sections[idx], keys[idx], value);
          } else if constexpr (std::is_same_v<T, bool>) {
            ini.SetBoolValue(sections[idx], keys[idx], value);
          } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            ini.SetDoubleValue(sections[idx], keys[idx], static_cast<double>(value));
          } else if constexpr (std::is_same_v<T, std::string>) {
            ini.SetValue(sections[idx], keys[idx], value.c_str());
          } else {
            static_assert(false, "Unsupported type for SetValue");
          }
        }
      }
  };

  class BaseUtil {
    public:
      RE::TESDataHandler* SEDH() const { return RE::TESDataHandler::GetSingleton(); }
      void ShowSkyrimMessage(const char* message) const { RE::DebugMessageBox(message); }
      RE::BGSKeyword* ProduceOrGetKw(const std::string& keyword);
      int HasKeywordInList(const RE::BGSKeywordForm* form, const std::vector<RE::BGSKeyword*>& keywords) const;
      SEFormLoc FormToLoc(const RE::TESForm* form) const;
      std::string LocToStr(const SEFormLoc& loc) const;
      SEFormLoc StrToLoc(const std::string& locStr) const;
      std::string FormToStr(const RE::TESForm* form) const { return LocToStr(FormToLoc(form)); }
      std::string Join(const std::vector<std::string>& strings, const std::string_view delimiter) const;
      std::vector<std::string> Split(const std::string& str, const std::string_view delimiter) const;
      std::string NameToStr(std::string name) const;
      std::string StrToName(std::string name) const;

    private:
      bool try_strtoul(const std::string& str, std::uint32_t& result, int base = 0) const;
      inline static constexpr std::string iniChars{"[=:]"};
  };
}
