#include "../hooks/DTPlayLayer.hpp"
#include <ctime>
using namespace geode::prelude;

bool DTPlayLayer::disableCompletedLevelTracking() {
    return Settings::isCompletedLevelTrackingDisabled()
        && m_level->m_newNormalPercent2.value() == 100
        && m_level->m_levelType == GJLevelType::Saved;
}

void DTPlayLayer::updateSessionLastPlayed() {
    auto session = StatsManager::getCurrentSession();

    if (!session) return;

    if (session == m_fields->prevSession && m_fields->isSessionExpired) return;
    StatsManager::updateCurrentSessionLastPlayed();
}

/* hooks
========== */
static void onModify(auto& self) {
    (void)self.setHookPriority("PlayLayer::levelComplete", -9999);
}

bool DTPlayLayer::init(GJGameLevel* level, bool p1, bool p2) {
    if (!PlayLayer::init(level, p1, p2)) return false;

    auto metaRes = StatsManager::getMetadata(level);
    if (metaRes.isErr() && metaRes.unwrapErr().size() && metaRes.unwrapErr()[0] == '1'){
        LevelMetadeta newMeta{};
        metaRes = Ok(newMeta);
    }
    else if (metaRes.isErr()){
        geode::Notification::create(fmt::format("Failed to load DT level data! {}", metaRes.unwrapErr()), NotificationIcon::Error)->show();
        //log::info("{}", metaRes.unwrapErr());
    }

    if (metaRes.isOk()){
        auto metadata = metaRes.unwrap();
        metadata.levelName = level->m_levelName;
        metadata.attempts = level->m_attempts;
        metadata.difficulty = StatsManager::getDifficulty(level);
        (void)StatsManager::setMetadata(metadata, level);
        // if (_.isErr())
        //     log::info("set meta res {}", _.unwrapErr());
        metaRes = Ok(metadata);
    }

    StatsManager::setCurrentLevel(level);

    // log::info("PlayLayer::init()");

    auto session = StatsManager::getCurrentSession();
    auto levelKey = StatsManager::getLevelKey(level);
    auto sessionLength = Settings::getMaxSessionLength();

    if (session == nullptr){
        StatsManager::scheduleCreateNewSession(true);
        DTPlayLayer::updateSessionLastPlayed();
        // log::info("first session created");
        return true;
    }

    // schedule create a new session
    // based on the session length setting
    // -2 means default session (no previous session)
    if (session->lastPlayed != -2) {
        switch (sessionLength) {
            // reset session per level play
            case -2: {
                m_fields->isSessionExpired = true;
                StatsManager::scheduleCreateNewSession(true);
                break;
            }

            // reset session per game launch
            case -1: {
                // returns true if a new session
                // was created during this game launch
                if (StatsManager::hasPlayedLevel()) break;

                    m_fields->isSessionExpired = true;
                    StatsManager::scheduleCreateNewSession(true);
                    break;
                }

            // // reset session based on session.lastPlayed
            // default: {
            //         auto now = StatsManager::getNowSeconds();

            //     // reset if the time since lastPlayed
            //     // is longer than the session length
            //     if (now - session->lastPlayed > sessionLength) {
            //         m_fields->isSessionExpired = true;
            //         StatsManager::scheduleCreateNewSession(true);
            //     }

            //     break;
            // }
        }
    }

    DTPlayLayer::updateSessionLastPlayed();
    DTPlayLayer::startPlaytime();

    return true;
}

#if defined(GEODE_IS_MACOS)
void DTPlayLayer::checkpointActivated(CheckpointGameObject* checkpt) {
    PlayLayer::checkpointActivated(checkpt);

    m_fields->currentRun.end++;
}
#endif

void DTPlayLayer::startPlaytime() {
    time_t timestamp;

    m_fields->sessionPlaytime.emplace_back(time(&timestamp), std::nullopt);
}

void DTPlayLayer::endPlaytime() {
    time_t timestamp;

    if (!m_fields->sessionPlaytime.size()) return;

    m_fields->sessionPlaytime[m_fields->sessionPlaytime.size() - 1].end = time(&timestamp);
}

void DTPlayLayer::resetLevel() {
    if (m_fields->lastOneDied){
        m_fields->lastOneDied = false;
    }
    else{
        if (!DTPlayLayer::disableCompletedLevelTracking() && isGameplayActive()){
            auto metaRes = StatsManager::getMetadata(m_level);

            if (metaRes.isOk() && metaRes.unwrap().resetAsDeath){
                if (!m_level->isPlatformer())
                    m_fields->currentRun.end = getActualProgress(this);

                saveRun();
            }
        }
    }
    PlayLayer::resetLevel();
    //log::info("PlayLayer::resetLevel()");

    m_fields->hasRespawned = true;

    if (!m_level->isPlatformer())
        m_fields->currentRun.start = getActualProgress(this);
    else
        m_fields->currentRun.start = m_fields->currentRun.end;
}

void DTPlayLayer::destroyPlayer(PlayerObject* player, GameObject* p1) {
    PlayLayer::destroyPlayer(player, p1);

    if (!player->m_isDead) return;
 
    if (!m_fields->hasRespawned) return;
    m_fields->hasRespawned = false;

    if (DTPlayLayer::disableCompletedLevelTracking()) return;

    if (!m_level->isPlatformer())
        m_fields->currentRun.end = getActualProgress(this);

    m_fields->lastOneDied = true;

    if (m_fields->currentRun.start < 0)
        return;

    saveRun();
}

void DTPlayLayer::saveRun(){
    if (!Settings::getLateSaveEnabled()){
        if (m_fields->currentRun.start == 0 && !m_isPracticeMode){
            StatsManager::logDeath(m_fields->currentRun.end);
        }
        else if (!m_isPracticeMode && m_level->isPlatformer()){
            StatsManager::logDeath(m_fields->currentRun.end);
        }

        else if (m_level->isPlatformer() && m_isPracticeMode){
            StatsManager::logRun(m_fields->currentRun);
        }
        else{
            StatsManager::logRun(m_fields->currentRun);
        }
    }
    else{
        if (m_fields->currentRun.start == 0 && !m_isPracticeMode){
            m_fields->fzeroToSave.push_back(m_fields->currentRun.end);
        }
        else if (!m_isPracticeMode && m_level->isPlatformer()){
            m_fields->fzeroToSave.push_back(m_fields->currentRun.end);
        }

        else if (m_level->isPlatformer() && m_isPracticeMode){
            m_fields->runsToSave.push_back(m_fields->currentRun);
        }
        else{
            m_fields->runsToSave.push_back(m_fields->currentRun);
        }
    }
}

void DTPlayLayer::levelComplete() {
    PlayLayer::levelComplete();

    // same as PlayLayer::destroyPlayer()
    if (!m_fields->hasRespawned) return;
    m_fields->hasRespawned = false;

    // disable tracking deaths on completed levels
    if (DTPlayLayer::disableCompletedLevelTracking()) return;

    if (!m_level->isPlatformer())
        m_fields->currentRun.end = 100;

    // log::info("PlayLayer::levelComplete()\ncurrentRun.start = {}\ncurrentRun.end = {}\nplatformer = {}",
    //     m_fields->currentRun.start,
    //     m_fields->currentRun.end,
    //     m_level->isPlatformer()
    // );

    m_fields->lastOneDied = true;

    if (m_fields->currentRun.start < 0)
        return;

    if (m_level->isPlatformer())
        m_fields->currentRun.end++;
        
    saveRun();
}

void DTPlayLayer::removeAllCheckpoints() {
    PlayLayer::removeAllCheckpoints();

    m_fields->currentRun.end = 0;
}

void DTPlayLayer::resume() {
    // if remove pauses is on
    DTPlayLayer::startPlaytime();
    PlayLayer::resume();
}

void DTPlayLayer::pauseGame(bool unfocused) {


    // if remove pauses
    DTPlayLayer::endPlaytime();

    PlayLayer::pauseGame(unfocused);
}

void DTPlayLayer::onQuit() {
    if (m_fields->fzeroToSave.size()){
        StatsManager::logDeaths(m_fields->fzeroToSave);
    }

    if (m_fields->runsToSave.size()){
        StatsManager::logRuns(m_fields->runsToSave);
    }

    DTPlayLayer::endPlaytime();

    auto session = StatsManager::getCurrentSession();
    session->playtime.insert(session->playtime.end(), m_fields->sessionPlaytime.begin(), m_fields->sessionPlaytime.end());
    (void)StatsManager::setSession(*session, m_level, session->sessionStartDate, false);
    StatsManager::currentFrom0.playtime.insert(StatsManager::currentFrom0.playtime.end(), m_fields->sessionPlaytime.begin(), m_fields->sessionPlaytime.end());
    (void)StatsManager::setGeneral(StatsManager::currentFrom0, m_level);


    // log::info("PlayLayer::onQuit()");

    // schedule session gets reset
    // this cancels creating a new session
    // if they back out before dying
    StatsManager::scheduleCreateNewSession(false);
    DTPlayLayer::updateSessionLastPlayed();

    StatsManager::setCurrentLevel(nullptr);

    PlayLayer::onQuit();
}

//tysm eclips menu ur awesome and mega goated :fire:
float DTPlayLayer::getActualProgress(GJBaseGameLayer* game) {
    float percent;
    if (game->m_level->m_timestamp > 0) {
        percent = static_cast<float>(game->m_gameState.m_levelTime * 240.f) / game->m_level->m_timestamp * 100.f;
        log::info("2.2 per, {}", percent);
    } else {
        percent = game->m_player1->getPositionX() / game->m_levelLength * 100.f;
        log::info("2.1 per, {}", percent);
    }
    return std::clamp(percent, 0.f, 100.f);
}
