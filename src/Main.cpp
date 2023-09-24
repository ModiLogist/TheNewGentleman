#include "Hooks.h"
#include "SKSE/Interfaces.h"

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    
    SKSE::Init(skse);

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded)
        {
            Hooks::Install();
        }
            
    });

    return true;
}