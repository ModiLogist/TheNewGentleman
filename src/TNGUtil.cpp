#include <TNGUtil.h>
using namespace TNG;

TNGUtil* ut = TNGUtil::GetSingleton();

RE::TESRace* TNGUtil::Race(const size_t idx) {
  if (idx >= racesCount) return nullptr;
  if (!races[idx]) races[idx] = ut->SEDH()->LookupForm<RE::TESRace>(raceIDs[idx].first, raceIDs[idx].second);
  return races[idx];
}

RE::BGSKeyword* TNGUtil::Key(const size_t idx) {
  if (idx >= keywordsCount) return nullptr;
  if (!keywords[idx]) keywords[idx] = ut->SEDH()->LookupForm<RE::BGSKeyword>(keyIDs[idx].first, keyIDs[idx].second);
  return keywords[idx];
}

std::vector<RE::BGSKeyword*> TNGUtil::Keys(const size_t first, const size_t last) {
  std::vector<RE::BGSKeyword*> res = {};
  if (last >= keywordsCount) return res;
  for (auto i = first; i <= last; i++) res.push_back(Key(i));
  return res;
}

RE::BGSKeyword* TNGUtil::SizeKey(const size_t idx) {
  if (!sizeKey[idx]) sizeKey[idx] = ut->SEDH()->LookupForm<RE::BGSKeyword>(sizeKeyIDs[idx], mainFile);
  return sizeKey[idx];
}

std::vector<RE::BGSKeyword*> TNGUtil::SizeKeys(const size_t last) {
  std::vector<RE::BGSKeyword*> res{};
  for (size_t i = 0; i < (last > sizeCatCount ? sizeCatCount : last); i++) res.push_back(SizeKey(i));
  return res;
}

RE::BGSListForm* TNGUtil::FormList(const size_t idx) { return ut->SEDH()->LookupForm<RE::BGSListForm>(formListIDs[idx].first, formListIDs[idx].second); }

bool TNGUtil::IsCovering(const RE::Actor* const actor, const RE::TESObjectARMO* const armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (armor->HasKeyword(Key(kyCovering))) return true;
  if (armor->HasKeyword(Key(kyRevealingF)) && !npc->IsFemale()) return true;
  if (armor->HasKeyword(Key(kyRevealingM)) && npc->IsFemale()) return true;
  return false;
}

std::vector<RE::TESObjectARMO*> TNGUtil::GetWornAmor(RE::Actor* const actor) const {
  std::vector<RE::TESObjectARMO*> res{};
  res.clear();
  if (!actor) return res;
  // TODO: Test variety of swaps before release
  auto inv = actor->GetInventory([=](RE::TESBoundObject& obj) { return obj.IsArmor() && !obj.As<RE::TESObjectARMO>()->HasPartOf(genitalSlot); });
  for (const auto& [item, invData] : inv) {
    const auto& [count, entry] = invData;
    if (count > 0 && entry && entry->IsWorn()) {
      res.push_back(item->As<RE::TESObjectARMO>());
    }
  }
  return res;
}

bool TNGUtil::HasCovering(RE::Actor* const actor, RE::TESObjectARMO* const exception) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return false;
  auto wornArmor = GetWornAmor(actor);
  for (const auto& armor : wornArmor)
    if (armor && IsCovering(actor, armor) && armor != exception) return true;
  return false;
}
