#include <Geode/modify/PlayLayer.hpp>
#include "../managers/StatsManager.hpp"
#include "../utils/Settings.hpp"

using namespace geode::prelude;

class $modify(DTPlayLayer, PlayLayer) {
    struct Fields {
        Session* prevSession = nullptr;
        bool isSessionExpired = false;
        bool hasRespawned = false;
        Run currentRun;

        std::vector<Playtime_pair> sessionPlaytime;

        bool lastOneDied = false;

        std::vector<int> fzeroToSave{};
        std::vector<Run> runsToSave{};
    };

    bool disableCompletedLevelTracking();

    void updateSessionLastPlayed();

    /* hooks
    ========== */

    bool init(GJGameLevel* level, bool p1, bool p2);

    void startPlaytime();

    void endPlaytime();

    void resetLevel();

    void destroyPlayer(PlayerObject* player, GameObject* p1);

    void levelComplete();

    void removeAllCheckpoints();

    #if defined(GEODE_IS_MACOS)

    void checkpointActivated(CheckpointGameObject* checkpt);
    
    #endif

    void pauseGame(bool unfocused);

    void resume();

    void onQuit();

    float getActualProgress(GJBaseGameLayer* game);

    void saveRun();
};
