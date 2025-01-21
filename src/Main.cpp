#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>
#include <Papyrus.h>

static bool CheckIncompatiblity() {
  if (GetModuleHandle(L"Data\\SKSE\\Plugins\\acon.dll")) {
    ShowSkyrimMessage("Warning: TNG is not compatible with acon.dll. Please don't use TNG with mods from that website!");
    return false;
  }
  return true;
}

static void InitializeLogging() {
  auto path{Tng::logger::log_directory()};
  if (!path) {
    SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
  }
  *path /= Version::PROJECT;
  *path += ".log"sv;

  std::shared_ptr<spdlog::logger> log;
  log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
  log->set_level(Inis::GetLogLvl());
  log->flush_on(spdlog::level::trace);
  spdlog::set_default_logger(std::move(log));
  spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

static void EventListener(SKSE::MessagingInterface::Message* message) {
  if (message->type == SKSE::MessagingInterface::kDataLoaded) {
    if (!CheckIncompatiblity()) return;
    if (!Tng::SEDH()->LookupModByName(Tng::cName)) {
      const char* err = fmt::format("Mod [{}] was not found! Make sure that the mod is active in your plugin load order!", Tng::cName).c_str();
      ShowSkyrimMessage(err);
      return;
    }
    Base::Init();
    Inis::LoadMainIni();
    Inis::LoadTngInis();
    Core::GenitalizeRaces();
    Core::GenitalizeNPCSkins();
    Core::CheckArmorPieces();
    Tng::logger::info("TheNewGentleman finished initialization.");
    Events::RegisterEvents();
  }
  if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
    Inis::LoadHoteKeys();
    Base::UnsetPlayerInfo();
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
  Tng::logger::info("Initializing TheNewGentleman {}!", Version::NAME.data());
  Tng::logger::info("Game version : {}", aSkse->RuntimeVersion().string());
  SKSE::GetMessagingInterface()->RegisterListener(EventListener);
  SKSE::GetPapyrusInterface()->Register(Papyrus::BindPapyrus);
  return true;
}
#ifdef SKYRIMVR
extern "C" __declspec(dllexport) const char* APIENTRY GetPluginVersion() { return Version::NAME.data(); }
#endif
