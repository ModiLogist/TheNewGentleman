#include <BaseUtil.h>
using namespace Common;

RE::BGSKeyword* BaseUtil::ProduceOrGetKw(const std::string& keyword) {
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

int BaseUtil::HasKeywordInList(const RE::BGSKeywordForm* form, const std::vector<RE::BGSKeyword*>& keywords) const {
  for (int i = 0; i < keywords.size(); i++) {
    if (keywords[i] && form->HasKeyword(keywords[i])) return i;
  }
  return -1;
}

SEFormLoc BaseUtil::FormToLoc(const RE::TESForm* form, const int choice) const {
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

std::string BaseUtil::LocToStr(const SEFormLoc& loc) const {
  if (loc.first == 0 && loc.second == nulStr) return nulStr;
  if (loc.first == 0 && loc.second == defStr) return defStr;
  if (loc.first == 0 || loc.second.empty()) return "";
  std::ostringstream oss;
  oss << std::hex << loc.first;
  return "0x" + oss.str() + delim + NameToStr(loc.second);
}

SEFormLoc BaseUtil::StrToLoc(const std::string& locStr) const {
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

std::vector<std::string> BaseUtil::Split(const std::string& str, const std::string_view delimiter) const {
  std::vector<std::string> tokens;
  for (auto part : std::views::split(str, delimiter)) {
    auto token = std::string(part.begin(), part.end());
    tokens.emplace_back(token);
  }
  return tokens;
}

std::string BaseUtil::NameToStr(std::string name) const {
  if (name.empty()) return name;
  if (name.length() > 2 && name.front() == '\"' && name.back() == '\"') return name;
  if (name.find_first_of(iniChars) != std::string::npos) return "\"" + name + "\"";
  return name;
}

std::string BaseUtil::StrToName(std::string name) const {
  auto res = name;
  if (name.length() < 2) return res;
  if (name.front() == '\"' && name.back() == '\"') res = name.substr(1, name.length() - 2);
  return res;
}

void BaseUtil::UpdateFormList(RE::BGSListForm* formList, RE::TESForm* form, const bool addRemove) const {
  if (!formList || !form) {
    SKSE::log::critical("UpdateFormList failed: formList is {} and form is {}.", formList ? "valid" : "invalid", form ? "valid" : "invalid");
    return;
  }
  if (!formList->scriptAddedTempForms) {
    if (addRemove) formList->AddForm(form);
    return;
  }
  auto idIt = std::find(formList->scriptAddedTempForms->begin(), formList->scriptAddedTempForms->end(), form->formID);
  if (addRemove && idIt == formList->scriptAddedTempForms->end()) {
    formList->AddForm(form);
  } else if (!addRemove && idIt != formList->scriptAddedTempForms->end()) {
    formList->scriptAddedTempForms->erase(idIt);
    return;
  }
}

void BaseUtil::DoDelayed(std::function<void()> func, std::function<bool()> condition, const int fixedDelay, const bool enforceCond, const std::string fmsg) const {
  if (fixedDelay == 0 && condition()) {
    func();
    return;
  }
  static bool isFirst{true};
  std::thread([=]() {
    __try {
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
    } __except (EXCEPTION_EXECUTE_HANDLER) {
      SKSE::log::debug("Hardware exception occurred in parallel processes. If the issue is causing you a problem, please report this issue.");
    }
  }).detach();
}

const bool BaseUtil::InInventory(RE::Actor* const actor, RE::TESBoundObject* const object) const {
  auto invChanges = actor->GetInventoryChanges(true);
  if (invChanges && invChanges->entryList) {
    for (auto& entry : *invChanges->entryList) {
      if (entry && entry->object && entry->object == object) {
        return true;
      }
    }
  }
  auto container = actor->GetContainer();
  auto res = false;
  if (container) {
    container->ForEachContainerObject([&](RE::ContainerObject& a_entry) {
      auto obj = a_entry.obj;
      if (obj && obj == object) {
        res = true;
        return RE::BSContainer::ForEachResult::kStop;
      }
      return RE::BSContainer::ForEachResult::kContinue;
    });
  }
  return res;
}

const std::vector<RE::TESObjectARMO*> BaseUtil::InvItemsWithKey(RE::Actor* const actor, RE::BGSKeyword* const key) const {
  auto container = actor->GetContainer();
  std::vector<RE::TESObjectARMO*> res = {};
  if (container) {
    container->ForEachContainerObject([&](RE::ContainerObject& a_entry) {
      auto obj = a_entry.obj;
      if (obj && obj->IsArmor() && obj->As<RE::TESObjectARMO>()->HasKeyword(key)) {
        res.push_back(obj->As<RE::TESObjectARMO>());
      }
      return RE::BSContainer::ForEachResult::kContinue;
    });
  }
  return res;
}

const bool BaseUtil::try_strtoul(const std::string& str, std::uint32_t& result, int base) const {
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
