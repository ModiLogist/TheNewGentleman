#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Hooks.h>
#include <Inis.h>
#include <Papyrus.h>
#include <Util.h>

static bool CheckIncompatibility() {
  if (GetModuleHandleW(L"Data\\SKSE\\Plugins\\acon.dll")) {
    ut->ShowSkyrimMessage("Warning: TNG is not compatible with acon.dll. Please don't use TNG with mods from that website!");
    return false;
  }
  return true;
}

static void InitializeLogging() {
  auto path{SKSE::log::log_directory()};
  if (!path) {
    SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
  }
  *path /= Version::PROJECT;
  *path += ".log"sv;

  std::shared_ptr<spdlog::logger> log;
  log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
  log->set_level(inis->GetLogLvl());
  log->flush_on(spdlog::level::trace);
  spdlog::set_default_logger(std::move(log));
  spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

static void EventListener(SKSE::MessagingInterface::Message* message) {
  if (message->type == SKSE::MessagingInterface::kDataLoaded) {
    if (!CheckIncompatibility()) return;
    if (!ut->SEDH()->LookupModByName(Util::mainFile)) {
      const char* err = fmt::format("Mod [{}] was not found! Make sure that the mod is active in your plugin load order!", Util::mainFile).c_str();
      ut->ShowSkyrimMessage(err);
      return;
    }
    base->Init();
    inis->LoadMainIni();
    inis->LoadTngInis();
    core->ProcessRaces();
    core->ProcessSkins();
    core->CheckArmorPieces();
    SKSE::log::info("TheNewGentleman finished initialization.");
    events->RegisterEvents();
    Hooks::Install();
  }
  if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
    base->UnsetPlayerInfo();
  }
}

#ifdef SKYRIMFLATRIM
extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = []() {
  SKSE::PluginVersionData v;
  v.PluginVersion(Version::MAJOR);
  v.PluginName(Version::PROJECT);
  v.AuthorName("ModiLogist");
  v.UsesAddressLibrary();
  v.UsesUpdatedStructs();
  v.CompatibleVersions({SKSE::RUNTIME_SSE_LATEST});
  return v;
}();
#endif

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* aInfo) {
  aInfo->infoVersion = SKSE::PluginInfo::kVersion;
  aInfo->name = Version::PROJECT.data();
  aInfo->version = Version::MAJOR;
  return true;
}

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* aSkse) {
  InitializeLogging();
  SKSE::Init(aSkse, false);
  SKSE::log::info("Initializing TheNewGentleman {}!", Version::NAME.data());
  SKSE::log::info("Game version : {}", aSkse->RuntimeVersion().string());
  SKSE::GetMessagingInterface()->RegisterListener(EventListener);
  SKSE::GetPapyrusInterface()->Register(Papyrus::BindPapyrus);
  return true;
}
#ifdef SKYRIMVR
extern "C" __declspec(dllexport) const char* APIENTRY GetPluginVersion() { return Version::NAME.data(); }
#endif
