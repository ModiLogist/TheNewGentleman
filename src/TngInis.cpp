#include <TngInis.h>

bool TngInis::IsTngIni(const std::string aFileName) noexcept {
  if (aFileName.size() < cTngIniEnding.size()) return false;
  return std::equal(cTngIniEnding.rbegin(), cTngIniEnding.rend(), aFileName.rbegin());
}

void TngInis::LoadTngInis() noexcept {
  for (const auto& entry : std::filesystem::directory_iterator(cTngInisPath)) {
    if (IsTngIni(entry.path().filename().string())) {
      CSimpleIniA aIni;
      aIni.SetUnicode();
      aIni.SetMultiKey();
      aIni.LoadFile(entry.path().string().c_str());
      if (aIni.SectionExists(cSkinSection)) LoadSkinMods(&aIni);
    }
  }
}

void TngInis::LoadSkinMods(CSimpleIniA* aFile) noexcept {
  std::list<CSimpleIniA::Entry> lMods;
  aFile->GetAllValues(cSkinSection, cSkinMod, lMods);
  for (const auto& aMod : lMods) {
    const auto lModName = std::basic_string_view(aMod.pItem);
    fSkinMods.insert(lModName);
  }
}
