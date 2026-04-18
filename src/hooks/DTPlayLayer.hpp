#include <Geode/modify/PlayLayer.hpp>
#include "../managers/StatsManager.hpp"
#include "../utils/Settings.hpp"
#include <chrono>

using namespace geode::prelude;

class $modify(DTPlayLayer, PlayLayer) {
    struct Fields {
        Session* prevSession = nullptr;
        bool isSessionExpired = false;
        bool hasRespawned = false;
        Run currentRun;

        bool lastOneDied = false;
        
        bool nolcip = false;
        bool speedhack = false;
        CCObject* disabledCheat = nullptr;
        std::optional<std::chrono::steady_clock::time_point> speedhackCompare;
        float ratioSum = 0.0f;
        int frameCount = 0;
        
        std::chrono::steady_clock::time_point lastSampleTime = std::chrono::steady_clock::now();
        const float sampleDurationSeconds = 0.1f;
        const int averagesToMeasure = 5;
        std::vector<float> lastAverages{};
        float currentTimeWarp = 1;

        float speedhackThreshold = 0.05f;

        bool didJustPause = false;
        PlaytimePair timePassedGeneral;
        PlaytimePair timePassedDead;
        PlaytimePair timePassedPaused;
        std::optional<std::chrono::steady_clock::time_point> startTime;

        std::vector<int> fzeroToSave{};
        std::vector<Run> runsToSave{};
        
        bool levelBeaten = false;
    };

    bool disableCompletedLevelTracking();

    void updateSessionLastPlayed();

    /* hooks
    ========== */
    static void onModify(auto& self);

    bool init(GJGameLevel* level, bool p1, bool p2);

    void cutoutPlaytime();

    void discardPlaytime();

    void startupPlaytime();

    void resetLevel();

    void destroyPlayer(PlayerObject* player, GameObject* p1);

    void levelComplete();

    void removeAllCheckpoints();

    #if defined(GEODE_IS_MACOS)

    void checkpointActivated(CheckpointGameObject* checkpt);
    
    #endif

    void pauseGame(bool unfocused);

    void resume();

    float getActualProgress(GJBaseGameLayer* game);

    void saveRun();

    void onExit();

    void startGame();

    void playEndAnimationToPos(cocos2d::CCPoint position);

    void playPlatformerEndAnimationToPos(cocos2d::CCPoint position, bool instant);

    bool isLegal();

    void postUpdate(float dt);

    void checkDelta(float delta);

    void updateTimeWarp(float timeWarp);
};
