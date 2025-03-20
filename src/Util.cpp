#include "Util.h"

Util* ut = Util::GetSingleton();

RE::TESDataHandler* Util::SEDH() {
  if (!sedh) sedh = RE::TESDataHandler::GetSingleton();
  return sedh;
}

RE::TESRace* Util::Race(const size_t idx) {
  if (idx >= racesCount) return nullptr;
  if (!races[idx]) races[idx] = SEDH()->LookupForm<RE::TESRace>(raceIDs[idx].first, raceIDs[idx].second);
  return races[idx];
}

RE::BGSKeyword* Util::Key(const size_t idx) {
  if (idx >= keywordsCount) return nullptr;
  if (!keywords[idx]) keywords[idx] = SEDH()->LookupForm<RE::BGSKeyword>(keyIDs[idx].first, keyIDs[idx].second);
  return keywords[idx];
}

std::vector<RE::BGSKeyword*> Util::Keys(const size_t first, const size_t last) {
  std::vector<RE::BGSKeyword*> res = {};
  if (last >= keywordsCount) return res;
  for (auto i = first; i <= last; i++) res.push_back(Key(i));
  return res;
}

RE::BGSKeyword* Util::SizeKey(const size_t idx) {
  if (!sizeKey[idx]) sizeKey[idx] = SEDH()->LookupForm<RE::BGSKeyword>(sizeKeyIDs[idx], mainFile);
  return sizeKey[idx];
}

std::vector<RE::BGSKeyword*> Util::SizeKeys(const size_t last) {
  std::vector<RE::BGSKeyword*> res{};
  for (size_t i = 0; i < (last > sizeKeyCount ? sizeKeyCount : last); i++) res.push_back(SizeKey(i));
  return res;
}

RE::TESGlobal* Util::PCAddon() {
  if (!pcAddon) pcAddon = SEDH()->LookupForm<RE::TESGlobal>(pcAddonID.first, pcAddonID.second);
  return pcAddon;
}

RE::BGSListForm* Util::TngFml(const size_t idx) {
  if (!tngFormLists[idx]) tngFormLists[idx] = SEDH()->LookupForm<RE::BGSListForm>(formListIDs[idx].first, formListIDs[idx].second);
  return tngFormLists[idx];
}

RE::TESObjectARMO* Util::Block() {
  if (!block) block = SEDH()->LookupForm<RE::TESObjectARMO>(coverID.first, coverID.second);
  return block;
}

RE::BGSKeyword* Util::ProduceOrGetKw(const std::string& keyword) {
  auto& allKeywords = SEDH()->GetFormArray<RE::BGSKeyword>();
  auto it = std::find_if(allKeywords.begin(), allKeywords.end(), [&](const auto& kw) { return kw && kw->formEditorID == keyword.c_str(); });
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

void Util::ShowSkyrimMessage(const char* message) { RE::DebugMessageBox(message); }

SEFormLoc Util::StrToLoc(const std::string recordStr, bool canBeNone) {
  const size_t sepLoc = recordStr.find(delim);
  if (canBeNone && recordStr == nulStr) return {0, nulStr};
  if (sepLoc == std::string::npos) return {0, ""};
  const RE::FormID formID = std::strtol(recordStr.substr(0, sepLoc).data(), nullptr, 0);
  const std::string modName = recordStr.substr(sepLoc + 1);
  return std::make_pair(formID, modName);
}

SEFormLoc Util::FormToLoc(const RE::TESForm* form) {
  std::string filename = form->GetFile(0) ? std::string(form->GetFile(0)->GetFilename()) : "NoFile";
  auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
  return {formID, filename};
}

SEFormLocView Util::FormToLocView(RE::TESForm* form) {
  auto filename = form->GetFile(0) ? form->GetFile(0)->GetFilename() : "NoFile";
  auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
  return {formID, filename};
}

std::string Util::FormToStr(RE::TESForm* form) {
  if (!form || !form->GetFile(0)) return "";
  std::ostringstream oss;
  auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
  oss << std::hex << formID;
  return "0x" + oss.str() + delim + std::string(form->GetFile(0)->GetFilename());
}