#include <Base.h>
#include <Core.h>
#include <Events.h>
#include <Inis.h>
#include <Papyrus.h>

static bool CheckIncompatiblity() {
  if (GetModuleHandle(L"Data\\SKSE\\Plugins\\acon.dll")) {
    RE::DebugMessageBox("Warning: TNG is not compatible with acon.dll. Please don't use TNG with mods from that website!");
    return false;
  }
  return true;
}

static void IssueWarning() {
  Tng::gLogger::error("TheNewGentleman did not initialize successfully!");
  RE::DebugMessageBox("TNG Error 1: The New Gentleman DLL cannot be loaded successfully! Make sure you have all the requirements installed!");
}

static void InitializeLogging() {
  auto lPath{Tng::gLogger::log_directory()};
  if (!lPath) {
    SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
  }
  *lPath /= Version::PROJECT;
  *lPath += ".log"sv;

  std::shared_ptr<spdlog::logger> lLog;
  lLog = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(lPath->string(), true));
  lLog->set_level(Inis::GetLogLvl() > static_cast<int>(spdlog::level::debug) && Inis::GetLogLvl() < static_cast<int>(spdlog::level::n_levels)
                      ? static_cast<spdlog::level::level_enum>(Inis::GetLogLvl())
                      : spdlog::level::info);
  lLog->flush_on(spdlog::level::trace);
  spdlog::set_default_logger(std::move(lLog));
  spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

static void EventListener(SKSE::MessagingInterface::Message* aMessage) noexcept {
  if (aMessage->type == SKSE::MessagingInterface::kDataLoaded) {
    if (!CheckIncompatiblity()) return;
    if (Base::Init()) {
      Base::LoadAddons();
    } else {
      IssueWarning();
      return;
    }
    if (Inis::Init()) {
      Inis::LoadMainIni();
      Inis::LoadTngInis();
    } else {
      IssueWarning();
      return;
    }
    if (Core::Initialize()) {
      Core::GenitalizeRaces();
      Core::GenitalizeNPCSkins();
      Core::CheckOutfits();
      Core::CheckArmorPieces();
      Tng::gLogger::info("TheNewGentleman finished initialization.");
      Events::RegisterEvents();
    } else {
      IssueWarning();
      return;
    }
  }
  if (aMessage->type == SKSE::MessagingInterface::kNewGame || aMessage->type == SKSE::MessagingInterface::kPostLoadGame) {
    Inis::LoadHoteKeys();
  }
}

#ifdef SKYRIMFLATRIM
extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = []() {
  SKSE::PluginVersionData v;
  v.PluginVersion(Version::MAJOR);
  v.PluginName(Version::PROJECT);
  v.AuthorName("ModiLogist");
  v.UsesAddressLibrary();
  v.CompatibleVersions({SKSE::RUNTIME_SSE_LATEST_AE});
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
  SKSE::Init(aSkse);
  Tng::gLogger::info("Initializing TheNewGentleman {}!", Version::NAME.data());
  Tng::gLogger::info("Game version : {}", aSkse->RuntimeVersion().string());
  SKSE::GetMessagingInterface()->RegisterListener(EventListener);
  SKSE::GetPapyrusInterface()->Register(Papyrus::BindPapyrus);
  return true;
}
#ifdef SKYRIMVR
extern "C" __declspec(dllexport) const char* APIENTRY GetPluginVersion() { return Version::NAME.data(); }
#endif
