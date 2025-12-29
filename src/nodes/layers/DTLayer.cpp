#include <nodes/layers/DTLayer.hpp>

#include <nodes/layers/DTGraphLayer.hpp>
#include <nodes/layers/DTLevelSpecificSettingsLayer.hpp>

#include <Geode/ui/GeodeUI.hpp>
#include <regex>
#include <utils/CCResizeWidthTo.hpp>

float DTLayer::transitionTime = .35f;

bool ColumnComperator::operator()(LayoutColumn* a, LayoutColumn* b) const {
    return a->info.orderPos < b->info.orderPos;
}

DTLayer* DTLayer::instance = nullptr;

DTLayer* DTLayer::create(GJGameLevel* const& Level) {
    auto ret = new DTLayer();
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (ret && ret->initAnchored(winSize.width - 150, winSize.height - 30, Level, "geode.loader/GE_square01.png")) {
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

    m_MyLevelStats = StatsManager::getLevelData(m_Level);
    if (m_MyLevelStats.isErr() && m_MyLevelStats.unwrapErr().size() && m_MyLevelStats.unwrapErr()[0] == '1'){
        LevelData newData;
        newData.levelKey = StatsManager::getLevelKey(level).unwrap();
        m_MyLevelStats = Ok(newData);
    }
    else if (m_MyLevelStats.isErr()){
        auto notif = geode::Notification::create(fmt::format("Failed to load DT level data! {}", m_MyLevelStats.unwrapErr()), NotificationIcon::Error, 3);
        notif->show();
        notif->setZOrder(101);
    }
    
    if (m_MyLevelStats.isOk()){
        auto stats = m_MyLevelStats.unwrap();
        stats.metadata.levelName = level->m_levelName;
        stats.metadata.attempts = level->m_attempts;
        stats.metadata.difficulty = StatsManager::getDifficulty(level);
        auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);
        m_MyLevelStats = Ok(stats);
    }

    StatsManager::setCurrentLevel(level);

    DTLayer::UpdateSharedStats();
    // ================================== //

    /*
     * main page
    */

    instance = this;

    this->setID("dt-layer");

    if (Save::getLastOpenedVersion() != Mod::get()->getVersion().toNonVString()){
        Save::setLastOpenedVersion(Mod::get()->getVersion().toNonVString());
        FLAlertLayer::create(nullptr, fmt::format("Death Tracker {} Changelog", Mod::get()->getVersion().toVString()).c_str(), fmt::format(
            "{}",
            "- <cg>iOS support</c>"
        ), "OK", nullptr, 415, false, 200, 0.75f)->show();
    }

    float height = 60;
    ogLimits = CCSize{m_size.width - 30 + 1, m_size.height - height  + 1};
    scrollLayer = AdvancedScrollLayer::create({m_size.width - 30, m_size.height - height}, ogLimits);
    //scrollLayer->drawGrid(50, .5f, ccColor4B{ 143, 143, 143, 255 });
    scrollLayer->setPosition(m_size / 2 - scrollLayer->getContentSize() / 2 + ccp(0, height / 4));
    scrollLayer->setZOrder(2);
    scrollLayer->maxZoom = 0.05f;
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
    outline->drawLines(&points[0], points.size(), .5f, ccc4FFromccc4B({ 143, 143, 143, 255 }));
    outline->setZOrder(2);
    m_mainLayer->addChild(outline);

    auto shadow = CCScale9Sprite::create("square.png");
    shadow->setContentSize(scrollLayer->getContentSize());
    shadow->setAnchorPoint({0, 0});
    shadow->setPosition(scrollLayer->getPosition());
    shadow->setColor({0, 0, 0});
    shadow->setOpacity(100);
    m_mainLayer->addChild(shadow);

    bottomMenu = CCMenu::create();
    bottomMenu->setContentSize({m_size.width - 15, height / 2.5f});
    bottomMenu->setAnchorPoint({.5f, .5f});
    bottomMenu->setPosition(bottomMenu->getContentSize() / 2 + ccp(0, 7.5f));
    bottomMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(-15)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Even)
        ->setMinRelativeScale(std::nullopt)
        ->setMaxRelativeScale(std::nullopt)
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

    auto graphBtnSpr = CCSprite::createWithSpriteFrameName("graph_button.png"_spr);
    auto graphBtn = CCMenuItemSpriteExtra::create(
        graphBtnSpr,
        this,
        menu_selector(DTLayer::graphBtnClicked)
    );
    bottomMenu->addChild(graphBtn);

    sessionSelector = SessionSelector::create(sessionsOrder.size());
    sessionSelector->setCallback([&](int newSession){ onSessionSelected(newSession, true); });
    sessionSelector->setScale(.75f);
    sessionSelector->setPosition({m_size.width / 2, 20});
    m_mainLayer->addChild(sessionSelector);

    auto fixerNode = CCNode::create();
    fixerNode->setContentWidth(sessionSelector->getContentWidth() / 2);
    bottomMenu->addChild(fixerNode);

    onSessionSelected(1, false);

    auto editLayoutBtnSprBG = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    editLayoutBtnSpr = CCSprite::createWithSpriteFrameName("layout_button.png"_spr);
    editLayoutBtnSpr->setPosition(editLayoutBtnSprBG->getContentSize() / 2);
    editLayoutBtnSprBG->addChild(editLayoutBtnSpr);
    editLayoutBtnSprBG->setScale(.75f);
    auto editLayoutBtn = CCMenuItemSpriteExtra::create(
        editLayoutBtnSprBG,
        this,
        menu_selector(DTLayer::onEditLayout)
    );
    editLayoutBtn->setPosition(scrollLayer->getPosition() + scrollLayer->getContentSize());
    bottomMenu->addChild(editLayoutBtn);

    auto settingsBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsBtnSpr->setScale(.75f);
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSpr,
        this,
        menu_selector(DTLayer::onSettings)
    );
    settingsBtn->setPosition({m_size.width - 3.f, 3.f});
    bottomMenu->addChild(settingsBtn);

    columnHolder = CCMenu::create();
    columnHolder->setAnchorPoint({0, 1});
    columnHolder->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setMainAxisScaling(AxisScaling::Fit)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
        ->setCrossAxisAlignment(CrossAxisAlignment::Start)
    );
    columnHolder->setPosition({0, scrollLayer->content->getContentHeight()});
    scrollLayer->content->addChild(columnHolder);

    labelsHolder = CCNode::create();
    labelsHolder->setPosition(columnHolder->getPosition());
    scrollLayer->content->addChild(labelsHolder);

    setLayoutBy(Save::getLayout());

    auto addColumnButtonSpr = CCSprite::create("GJ_button_01.png");
    addColumnButtonSpr->setScale(.5f);
    addColumnButton = CCMenuItemSpriteExtra::create(
        addColumnButtonSpr,
        this,
        menu_selector(DTLayer::addColumnBtnClicked)
    );
    addColumnButton->setZOrder(1000);
    columnHolder->addChild(addColumnButton);
    addColumnButton->setOpacity(0);
    columnHolder->setEnabled(false);

    CCTouchDispatcher::get()->removeDelegate(scrollLayer);

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    populateSpecialStrings();

    this->organizeLayout();

    this->scheduleUpdate();

    bottomMenu->updateLayout();

    layoutOptionsLayer = LayoutOptionsLayer::create({150, m_size.height});
    layoutOptionsLayer->setPosition({winSize.width + 10, (winSize.height - layoutOptionsLayer->getContentHeight()) / 2});
    layoutOptionsLayer->onBackedOut = [&](){closeOptionsLayer();};
    this->addChild(layoutOptionsLayer);

    editLayoutMenu = CCMenu::create();
    editLayoutMenu->setEnabled(false);
    editLayoutMenu->setPosition({winSize.width / 2,160});
    editLayoutMenu->ignoreAnchorPointForPosition(false);
    editLayoutMenu->setAnchorPoint({0,0});
    m_mainLayer->addChild(editLayoutMenu);
    
    applyChangesButtonSpr = ButtonSprite::create("Apply Changes", "goldFont.fnt", "GJ_button_01.png");
    applyChangesButtonSpr->m_BGSprite->setOpacity(0);
    applyChangesButtonSpr->m_label->setOpacity(0);
    applyChangesButtonSpr->setScale(.5f);
    auto applyChangesButton = CCMenuItemSpriteExtra::create(
        applyChangesButtonSpr,
        this,
        menu_selector(DTLayer::onApplyLayoutChanges)
    );
    applyChangesButton->setPosition({41, -140});
    editLayoutMenu->addChild(applyChangesButton);
    
    discardChangesButtonSpr = ButtonSprite::create("Discard Changes", "goldFont.fnt", "GJ_button_06.png");
    discardChangesButtonSpr->m_BGSprite->setOpacity(0);
    discardChangesButtonSpr->m_label->setOpacity(0);
    discardChangesButtonSpr->setScale(.45f);
    auto discardChangesButton = CCMenuItemSpriteExtra::create(
        discardChangesButtonSpr,
        this,
        menu_selector(DTLayer::onDiscardLayoutChanges)
    );
    discardChangesButton->setPosition({-191, -140});
    editLayoutMenu->addChild(discardChangesButton);

    // tutorial test
    // auto dark = CCScale9Sprite::create("pixel.png");
    // dark->setOpacity(180);
    // dark->setPosition(ccp(winSize.width/2, winSize.height/2));
    // dark->setContentSize(winSize);
    // dark->setColor({0,0,0});
    // this->addChild(dark, 100);

    // ccBlendFunc bf = { GL_ONE, GL_ONE };

    // auto glowNode = CCMenu::create();
    // this->addChild(glowNode, 101);

    // auto lightSprite1 = CCSprite::createWithSpriteFrameName("d_gradient_c_02_001.png");
    // lightSprite1->setBlendFunc(bf);
    // lightSprite1->setAnchorPoint({1, 0});
    // glowNode->addChild(lightSprite1);

    // auto lightSprite2 = CCSprite::createWithSpriteFrameName("d_gradient_c_02_001.png");
    // lightSprite2->setBlendFunc(bf);
    // lightSprite2->setRotation(90);
    // lightSprite2->setAnchorPoint({1, 0});
    // glowNode->addChild(lightSprite2);

    // auto lightSprite3 = CCSprite::createWithSpriteFrameName("d_gradient_c_02_001.png");
    // lightSprite3->setBlendFunc(bf);
    // lightSprite3->setRotation(180);
    // lightSprite3->setAnchorPoint({1, 0});
    // glowNode->addChild(lightSprite3);

    // auto lightSprite4 = CCSprite::createWithSpriteFrameName("d_gradient_c_02_001.png");
    // lightSprite4->setBlendFunc(bf);
    // lightSprite4->setRotation(270);
    // lightSprite4->setAnchorPoint({1, 0});
    // glowNode->addChild(lightSprite4);

    
    return true;
}

void DTLayer::onEditLayout(CCObject*){
    isEditingLayout = true;

    std::set<DTLabel*> visitedLabels{};

    for (const auto& column : columns)
    {
        column->setVisibility(true);

        for (const auto& [_, label] : column->labels)
        {
            if (visitedLabels.contains(label)) continue;

            visitedLabels.insert(label);
            label->setEditable(true);
        }
    }

    bottomMenu->setEnabled(false);
    bottomMenu->stopAllActions();
    bottomMenu->runAction(CCFadeTo::create(.15f, 0));
    m_buttonMenu->setEnabled(false);
    m_buttonMenu->stopAllActions();
    m_buttonMenu->runAction(CCFadeTo::create(.15f, 0));
    editLayoutBtnSpr->stopAllActions();
    editLayoutBtnSpr->runAction(CCFadeTo::create(.15f, 0));
    
    editLayoutMenu->setEnabled(true);
    applyChangesButtonSpr->m_BGSprite->stopAllActions();
    applyChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 255));
    applyChangesButtonSpr->m_label->stopAllActions();
    applyChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 255));
    discardChangesButtonSpr->m_BGSprite->stopAllActions();
    discardChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 255));
    discardChangesButtonSpr->m_label->stopAllActions();
    discardChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 255));

    addColumnButton->stopAllActions();
    addColumnButton->runAction(CCFadeTo::create(.15f, 255));
    columnHolder->setEnabled(true);

    scrollLayer->moveBy(ccp(0, -LayoutColumn::topHeight));

    this->organizeLayout();
}

bool DTLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent){
    if (doMoveScroll)
        scrollLayer->ccTouchBegan(pTouch, pEvent);

    return true;
}

void DTLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent){
    if (doMoveScroll)
        scrollLayer->ccTouchMoved(pTouch, pEvent);
}

void DTLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent){
    scrollLayer->ccTouchEnded(pTouch, pEvent);
}

void DTLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent){
    scrollLayer->ccTouchCancelled(pTouch, pEvent);
}

void DTLayer::ccTouchesMoved(CCSet* touches, CCEvent* event){
    if (doMoveScroll)
        scrollLayer->ccTouchesMoved(touches, event);
}

void DTLayer::updateRunsAllowed(){
    // if (m_MyLevelStats.currentBest != -1)
    //     StatsManager::setLevelStats(m_MyLevelStats, m_Level, false);
    //DTLayer::refreshAll();
}

void DTLayer::graphBtnClicked(CCObject*){
    DTGraphLayer::create()->show();
}

void DTLayer::addSpecialString(const std::shared_ptr<SpecialKey>& key){
    key->setUpdateStartedCallback([this](const std::shared_ptr<SpecialKey>& k){ this->specialKeyUpdateStarted(k); });
    key->setUpdateCompletedCallback([this](const std::shared_ptr<SpecialKey>& k){ this->specialKeyUpdateCompleted(k); });
    specialStrings.emplace(key->getKey(), key);
    key->updateContent();
}

void DTLayer::populateSpecialStrings(){
    auto nlKey = std::make_shared<SpecialKey>("nl", "Adds a new line");
    nlKey->setUpdateFunction([&](){ return UpdateTask::immediate(Ok("\n"));});
    addSpecialString(nlKey);

    auto attemptsKey = std::make_shared<SpecialKey>("att", "Adds your geometry dash attempt count (shared with linked levels)");
    attemptsKey->setUpdateFunction([&](){ return UpdateTask::immediate(Ok(std::to_string(m_Level->m_attempts.value())));});
    addSpecialString(attemptsKey);

    auto levelNameKey = std::make_shared<SpecialKey>("lvln", "Adds the current levels name");
    levelNameKey->setUpdateFunction([&](){ return UpdateTask::immediate(Ok(std::string(m_Level->m_levelName)));});
    addSpecialString(levelNameKey);

    //f0
    auto from0Key = std::make_shared<SpecialKey>("f0", "Adds all your runs from 0% (shared with linked levels)");
    from0Key->setUpdateFunction([&](){ return UpdateTask::run([&](auto progress, auto hasBeenCancelled) -> UpdateTask::Result {
        if (m_MyLevelStats.isErr()) return Err("Failed to create from0 deaths string");
        auto& myStats = m_MyLevelStats.unwrap();
        if (myStats.from0.isErr()) return Err("Failed to create from0 deaths string");
        auto& myFrom0Stats = myStats.from0.unwrap();

        Deaths sharedDeaths = myFrom0Stats.deaths;
        NewBests sharedNBs = myFrom0Stats.newBests;

        for (const auto& levelData : linkedLevelsData)
        {
            if (levelData.from0.isErr()) continue;
            auto& levelFrom0Stats = levelData.from0.unwrap();

            sharedDeaths.insert(levelFrom0Stats.deaths.begin(), levelFrom0Stats.deaths.end());
            sharedNBs.insert(levelFrom0Stats.newBests.begin(), levelFrom0Stats.newBests.end());
        }

        std::string out;

        if (!createDeathsString(sharedDeaths, Save::getFrom0Customazations(), out, &sharedNBs, "{nbc}")) return Err("Failed to create from0 deaths string");

        return Ok(out);
    }, "Creating from0 deaths string");});
    addSpecialString(from0Key);

    //dtatt
    auto dtattKey = std::make_shared<SpecialKey>("dtatt", "Adds your death tracker attempt count (shared with linked levels)");
    dtattKey->setUpdateFunction([&](){ return UpdateTask::run([&](auto progress, auto hasBeenCancelled) -> UpdateTask::Result {
        if (m_MyLevelStats.isErr()) return Err("Failed to create death tracker attempts string");
        auto& myStats = m_MyLevelStats.unwrap();
        if (myStats.from0.isErr()) return Err("Failed to create death tracker attempts string");
        auto& myFrom0Stats = myStats.from0.unwrap();

        unsigned long long attempts = 0;

        auto deaths = [&attempts](const Deaths& deaths){
            for (const auto& [_, count] : deaths)
                attempts += count;
        };

        deaths(myFrom0Stats.deaths);

        for (const auto& levelData : linkedLevelsData)
        {
            if (levelData.from0.isErr()) continue;
            auto& levelFrom0Stats = levelData.from0.unwrap();

            deaths(levelFrom0Stats.deaths);
        }

        return Ok(std::to_string(attempts));
    }, "Creating death tracker attempts string");});
    addSpecialString(dtattKey);

    auto runsKey = std::make_shared<SpecialKey>("runs", "Adds all your runs from practice mode/start positions (shared with linked levels)");
    runsKey->setUpdateFunction([&](){ return UpdateTask::run([&](auto progress, auto hasBeenCancelled) -> UpdateTask::Result {
        if (m_MyLevelStats.isErr()) return Err("Failed to create run deaths string");
        auto& myStats = m_MyLevelStats.unwrap();
        if (myStats.from0.isErr()) return Err("Failed to create run deaths string");
        auto& myFrom0Stats = myStats.from0.unwrap();

        Deaths sharedRuns = myFrom0Stats.runs;

        for (const auto& levelData : linkedLevelsData)
        {
            if (levelData.from0.isErr()) continue;
            auto& levelFrom0Stats = levelData.from0.unwrap();
            
            sharedRuns.insert(levelFrom0Stats.runs.begin(), levelFrom0Stats.runs.end());
        }

        std::string out;

        if (!createDeathsString(sharedRuns, Save::getRunsCustomazations(), out)) return Err("Failed to create run deaths string");

        return Ok(out);
    }, "Creating runs deaths string");});
    addSpecialString(runsKey);

    auto sessionFrom0Key = std::make_shared<SpecialKey>("s0", "Adds all your runs on the selected session from 0");
    sessionFrom0Key->setUpdateFunction([&](){ return UpdateTask::run([&](auto progress, auto hasBeenCancelled) -> UpdateTask::Result {
        auto i = sessionSelector->getCurrentCount();

        if (i == 0 || i > sessionsOrder.size())
            return Err("Failed to create session from 0 deaths string");

        auto it = sessionsOrder.begin();
        std::advance(it, i - 1);
        
        auto levelKey = it->second;

        Result<Session> sessionRes = StatsManager::getSession(levelKey, it->first);
        if (sessionRes.isErr()) return Err("Failed to get session from0 deaths");

        std::string out;

        auto session = sessionRes.unwrap();

        if (!createDeathsString(session.deaths, Save::getSessionF0Customazations(), out, &session.newBests, "{sbc}")) return Err("Failed to create session from0 deaths string");

        return Ok(out);
    }, "Creating session from0 deaths string");});
    addSpecialString(sessionFrom0Key);

    auto sessionRuns0Key = std::make_shared<SpecialKey>("sruns", "Adds all your runs on the selected session");
    sessionRuns0Key->setUpdateFunction([&](){ return UpdateTask::run([&](auto progress, auto hasBeenCancelled) -> UpdateTask::Result {
        auto i = sessionSelector->getCurrentCount();

        if (i == 0 || i > sessionsOrder.size())
            return Err("Failed to create session run deaths string");

        auto it = sessionsOrder.begin();
        std::advance(it, i - 1);
        
        auto levelKey = it->second;

        Result<Session> sessionRes = StatsManager::getSession(levelKey, it->first);
        if (sessionRes.isErr()) return Err("Failed to get session run deaths");

        std::string out;

        auto session = sessionRes.unwrap();
        
        if (!createDeathsString(session.runs, Save::getRunsCustomazations(), out)) return Err("Failed to create session run deaths string");

        return Ok(out);
    }, "Creating session run deaths string");});
    addSpecialString(sessionRuns0Key);
}

void DTLayer::UpdateSharedStats(){
    if (m_MyLevelStats.isErr()){
        linkedLevelsData.clear();
        return;
    }
    auto sharedStats = m_MyLevelStats.unwrap();

    linkedLevelsData.clear();
    sessionsOrder.clear();

    std::set<std::string> linkedLevels{};
    std::map<std::string, LevelData> visitedLevels{};
    linkedLevels.insert(sharedStats.metadata.LinkedLevels.begin(), sharedStats.metadata.LinkedLevels.end());

    while (true){
        auto startSize = linkedLevels.size();
        for (const auto& linkedLevel : linkedLevels)
        {
            if (visitedLevels.contains(linkedLevel)) continue;
            
            auto currStatsRes = StatsManager::getLevelData(linkedLevel);
            if (currStatsRes.isErr()){
                Notification::create(fmt::format("failed to get data for linked level - {} | {}", linkedLevel, currStatsRes.unwrapErr()))->show();
                continue;
            }
            auto currStats = currStatsRes.unwrap();

            linkedLevels.insert(currStats.metadata.LinkedLevels.begin(), currStats.metadata.LinkedLevels.end());
            visitedLevels.insert({currStats.levelKey, currStats});
        }

        if (startSize == linkedLevels.size()) break;
    }

    visitedLevels.insert({sharedStats.levelKey, sharedStats});

    for (const auto& [_, level] : visitedLevels){
        std::for_each(level.sessionNames.begin(), level.sessionNames.end(), [&](long long key) {
            this->sessionsOrder.emplace(key, level.levelKey);
        });

        linkedLevelsData.push_back(level);
    }

    if (sessionSelector != nullptr)
        sessionSelector->setMaximumCount(sessionsOrder.size(), false);

    UpdateDeathRelatedStrings();
}

void DTLayer::onSettings(CCObject*){
    geode::openSettingsPopup(Mod::get());
}

void DTLayer::keyBackClicked(){
    if (isEditingLayout){
        if (layoutOptionsLayer != nullptr){
            if (!layoutOptionsLayer->isEditingNode())
                onApplyLayoutChanges(nullptr);
            else{
                layoutOptionsLayer->keyBackClicked();
            }
        }
    }
    else
        DTLayer::onClose(nullptr);
}

void DTLayer::onClose(CCObject* sender){
    Popup<GJGameLevel* const&>::onClose(sender);
    instance = nullptr;
}

void DTLayer::show(){
    Popup<GJGameLevel* const&>::show();
    this->setZOrder(100);

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

AdvancedScrollLayer* DTLayer::getScrollLayer(){
    return scrollLayer;
}

void DTLayer::UpdateDeathRelatedStrings(){
    if (m_MyLevelStats.isErr()) return;

    for (const auto& [_, key] : specialStrings)
    {
        if (key->getKey() == "f0" || key->getKey() == "s0" || key->getKey() == "runs" || key->getKey() == "sruns")
            key->updateContent();
    }
}

bool DTLayer::createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, NewBests* const newBests, const std::string& newBestColoring){
    out = "";
    if (m_MyLevelStats.isErr()) return false;
    auto& myStats = m_MyLevelStats.unwrap();

    std::vector<std::pair<std::string, int>> deathVec(deaths.begin(), deaths.end());
    std::sort(deathVec.begin(), deathVec.end(), [](const auto& a, const auto& b){
        auto runARes = StatsManager::splitRunKey(a.first);
        auto runBRes = StatsManager::splitRunKey(b.first);
        if (runARes.isErr() || runBRes.isErr()){
            log::error("Failed to split run key! {} | {}", runARes.isErr() ? runARes.unwrapErr() : "", runBRes.isErr() ? runBRes.unwrapErr() : "");
            return a.first < b.first;
        }

        auto& runA = runARes.unwrap();
        auto& runB = runBRes.unwrap();

        if (runA.start == runB.start)
            return runA.end < runB.end;

        return runA.start < runB.start;
    });

    for (const auto& [run, amount] : deathVec)
    {
        auto runSplitRes = StatsManager::splitRunKey(run);
        if (runSplitRes.isErr()){
            log::error("Failed to split run key! {}", runSplitRes.unwrapErr());
            continue;
        }

        auto& runSplit = runSplitRes.unwrap();

        bool includeRunStart = runSplit.start != -1;

        std::string nbDeColor = "";
        std::string nbColor = "";

        if (includeRunStart){
            if (myStats.metadata.hideRunLength > runSplit.end - runSplit.start)
                continue;
        }
        else{
            if (myStats.metadata.hideUpto > runSplit.end)
                continue;

            if (newBests != nullptr && newBests->contains(runSplit.end)){
                nbDeColor = "{\\color}";
                nbColor = newBestColoring;
            }
        }

        auto format = custom.format;

        std::string toReplaceWith = includeRunStart ? fmt::format("{}-{}", runSplit.start, runSplit.end) : fmt::format("{}", runSplit.end);
        format = std::regex_replace(
            format,
            std::regex("\\{per\\}"),
            toReplaceWith
        );
        
        format = std::regex_replace(
            format,
            std::regex("\\{d\\}"),
            std::to_string(amount)
        );
        
        //old coloring
        //out += fmt::format("{}{}{}{}", nbColor, format, nbDeColor, custom.seperator);
        auto toAdd = fmt::format("{}{}", format, custom.seperator);
        out += toAdd;
    }

    if (out == "")
        out = "No Deaths Found!";

    return true;
}

int DTLayer::getCurrentSelectedSession(){
    return sessionSelector->getCurrentCount();
}

void DTLayer::onSessionSelected(int sessionNum, bool updateContent){
    auto it = sessionsOrder.begin();
    std::advance(it, sessionNum - 1);
    
    if (it == sessionsOrder.end()) return;

    if (sessionNum - 1 == currentSession) return;
    currentSession = sessionNum - 1;

    if (sessionSelector->getCurrentCount() != currentSession + 1) sessionSelector->setCurrentCount(currentSession + 1);

    if (updateContent){
        specialStrings["s0"]->updateContent();
        specialStrings["sruns"]->updateContent();
    }
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

void DTLayer::update(float dt){
    columnHolder->updateLayout();

    float oldWidth = scrollLayer->content->getContentWidth();

    float width = std::max(ogLimits.width, columnHolder->getContentWidth() - (isEditingLayout ? 0 : addColumnButton->getContentWidth()));

    if (oldWidth != width){
        scrollLayer->setLimitsWidth(width);
        scrollLayer->zoomBy(0);
        float delta = scrollLayer->content->getContentWidth() - oldWidth;

        scrollLayer->moveBy(ccp(delta / 2, 0));
    }
}

void DTLayer::organizeLayout(){
    organizationListener.bind([this](organizationTask::Event* event){
        if (auto result = event->getValue()){
            float fixedhighest = result->highestColumn - (isEditingLayout ? 0 : LayoutColumn::topHeight + LayoutColumn::addNewBtnOffset * 2);
            for (const auto& column : columns){
                column->setContentHeight(fixedhighest);
            }

            float cappedHeight = std::max(fixedhighest, ogLimits.height);

            float oldHeightLimits = scrollLayer->content->getContentHeight();

            scrollLayer->setLimitsHeight(cappedHeight);
            columnHolder->setPositionY(cappedHeight);
            labelsHolder->setPosition(columnHolder->getPosition());

            float delta = 0;
            
            delta = oldHeightLimits - scrollLayer->content->getContentHeight();

            log::info("delta {}", delta);
            log::info("{} | {}", oldHeightLimits, scrollLayer->content->getContentHeight());
            scrollLayer->moveBy(ccp(0, delta / 2));
            
            for (const auto& [label, newPos, newWidth] : result->labelData)
            {
                const int MOVEMENT_TAG = 2;
                const int RESIZE_TAG = 7;

                auto taggedMovementAction = label->getActionByTag(MOVEMENT_TAG);
                auto taggedResizeAction = label->getActionByTag(RESIZE_TAG);

                bool doCreateNewMoveAction = true;
                bool doCreateNewResizeAction = true;
                
                //movement action check
                if (taggedMovementAction != nullptr){
                    auto currentMovementActionEase = static_cast<CCEaseInOut*>(taggedMovementAction);
                    auto currentMovementAction = static_cast<CCMoveTo*>(currentMovementActionEase->getInnerAction());

                    if (!currentMovementAction->m_endPosition.equals(newPos)){
                        label->stopActionByTag(MOVEMENT_TAG);
                    }
                    else doCreateNewMoveAction = false;
                }

                if (doCreateNewMoveAction){
                    auto pos = newPos + ccp(0, isEditingLayout ? 0 : LayoutColumn::topHeight);
                    if (!cornerOnNextOrganization){
                        auto movementAction = CCEaseInOut::create(CCMoveTo::create(DTLayer::transitionTime, pos), 2);
                        movementAction->setTag(MOVEMENT_TAG);

                        label->runAction(movementAction);
                    }
                    else{
                        label->setPosition(pos);
                    }
                }

                //resize action check
                if (taggedResizeAction != nullptr){
                    auto currentResizeActionEase = static_cast<CCEaseInOut*>(taggedResizeAction);
                    auto currentResizeAction = static_cast<CCResizeWidthTo*>(currentResizeActionEase->getInnerAction());

                    if (currentResizeAction->endWidth != newWidth){
                        label->stopActionByTag(RESIZE_TAG);
                    }
                    else doCreateNewResizeAction = false;
                }

                if (doCreateNewResizeAction){
                    if (!cornerOnNextOrganization){
                        auto resizeAction = CCEaseInOut::create(CCResizeWidthTo::create(DTLayer::transitionTime, newWidth), 2);
                        resizeAction->setTag(RESIZE_TAG);

                        label->runAction(resizeAction);
                    }
                    else{
                        label->setContentWidth(newWidth);
                    }
                }
            }

            for (const auto& [target, callback] : onOrganizationCompleteEvent)
            {
                callback(delta);
            }
            
            if (cornerOnNextOrganization){
                cornerOnNextOrganization = false;
                scrollLayer->moveToCorner(true, false);
            }
        }
    });
    organizationListener.setFilter(organizeLayoutTask());
}

organizationTask DTLayer::organizeLayoutTask(){
    struct LabelData {
        DTLabel* label;
        std::set<LayoutColumn*> holders;
    };
    
    struct ColumnData {
        LayoutColumn* column;
        int orderPos;
        std::vector<std::pair<int, DTLabel*>> labels;
    };
    
    std::vector<ColumnData> columnSnapshots;
    std::map<DTLabel*, LabelData> labelSnapshots;
    
    for (const auto& column : columns)
    {
        ColumnData colData;
        colData.column = column;
        colData.orderPos = column->info.orderPos;
        
        for (const auto& [layer, label] : column->labels)
        {
            colData.labels.push_back({layer, label});
            
            if (labelSnapshots.find(label) == labelSnapshots.end())
            {
                auto holders = label->getHolders();
                std::set<LayoutColumn*> holderSet;
                for (const auto& holder : holders)
                {
                    holderSet.insert(holder);
                }
                labelSnapshots[label] = {label, holderSet};
            }
        }
        
        columnSnapshots.push_back(colData);
    }
    
    return organizationTask::run([columnSnapshots, labelSnapshots](auto progress, auto hasBeenCancelled) -> organizationTask::Result {
        std::set<DTLabel*> allLabels{};
        std::map<DTLabel*, std::set<LayoutColumn*>> labelHolders{};

        for (const auto& colData : columnSnapshots)
        {
            for (const auto& [layer, label] : colData.labels)
            {
                if (allLabels.contains(label)) continue;

                allLabels.insert(label);
                label->tempPos = ccp(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
                label->tempWidth = 0;
            }
        }

        for (const auto& [label, data] : labelSnapshots)
        {
            labelHolders[label] = data.holders;
        }

        std::map<LayoutColumn*, DTLabel*> lastVisitedLabelForColumn{};
        std::map<DTLabel*, std::map<LayoutColumn*, std::optional<int>>> labelAwaitingColumnValues{};
        std::set<DTLabel*> processedLabels{};

        auto UpdateTempPos = [&](LayoutColumn* column, DTLabel* label, DTLabel* prevLabel){
            auto startPosInLabelSpace = label->getParent()->convertToNodeSpace(column->convertToWorldSpace(column->bgSpr->getPosition()));

            float prevHeight = startPosInLabelSpace.y;

            if (label->info.layer != 0 && prevLabel != nullptr) prevHeight = prevLabel->tempPos.y - prevLabel->getContentHeight();

            if (prevHeight < label->tempPos.y) label->tempPos.y = prevHeight;

            float newX = startPosInLabelSpace.x;

            if (newX < label->tempPos.x) label->tempPos.x = newX;
        };

        auto UpdateTempWidth = [](DTLabel* label){
            label->tempWidth = 0;
            for (const auto& labelColumn : label->getHolders())
            {
                label->tempWidth += labelColumn->getContentWidth();
            }
        };

        while (true){
            for (const auto& colData : columnSnapshots)
            {
                const auto& column = colData.column;
                // log::info("goind over column {}", colData.orderPos);
                DTLabel* prevLabel = nullptr;

                bool foundLastLabel = false;

                for (const auto& [labelLayer, label] : colData.labels)
                {
                    // log::info("going through label");
                    if (!lastVisitedLabelForColumn.contains(column)){
                        lastVisitedLabelForColumn.insert({column, label});
                        foundLastLabel = true;
                    }
                    else if (label != lastVisitedLabelForColumn[column] && !foundLastLabel){
                        UpdateTempPos(column, label, prevLabel);
                        prevLabel = label;
                        continue;
                    }
                    else foundLastLabel  = true;

                    if (processedLabels.contains(label)){
                        UpdateTempPos(column, label, prevLabel);
                        prevLabel = label;
                        continue;
                    }

                    // log::info("label valid");
                    
                    lastVisitedLabelForColumn[column] = label;

                    int newLayer = prevLabel == nullptr ? 0 : prevLabel->info.layer + 1;

                    // log::info("destenation layer {}", newLayer);

                    auto& holdersBack = labelHolders[label];

                    if (holdersBack.size() > 1){
                        // log::info("label is multicolumn");
                        if (!labelAwaitingColumnValues.contains(label)){
                            std::map<LayoutColumn*, std::optional<int>> mapToSet{};

                            for (const auto& holder : holdersBack)
                                mapToSet.insert({holder, std::nullopt});

                            // log::info("populated list for label with {} holders", mapToSet.size());

                            labelAwaitingColumnValues.insert({label, mapToSet});
                        }

                        // log::info("adding label value?");

                        if (labelAwaitingColumnValues[label].contains(column) && !labelAwaitingColumnValues[label][column].has_value()){
                            // log::info("label value added");
                            labelAwaitingColumnValues[label][column] = newLayer;
                        }

                        int highestOptLayer = 0;
                        bool wereAllLayersFound = true;

                        UpdateTempPos(column, label, prevLabel);

                        // log::info("checking conclusion..");

                        for (const auto& [column, optLayer] : labelAwaitingColumnValues[label]){
                            if (!optLayer.has_value()){
                                wereAllLayersFound = false;
                                break;
                            }

                            highestOptLayer = std::max(highestOptLayer, optLayer.value());
                        }

                        if (!wereAllLayersFound){
                            // log::info("invalid labels were found");
                            break;
                        }

                        // log::info("all layer values were found for label");

                        label->info.layer = highestOptLayer;
                        processedLabels.insert(label);
                        
                        UpdateTempWidth(label);

                        // log::info("combo found at {}", highestOptLayer);

                        prevLabel = label;
                        continue;
                    }

                    // log::info("non double found! adding..");
                    
                    label->info.layer = newLayer;
                    processedLabels.insert(label);

                    UpdateTempPos(column, label, prevLabel);
                    UpdateTempWidth(label);

                    // log::info("single found at {}", newLayer);

                    prevLabel = label;
                }
            }

            // log::info("res: {} | {}", processedLabels.size(), allLabels.size());

            if (processedLabels.size() == allLabels.size()) break;
        }

        for (const auto& colData : columnSnapshots){
            colData.column->refreshAllLabelsLayer();
        }

        float heighestHeight = 0;
        
        for (const auto& colData : columnSnapshots){
            for (const auto& [_, label] : colData.labels)
            {
                auto height = std::abs(label->tempPos.y) + label->getContentHeight() + LayoutColumn::addNewBtnOffset * 2;
                if (heighestHeight < height) heighestHeight = height;
            }
        }

        organizationResult data{};
        data.highestColumn = heighestHeight;

        for (const auto& label : allLabels)
        {
            auto targetPosition = label->tempPos;
            auto targetWidth = label->tempWidth - LayoutColumn::borderWidth;

            data.labelData.push_back({label, targetPosition, targetWidth});
        }

        return data;
    }, "DT layout organization");
}

std::pair<LayoutColumn*, int> DTLayer::getColumnLayerFromPosition(CCPoint posInWorldSpace){

    auto posInColumnHolderSpace = columnHolder->convertToNodeSpace(posInWorldSpace);

    LayoutColumn* columnFound = nullptr;

    for (const auto& column : columns)
    {
        if (column->boundingBox().containsPoint(posInColumnHolderSpace + ccp(DTColumnInfo::minWidth / 2, 0))){
            columnFound = column;
            // log::info("found column {}", columnFound->orderPos);
            break;
        }
    }

    if (columnFound == nullptr) return {nullptr, 0};

    auto posInLabelHolderSpace = labelsHolder->convertToNodeSpace(posInWorldSpace);

    if (!columnFound->labels.size()) return {columnFound, 0};

    int highestLayerFound = 0;

    for (const auto& [labelLayer, label] : columnFound->labels)
    {
        auto highestLabelPoint = label->getPositionY() - label->getContentHeight() + DTLabel::labelTitleHeight / 2;

        if (highestLayerFound < labelLayer) highestLayerFound = labelLayer;

        if (highestLabelPoint < posInLabelHolderSpace.y){

            return {columnFound, labelLayer};
        }
    }

    return {columnFound, highestLayerFound + 1};
}

std::multiset<LayoutColumn*, ColumnComperator> DTLayer::getColumnsBetween(CCPoint a, CCPoint b){

    std::multiset<LayoutColumn*, ColumnComperator> toReturn{};

    a = columnHolder->convertToNodeSpace(a);
    b = columnHolder->convertToNodeSpace(b);

    auto x = std::min(a.x, b.x);
    auto y = std::min(a.y, b.y);
    auto w = fabs(b.x - a.x);
    auto h = fabs(b.y - a.y);

    auto rect = CCRect{x, y, w, h};

    // log::info("checking rects... {}", rect);

    for (const auto& column : columns)
    {
        if (rect.intersectsRect(column->boundingBox()))
            toReturn.insert(column);
    }

    return toReturn;
}

void DTLayer::addColumnBtnClicked(CCObject*){
    addColumn();
}

LayoutColumn* DTLayer::addColumn(std::optional<DTColumnInfo> info){
    if (!info.has_value()){
        int position = 0;

        if (columnHolder->getChildrenCount() > 1){
            auto highestColumn = static_cast<CCNode*>(columnHolder->getChildren()->objectAtIndex(columnHolder->getChildrenCount() - 2));
            position = highestColumn->getZOrder() + 1;
        }

        info = DTColumnInfo{
            .orderPos = position
        };
    }
    
    auto column = LayoutColumn::create(info.value(), isEditingLayout, scrollLayer->getContentHeight());
    columnHolder->addChild(column);
    columns.insert(column);

    return column;
}


DTLabel* DTLayer::createNewLabel(DTLabelInfo info){
    auto newLabel = DTLabel::create(info);
    labelsHolder->addChild(newLabel);
    if (isEditingLayout) newLabel->setEditable(true);
    return newLabel;
}


void DTLayer::subscribeToOrganizationEvent(CCNode* target, const std::function<void(float)>& callback){
    if (onOrganizationCompleteEvent.contains(target)) return;

    onOrganizationCompleteEvent.insert({target, callback});
}
void DTLayer::unsubscribeToOrganizationEvent(CCNode* target){
    if (!onOrganizationCompleteEvent.contains(target)) return;

    onOrganizationCompleteEvent.erase(target);
}

void DTLayer::setLayoutBy(const DTLayoutV3& layout){
    for (const auto& column : columns)
    {
        column->destroyColumnAndCleanup();
    }

    columns.clear();

    for (const auto& column : layout.columns){
        addColumn(column);
    }

    fixUpColumnPositions();

    columnHolder->updateLayout();

    std::map<LayoutColumn*, std::vector<DTLabel*>> labelsForColumns{};
    std::set<DTLabel*> labels{};

    for (const auto& label : layout.labels){
        auto labelNode = createNewLabel(label);

        for (const auto& column : columns){
            if (column->info.orderPos >= labelNode->info.minPlacementRange && column->info.orderPos <= labelNode->info.maxPlacementRange){
                if (labelsForColumns.contains(column))
                    labelsForColumns[column].push_back(labelNode);
                else
                    labelsForColumns.insert({column, {labelNode}});
            }
        }

        labels.insert(labelNode);
    }

    for (const auto& [column, labels] : labelsForColumns)
    {
        for (const auto& label : labels)
        {
            column->addLabel(label);
        }
        
    }

    for (const auto& label : labels)
    {
        if (label->isAlone())
            label->removeMeAndCleanup();
    }

    cornerOnNextOrganization = true;
}

void DTLayer::fixUpColumnPositions(){

    int supposedOrder = 0;

    for (const auto& column : columns)
    {
        column->info.orderPos = supposedOrder;
        column->setZOrder(column->info.orderPos);

        supposedOrder++;
    }
}

void DTLayer::saveCurrentLayout(){
    std::set<DTLabel*> labels{};

    DTLayoutV3 layout{};

    for (const auto& column : columns)
    {
        layout.columns.push_back(column->info);

        for (const auto& [labelLayer, label] : column->labels)
        {
            if (labels.contains(label)) continue;

            labels.insert(label);

            layout.labels.push_back(label->info);
        }
    }

    // log::info("saving {} columns and {} labels", layout.columns.size(), layout.labels.size());
    
    Save::setLayout(layout);
}

void DTLayer::specialKeyUpdateStarted(const std::shared_ptr<SpecialKey>& key){
    for (const auto& label : keyListeners)
    {
        label->setLoading(key);
    }
}

void DTLayer::specialKeyUpdateCompleted(const std::shared_ptr<SpecialKey>& key){
    if (key->getKey() == "f0"){
        specialStrings["dtatt"]->updateContent();
    }

    for (const auto& label : keyListeners)
    {
        label->completeLoading(key);
    }
}

void DTLayer::setOptionsLayerTo(DTLabel* label){
    if (layoutOptionsLayer == nullptr) return;
    if (!layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-70.5f, 0))
            )
        );
        layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );
    }

    layoutOptionsLayer->setEditedNodeTo(label);
}
void DTLayer::setOptionsLayerTo(LayoutColumn* column){
    if (layoutOptionsLayer == nullptr) return;
    if (!layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-70.5f, 0))
            )
        );
        layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );
    }

    layoutOptionsLayer->setEditedNodeTo(column);
}
void DTLayer::closeOptionsLayer(){
    if (layoutOptionsLayer == nullptr) return;
    if (layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(70.5f, 0))
            )
        );
        layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(160, 0))
            )
        );
    }

    layoutOptionsLayer->close();
}
void DTLayer::removeColumn(LayoutColumn* column){
    if (!columns.contains(column)) return;

    columns.erase(column);
}

void DTLayer::onApplyLayoutChanges(CCObject*){

    saveCurrentLayout();

    exitLayoutEditing();
}
void DTLayer::onDiscardLayoutChanges(CCObject*){

    setLayoutBy(Save::getLayout());

    exitLayoutEditing();
}

void DTLayer::exitLayoutEditing(){
    isEditingLayout = false;

    closeOptionsLayer();

    std::set<DTLabel*> visitedLabels{};

    for (const auto& column : columns)
    {
        column->setVisibility(false);

        for (const auto& [_, label] : column->labels)
        {
            if (visitedLabels.contains(label)) continue;

            visitedLabels.insert(label);
            label->setEditable(false);
        }
    }

    bottomMenu->setEnabled(true);
    bottomMenu->stopAllActions();
    bottomMenu->runAction(CCFadeTo::create(.15f, 255));
    m_buttonMenu->setEnabled(true);
    m_buttonMenu->stopAllActions();
    m_buttonMenu->runAction(CCFadeTo::create(.15f, 255));
    editLayoutBtnSpr->stopAllActions();
    editLayoutBtnSpr->runAction(CCFadeTo::create(.15f, 255));
    
    editLayoutMenu->setEnabled(false);
    applyChangesButtonSpr->m_BGSprite->stopAllActions();
    applyChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 0));
    applyChangesButtonSpr->m_label->stopAllActions();
    applyChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 0));
    discardChangesButtonSpr->m_BGSprite->stopAllActions();
    discardChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 0));
    discardChangesButtonSpr->m_label->stopAllActions();
    discardChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 0));

    addColumnButton->stopAllActions();
    addColumnButton->runAction(CCFadeTo::create(.15f, 0));
    columnHolder->setEnabled(false);

    scrollLayer->moveBy(ccp(0, LayoutColumn::topHeight));

    this->organizeLayout();   
}

void DTLayer::subscribeKeyListener(DTLabel* label){
    if (keyListeners.contains(label)) return;

    keyListeners.insert(label);
}
void DTLayer::unsubscribeKeyListener(DTLabel* label){
    if (!keyListeners.contains(label)) return;

    keyListeners.erase(label);
}

void DTLayer::modifyRun(int startPer, int amount, std::optional<int> sessionNumber){
    if (m_MyLevelStats.isErr()) return;

    auto processRun = [&, amount, startPer](Deaths& data) -> bool {
        auto runStr = std::to_string(startPer);
        if (!data.contains(runStr)){
            if (amount < 0)
                return false;

            data.insert({runStr, amount});
            return true;
        }

        auto newNum = data[runStr] + amount;

        if (newNum <= 0){
            data.erase(runStr);
            return true;
        }

        data[runStr] = newNum;

        return true;
    };

    if (sessionNumber.has_value()){
        auto it = sessionsOrder.begin();
        std::advance(it, sessionNumber.value() - 1);
        
        if (it == sessionsOrder.end()) return;

        auto sessionRes = StatsManager::getSession(it->second, it->first);
        if (sessionRes.isErr()) return;
        auto session = sessionRes.unwrap();

        if (!processRun(session.deaths)) return;

        auto _ = StatsManager::setSession(session, it->second, it->first, false);
        if (_.isErr()) log::error("{}", _.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processRun(from0Stats.deaths)){
            auto _ = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (_.isErr()) log::error("{}", _.unwrapErr());
            return;
        }

        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr()) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();

            if (processRun(linkedLevelFrom0Stats.deaths)){
                auto _ = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (_.isErr()) log::error("{}", _.unwrapErr());
                return;
            }
        }
    }
}
void DTLayer::modifyRun(int startPer, int endPer, int amount, std::optional<int> sessionNumber){
    if (m_MyLevelStats.isErr()) return;

    auto processRun = [&, amount, startPer, endPer](Deaths& data) -> bool {
        auto runStr = fmt::format("{}-{}", startPer, endPer);
        if (!data.contains(runStr)){
            if (amount < 0)
                return false;

            data.insert({runStr, amount});
            return true;
        }

        auto newNum = data[runStr] + amount;

        if (newNum <= 0){
            data.erase(runStr);
            return true;
        }

        data[runStr] = newNum;

        return true;
    };

    if (sessionNumber.has_value()){
        auto it = sessionsOrder.begin();
        std::advance(it, sessionNumber.value() - 1);
        
        if (it == sessionsOrder.end()) return;

        auto sessionRes = StatsManager::getSession(it->second, it->first);
        if (sessionRes.isErr()) return;
        auto session = sessionRes.unwrap();

        if (!processRun(session.runs)) return;

        auto _ = StatsManager::setSession(session, it->second, it->first, false);
        if (_.isErr()) log::error("{}", _.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processRun(from0Stats.runs)){
            auto _ = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (_.isErr()) log::error("{}", _.unwrapErr());
            return;
        }

        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr()) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();

            if (processRun(linkedLevelFrom0Stats.runs)){
                auto _ = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (_.isErr()) log::error("{}", _.unwrapErr());
                return;
            }
        }
    }
}

void DTLayer::modifyNewBest(int percent, bool makeTrue, std::optional<int> sessionNumber){
    if (m_MyLevelStats.isErr()) return;

    auto processBest = [&, percent, makeTrue](NewBests& bests) -> bool {
        if (makeTrue){
            if (!bests.contains(percent)) return false;

            bests.insert(percent);
        }
        else{
            if (bests.contains(percent)) return false;

            bests.erase(percent);
        }

        return true;
    };

    if (sessionNumber.has_value()){
        auto it = sessionsOrder.begin();
        std::advance(it, sessionNumber.value() - 1);
        
        if (it == sessionsOrder.end()) return;

        auto sessionRes = StatsManager::getSession(it->second, it->first);
        if (sessionRes.isErr()) return;
        auto session = sessionRes.unwrap();

        processBest(session.newBests);

        auto _ = StatsManager::setSession(session, it->second, it->first, false);
        if (_.isErr()) log::error("{}", _.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processBest(from0Stats.newBests)){
            auto _ = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (_.isErr()) log::error("{}", _.unwrapErr());
            return;
        }

        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr()) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();
            
            if (processBest(linkedLevelFrom0Stats.newBests)){
                auto _ = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (_.isErr()) log::error("{}", _.unwrapErr());
                return;
            }
        }
    }
}

bool DTLayer::DeleteSave(){
    auto deleteRes = StatsManager::deleteLevelStats(m_MyLevelStats.unwrap().levelKey);
    if (deleteRes.isErr()){
        log::error("{}", deleteRes.unwrapErr());
        return false;
    }

    onClose(nullptr);
    return true;
}