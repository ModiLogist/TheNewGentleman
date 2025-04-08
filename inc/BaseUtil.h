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
      CSimpleIniA& ini;
      const U count;
      std::vector<T> values;
      const std::vector<T> defValues;
      const std::vector<const char*> sections;
      const std::vector<const char*> keys;
      std::vector<std::function<void()>> events;

    public:
      TypedSetting(CSimpleIniA& ini, U count, std::vector<T> defValues, std::vector<const char*> sections, std::vector<const char*> keys)
          : ini(ini), count(count), defValues(std::move(defValues)), values(this->defValues), sections(std::move(sections)), keys(std::move(keys)) {}

      void Load() {
        for (size_t i = 0; i < count; ++i) {
          if constexpr (std::is_same_v<T, int>) {
            values[i] = ini.GetLongValue(sections[i], keys[i], defValues[i]);
          } else if constexpr (std::is_same_v<T, bool>) {
            values[i] = ini.GetBoolValue(sections[i], keys[i], defValues[i]);
          } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
            values[i] = static_cast<T>(ini.GetDoubleValue(sections[i], keys[i], static_cast<double>(defValues[i])));
          } else if constexpr (std::is_same_v<T, std::string>) {
            values[i] = ini.GetValue(sections[i], keys[i], defValues[i]);
          } else {
            static_assert(false, "Unsupported type for GetValue");
          }
          auto valueChar = ini.GetValue(sections[i], keys[i]);
          SKSE::log::debug("\tThe setting [{}] was restored to [{}({})].", keys[i], valueChar, values[i] == defValues[i] ? "default" : "user");
        }
      }

      T Get(const U idx) const { return values[idx]; }

      void Set(const U idx, const T value) {
        if (values[idx] != value) {
          values[idx] = value;
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
          if (events[idx]) {
            events[idx]();
          }
        }
      }

      void SetEvent(const U idx, std::function<void()> event) { events[idx] = std::move(event); }
  };
  ;

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
