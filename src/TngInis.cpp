#include <TngInis.h>

void TngInis::LoadModRecodPairs(CSimpleIniA::TNamesDepend aModRecords, std::set<std::pair<std::string, RE::FormID>>& aField) {
  CSimpleIniA::TNamesDepend::const_iterator lEntry;
  for (lEntry = aModRecords.begin(); lEntry != aModRecords.end(); lEntry++) {
    const std::string lModRecord(lEntry->pItem);
    const size_t lSepLoc = lModRecord.find(cDelimChar);
    const RE::FormID lID = std::strtol(lModRecord.substr(0, lSepLoc).data(), nullptr, 0);
    const std::string lModName = lModRecord.substr(lSepLoc + 1);
    aField.insert(std::make_pair(lModName, lID));
  }
}

bool TngInis::IsTngIni(const std::string_view aFileName) noexcept {
  if (aFileName.size() < cTngIniEnding.size()) return false;
  return std::equal(cTngIniEnding.rbegin(), cTngIniEnding.rend(), aFileName.rbegin());
}

void TngInis::LoadTngInis() noexcept {
  if (!std::filesystem::exists(cTngInisPath)) return;
  Tng::gLogger::info("Loading ini files:");
  for (const auto& entry : std::filesystem::directory_iterator(cTngInisPath)) {
    std::string lFileName = entry.path().filename().string();
    if (IsTngIni(lFileName)) {
      Tng::gLogger::info("\tFound ini file {}:", lFileName);
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
          Tng::gLogger::info("\t\t- Found skin records in ini file [{}].", lFileName);
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
          Tng::gLogger::info("\t\t- Found revealing records in ini file [{}].", lFileName);
        }
        if (aIni.KeyExists(cArmorSection, cCoveringRecord)) {
          CSimpleIniA::TNamesDepend lModRecords;
          aIni.GetAllValues(cArmorSection, cCoveringRecord, lModRecords);
          LoadModRecodPairs(lModRecords, fSingleCoveringIDs);
          Tng::gLogger::info("\t\t- Found covering records in ini file [{}].", lFileName);
        }
      }
    } else {
      Tng::gLogger::warn("The file {} in TNG ini folder is not named correctly or is not a TNG ini file.", lFileName);
    }
  }
}

void TngInis::LoadMainIni(bool* aRevealWomen, bool* aRevealMen) noexcept {
  if (!std::filesystem::exists(cTngInisPath)) return;
  if (!std::filesystem::exists(cSettings)) {
    std::ofstream lTngSettings(cSettings);
    lTngSettings << ";TNG Settings File" << std::endl;
    lTngSettings.close();
  }
  Tng::gLogger::info("Loading TNG settings...");
  CSimpleIniA aIni;
  aIni.SetUnicode();
  aIni.SetMultiKey();
  aIni.LoadFile(cSettings);
  if (!aIni.KeyExists(cAutoReveal, cFAutoReveal)) aIni.SetBoolValue(cAutoReveal, cFAutoReveal, true);
  if (!aIni.KeyExists(cAutoReveal, cMAutoReveal)) aIni.SetBoolValue(cAutoReveal, cMAutoReveal, false);
  *aRevealWomen = aIni.GetBoolValue(cAutoReveal, cFAutoReveal, true);
  *aRevealMen = aIni.GetBoolValue(cAutoReveal, cMAutoReveal, false);  
}
