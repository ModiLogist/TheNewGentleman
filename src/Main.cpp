#include <TngInis.h>
#include <TngUtil.h>
#include <TngEvents.h>

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
    if (TngUtil::Initialize()) {
      TngUtil::GenitalizeRaces();
      TngUtil::GenitalizeNPCSkins();
      TngUtil::CheckArmorPieces();
      Tng::gLogger::info("TheNewGentleman finished initialization.");
      TngEvents::RegisterEvents();
    } else {
      Tng::gLogger::error("TheNewGentleman did not initialize successfully!");
    }
  }
}

SKSEPluginLoad(const SKSE::LoadInterface* aSkse) {
  const auto lPlugin{SKSE::PluginDeclaration::GetSingleton()};
  const auto lVersion{lPlugin->GetVersion()};
  InitializeLogging(lPlugin);
  Tng::gLogger::info("Initializing TheNewGentleman {}!", lVersion);
  SKSE::Init(aSkse);
  const auto lMsgInterface{SKSE::GetMessagingInterface()};
  const bool lRegistered = lMsgInterface->RegisterListener(EventListener);  
  return lRegistered;
}