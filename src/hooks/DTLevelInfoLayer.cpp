#include "../hooks/DTLevelInfoLayer.hpp"
#include "../utils/CCShowHide.hpp"

bool DTLevelInfoLayer::init(GJGameLevel* p0, bool p1){
    if (!LevelInfoLayer::init(p0, p1)) return false;

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    if (!Settings::getLeftMenuEnabled())
        s->setScale(0.75f);
    m_fields->btn = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTLevelInfoLayer::openDTLayer)
    );
    m_fields->btn->setID("dt-skull-button");
    m_fields->btn->setZOrder(1);
    m_fields->btn->setVisible(false);

    if (Settings::getLeftMenuEnabled()){
        auto leftSideMenu = getChildByID("left-side-menu");
        leftSideMenu->addChild(m_fields->btn);
        leftSideMenu->updateLayout();
    }
    else{
        auto otherMenu = getChildByID("other-menu");
        auto settingsMenu = getChildByID("settings-menu");

        otherMenu->addChild(m_fields->btn);
        if (otherMenu->getChildByID("favorite-button")->isVisible())
            m_fields->btn->setPosition({otherMenu->getChildByID("favorite-button")->getPositionX(), settingsMenu->getChildByID("settings-button")->getPositionY()});
        else
            m_fields->btn->setPosition({otherMenu->getChildByID("favorite-button")->getPosition()});
        otherMenu->updateLayout();
    }

    auto statsRes = StatsManager::getLevelStats(p0, false);
    if (statsRes.isOk() || statsRes.unwrapErr()[0] == '0'){
        auto stats = statsRes.unwrapOrDefault();
        stats.attempts = p0->m_attempts;
        stats.levelName = p0->m_levelName;
        stats.difficulty = StatsManager::getDifficulty(p0);

        StatsManager::setLevelStats(stats, p0, false);
        StatsManager::setLevelStats(stats, p0, true);
    }
    else{
        Notification::create("Failed to load Deaths json.", CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png"))->show();
    }

    schedule(schedule_selector(DTLevelInfoLayer::checkIfPlayVisible));

    return true;
}

void DTLevelInfoLayer::checkIfPlayVisible(float delta){
    if (this->m_playBtnMenu->isVisible()){
        m_fields->btn->setVisible(true);

        auto statsRes = StatsManager::getLevelStats(this->m_level, false);
        if (statsRes.isOk()){
            auto stats = statsRes.unwrap();
            stats.difficulty = StatsManager::getDifficulty(this->m_level);

            StatsManager::setLevelStats(stats, this->m_level, false);
        }
        unschedule(schedule_selector(DTLevelInfoLayer::checkIfPlayVisible));
    }
}

void DTLevelInfoLayer::openDTLayer(CCObject*){
    auto dtLayer = DTLayer::create(this->m_level);
    dtLayer->show();
}