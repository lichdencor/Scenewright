// Epic 1, Milestone 0: minimal plugin skeleton.
// Camera/fade/scene logic intentionally not started yet — this only proves
// the plugin loads under SKSE and can log, per docs/epic-1-camera-scene-engine.md.

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            RE::ConsoleLog::GetSingleton()->Print("Scenewright: plugin loaded.");
        }
    });

    return true;
}
