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
        m_MyLevelStats = Ok(newData);
    }
    else if (m_MyLevelStats.isErr())
        geode::Notification::create(fmt::format("Failed to load DT level data! {}", m_MyLevelStats.unwrapErr()), NotificationIcon::Error)->show();
    
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
    this->setZOrder(100);

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

    auto bottomMenu = CCMenu::create();
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
    sessionSelector->setCallback([&](int newSession){ onSessionSelected(newSession); });
    bottomMenu->addChild(sessionSelector);

    onSessionSelected(1);

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
    auto addColumnButton = CCMenuItemSpriteExtra::create(
        addColumnButtonSpr,
        this,
        menu_selector(DTLayer::addColumnBtnClicked)
    );
    addColumnButton->setZOrder(1000);
    columnHolder->addChild(addColumnButton);

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
// move by 84.5f to the left
    return true;
}

void DTLayer::onEditLayout(CCObject*){
    // if (layoutTopbar != nullptr) return;

    // for (const auto& label : labels)
    //     label->enterEditMode();

    // auto winSize = CCDirector::sharedDirector()->getWinSize();

    // static_cast<CCMenu*>(m_mainLayer->getChildByID("bottom-menu"))->setEnabled(false);
    // m_buttonMenu->setEnabled(false);
    // m_buttonMenu->setOpacity(100);
    // sessionSelector->setEnabled(false);
    
    // layoutTopbar = EditLayoutTopbar::create();
    // layoutTopbar->setZOrder(10);
    // layoutTopbar->setPosition({
    //     (winSize.width - layoutTopbar->getContentWidth()) / 2,
    //     10
    // });
    // layoutTopbar->onExit = [&](bool didApply){
    //     if (!didApply)
    //         for (const auto& label : labels)
    //             label->revert();

    //     std::vector<DTLabelInfo> newInfo{};

    //     for (const auto& label : labels){
    //         label->exitEditMode();
    //         if (didApply && labels.contains(label))
    //             newInfo.push_back(label->labelInfo);
    //     }

    //     if (didApply)
    //         Save::setLayout(newInfo);

    //     layoutTopbar = nullptr;

    //     static_cast<CCMenu*>(m_mainLayer->getChildByID("bottom-menu"))->setEnabled(true);
    //     m_buttonMenu->setEnabled(true);
    //     m_buttonMenu->setOpacity(255);
    //     sessionSelector->setEnabled(true);
    // };
    // this->addChild(layoutTopbar);
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
    saveCurrentLayout();

    // auto graph = DTGraphLayer::create(this);
    // graph->setZOrder(100);
    // this->addChild(graph);
}

void DTLayer::addSpecialString(const std::shared_ptr<SpecialKey>& key){
    key->setUpdateStartedCallback(std::bind(&DTLayer::specialKeyUpdateStarted, this, std::placeholders::_1));
    key->setUpdateCompletedCallback(std::bind(&DTLayer::specialKeyUpdateCompleted, this, std::placeholders::_1));
    specialStrings.emplace(key->getKey(), key);
    key->updateContent();
}

void DTLayer::populateSpecialStrings(){
    auto from0Key = std::make_shared<SpecialKey>("f0");
    from0Key->setUpdateFunction(UpdateTask::run([&](auto progress, auto hasBeenCancelled) -> UpdateTask::Result {
        auto& myStats = m_MyLevelStats.unwrap();

        Deaths sharedDeaths = myStats.from0.deaths;
        NewBests sharedNBs = myStats.from0.newBests;

        for (const auto& levelData : linkedLevelsData)
        {
            sharedDeaths.insert(levelData.from0.deaths.begin(), levelData.from0.deaths.end());
            sharedNBs.insert(levelData.from0.newBests.begin(), levelData.from0.newBests.end());
        }

        std::string out;

        if (!createDeathsString(sharedDeaths, Save::getFrom0Customazations(), out, &sharedNBs, "{nbc}")) return Err("Failed to create from0 deaths string");

        return Ok(out);
    }, "Creating from0 deaths string"));
    addSpecialString(from0Key);

    auto runsKey = std::make_shared<SpecialKey>("runs");
    runsKey->setUpdateFunction(UpdateTask::run([&](auto progress, auto hasBeenCancelled) -> UpdateTask::Result {
        auto& myStats = m_MyLevelStats.unwrap();

        Deaths sharedRuns = myStats.from0.runs;

        for (const auto& levelData : linkedLevelsData)
        {
            sharedRuns.insert(levelData.from0.runs.begin(), levelData.from0.runs.end());
        }

        std::string out;

        if (!createDeathsString(sharedRuns, Save::getRunsCustomazations(), out)) return Err("Failed to create run deaths string");

        return Ok(out);
    }));
    addSpecialString(runsKey);

    auto nlKey = std::make_shared<SpecialKey>("nl");
    nlKey->setUpdateFunction(UpdateTask::immediate(Ok("\n")));
    addSpecialString(nlKey);
    
    auto attemptsKey = std::make_shared<SpecialKey>("att");
    attemptsKey->setUpdateFunction(UpdateTask::immediate(Ok(std::to_string(m_Level->m_attempts.value()))));
    addSpecialString(attemptsKey);

    auto levelNameKey = std::make_shared<SpecialKey>("lvln");
    levelNameKey->setUpdateFunction(UpdateTask::immediate(Ok(m_Level->m_levelName)));
    addSpecialString(levelNameKey);
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

    for (const auto& [_, level] : visitedLevels){
        std::for_each(level.sessionNames.begin(), level.sessionNames.end(), [&](long long key) {
            this->sessionsOrder.emplace(key, level.levelKey);
        });

        linkedLevelsData.push_back(level);
    }

    UpdateDeathRelatedStrings();
}

void DTLayer::onSettings(CCObject*){
    geode::openSettingsPopup(Mod::get());
}

void DTLayer::keyBackClicked(){
    DTLayer::onClose(nullptr);
}

void DTLayer::onClose(CCObject* sender){
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

// DTLabel* DTLayer::createLabel(DTLabelInfo info){
//     auto label = DTLabel::create(info, 50 / 10);
//     scrollLayer->content->addChild(label);
//     labels.insert(label);
//     label->onClicked = [&](DTLabel* clickedLabel){
//         if (layoutTopbar == nullptr || !labels.contains(clickedLabel)) return;

//         layoutTopbar->setTarget(clickedLabel);
//     };

//     if (layoutTopbar != nullptr){
//         label->enterEditMode();
//         label->wasCreatedThisEdit = true;
//         label->onSelected(nullptr);
//     }

//     return label;
// }

// void DTLayer::removeLabel(DTLabel* label, bool forceDelete){
//     if (!labels.contains(label)) return;

//     if (forceDelete || layoutTopbar == nullptr){
//         label->removeMeAndCleanup();
//         labels.erase(label);
//     }
//     else if (layoutTopbar != nullptr){
//         label->softDelete();
//     }
// }

AdvancedScrollLayer* DTLayer::getScrollLayer(){
    return scrollLayer;
}

// void DTLayer::saveAndUpdateStats(bool updateShared){
//     if (m_MyLevelStats.isErr()) return;
    
//     auto& levelStats = m_MyLevelStats.unwrap();
//     StatsManager::setLevelStats(levelStats, m_Level, false);
//     if (updateShared)
//         UpdateSharedStats();
// }

// void DTLayer::UpdateOnAllShared(const std::function<void(LevelStats& stats)>& lambda){
//     if (m_MyLevelStats.isErr() || m_SharedLevelStats.isErr() || lambda == NULL) return;

//     auto& sharedStats = m_SharedLevelStats.unwrap();

//     for (const auto& linkedLevel : sharedStats.LinkedLevels)
//     {
//         auto statsRes = StatsManager::getLevelStats(linkedLevel, false);
//         if (statsRes.isErr()) continue;
//         auto currentStats = statsRes.unwrap();

//         lambda(currentStats);

//         StatsManager::setLevelStats(currentStats, linkedLevel, false);
//     }
    
//     auto& myStats = m_MyLevelStats.unwrap();
//     lambda(myStats);
//     lambda(sharedStats);

//     UpdateDeathRelatedStrings();
// }

void DTLayer::UpdateDeathRelatedStrings(){
    if (m_MyLevelStats.isErr()) return;

    // auto& myStats = m_MyLevelStats.unwrap();

    // Deaths sharedDeaths = myStats.from0.deaths;
    // NewBests sharedNBs = myStats.from0.newBests;
    // Deaths sharedRuns = myStats.from0.runs;

    // for (const auto& levelData : linkedLevelsData)
    // {
    //     sharedDeaths.insert(levelData.from0.deaths.begin(), levelData.from0.deaths.end());
    //     sharedNBs.insert(levelData.from0.newBests.begin(), levelData.from0.newBests.end());
    //     sharedRuns.insert(levelData.from0.runs.begin(), levelData.from0.runs.end());
    // }

    // createDeathsString(myStats.from0.deaths, Save::getFrom0Customazations(), specialStrings["totalLocalDeaths"], &myStats.from0.newBests, "{nbc}");
    // createDeathsString(sharedDeaths, Save::getFrom0Customazations(), specialStrings["totalSharedDeaths"], &sharedNBs, "{nbc}");

    // createDeathsString(myStats.from0.runs, Save::getRunsCustomazations(), specialStrings["totalLocalRuns"]);
    // createDeathsString(sharedRuns, Save::getRunsCustomazations(), specialStrings["totalSharedRuns"]);

    // if (currentSessionInfo != std::nullopt){
    //     auto& sessionObj = currentSessionInfo.value();

    //     createDeathsString(sessionObj.deaths, Save::getSessionF0Customazations(), specialStrings["totalSessionDeaths"], &sessionObj.newBests, "{sbc}");
    //     createDeathsString(sessionObj.runs, Save::getSessionRunCustomazations(), specialStrings["totalSessionRuns"]);
    // }
    // else{
    //     specialStrings["totalSessionDeaths"] = "No Deaths Found!";
    //     specialStrings["totalSessionRuns"] = "No Deaths Found!";
    // }
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

        format = std::regex_replace(
            format,
            std::regex("\\{per\\}"),
            includeRunStart ? fmt::format("{}-{}", runSplit.start, runSplit.end) : fmt::format("{}", runSplit.end)
        );

        format = std::regex_replace(
            format,
            std::regex("\\{d\\}"),
            std::to_string(amount)
        );

        //old coloring
        //out += fmt::format("{}{}{}{}", nbColor, format, nbDeColor, custom.seperator);
        out += fmt::format("{}{}", format, custom.seperator);
    }

    if (out == "")
        out = "No Deaths Found!";

    return true;
}

void DTLayer::onSessionSelected(int sessionNum){
    currentSession = sessionNum - 1;

    if (!sessionsOrder.contains(currentSession)) return;

    auto it = sessionsOrder.begin();
    std::advance(it, currentSession);
    
    if (it == sessionsOrder.end()) return;

    auto sessionRes = StatsManager::getSession(it->second, it->first);
    if (sessionRes.isErr()) return;

    currentSessionInfo = sessionRes.unwrap();
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

    scrollLayer->setLimitsWidth(std::max(ogLimits.width, columnHolder->getContentWidth()));
    scrollLayer->zoomBy(0);

    if (oldWidth != scrollLayer->content->getContentWidth()){
        float delta = scrollLayer->content->getContentWidth() - oldWidth;

        scrollLayer->moveBy(ccp(delta / 2, 0));
    }
}

void DTLayer::organizeLayout(){
    organizationListener.bind([this](organizationTask::Event* event){
        if (auto result = event->getValue()){
            for (const auto& column : columns){
                column->setContentHeight(result->highestColumn);
            }

            float cappedHeight = std::max(result->highestColumn, ogLimits.height);

            float oldHeightLimits = scrollLayer->content->getContentHeight();

            scrollLayer->setLimitsHeight(cappedHeight);
            columnHolder->setPositionY(cappedHeight);
            labelsHolder->setPosition(columnHolder->getPosition());

            float delta = oldHeightLimits - scrollLayer->content->getContentHeight();

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
                    auto movementAction = CCEaseInOut::create(CCMoveTo::create(DTLayer::transitionTime, newPos), 2);
                    movementAction->setTag(MOVEMENT_TAG);

                    label->runAction(movementAction);
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
                    auto resizeAction = CCEaseInOut::create(CCResizeWidthTo::create(DTLayer::transitionTime, newWidth), 2);
                    resizeAction->setTag(RESIZE_TAG);

                    label->runAction(resizeAction);
                }
            }

            for (const auto& [target, callback] : onOrganizationCompleteEvent)
            {
                callback(delta);
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

            colData.column->tempHeight = 0;
        }

        for (const auto& [label, data] : labelSnapshots)
        {
            labelHolders[label] = data.holders;
        }

        std::map<LayoutColumn*, DTLabel*> lastVisitedLabelForColumn{};
        std::map<DTLabel*, std::map<LayoutColumn*, std::optional<int>>> labelAwaitingColumnValues{};
        std::set<DTLabel*> processedLabels{};

        auto UpdateTempPos = [](LayoutColumn* column, DTLabel* label, DTLabel* prevLabel){
            auto startPosInLabelSpace = label->getParent()->convertToNodeSpace(column->convertToWorldSpace(column->bgSpr->getPosition()));

            float prevHeight = startPosInLabelSpace.y;

            if (label->info.layer != 0 && prevLabel != nullptr) prevHeight = prevLabel->tempPos.y - prevLabel->getContentHeight();

            if (prevHeight < label->tempPos.y) label->tempPos.y = prevHeight;

            float newX = startPosInLabelSpace.x;

            if (newX < label->tempPos.x) label->tempPos.x = newX;

            for (const auto& labelColumn : label->getHolders())
            {
                auto height = std::abs(label->tempPos.y) + label->getContentHeight() / 2 + labelColumn->topHeight + LayoutColumn::addNewBtnOffset * 2;

                if (labelColumn->tempHeight < height) labelColumn->tempHeight = height;
            }
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
            if (heighestHeight < colData.column->tempHeight) heighestHeight = colData.column->tempHeight;
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
    
    auto column = LayoutColumn::create(info.value(), 20, scrollLayer->getContentHeight());
    columnHolder->addChild(column);
    columns.insert(column);

    return column;
}


DTLabel* DTLayer::createNewLabel(DTLabelInfo info){
    auto newLabel = DTLabel::create(info);
    labelsHolder->addChild(newLabel);
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

void DTLayer::specialKeyUpdateStarted(SpecialKey* key){
    std::set<DTLabel*> allLabels{};

    // log::info("special key update started for key {}", key->getKey());

    for (const auto& column : columns)
    {
        for (const auto& [layer, label] : column->labels)
        {
            if (allLabels.contains(label)) continue;
            allLabels.insert(label);

            label->setLoading(key);
        }
    }
}

void DTLayer::specialKeyUpdateCompleted(SpecialKey* key){
    std::set<DTLabel*> allLabels{};

    // log::info("special key update completed for key {}", key->getKey());

    for (const auto& column : columns)
    {
        for (const auto& [layer, label] : column->labels)
        {
            if (allLabels.contains(label)) continue;
            allLabels.insert(label);

            label->completeLoading(key);
        }
    }
}

void DTLayer::setOptionsLayerTo(DTLabel* label){
    if (layoutOptionsLayer == nullptr) return;
    if (!layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(-85.5f, 0))));
        layoutOptionsLayer->runAction(CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(-164, 0))));
    }

    layoutOptionsLayer->setEditedNodeTo(label);
}
void DTLayer::setOptionsLayerTo(LayoutColumn* column){
    if (layoutOptionsLayer == nullptr) return;
    if (!layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(-85.5f, 0))));
        layoutOptionsLayer->runAction(CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(-164, 0))));
    }

    layoutOptionsLayer->setEditedNodeTo(column);
}
void DTLayer::closeOptionsLayer(){
    if (layoutOptionsLayer == nullptr) return;
    if (layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(85.5f, 0))));
        layoutOptionsLayer->runAction(CCEaseBackOut::create(CCMoveBy::create(0.5f, ccp(164, 0))));
    }

    layoutOptionsLayer->close();
}
void DTLayer::removeColumn(LayoutColumn* column){
    if (!columns.contains(column)) return;

    columns.erase(column);
}