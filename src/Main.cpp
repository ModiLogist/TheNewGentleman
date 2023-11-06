#include <TngInis.h>
#include <TngUtil.h>
#include <stddef.h>

void InitializeLogging() {
  auto path{SKSE::log::log_directory()};
  if (!path) {
    SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
  }
  *path /= SKSE::PluginDeclaration::GetSingleton()->GetName();
  *path += L".log";

  std::shared_ptr<spdlog::logger> log;
  if (IsDebuggerPresent())
    log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
  else
    log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));

  log->set_level(spdlog::level::level_enum::info);
  log->flush_on(spdlog::level::level_enum::trace);

  spdlog::set_default_logger(std::move(log));
  spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

void EventListener(SKSE::MessagingInterface::Message* aMessage) noexcept {
  if (aMessage->type == SKSE::MessagingInterface::kDataLoaded) {
    if (TngUtil::Initialize()) {
      TngUtil::GenitalizeRaces();
      TngUtil::GenitalizeNPCSkins();
      TngUtil::CheckArmorPieces();
    }
  }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
  InitializeLogging();

  const auto plugin{SKSE::PluginDeclaration::GetSingleton()};
  const auto version{plugin->GetVersion()};
  ;
  SKSE::Init(skse);
  const auto lMsgInterface{SKSE::GetMessagingInterface()};
  if (!lMsgInterface->RegisterListener(EventListener)) return false;
  return true;
}