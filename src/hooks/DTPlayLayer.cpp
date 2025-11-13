#include "../hooks/DTPlayLayer.hpp"

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
    auto _ = self.setHookPriority("PlayLayer::levelComplete", -9999);
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
        log::info("{}", metaRes.unwrapErr());
    }

    if (metaRes.isOk()){
        auto metadata = metaRes.unwrap();
        metadata.levelName = level->m_levelName;
        metadata.attempts = level->m_attempts;
        metadata.difficulty = StatsManager::getDifficulty(level);
        auto _ = StatsManager::setMetadata(metadata, level);
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
        log::info("first session created");
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
    return true;
}

#if defined(GEODE_IS_MACOS)
void DTPlayLayer::checkpointActivated(CheckpointGameObject* checkpt) {
    PlayLayer::checkpointActivated(checkpt);

    m_fields->currentRun.end++;
}
#endif

void DTPlayLayer::resetLevel() {
    PlayLayer::resetLevel();
    // log::info("PlayLayer::resetLevel()");

    m_fields->hasRespawned = true;

    if (!m_level->isPlatformer())
        m_fields->currentRun.start = static_cast<int>(this->getCurrentPercent());
    else
        m_fields->currentRun.start = m_fields->currentRun.end;
}

void DTPlayLayer::destroyPlayer(PlayerObject* player, GameObject* p1) {
    PlayLayer::destroyPlayer(player, p1);

    if (!player->m_isDead) return;

    // just in case some mod accidentally calls
    // PlayLayer::destroyPlayer() twice
    if (!m_fields->hasRespawned) return;
    m_fields->hasRespawned = false;

    // disable tracking deaths on completed levels
    if (DTPlayLayer::disableCompletedLevelTracking()) return;

    if (!m_level->isPlatformer())
        m_fields->currentRun.end = getActualProgress(this);

    // log::info("PlayLayer::destroyPlayer()\ncurrentRun.start = {}\ncurrentRun.end = {}\nplatformer = {}",
    //     m_fields->currentRun.start,
    //     m_fields->currentRun.end,
    //     m_level->isPlatformer()
    // );

    if (m_fields->currentRun.start < 0)
        return;

    if (!Settings::getLateSaveEnabled()){
        // log deaths from 0 in normal mode
        if (m_fields->currentRun.start == 0 && !m_isPracticeMode)
            StatsManager::logDeath(m_fields->currentRun.end);
        else if (!m_isPracticeMode && m_level->isPlatformer())
            StatsManager::logDeath(m_fields->currentRun.end);

        // anything else is a run
        // platformer runs only from 0
        else if (m_level->isPlatformer() && m_isPracticeMode)
            StatsManager::logRun(m_fields->currentRun);
        else
            StatsManager::logRun(m_fields->currentRun);
    }
    else{
        // log deaths from 0 in normal mode
        if (m_fields->currentRun.start == 0 && !m_isPracticeMode)
        m_fields->fzeroToSave.push_back(m_fields->currentRun.end);
        else if (!m_isPracticeMode && m_level->isPlatformer())
            m_fields->fzeroToSave.push_back(m_fields->currentRun.end);

        // anything else is a run
        // platformer runs only from 0
        else if (m_level->isPlatformer() && m_isPracticeMode)
            m_fields->runsToSave.push_back(m_fields->currentRun);
        else
            m_fields->runsToSave.push_back(m_fields->currentRun);
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

    if (m_fields->currentRun.start < 0)
        return;

    if (m_level->isPlatformer())
        m_fields->currentRun.end++;
        
    if (!Settings::getLateSaveEnabled()){
        if (m_fields->currentRun.start == 0)
            StatsManager::logDeath(m_fields->currentRun.end);
        else
            StatsManager::logRun(m_fields->currentRun);
    }
    else{
        if (m_fields->currentRun.start == 0)
            m_fields->fzeroToSave.push_back(m_fields->currentRun.end);
        else
            m_fields->runsToSave.push_back(m_fields->currentRun);
    }            
}

void DTPlayLayer::removeAllCheckpoints() {
    PlayLayer::removeAllCheckpoints();

    m_fields->currentRun.end = 0;
}

void DTPlayLayer::onQuit() {
    if (m_fields->fzeroToSave.size()){
        StatsManager::logDeaths(m_fields->fzeroToSave);
    }

    if (m_fields->runsToSave.size()){
        StatsManager::logRuns(m_fields->runsToSave);
    }

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
    } else {
        percent = game->m_player1->getPositionX() / game->m_levelLength * 100.f;
    }
    return std::clamp(percent, 0.f, 100.f);
}
