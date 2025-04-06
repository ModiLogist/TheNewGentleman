#include <BaseUtil.h>

RE::BGSKeyword* Common::BaseUtil::ProduceOrGetKw(const std::string& keyword) {
  auto& allKeywords = SEDH()->GetFormArray<RE::BGSKeyword>();
  auto it = std::ranges::find_if(allKeywords, [&](const auto& kw) { return kw && kw->formEditorID == keyword.c_str(); });
  RE::BGSKeyword* res{nullptr};
  if (it != allKeywords.end()) {
    res = *it;
  } else {
    const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
    if (res = factory ? factory->Create() : nullptr; res) {
      res->formEditorID = keyword;
      allKeywords.push_back(res);
    }
  }
  return res;
}

int Common::BaseUtil::HasKeywordInList(const RE::BGSKeywordForm* form, const std::vector<RE::BGSKeyword*>& keywords) const {
  for (int i = 0; i < keywords.size(); i++) {
    if (keywords[i] && form->HasKeyword(keywords[i])) return i;
  }
  return -1;
}

SEFormLoc Common::BaseUtil::FormToLoc(const RE::TESForm* form) const {
  if (!form || !form->GetFile(0)) return {0, ""};
  std::string filename = std::string(form->GetFile(0)->GetFilename());
  auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
  return {formID, filename};
}

std::string Common::BaseUtil::LocToStr(const SEFormLoc& loc) const {
  if (loc.first == 0 && loc.second == nulStr) return nulStr;
  if (loc.first == 0 && loc.second == defStr) return defStr;
  if (loc.first == 0 || loc.second.empty()) return "";
  std::ostringstream oss;
  oss << std::hex << loc.first;
  return "0x" + oss.str() + delim + NameToStr(loc.second);
}

SEFormLoc Common::BaseUtil::StrToLoc(const std::string& locStr) const {
  if (locStr.empty()) return {0, ""};
  if (locStr == nulStr) return {0, nulStr};
  if (locStr == defStr) return {0, defStr};
  const size_t sepLoc = locStr.find(delim);
  RE::FormID formID;
  if (sepLoc == std::string::npos) {
    if (try_strtoul(locStr, formID)) {
      return {formID, std::string(skyrimFile)};
    } else {
      return {0, ""};
    }
  }
  if (try_strtoul(locStr.substr(0, sepLoc).data(), formID)) {
    const std::string modName = StrToName(locStr.substr(sepLoc + 1));
    return {formID, modName};
  } else {
    return {0, ""};
  }
}

std::string Common::BaseUtil::Join(const std::vector<std::string>& strings, const std::string_view delimiter) const { auto res = fmt::join(strings, delimiter); }

std::vector<std::string> Common::BaseUtil::Split(const std::string& str, const std::string_view delimiter) const {
  std::vector<std::string> tokens;
  for (auto part : std::views::split(str, delimiter)) {
    auto token = std::string(part.begin(), part.end());
    tokens.emplace_back(token);
  }
  return tokens;
}

bool Common::BaseUtil::try_strtoul(const std::string& str, std::uint32_t& result, int base) const {
  char* end;
  errno = 0;
  unsigned long value = std::strtoul(str.c_str(), &end, base);
  if (errno == ERANGE || end == str.c_str() || *end != '\0') {
    return false;
  }
  if (value > UINT_MAX) {
    return false;
  }
  result = static_cast<std::uint32_t>(value);
  return true;
}
