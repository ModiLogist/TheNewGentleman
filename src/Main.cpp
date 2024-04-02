#include <Core.h>
#include <Base.h>
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

#ifdef SKYRIM_AE
extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = []() {
  SKSE::PluginVersionData v;
  v.PluginVersion(Version::MAJOR);
  v.PluginName("TheNewGentleman");
  v.AuthorName("ModiLogist");
  v.UsesAddressLibrary();
  v.UsesUpdatedStructs();
  #ifdef SKYRIM_353
  v.CompatibleVersions({SKSE::RUNTIME_1_6_353});
  #else
  v.CompatibleVersions({SKSE::RUNTIME_LATEST});
  #endif
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
  SKSE::GetPapyrusInterface()->Register(Papyrus::BindPapyrus);
  return true;
}

extern "C" __declspec(dllexport) const char* APIENTRY GetPluginVersion() { return Version::NAME.data(); }
