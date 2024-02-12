#include <TngCore.h>
#include <TngEvents.h>
#include <TngInis.h>
#include <TngPapyrus.h>
#include <TngSizeShape.h>
bool CheckIncompatiblity() {
  if (GetModuleHandle(L"Data\\SKSE\\Plugins\\acon.dll")) {
    RE::DebugMessageBox("Warning: TNG is not compatible with acon.dll. Please don't use TNG with mods from that website!");
    return false;
  }
  return true;
}

void IssueWarning() {
  Tng::gLogger::error("TheNewGentleman did not initialize successfully!");
  RE::DebugMessageBox("$TNG_E_0");
}

void InitializeLogging(const SKSE::PluginDeclaration* aPlugin) {
  auto lPath{Tng::gLogger::log_directory()};
  if (!lPath) {
    SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
  }
  *lPath /= aPlugin->GetName();
  *lPath += L".log";

  std::shared_ptr<spdlog::logger> lLog;
  lLog = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(lPath->string(), true));

  lLog->set_level(spdlog::level::level_enum::info);
  lLog->flush_on(spdlog::level::level_enum::trace);

  spdlog::set_default_logger(std::move(lLog));
  spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

void EventListener(SKSE::MessagingInterface::Message* aMessage) noexcept {
  if (aMessage->type == SKSE::MessagingInterface::kDataLoaded) {
    if (!CheckIncompatiblity()) return;
    if (TngSizeShape::Init()) {
      TngSizeShape::LoadAddons();
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
  }
}

extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = []() {
  SKSE::PluginVersionData v;
  v.PluginVersion(Version::MAJOR);
  v.PluginName(Version::PROJECT);
  v.AuthorName("ModiLogist");
  v.UsesAddressLibrary(true);
  v.CompatibleVersions({SKSE::RUNTIME_SSE_LATEST_AE});
  v.UsesNoStructs(true);

  return v;
}();

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info) {
  a_info->infoVersion = SKSE::PluginInfo::kVersion;
  a_info->name = Version::PROJECT.data();
  a_info->version = Version::MAJOR;

  return true;
}


SKSEPluginLoad(const SKSE::LoadInterface* aSkse) {
  const auto lPlugin{SKSE::PluginDeclaration::GetSingleton()};
  const auto lVersion{lPlugin->GetVersion()};
  InitializeLogging(lPlugin);
  Tng::gLogger::info("Initializing TheNewGentleman {}!", lVersion);
  SKSE::Init(aSkse);
  const bool lRegistered = SKSE::GetMessagingInterface()->RegisterListener(EventListener);
  if (lRegistered) SKSE::GetPapyrusInterface()->Register(TngPapyrus::BindPapyrus);
  return lRegistered;
}