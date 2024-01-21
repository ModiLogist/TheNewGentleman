#include <TngSizeShape.h>
#include <TngInis.h>
#include <TngCore.h>
#include <TngEvents.h>
#include <TngPapyrus.h>
bool CheckIncompatiblity() {
  if (GetModuleHandle(L"Data\\SKSE\\Plugins\\acon.dll")) {
    RE::DebugMessageBox("Warning: TNG is not compatible with acon.dll. Please don't use it with mods from that website!"); 
    return false;
  }
  return true;
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
    if (!TngSizeShape::LoadAddons()) return;
    if (!TngInis::LoadMainIni()) return;
    if (!CheckIncompatiblity()) return;
    if (TngCore::Initialize()) {
      TngCore::GenitalizeRaces();
      TngCore::GenitalizeNPCSkins();
      TngCore::CheckArmorPieces();
      Tng::gLogger::info("TheNewGentleman finished initialization.");
      TngEvents::RegisterEvents();
    } else {
      Tng::gLogger::error("TheNewGentleman did not initialize successfully!");
      return;
    }
  }
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