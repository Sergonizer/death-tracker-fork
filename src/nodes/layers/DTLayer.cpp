#include <nodes/layers/DTLayer.hpp>

#include <nodes/layers/DTGraphLayer.hpp>
#include <nodes/layers/DTLinkLayer.hpp>

#include <Geode/ui/GeodeUI.hpp>

DTLayer* DTLayer::instance = nullptr;

DTLayer* DTLayer::create(GJGameLevel* const& Level) {
    auto ret = new DTLayer();
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (ret && ret->initAnchored(winSize.width - 30, winSize.height - 30, Level, "geode.loader/GE_square01.png")) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTLayer::setup(GJGameLevel* const& level) {
    //create trackerLayer
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_Level = level;

    // ================================== //
    // loading data

    auto lvlStatsRes = StatsManager::getLevelStats(m_Level, false);
    m_MyLevelStats = lvlStatsRes.unwrapOrDefault();
    if (lvlStatsRes.isErr()) {
        StatsManager::setLevelStats(m_MyLevelStats, m_Level, false);
    }

    StatsManager::setCurrentLogLevel(level);

    DTLayer::UpdateSharedStats();
    // ================================== //

    /*
     * main page
    */

    instance = this;

    this->setID("dt-layer");
    this->setZOrder(100);

    if (Save::getLastOpenedVersion() != Mod::get()->getVersion().toNonVString()){
        Save::setLastOpenedVersion(Mod::get()->getVersion().toNonVString());
        FLAlertLayer::create(nullptr, fmt::format("Death Tracker {} Changelog", Mod::get()->getVersion().toVString()).c_str(), fmt::format(
            "{}",
            "- <cg>iOS support</c>"
        ), "OK", nullptr, 415, false, 200, 0.75f)->show();
    }

    float height = 60;

    scrollLayer = AdvancedScrollLayer::create({m_size.width - 30, m_size.height - height}, {5000, 5000});
    scrollLayer->drawGrid(50, .5f, ccColor4B{ 143, 143, 143, 255 });
    scrollLayer->setPosition(m_size / 2 - scrollLayer->getContentSize() / 2 + ccp(0, height / 4));
    scrollLayer->setZOrder(2);
    scrollLayer->maxZoom = 0.35f;
    m_mainLayer->addChild(scrollLayer);

    std::vector<CCPoint> points{
        scrollLayer->getPosition(),
        scrollLayer->getPosition() + ccp(scrollLayer->getContentSize().width, 0),

        scrollLayer->getPosition() + ccp(scrollLayer->getContentSize().width, 0),
        scrollLayer->getPosition() + scrollLayer->getContentSize(),

        scrollLayer->getPosition() + scrollLayer->getContentSize(),
        scrollLayer->getPosition() + ccp(0, scrollLayer->getContentSize().height),

        scrollLayer->getPosition() + ccp(0, scrollLayer->getContentSize().height),
        scrollLayer->getPosition(),
    };

    auto outline = CCDrawNode::create();
    outline->m_bUseArea = false;
    outline->drawLines(&points[0], points.size(), .25f, ccc4FFromccc4B({ 143, 143, 143, 255 }));
    outline->setZOrder(2);
    m_mainLayer->addChild(outline);

    auto shadow = CCScale9Sprite::create("square.png");
    shadow->setContentSize(scrollLayer->getContentSize());
    shadow->setAnchorPoint({0, 0});
    shadow->setPosition(scrollLayer->getPosition());
    shadow->setColor({0, 0, 0});
    shadow->setOpacity(100);
    m_mainLayer->addChild(shadow);

    auto bottomMenu = CCMenu::create();
    bottomMenu->setContentSize({m_size.width, height / 2.5f});
    bottomMenu->setAnchorPoint({.5f, .5f});
    bottomMenu->setPosition(bottomMenu->getContentSize() / 2 + ccp(0, 7.5f));
    bottomMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(50)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
    );
    bottomMenu->setID("bottom-menu");
    m_mainLayer->addChild(bottomMenu);

    auto levelSpecificOptionsSpr = CCSprite::createWithSpriteFrameName("GJ_creatorBtn_001.png");
    auto levelSpecificOptionsBtn = CCMenuItemSpriteExtra::create(
        levelSpecificOptionsSpr,
        this,
        menu_selector(DTLayer::onLSOClicked)
    );
    bottomMenu->addChild(levelSpecificOptionsBtn);

    sessionSelector = SessionSelector::create(m_SharedLevelStats.sessions.size());
    sessionSelector->setCallback([&](int newSession){
        log::info("chosen new session {}", newSession);
    });
    bottomMenu->addChild(sessionSelector);

    auto graphBtnSpr = CCSprite::createWithSpriteFrameName("graph_button.png"_spr);
    auto graphBtn = CCMenuItemSpriteExtra::create(
        graphBtnSpr,
        this,
        menu_selector(DTLayer::graphBtnClicked)
    );
    bottomMenu->addChild(graphBtn);

    bottomMenu->updateLayout();

    auto settingsBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsBtnSpr->setScale(.75f);
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSpr,
        this,
        menu_selector(DTLayer::onSettings)
    );
    settingsBtn->setPosition({m_size.width - 3.f, 3.f});
    m_buttonMenu->addChild(settingsBtn);

    auto layout = Save::getLayout();

    for (const auto& labelInfo : layout)
    {
        createLabel(labelInfo);
    }

    auto editLayoutMenu = CCMenu::create();
    editLayoutMenu->setPosition({0, 0});
    editLayoutMenu->setID("edit-layout-menu");
    editLayoutMenu->setZOrder(3);
    m_mainLayer->addChild(editLayoutMenu);

    auto editLayoutBtnSprBG = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto editLayoutBtnSpr = CCSprite::createWithSpriteFrameName("layout_button.png"_spr);
    editLayoutBtnSpr->setPosition(editLayoutBtnSprBG->getContentSize() / 2);
    editLayoutBtnSprBG->addChild(editLayoutBtnSpr);
    editLayoutBtnSprBG->setScale(.75f);
    auto editLayoutBtn = CCMenuItemSpriteExtra::create(
        editLayoutBtnSprBG,
        this,
        menu_selector(DTLayer::onEditLayout)
    );
    editLayoutBtn->setPosition(scrollLayer->getPosition() + scrollLayer->getContentSize());
    editLayoutMenu->addChild(editLayoutBtn);

    CCTouchDispatcher::get()->removeDelegate(scrollLayer);

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    return true;
}

void DTLayer::onEditLayout(CCObject*){
    if (layoutTopbar != nullptr) return;

    for (const auto& label : labels)
        label->enterEditMode();

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    static_cast<CCMenu*>(m_mainLayer->getChildByID("bottom-menu"))->setEnabled(false);
    m_buttonMenu->setEnabled(false);
    m_buttonMenu->setOpacity(100);
    sessionSelector->setEnabled(false);
    
    layoutTopbar = EditLayoutTopbar::create();
    layoutTopbar->setZOrder(10);
    layoutTopbar->setPosition({
        (winSize.width - layoutTopbar->getContentWidth()) / 2,
        10
    });
    layoutTopbar->onExit = [&](bool didApply){
        if (!didApply)
            for (const auto& label : labels)
                label->revert();

        std::vector<DTLabelInfo> newInfo{};

        for (const auto& label : labels){
            label->exitEditMode();
            if (didApply && labels.contains(label))
                newInfo.push_back(label->labelInfo);
        }

        if (didApply)
            Save::setLayout(newInfo);

        layoutTopbar = nullptr;

        static_cast<CCMenu*>(m_mainLayer->getChildByID("bottom-menu"))->setEnabled(true);
        m_buttonMenu->setEnabled(true);
        m_buttonMenu->setOpacity(255);
        sessionSelector->setEnabled(true);
    };
    this->addChild(layoutTopbar);
}

bool DTLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent){
    bool doMoveScroll = true;
    if (layoutTopbar != nullptr){
        for (const auto& label : labels)
        {
            if (label->touchMoved(pTouch))
                doMoveScroll = false;
        }
    }

    if (doMoveScroll)
        scrollLayer->ccTouchBegan(pTouch, pEvent);

    return true;
}

void DTLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent){
    bool doMoveScroll = true;
    if (layoutTopbar != nullptr){
        for (const auto& label : labels)
        {
            if (label->touchMoved(pTouch))
                doMoveScroll = false;
        }
    }

    if (doMoveScroll)
        scrollLayer->ccTouchMoved(pTouch, pEvent);
}

void DTLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent){
    scrollLayer->ccTouchEnded(pTouch, pEvent);
    if (layoutTopbar != nullptr){
        for (const auto& label : labels)
        {
            label->touchEnded(pTouch);
        }
    }
}

void DTLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent){
    scrollLayer->ccTouchCancelled(pTouch, pEvent);
    if (layoutTopbar != nullptr){
        for (const auto& label : labels)
        {
            label->touchEnded(pTouch);
        }
    }
}

void DTLayer::ccTouchesMoved(CCSet* touches, CCEvent* event){
    scrollLayer->ccTouchesMoved(touches, event);
}

void DTLayer::updateRunsAllowed(){
    if (m_MyLevelStats.currentBest != -1)
        StatsManager::setLevelStats(m_MyLevelStats, m_Level, false);
    //DTLayer::refreshAll();
}

void DTLayer::graphBtnClicked(CCObject*){
    auto graph = DTGraphLayer::create(this);
    graph->setZOrder(100);
    this->addChild(graph);
}

void DTLayer::UpdateSharedStats(){
    m_SharedLevelStats = m_MyLevelStats;

    for (int i = 0; i < m_MyLevelStats.LinkedLevels.size(); i++)
    {
        auto currStats = StatsManager::getLevelStats(m_MyLevelStats.LinkedLevels[i], false).unwrapOrDefault();
        if (currStats.levelName == "Unknown name"){
            Notification::create("failed to get data for linked level - " + m_MyLevelStats.LinkedLevels[i])->show();
            continue;
        }

        m_SharedLevelStats.attempts += currStats.attempts;

        m_SharedLevelStats.sessions.reserve(m_SharedLevelStats.sessions.size() + std::distance(currStats.sessions.begin(),currStats.sessions.end()));
        m_SharedLevelStats.sessions.insert(m_SharedLevelStats.sessions.end(),currStats.sessions.begin(),currStats.sessions.end());

        for (int r = 0; r < m_MyLevelStats.RunsToSave.size(); r++)
        {
            bool addMeRun = true;
            for (int r2 = 0; r2 < currStats.RunsToSave.size(); r2++)
            {
                if (currStats.RunsToSave[r2] == m_MyLevelStats.RunsToSave[r])
                    addMeRun = false;
            }
            
            if (addMeRun)
                currStats.RunsToSave.push_back(m_MyLevelStats.RunsToSave[r]);
        }
        for (int r = 0; r < currStats.RunsToSave.size(); r++)
        {
            bool addMeRun = true;
            for (int r2 = 0; r2 < m_MyLevelStats.RunsToSave.size(); r2++)
            {
                if (m_MyLevelStats.RunsToSave[r2] == currStats.RunsToSave[r])
                    addMeRun = false;
            }
            
            if (addMeRun)
                m_MyLevelStats.RunsToSave.push_back(currStats.RunsToSave[r]);
        }

        std::ranges::sort(m_MyLevelStats.RunsToSave, [](const int a, const int b) {
            return a < b;
        });

        std::ranges::sort(currStats.RunsToSave, [](const int a, const int b) {
            return a < b;
        });

        if (m_MyLevelStats.currentBest != -1)
            StatsManager::setLevelStats(m_MyLevelStats, m_Level, false);
        if (currStats.currentBest != -1)
            StatsManager::setLevelStats(currStats, m_MyLevelStats.LinkedLevels[i], false);
        
        for (const auto& [death, count] : currStats.deaths)
        {
            m_SharedLevelStats.deaths[death] += count;
        }
        
        for (const auto& [run, count] : currStats.runs)
        {
            m_SharedLevelStats.runs[run] += count;
        }

        if (m_SharedLevelStats.currentBest < currStats.currentBest)
            m_SharedLevelStats.currentBest = currStats.currentBest;

        m_SharedLevelStats.newBests.insert(currStats.newBests.begin(), currStats.newBests.end());
    }

    std::ranges::sort(m_SharedLevelStats.sessions, [](const Session a, const Session b) {
        return a.lastPlayed > b.lastPlayed;
    });
}

void DTLayer::onSettings(CCObject*){
    geode::openSettingsPopup(Mod::get());
}

void DTLayer::keyBackClicked(){
    if (layoutTopbar != nullptr){
        layoutTopbar->keyBackClicked();
        return;
    }
    this->removeMeAndCleanup();
    instance = nullptr;
}

void DTLayer::show(){
    Popup<GJGameLevel* const&>::show();

    m_mainLayer->stopAllActions();
    m_mainLayer->setScale(1);
}

void DTLayer::keyDown(enumKeyCodes key){
    scrollLayer->keyDown(key);
}
void DTLayer::keyUp(enumKeyCodes key){
    scrollLayer->keyUp(key);
}

void DTLayer::onLSOClicked(CCObject*){

}

DTLayer* DTLayer::get() { return instance; }

DTLabel* DTLayer::createLabel(DTLabelInfo info){
    auto label = DTLabel::create(info, 50 / 10);
    scrollLayer->content->addChild(label);
    labels.insert(label);
    label->onClicked = [&](DTLabel* clickedLabel){
        if (layoutTopbar == nullptr || !labels.contains(clickedLabel)) return;

        layoutTopbar->setTarget(clickedLabel);
    };

    if (layoutTopbar != nullptr){
        label->enterEditMode();
        label->wasCreatedThisEdit = true;
        label->onSelected(nullptr);
    }

    return label;
}

void DTLayer::removeLabel(DTLabel* label, bool forceDelete){
    if (!labels.contains(label)) return;

    if (forceDelete || layoutTopbar == nullptr){
        label->removeMeAndCleanup();
        labels.erase(label);
    }
    else if (layoutTopbar != nullptr){
        label->softDelete();
    }
}

AdvancedScrollLayer* DTLayer::getScrollLayer(){
    return scrollLayer;
}

//better info time calc

uint64_t DTLayer::timeInMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string DTLayer::decodeBase64Gzip(const std::string& input) {
    return ZipUtils::decompressString(input, false, 0);
}

inline bool objectIDIsSpeedPortal(int id) {
    return (id == 200 || id == 201 || id == 202 || id == 203 || id == 1334);
}

inline int speedToPortalId(int speed) {
    switch(speed) {
    default:
        return 201;
        break;
    case 1:
        return 200;
        break;
    case 2:
        return 202;
        break;
    case 3:
        return 203;
        break;
    case 4:
        return 1334;
        break;
    }
}

inline float travelForPortalId(int speed) {
    switch (speed)
    {
    case 200:
        return 251.16008f;
        break;
    default:
        return 311.58011f;
        break;
    case 202:
        return 387.42014f;
        break;
    case 203:
        return 468.00015f;
        break;
    case 1334:
        return 576.00018f;
        break;
    }
}

float DTLayer::timeForLevelString(const std::string& levelString) {
    try {
        auto a = timeInMs();

        auto decompressString = decodeBase64Gzip(levelString);
        auto c = timeInMs();
        std::stringstream responseStream(decompressString);
        std::string currentObject;
        std::string currentKey;
        std::string keyID;

        float prevPortalX = 0;
        int prevPortalId = 0;

        float timeFull = 0;

        float maxPos = 0;
        while(getline(responseStream, currentObject, ';')){
            size_t i = 0;
            int objID = 0;
            float xPos = 0;
            bool checked = false;

            std::stringstream objectStream(currentObject);
            while(getline(objectStream, currentKey, ',')) {
                if(i % 2 == 0) keyID = currentKey;
                else {
                    if(keyID == "1") objID = geode::utils::numFromString<int>(currentKey).unwrapOr(0);
                    else if(keyID == "2") xPos = geode::utils::numFromString<float>(currentKey).unwrapOr(0);
                    else if(keyID == "13") checked = geode::utils::numFromString<int>(currentKey).unwrapOr(0);
                    else if(keyID == "kA4") prevPortalId = speedToPortalId(geode::utils::numFromString<int>(currentKey).unwrapOr(0));
                }
                i++;

                if(xPos != 0 && objID != 0 && checked == true) break;
            }

            if(maxPos < xPos) maxPos = xPos;
            if(!checked || !objectIDIsSpeedPortal(objID)) continue;

            timeFull += (xPos - prevPortalX) / travelForPortalId(prevPortalId);
            prevPortalId = objID;
            prevPortalX = xPos;
        }

        timeFull += (maxPos - prevPortalX) / travelForPortalId(prevPortalId);
        auto b = timeInMs() - a;
        return timeFull;
    } catch(std::exception e) {
        log::error("An exception has occured while calculating time for levelString: {}", e.what());
        return 0;
    }
}