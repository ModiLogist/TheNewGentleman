#include <Core.h>
#include <Hooks.h>
#include <Papyrus.h>
#include <SEEvents.h>
#include <Util.h>

bool CheckIncompatibility() {
  static bool shownMessage = false;
  if (GetModuleHandleW(L"Data\\SKSE\\Plugins\\acon.dll")) {
    if (!shownMessage) {
      ut->ShowSkyrimMessage("Warning: TNG is not compatible with acon.dll. Please don't use TNG with mods from that website!");
      shownMessage = true;
    }
    return false;
  }
  return true;
}

void InitializeLogging() {
  auto path{SKSE::log::log_directory()};
  if (!path) {
    SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
  }
  *path /= Version::PROJECT;
  *path += ".log"sv;

  std::shared_ptr<spdlog::logger> log;
  log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
  log->set_level(core->GetLogLvl());
  log->flush_on(spdlog::level::trace);
  spdlog::set_default_logger(std::move(log));
  spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

void EventListener(SKSE::MessagingInterface::Message* message) {
  switch (message->type) {
    case SKSE::MessagingInterface::kDataLoaded: {
      if (!CheckIncompatibility()) return;
      if (!ut->SEDH()->LookupModByName(Common::mainFile)) {
        const char* err = fmt::format("Mod [{}] was not found! Make sure that the mod is active in your plugin load order!", Common::mainFile).c_str();
        ut->ShowSkyrimMessage(err);
        return;
      }
      core->Process();
      events->RegisterEvents();
      Hooks::Install();
      SKSE::log::info("TheNewGentleman finished initialization.");
    } break;

    case SKSE::MessagingInterface::kPreLoadGame: {
      if (!CheckIncompatibility()) return;
      const std::string savePath{static_cast<char*>(message->data), message->dataLen};
      core->LoadPlayerInfos(savePath);
    } break;

    case SKSE::MessagingInterface::kSaveGame: {
      if (!CheckIncompatibility()) return;
      core->SaveMainIni();
    } break;
    default:
      break;
  }
}

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

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* info) {
  info->infoVersion = SKSE::PluginInfo::kVersion;
  info->name = Version::PROJECT.data();
  info->version = Version::MAJOR;
  return true;
}

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
  InitializeLogging();
  SKSE::Init(skse, false);
  SKSE::log::info("Initializing TheNewGentleman {}!", Version::NAME.data());
  SKSE::log::info("Game version : {}", skse->RuntimeVersion().string());
  SKSE::GetMessagingInterface()->RegisterListener(EventListener);
  SKSE::GetPapyrusInterface()->Register(Papyrus::BindPapyrus);
  return true;
}

extern "C" __declspec(dllexport) const char* APIENTRY GetPluginVersion() { return Version::NAME.data(); }
