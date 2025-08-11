#include <nodes/layers/DTLayer.hpp>

#include <nodes/layers/DTGraphLayer.hpp>
#include <nodes/layers/DTLevelSpecificSettingsLayer.hpp>

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

    m_MyLevelStats = StatsManager::getLevelStats(m_Level, false);
    if (m_MyLevelStats.isErr() && m_MyLevelStats.unwrapErr() == "No stats exist for level!"){
        LevelStats newStats;
        m_MyLevelStats = Ok(newStats);
    }
    else if (m_MyLevelStats.isErr())
        geode::Notification::create(fmt::format("Failed to load DT level data! {}", m_MyLevelStats.unwrapErr()), NotificationIcon::Error)->show();
    
    if (m_MyLevelStats.isOk()){
        auto stats = m_MyLevelStats.unwrap();
        stats.levelName = level->m_levelName;
        stats.attempts = level->m_attempts;
        stats.difficulty = StatsManager::getDifficulty(level);
        StatsManager::setLevelStats(stats, level, false);
        m_MyLevelStats = Ok(stats);
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

    auto viewState = Save::getLastViewState();
    if (viewState.pos.x != -100000){
        scrollLayer->moveTo(viewState.pos);
        scrollLayer->zoomTo(viewState.zoom);
    }

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

    int sessionAmount = 0;
    if (m_SharedLevelStats.isOk())
        sessionAmount = m_SharedLevelStats.unwrap().sessions.size();

    sessionSelector = SessionSelector::create(sessionAmount);
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
    // if (m_MyLevelStats.currentBest != -1)
    //     StatsManager::setLevelStats(m_MyLevelStats, m_Level, false);
    //DTLayer::refreshAll();
}

void DTLayer::graphBtnClicked(CCObject*){
    auto graph = DTGraphLayer::create(this);
    graph->setZOrder(100);
    this->addChild(graph);
}

void DTLayer::UpdateSharedStats(){
    if (m_MyLevelStats.isErr()){
        m_SharedLevelStats = Err("Failed to create shared stats!");
        return;
    }
    auto sharedStats = m_MyLevelStats.unwrap();

    std::vector<std::string> linkedLevels{};
    linkedLevels.insert(linkedLevels.end(), sharedStats.LinkedLevels.begin(), sharedStats.LinkedLevels.end());

    for (int i = 0; i < linkedLevels.size(); i++)
    {
        auto currStatsRes = StatsManager::getLevelStats(linkedLevels[i], false);
        if (currStatsRes.isErr()){
            Notification::create(fmt::format("failed to get data for linked level - {} | {}", linkedLevels[i], currStatsRes.unwrapErr()))->show();
            continue;
        }
        auto currStats = currStatsRes.unwrap();

        linkedLevels.insert(linkedLevels.end(), currStats.LinkedLevels.begin(), currStats.LinkedLevels.end());

        sharedStats.CombineStats(currStats);
    }

    m_SharedLevelStats = Ok(sharedStats);
}

void DTLayer::onSettings(CCObject*){
    geode::openSettingsPopup(Mod::get());
}

void DTLayer::keyBackClicked(){
    DTLayer::onClose(nullptr);
}

void DTLayer::onClose(CCObject* sender){
    ViewState state;
    state.pos = scrollLayer->content->getPosition();
    state.zoom = scrollLayer->getCurrentZoom();

    Save::setLastViewState(state);

    if (layoutTopbar != nullptr){
        layoutTopbar->keyBackClicked();
        return;
    }
    Popup<GJGameLevel* const&>::onClose(sender);
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
    DTLevelSpecificSettingsLayer::create()->show();
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

void DTLayer::saveAndUpdateStats(bool updateShared){
    if (m_MyLevelStats.isErr()) return;
    
    auto& levelStats = m_MyLevelStats.unwrap();
    StatsManager::setLevelStats(levelStats, m_Level, false);
    if (updateShared)
        UpdateSharedStats();
}

void DTLayer::UpdateOnAllShared(const std::function<void(LevelStats& stats)>& lambda){
    if (m_MyLevelStats.isErr() || m_SharedLevelStats.isErr() || lambda == NULL) return;

    auto& sharedStats = m_SharedLevelStats.unwrap();

    for (const auto& linkedLevel : sharedStats.LinkedLevels)
    {
        auto statsRes = StatsManager::getLevelStats(linkedLevel, false);
        if (statsRes.isErr()) continue;
        auto currentStats = statsRes.unwrap();

        lambda(currentStats);

        StatsManager::setLevelStats(currentStats, linkedLevel, false);
    }
    
    auto& myStats = m_MyLevelStats.unwrap();
    lambda(myStats);
    lambda(sharedStats);
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