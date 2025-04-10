#include <Util.h>

Common::Util* ut = Common::Util::GetSingleton();

RE::TESRace* Common::Util::Race(const size_t idx) {
  if (idx >= racesCount) return nullptr;
  if (!races[idx]) races[idx] = ut->SEDH()->LookupForm<RE::TESRace>(raceIDs[idx].first, raceIDs[idx].second);
  return races[idx];
}

RE::BGSKeyword* Common::Util::Key(const size_t idx) {
  if (idx >= keywordsCount) return nullptr;
  if (!keywords[idx]) keywords[idx] = ut->SEDH()->LookupForm<RE::BGSKeyword>(keyIDs[idx].first, keyIDs[idx].second);
  return keywords[idx];
}

std::vector<RE::BGSKeyword*> Common::Util::Keys(const size_t first, const size_t last) {
  std::vector<RE::BGSKeyword*> res = {};
  if (last >= keywordsCount) return res;
  for (auto i = first; i <= last; i++) res.push_back(Key(i));
  return res;
}

RE::BGSKeyword* Common::Util::SizeKey(const size_t idx) {
  if (!sizeKey[idx]) sizeKey[idx] = ut->SEDH()->LookupForm<RE::BGSKeyword>(sizeKeyIDs[idx], mainFile);
  return sizeKey[idx];
}

std::vector<RE::BGSKeyword*> Common::Util::SizeKeys(const size_t last) {
  std::vector<RE::BGSKeyword*> res{};
  for (size_t i = 0; i < (last > sizeCatCount ? sizeCatCount : last); i++) res.push_back(SizeKey(i));
  return res;
}

RE::BGSListForm* Common::Util::FormList(const size_t idx) {
  if (!tngFormLists[idx]) tngFormLists[idx] = ut->SEDH()->LookupForm<RE::BGSListForm>(formListIDs[idx].first, formListIDs[idx].second);
  return tngFormLists[idx];
}

RE::TESGlobal* Common::Util::PCAddon() {
  if (!pcAddon) pcAddon = ut->SEDH()->LookupForm<RE::TESGlobal>(pcAddonID.first, pcAddonID.second);
  return pcAddon;
}

RE::TESObjectARMO* Common::Util::Block() {
  if (!block) block = ut->SEDH()->LookupForm<RE::TESObjectARMO>(coverID.first, coverID.second);
  return block;
}

bool Common::Util::IsCovering(const RE::Actor* const actor, const RE::TESObjectARMO* const armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (armor->HasKeyword(Key(Common::kyCovering))) return true;
  if (armor->HasKeyword(Key(Common::kyRevealingF)) && !npc->IsFemale()) return true;
  if (armor->HasKeyword(Key(Common::kyRevealingM)) && npc->IsFemale()) return true;
  return false;
}

std::vector<RE::TESObjectARMO*> Common::Util::GetWornAmor(RE::Actor* const actor) const {
  std::vector<RE::TESObjectARMO*> res{};
  res.clear();
  if (!actor) return res;
  auto inv = actor->GetInventory([=](RE::TESBoundObject& obj) { return obj.IsArmor(); });
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count > 0 && entry && entry->IsWorn() && !IsBlock(item)) {
      res.push_back(item->As<RE::TESObjectARMO>());
    }
  }
  return res;
}

bool Common::Util::HasCovering(RE::Actor* const actor, RE::TESObjectARMO* const exception) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return false;
  auto wornArmor = GetWornAmor(actor);
  for (const auto& armor : wornArmor)
    if (armor && IsCovering(actor, armor) && armor != exception) return true;
  return false;
}

std::string Common::PlayerInfo::IdStr() const { return ut->NameToStr(name) + "|" + ut->LocToStr(race) + "|" + (isFemale ? "F" : "M"); }

std::string Common::PlayerInfo::InfoStr() const { return ut->LocToStr(addon) + "|" + std::to_string(sizeCat); }

bool Common::PlayerInfo::FromStr(const std::string& IdStr, const std::string& InfoStr) {
  PlayerInfo res{};
  auto idTokens = ut->Split(IdStr, "|");
  auto infoTokens = ut->Split(InfoStr, "|");
}
