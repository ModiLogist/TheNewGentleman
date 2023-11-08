#include <TngInis.h>

void TngInis::LoadModRecodPairs(std::list<CSimpleIniA::Entry> aModRecords, std::set<std::pair<std::string_view, RE::FormID>> aField) {
  for (const auto& lEntry : aModRecords) {
    const std::string_view lModRecord = std::basic_string_view(lEntry.pItem);
    const size_t lSepLoc = lModRecord.find(cDelimChar);
    const RE::FormID lID = std::strtol(lModRecord.substr(0, lSepLoc).data(), nullptr, 0);
    const std::string_view lModName = lModRecord.substr(lSepLoc + 1);
    aField.insert(std::make_pair(lModName, lID));
  }
}

bool TngInis::IsTngIni(const std::string aFileName) noexcept {
  if (aFileName.size() < cTngIniEnding.size()) return false;
  return std::equal(cTngIniEnding.rbegin(), cTngIniEnding.rend(), aFileName.rbegin());
}

void TngInis::LoadTngInis() noexcept {
  if (!std::filesystem::exists(cTngInisPath)) return;
  for (const auto& entry : std::filesystem::directory_iterator(cTngInisPath)) {
    if (IsTngIni(entry.path().filename().string())) {
      CSimpleIniA aIni;
      aIni.SetUnicode();
      aIni.SetMultiKey();
      aIni.LoadFile(entry.path().string().c_str());
      if (aIni.SectionExists(cSkinSection)) {
        if (aIni.KeyExists(cSkinSection, cSkinMod)) LoadSkinMods(&aIni);
        if (aIni.KeyExists(cSkinSection, cSkinRecord)) LoadSkinRecords(&aIni);
      }
      if (aIni.SectionExists(cArmorSection)) {
        if (aIni.KeyExists(cArmorSection, cRevealingMod)) LoadRevealingMods(&aIni);
        if (aIni.KeyExists(cArmorSection, cRevealingRecord)) LoadRevealingRecords(&aIni);
        if (aIni.KeyExists(cArmorSection, cCoveringRecord)) LoadCoveringRecords(&aIni);
      }
      if (aIni.SectionExists(cSwapSection)) LoadSwapMods(&aIni);
    }
  }
}

void TngInis::LoadSkinMods(CSimpleIniA* aFile) noexcept {
  std::list<CSimpleIniA::Entry> lMods;
  aFile->GetAllValues(cSkinSection, cSkinMod, lMods);
  for (const auto& lMod : lMods) {
    const auto lModName = std::basic_string_view(lMod.pItem);
    fSkinMods.insert(lModName);
  }
}

void TngInis::LoadSkinRecords(CSimpleIniA* aFile) noexcept {
  std::list<CSimpleIniA::Entry> lModRecords;
  aFile->GetAllValues(cSkinSection, cSkinRecord, lModRecords);
  LoadModRecodPairs(lModRecords, fSingleSkinIDs);
}

void TngInis::LoadRevealingMods(CSimpleIniA* aFile) noexcept {
  std::list<CSimpleIniA::Entry> lMods;
  aFile->GetAllValues(cArmorSection, cRevealingMod, lMods);
  for (const auto& lMod : lMods) {
    const auto lModName = std::basic_string_view(lMod.pItem);
    fRevealingMods.insert(lModName);
  }
}

void TngInis::LoadRevealingRecords(CSimpleIniA* aFile) noexcept {
  std::list<CSimpleIniA::Entry> lModRecords;
  aFile->GetAllValues(cArmorSection, cRevealingMod, lModRecords);
  LoadModRecodPairs(lModRecords, fSingleRevealingIDs);
}

void TngInis::LoadCoveringRecords(CSimpleIniA* aFile) noexcept {
  std::list<CSimpleIniA::Entry> lModRecords;
  aFile->GetAllValues(cArmorSection, cCoveringRecord, lModRecords);
  LoadModRecodPairs(lModRecords, fSingleCoveringIDs);
}

void TngInis::LoadSwapMods(CSimpleIniA* aFile) noexcept {
  std::list<CSimpleIniA::Entry> lEntries;
  aFile->GetAllValues(cSwapSection, cSwapMod, lEntries);
  for (const auto& lEntry : lEntries) {
    const std::string_view lModSwap = std::basic_string_view(lEntry.pItem);
    const size_t lSepLoc = lModSwap.find(cColonChar);
    const std::string_view lModName = lModSwap.substr(0, lSepLoc);
    int lSlotInt = (int) std::pow(2,std::strtol(lModSwap.substr(lSepLoc + 1).data(), nullptr, 0)-30);    
    RE::BGSBipedObjectForm::BipedObjectSlot lID = static_cast<RE::BIPED_MODEL::BipedObjectSlot>(lSlotInt);
    fSwapMods.insert(std::make_pair(lModName, lID));
  }
}
