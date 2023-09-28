#include <stddef.h>
#include <Hooks.h>

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;


void InitializeLogging() {
    auto path{SKSE::log::log_directory()};
    if (!path) {
        report_and_fail("Unable to lookup SKSE logs directory.");
    }
    *path /= PluginDeclaration::GetSingleton()->GetName();
    *path += L".log";

    std::shared_ptr<spdlog::logger> log;
    if (IsDebuggerPresent())
        log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
    else
        log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));

    log->set_level(spdlog::level::level_enum::info);
    log->flush_on(spdlog::level::level_enum::trace);

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("[%H:%M:%S.%e] [%n] [%l] %v");
}



SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    InitializeLogging();

    const auto plugin{SKSE::PluginDeclaration::GetSingleton()};
    const auto version{plugin->GetVersion()};
    
    SKSE::Init(skse);

    if(!SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded)
        {
            Hooks::Install();
        }
            
    })) {
        return false;
    };

    return true;
}