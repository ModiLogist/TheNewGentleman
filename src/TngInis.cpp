#include <TngInis.h>

void TngInis::LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>> aField) {
  CSimpleIniA::TNamesDepend::const_iterator lEntry;
  for (lEntry = aModRecords.begin(); lEntry != aModRecords.end(); lEntry++) {
    const std::string lModRecord(lEntry->pItem);
    const size_t lSepLoc = lModRecord.find(cDelimChar);
    const RE::FormID lID = std::strtol(lModRecord.substr(0, lSepLoc).data(), nullptr, 0);
    const std::string lModName = lModRecord.substr(lSepLoc + 1);
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
        if (aIni.KeyExists(cSkinSection, cSkinMod)) {
          CSimpleIniA::TNamesDepend lMods;
          aIni.GetAllValues(cSkinSection, cSkinMod, lMods);

          CSimpleIniA::TNamesDepend::const_iterator lMod;
          for (lMod = lMods.begin(); lMod != lMods.end(); lMod++) {
            const std::string lModName(lMod->pItem);
            fSkinMods.insert(lModName);
          }
        }
        if (aIni.KeyExists(cSkinSection, cSkinRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          aIni.GetAllValues(cSkinSection, cSkinRecord, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleSkinIDs);
        }
      }
      if (aIni.SectionExists(cArmorSection)) {
        if (aIni.KeyExists(cArmorSection, cRevealingMod)) {
          CSimpleIniA::TNamesDepend lMods;
          aIni.GetAllValues(cArmorSection, cRevealingMod, lMods);
          for (const auto& lMod : lMods) {
            const std::string lModName(lMod.pItem);
            fRevealingMods.insert(lModName);
          }
        }
        if (aIni.KeyExists(cArmorSection, cRevealingRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          aIni.GetAllValues(cArmorSection, cRevealingMod, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleRevealingIDs);
        }
        if (aIni.KeyExists(cArmorSection, cCoveringRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          aIni.GetAllValues(cArmorSection, cCoveringRecord, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleCoveringIDs);
        }
      }
      if (aIni.SectionExists(cSwapSection)) {
        CSimpleIniA::TNamesDepend lEntries;
        aIni.GetAllValues(cSwapSection, cSwapMod, lEntries);
        for (const auto& lEntry : lEntries) {
          const std::string lModSwap(lEntry.pItem);
          const size_t lSepLoc = lModSwap.find(cColonChar);
          const std::string lModName(0, lSepLoc);
          int lSlotInt = (int)std::pow(2, std::strtol(lModSwap.substr(lSepLoc + 1).data(), nullptr, 0) - 30);
          RE::BGSBipedObjectForm::BipedObjectSlot lID = static_cast<RE::BIPED_MODEL::BipedObjectSlot>(lSlotInt);
          fSwapMods.insert(std::make_pair(lModName, lID));
        }
      }
    }
  }
}
