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

SEFormLoc Common::BaseUtil::FormToLoc(const RE::TESForm* form, const int choice) const {
  switch (choice) {
    case def:
      return {0, defStr};
    case nul:
      return {0, nulStr};
    default: {
      if (!form || !form->GetFile(0)) return {0, ""};
      std::string filename = std::string(form->GetFile(0)->GetFilename());
      auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
      return {formID, filename};
    }
  }
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

std::string Common::BaseUtil::Join(const std::vector<std::string>& strings, const std::string_view delimiter) const { return fmt::format("{}", fmt::join(strings, delimiter)); }

std::vector<std::string> Common::BaseUtil::Split(const std::string& str, const std::string_view delimiter) const {
  std::vector<std::string> tokens;
  for (auto part : std::views::split(str, delimiter)) {
    auto token = std::string(part.begin(), part.end());
    tokens.emplace_back(token);
  }
  return tokens;
}

std::string Common::BaseUtil::NameToStr(std::string name) const {
  if (name.empty()) return name;
  if (name.length() > 2 && name.front() == '\"' && name.back() == '\"') return name;
  if (name.find_first_of(iniChars) != std::string::npos) return "\"" + name + "\"";
  return name;
}

std::string Common::BaseUtil::StrToName(std::string name) const {
  auto res = name;
  if (name.length() < 2) return res;
  if (name.front() == '\"' && name.back() == '\"') res = name.substr(1, name.length() - 2);
  return res;
}

void Common::BaseUtil::QueueNiNodeUpdate(const RE::Actor* actor) const {
  if (!actor) return;
  if (actor->IsOnMount()) return;
  if (const auto vm = RE::SkyrimVM::GetSingleton(); vm) {
    if (auto vmi = vm->impl; vmi) {
      RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
      auto args = RE::MakeFunctionArguments();
      auto handle = vm->handlePolicy.GetHandleForObject(static_cast<RE::VMTypeID>(actor->FORMTYPE), actor);
      vmi->DispatchMethodCall2(handle, "Actor", "QueueNiNodeUpdate", args, callback);
    }
  }
}

void Common::BaseUtil::UpdateFormList(RE::BGSListForm* formList, RE::TESForm* form, const bool addRemove) const {
  if (addRemove && !formList->HasForm(form)) {
    formList->AddForm(form);
  } else if (!addRemove && formList->HasForm(form)) {
    auto ptrIt = std::find(formList->forms.begin(), formList->forms.end(), form);
    if (ptrIt != formList->forms.end()) {
      formList->forms.erase(ptrIt);
      return;
    }
    auto idIt = std::find(formList->scriptAddedTempForms->begin(), formList->scriptAddedTempForms->end(), form->formID);
    if (idIt != formList->scriptAddedTempForms->end()) {
      formList->scriptAddedTempForms->erase(idIt);
      return;
    }
  }
}

void Common::BaseUtil::DoDelayed(std::function<void()> func, std::function<bool()> condition, const int fixedDelay, const bool enforceCond, const std::string fmsg) const {
  if (fixedDelay == 0 && condition()) {
    func();
    return;
  }
  static bool isFirst{true};
  std::thread([=]() {
    auto delayMult = isFirst ? newGameDelayMult : 1;
    if (fixedDelay) {
      auto fixedTime = fixedDelay < 0 ? fixedDelayTime : fixedDelay;
      std::this_thread::sleep_for(std::chrono::milliseconds(delayMult * fixedTime));
      isFirst = false;
    } else {
      size_t count = 0;
      size_t maxCount = delayMult * maxDelayCount;
      while (!condition() && count < maxCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delayTime));
        count++;
      }
      if (count < maxCount) std::this_thread::sleep_for(std::chrono::milliseconds(delayTime));
    }
    if (enforceCond && !condition()) {
      if (!fmsg.empty()) SKSE::log::debug("{}", fmsg.c_str());
      return;
    }
    func();
  }).detach();
}

const bool Common::BaseUtil::try_strtoul(const std::string& str, std::uint32_t& result, int base) const {
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
