#include <TngCore.h>
#include <TngCoreBase.h>
#include <TngEvents.h>
#include <TngInis.h>
#include <TngPapyrus.h>
#include <TngHooks.h>

static bool CheckIncompatiblity() {
  if (GetModuleHandle(L"Data\\SKSE\\Plugins\\acon.dll")) {
    RE::DebugMessageBox("Warning: TNG is not compatible with acon.dll. Please don't use TNG with mods from that website!");
    return false;
  }
  return true;
}

static void IssueWarning() {
  Tng::gLogger::error("TheNewGentleman did not initialize successfully!");
  RE::DebugMessageBox("$TNG_E_0");
}

static void InitializeLogging() {
  auto lPath{Tng::gLogger::log_directory()};
  if (!lPath) {
    SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
  }
  *lPath /= "TheNewGentleman.log"sv;

  std::shared_ptr<spdlog::logger> lLog;
  lLog = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(lPath->string(), true));

  lLog->set_level(spdlog::level::level_enum::info);
  lLog->flush_on(spdlog::level::level_enum::trace);

  spdlog::set_default_logger(std::move(lLog));
  spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

static void EventListener(SKSE::MessagingInterface::Message* aMessage) noexcept {
  if (aMessage->type == SKSE::MessagingInterface::kDataLoaded) {
    if (!CheckIncompatiblity()) return;
    if (TngCoreBase::Init()) {
      TngCoreBase::LoadAddons();
    } else {
      IssueWarning();
      return;
    }
    if (TngInis::Init()) {
      TngInis::LoadMainIni();
      TngInis::LoadTngInis();
    } else {
      IssueWarning();
      return;
    }
    if (TngCore::Initialize()) {
      TngCore::GenitalizeRaces();
      TngCore::GenitalizeNPCSkins();
      TngCore::CheckArmorPieces();
      Tng::gLogger::info("TheNewGentleman finished initialization.");
      TngEvents::RegisterEvents();
    } else {
      IssueWarning();
      return;
    }
    TngHooks::Install();
  }
  if (aMessage->type == SKSE::MessagingInterface::kNewGame || aMessage->type == SKSE::MessagingInterface::kPostLoadGame) {
    TngInis::LoadHoteKeys();
  }
}

#ifdef SKYRIM_AE
extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = []() {
  SKSE::PluginVersionData v;
  v.PluginVersion(Version::MAJOR);
  v.PluginName("TheNewGentleman");
  v.AuthorName("ModiLogist");
  v.UsesAddressLibrary();
  v.UsesUpdatedStructs();
  v.CompatibleVersions({SKSE::RUNTIME_LATEST});

  return v;
}();
#else
extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info) {
  a_info->infoVersion = SKSE::PluginInfo::kVersion;
  a_info->name = "TheNewGentleman";
  a_info->version = Version::MAJOR;

  if (a_skse->IsEditor()) {
    Tng::gLogger::critical("Loaded in editor, marking as incompatible"sv);
    return false;
  }

  const auto ver = a_skse->RuntimeVersion();
  if (ver
  #ifndef SKYRIMVR
      < SKSE::RUNTIME_1_5_39
  #else
      > SKSE::RUNTIME_VR_1_4_15_1
  #endif
  ) {
    Tng::gLogger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
    return false;
  }

  return true;
}
#endif

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
  InitializeLogging();
  SKSE::Init(a_skse);
  Tng::gLogger::info("Initializing TheNewGentleman {}!", Version::NAME.data());
  Tng::gLogger::info("Game version : {}", a_skse->RuntimeVersion().string());
  SKSE::GetMessagingInterface()->RegisterListener(EventListener);
  SKSE::GetPapyrusInterface()->Register(TngPapyrus::BindPapyrus);
  return true;
}

extern "C" __declspec(dllexport) const char* APIENTRY GetPluginVersion() { return Version::NAME.data(); }
