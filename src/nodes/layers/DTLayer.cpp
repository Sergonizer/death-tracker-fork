#include <nodes/layers/DTLayer.hpp>

#include <nodes/layers/DTGraphLayer.hpp>
#include <nodes/layers/DTLevelSpecificSettingsLayer.hpp>
#include <nodes/layers/CalculatorPopup.hpp>

#include <Geode/ui/GeodeUI.hpp>
#include <regex>
#include <utils/CCResizeWidthTo.hpp>

#include <arc/task/Yield.hpp>
#include <arc/time/Sleep.hpp>

#include <utils/Settings.hpp>

float DTLayer::transitionTime = .35f;

bool ColumnComperator::operator()(LayoutColumn* a, LayoutColumn* b) const {
    return a->info.orderPos < b->info.orderPos;
}

DTLayer* DTLayer::instance = nullptr;

DTLayer* DTLayer::create(GJGameLevel* const& Level) {
    auto popup = new DTLayer;
    if (popup->init(Level)) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool DTLayer::init(GJGameLevel* const& level) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(winSize.width - 150, winSize.height - 30, "geode.loader/GE_square01.png"))
        return false;

    m_Level = level;

    // ================================== //
    // loading data

    CleanGetStats();

    // ================================== //

    /*
     * main page
    */

    instance = this;

    this->setID("dt-layer");

    if (Save::getLastOpenedVersion() != Mod::get()->getVersion().toNonVString()){
        Save::setLastOpenedVersion(Mod::get()->getVersion().toNonVString());
        // FLAlertLayer::create(nullptr, fmt::format("Death Tracker {} Changelog", Mod::get()->getVersion().toVString()).c_str(), fmt::format(
        //     "{}",
        //     "- <cg>iOS support</c>"
        // ), "OK", nullptr, 415, false, 200, 0.75f)->show();
    }

    float height = 60;
    ogLimits = CCSize{m_size.width - 30 + 1, m_size.height - height  + 1};
    scrollLayer = AdvancedScrollLayer::create({m_size.width - 30, m_size.height - height}, ogLimits);
    //scrollLayer->drawGrid(50, .5f, ccColor4B{ 143, 143, 143, 255 });
    scrollLayer->setPosition(m_size / 2 - scrollLayer->getContentSize() / 2 + ccp(0, height / 4));
    scrollLayer->setZOrder(2);
    scrollLayer->maxZoom = 0.05f;
    scrollLayer->minZoom = 10;
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

    sessionSelector = SessionSelector::create(getCurrentGrouping().grouping.size());
    sessionSelector->setCallback([&](int newSession){ onSessionSelected(newSession, true); });
    sessionSelector->setScale(.75f);
    sessionSelector->setPosition({m_size.width / 2, 20});
    m_mainLayer->addChild(sessionSelector);

    bottomLeftMenu = CCMenu::create();
    bottomLeftMenu->setContentSize({m_size.width / 2 - sessionSelector->getScaledContentWidth() / 2 - 10, height / 2.5f});
    bottomLeftMenu->setAnchorPoint({0, .5f});
    bottomLeftMenu->setPosition(ccp(10, height / 2.5f  / 2) + ccp(0, 7.5f));
    bottomLeftMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(10)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Even)
        ->setMinRelativeScale(std::nullopt)
        ->setMaxRelativeScale(std::nullopt)
    );
    bottomLeftMenu->setID("bottom-left-menu");
    m_mainLayer->addChild(bottomLeftMenu);

    bottomRightMenu = CCMenu::create();
    bottomRightMenu->setContentSize({m_size.width / 2 - sessionSelector->getScaledContentWidth() / 2 - 10, height / 2.5f});
    bottomRightMenu->setAnchorPoint({1, .5f});
    bottomRightMenu->setPosition(ccp(m_size.width - 10, height / 2.5f  / 2) + ccp(0, 7.5f));
    bottomRightMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(10)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Even)
        ->setMinRelativeScale(std::nullopt)
        ->setMaxRelativeScale(std::nullopt)
    );
    bottomRightMenu->setID("bottom-right-menu");
    m_mainLayer->addChild(bottomRightMenu);

    auto levelSpecificOptionsSpr = CCSprite::createWithSpriteFrameName("GJ_creatorBtn_001.png");
    auto levelSpecificOptionsBtn = CCMenuItemSpriteExtra::create(
        levelSpecificOptionsSpr,
        this,
        menu_selector(DTLayer::onLSOClicked)
    );
    bottomLeftMenu->addChild(levelSpecificOptionsBtn);

    auto graphBtnSpr = CCSprite::createWithSpriteFrameName("graph_button.png"_spr);
    auto graphBtn = CCMenuItemSpriteExtra::create(
        graphBtnSpr,
        this,
        menu_selector(DTLayer::graphBtnClicked)
    );
    bottomLeftMenu->addChild(graphBtn);

    auto calculatorSpr = CCSprite::createWithSpriteFrameName("caluclator.png"_spr);
    auto calculatorBtnSpr = CCSprite::create("GJ_button_01.png");
    calculatorSpr->setPosition(calculatorBtnSpr->getContentSize() / 2);
    calculatorSpr->setScale(.9f);
    calculatorBtnSpr->addChild(calculatorSpr);
    auto calculatorBtn = CCMenuItemSpriteExtra::create(
        calculatorBtnSpr,
        this,
        menu_selector(DTLayer::onCalculator)
    );
    bottomLeftMenu->addChild(calculatorBtn);

    onSessionSelected(1, false);

    auto groupsBtnSpr = ButtonSprite::create(
        sessionsOrder.groupName.c_str(),
        100,
        100,
        1,
        false,
        "bigFont.fnt", 
        "GJ_button_04.png"
    );
    groupsBtnSpr->setCascadeOpacityEnabled(true);
    groupsBtnSpr->setScale(.45f);
    groupsBtn = CCMenuItemSpriteExtra::create(
        groupsBtnSpr,
        this,
        menu_selector(DTLayer::onGroups)
    );

    auto groupsHolder = CCMenu::create();
    groupsHolder->setContentSize(ccp(45, 0));
    groupsBtn->setPosition(groupsHolder->getContentSize() / 2);
    bottomRightMenu->addChild(groupsHolder);
    groupsHolder->addChild(groupsBtn);

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
    bottomRightMenu->addChild(editLayoutBtn);

    auto settingsBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsBtnSpr->setScale(.75f);
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSpr,
        this,
        menu_selector(DTLayer::onSettings)
    );
    settingsBtn->setPosition({m_size.width - 3.f, 3.f});
    bottomRightMenu->addChild(settingsBtn);

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

    lc = LoadingCircle::create();
    lc->setContentSize({0, 0});
    lc->m_sprite->setPosition(m_size / 2);
    m_mainLayer->addChild(lc);

    CCTouchDispatcher::get()->removeDelegate(scrollLayer);

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    populateSpecialStrings();

    this->organizeLayout();

    this->scheduleUpdate();

    bottomRightMenu->updateLayout();
    bottomLeftMenu->updateLayout();

    groupsList = FloatingList::create({
        groupsBtn->getScaledContentWidth(),
        120
    });
    groupsList->setZOrder(2);
    groupsList->addItems({
        FloatingListItem{
            .id = -4,
            .text = sessionsOrder.groupName,
            .BGTexture = "GJ_button_04.png"
        },
        FloatingListItem{
            .id = -3,
            .text = daySGroup.groupName,
            .BGTexture = "GJ_button_05.png"
        },
        FloatingListItem{
            .id = -2,
            .text = weekSGroup.groupName,
            .BGTexture = "GJ_button_05.png"
        },
        FloatingListItem{
            .id = -1,
            .text = monthSGroup.groupName,
            .BGTexture = "GJ_button_05.png"
        },
    });
    groupsList->setCallback([&](auto id){
        this->onGroupSelected(id);
    });
    groupsList->setAnchorPoint({.5f, 0});
    m_mainLayer->addChild(groupsList);
    groupsList->setPosition(
        groupsList->getParent()->convertToNodeSpace(
            groupsBtn->convertToWorldSpace({groupsBtn->getContentWidth() / 2,groupsBtn->getContentHeight()})
        )
    );

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

    auto mainInfo = TutorialButton::create(1, "main-overall", [&, levelSpecificOptionsBtn, graphBtn, editLayoutBtn, settingsBtn](DTTutorialLayer* tutorialLayer){
        tutorialLayer->appendDialogue("Welcome to the <cy>main death tracker page!</c>", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("This is the <cy>main view</c> where you can view <cg>all your data!</c>", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottom, .75f)
            ->joinHighlight(scrollLayer, 0, true)
            ->joinTextToHighlight("Main Scroll View", .5f, TutorialTextPlacement::TTTop)
            ->appendDialogue("You can hold <cg>control</c> and <cp>scroll</c> to zoom in! and <cc>shift</c> and <cp>scroll</c> to move side to side.", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(scrollLayer, 0, true)
            ->appendDialogue("Of course you can also scroll normally :D", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter)
            ->appendDialogue("You also have many options <cy>at the bottom</c> here!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .75f)
            ->joinHighlight(bottomRightMenu)
            ->joinHighlight(bottomLeftMenu)
            ->joinHighlight(sessionSelector)
            ->appendDialogue("There's the <cy>level options</c> which allow you to change may things about how you <cr>track/display your data</c>", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(levelSpecificOptionsBtn)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinTextToHighlight("level options", .3f, TutorialTextPlacement::TTTop)
            ->appendDialogue("You have the <cg>graphs</c> which allow you to visually see your consistancy and other aspects of your data", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(graphBtn)
            ->joinTextToHighlight("graphs", .3f, TutorialTextPlacement::TTTop)
            ->appendDialogue("You have the <co>session selector</c>, allowing you to choose which session to view", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(sessionSelector)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .75f)
            ->joinTextToHighlight("Session Selector", .3f, TutorialTextPlacement::TTTop)
            ->appendDialogue("The <cy>higher</c> the number, the <cr>older</c> the session! so <co>session 1</c> is the most recent and <co>the last</c> is the oldest!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(sessionSelector)
            ->appendDialogue("You also have the option to <cy>edit how your data is layed out</c> using this button!", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(editLayoutBtn)
            ->joinTextToHighlight("Layout Editor", .3f, TutorialTextPlacement::TTTop)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)
            ->appendDialogue("And lastly you have quick access to the mod settings for death tracker right here!", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(settingsBtn)
            ->joinTextToHighlight("Mod Options", .3f, TutorialTextPlacement::TTTop)
            ->appendDialogue("Have fun playing around with the features!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter);
    });
    mainInfo->setPosition(m_size);
    m_buttonMenu->addChild(mainInfo);

    layoutInfo = TutorialButton::create(1, "layout-overall", [&, applyChangesButton, discardChangesButton](DTTutorialLayer* tutorialLayer){
        tutorialLayer
            ->appendDialogue("This is where you can <cg>Edit how death tracker looks!</c>", TutorialCharacterFace::TCFNormal)
            ->appendDialogue("Here you have different labels! which can display any text you want!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottom, .65f);

        std::set<DTLabel*> allLabels{};

        int index = 0;
        for (const auto& column : columns)
        {
            for (const auto& [_, label] : column->labels)
            {
                if (allLabels.contains(label)) continue;

                allLabels.insert(label);
                tutorialLayer->joinHighlight(label->labelTitleBG, .1f * index);
                index++;
            }
        }

        tutorialLayer
            ->appendDialogue("You can <cy>click</c> on them to enter the label settings", TutorialCharacterFace::TCFNormal)
            ->joinPreviousHighlight()
            ->appendDialogue("<cy>Drag them around</c> to move them", TutorialCharacterFace::TCFNormal)
            ->joinPreviousHighlight()
            ->appendDialogue("And also <cy>drag the edges</c> of the labels to <cg>expand</c> them", TutorialCharacterFace::TCFNormal);

        index = 0;
        for (const auto& label : allLabels){
            tutorialLayer->joinHighlight(label->leftExpandLine, .2f * index);
            tutorialLayer->joinHighlight(label->rightExpandLine, .2f * index);

            index++;
        }

        tutorialLayer
            ->appendDialogue("Every <cy>label</c> has to be attached to some <cp>column</c>", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .65f);

        index = 0;
        for (const auto& column : columns)
        {
            tutorialLayer->joinHighlight(column->topSpr, .1f * index);
            index++;
        }

        tutorialLayer
            ->appendDialogue("Each <cp>column</c> can be <cy>clicked</c> to enter its settings", TutorialCharacterFace::TCFNormal)
            ->joinPreviousHighlight()
            ->appendDialogue("And can be <cg>expanded</c> when <cy>dragging its edges</c>", TutorialCharacterFace::TCFNormal);

        index = 0;
        for (const auto& column : columns)
        {
            tutorialLayer->joinHighlight(column->topBorder2, .2f * index);
            index++;
        }

        tutorialLayer
            ->appendDialogue("One you are done can click the <cg>Apply Changes</c> button to save your changes", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(applyChangesButton)
            ->appendDialogue("Or delete all the changes you have made using the <cr>Discard Changes</c> button!", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(discardChangesButton)
            ->appendDialogue("Feel free to play around and explore the different settings!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1);
    });
    layoutInfo->setPosition({100, 128});
    layoutInfo->setOpacity(0);
    editLayoutMenu->addChild(layoutInfo);

    scrollLayer->setVisible(false);

    return true;
}

void DTLayer::onEditLayout(CCObject*){
    if (layoutOptionsLayer == nullptr){
        layoutOptionsLayer = LayoutOptionsLayer::create({150, m_size.height});
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        layoutOptionsLayer->setPosition({winSize.width + 10, (winSize.height - layoutOptionsLayer->getContentHeight()) / 2});
        layoutOptionsLayer->onBackedOut = [&](){closeOptionsLayer();};
        this->addChild(layoutOptionsLayer);
    }

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

    bottomLeftMenu->setEnabled(false);
    bottomLeftMenu->stopAllActions();
    bottomLeftMenu->runAction(CCFadeTo::create(.15f, 0));

    bottomRightMenu->setEnabled(false);
    bottomRightMenu->stopAllActions();
    bottomRightMenu->runAction(CCFadeTo::create(.15f, 0));

    m_buttonMenu->setEnabled(false);
    m_buttonMenu->stopAllActions();
    m_buttonMenu->runAction(CCFadeTo::create(.15f, 0));
    editLayoutBtnSpr->stopAllActions();
    editLayoutBtnSpr->runAction(CCFadeTo::create(.15f, 0));

    editLayoutMenu->setEnabled(true);
    layoutInfo->stopAllActions();
    layoutInfo->runAction(CCFadeTo::create(.15f, 255));
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

void DTLayer::graphBtnClicked(CCObject*){
    DTGraphLayer::create()->show();
}

void DTLayer::addSpecialString(const std::shared_ptr<SpecialKey>& key){
    key->setUpdateStartedCallback([&](const std::shared_ptr<SpecialKey>& k){ this->specialKeyUpdateStarted(k); });
    key->setUpdateCompletedCallback([&](const std::shared_ptr<SpecialKey>& k) { this->specialKeyUpdateCompleted(k); });
    specialStrings.emplace(key->getKey(), key);
    key->updateContent();
}

void DTLayer::transferPlaytimeFromPT() {
    
}

void DTLayer::populateSpecialStrings(){
    auto nlKey = std::make_shared<SpecialKey>("nl", "Adds a new line");
    nlKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onNLKey));
    addSpecialString(nlKey);

    auto attemptsKey = std::make_shared<SpecialKey>("att", "Adds your geometry dash attempt count (shared with linked levels)");
    attemptsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onATTKey));
    addSpecialString(attemptsKey);

    auto levelNameKey = std::make_shared<SpecialKey>("lvln", "Adds the current levels name");
    levelNameKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onLVLNKey));
    addSpecialString(levelNameKey);

    auto generalKey = std::make_shared<SpecialKey>("general", "Adds all your runs from 0% (shared with linked levels)");
    generalKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onGeneralKey));
    addSpecialString(generalKey);

    auto dtattKey = std::make_shared<SpecialKey>("dtatt", "Adds your death tracker attempt count (shared with linked levels)");
    dtattKey->refreshWith(generalKey->getKey());
    dtattKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDTATTKey));
    addSpecialString(dtattKey);

    auto runsKey = std::make_shared<SpecialKey>("runs", "Adds all your runs from practice mode/start positions (shared with linked levels)");
    runsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onRUNSKey));
    addSpecialString(runsKey);

    auto sessionFrom0Key = std::make_shared<SpecialKey>("s0", "Adds all your runs on the selected session from 0");
    sessionFrom0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onS0Key));
    addSpecialString(sessionFrom0Key);

    auto sessionRunsKey = std::make_shared<SpecialKey>("sruns", "Adds all your runs on the selected session");
    sessionRunsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSRUNSKey));
    addSpecialString(sessionRunsKey);

    auto aptallKey = std::make_shared<SpecialKey>("aptgen", "Adds your total estimated calculated playtime (shared with linked levels)");
    aptallKey->refreshWith({
        generalKey->getKey(),
        runsKey->getKey()
    });
    aptallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTALLSKey));
    addSpecialString(aptallKey);

    auto aptf0Key = std::make_shared<SpecialKey>("aptf0", "Adds your total estimated calculated playtime from 0 (shared with linked levels)");
    aptf0Key->refreshWith(generalKey->getKey());
    aptf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTF0SKey));
    addSpecialString(aptf0Key);

    auto aptrunKey = std::make_shared<SpecialKey>("aptruns", "Adds your total estimated calculated playtime in runs (shared with linked levels)");
    aptrunKey->refreshWith(runsKey->getKey());
    aptrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTRUNSKey));
    addSpecialString(aptrunKey);

    auto aptsallKey = std::make_shared<SpecialKey>("aptsgen", "Adds your total estimated calculated session playtime");
    aptsallKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    aptsallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTSALLSKey));
    addSpecialString(aptsallKey);

    auto aptsf0Key = std::make_shared<SpecialKey>("aptsf0", "Adds your total estimated calculated session playtime from 0");
    aptsf0Key->refreshWith(sessionFrom0Key->getKey());
    aptsf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTSF0Key));
    addSpecialString(aptsf0Key);

    auto aptsrunKey = std::make_shared<SpecialKey>("aptsruns", "Adds your total estimated calculated session playtime in runs");
    aptsrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTSRUNSKey));
    addSpecialString(aptsrunKey);

    auto runsTo100Key = std::make_shared<SpecialKey>("rt100", "Adds all your runs to 100");
    runsTo100Key->refreshWith(generalKey->getKey());
    runsTo100Key->refreshWith(runsKey->getKey());
    runsTo100Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onRunsTo100Key));
    addSpecialString(runsTo100Key);

    auto bRunsKey = std::make_shared<SpecialKey>("bruns", "Adds all your best runs from each percent");
    bRunsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onBestRunsKey));
    addSpecialString(bRunsKey);

    auto sAttKey = std::make_shared<SpecialKey>("satt", "Adds your attempt count for the selected session");
    sAttKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    sAttKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSAttKey));
    addSpecialString(sAttKey);

    auto ptallKey = std::make_shared<SpecialKey>("ptgen", "Adds your total accurate calculated playtime (shared with linked levels)");
    ptallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTALLSKey));
    addSpecialString(ptallKey);

    auto ptf0Key = std::make_shared<SpecialKey>("ptf0", "Adds your total accurate calculated playtime from 0 (shared with linked levels)");
    ptf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTF0SKey));
    addSpecialString(ptf0Key);

    auto ptrunKey = std::make_shared<SpecialKey>("ptruns", "Adds your total accurate calculated playtime in runs (shared with linked levels)");
    ptrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTRUNSKey));
    addSpecialString(ptrunKey);

    auto ptsallKey = std::make_shared<SpecialKey>("ptsgen", "Adds your total accurate calculated session playtime");
    ptsallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTSALLSKey));
    addSpecialString(ptsallKey);

    auto ptsf0Key = std::make_shared<SpecialKey>("ptsf0", "Adds your total accurate calculated session playtime from 0");
    ptsf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTSF0Key));
    addSpecialString(ptsf0Key);

    auto ptsrunKey = std::make_shared<SpecialKey>("ptsruns", "Adds your total accurate calculated session playtime in runs");
    ptsrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTSRUNSKey));
    addSpecialString(ptsrunKey);

    auto sectionKey = std::make_shared<SpecialKey>("section", "Adds your section runs");
    sectionKey->refreshWith(runsKey->getKey());
    sectionKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSectionKey));
    addSpecialString(sectionKey);

    auto sdateKey = std::make_shared<SpecialKey>("sdate", "Adds the date of the current session grouping");
    sdateKey->refreshWith(sessionFrom0Key->getKey());
    sdateKey->refreshWith(sessionRunsKey->getKey());
    sdateKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSessionDateKey));
    addSpecialString(sdateKey);
}

void DTLayer::UpdateSharedStats(){
    if (m_MyLevelStats.isErr()){
        linkedLevelsData.clear();
        return;
    }
    auto sharedStats = m_MyLevelStats.unwrap();

    linkedLevelsData.clear();
    sessionsOrder.grouping.clear();
    sessionsOrder.groupName = "None";

    std::set<std::string> linkedLevels{};
    std::map<std::string, LevelData> visitedLevels{};
    linkedLevels.insert(sharedStats.metadata.linkedLevels.begin(), sharedStats.metadata.linkedLevels.end());

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

            linkedLevels.insert(currStats.metadata.linkedLevels.begin(), currStats.metadata.linkedLevels.end());
            visitedLevels.insert({currStats.levelKey, currStats});
        }

        if (startSize == linkedLevels.size()) break;
    }

    visitedLevels.insert({sharedStats.levelKey, sharedStats});

    for (const auto& [_, level] : visitedLevels){
        std::for_each(level.sessionNames.begin(), level.sessionNames.end(), [&](long long key) {
            if (this->sessionsOrder.grouping.contains(key)){
                if (!this->sessionsOrder.grouping[key].group[key].contains(level.levelKey))
                    this->sessionsOrder.grouping[key].group[key].insert(level.levelKey);
            }
            else{
                SessionGrouping grouping{};
                grouping.group.insert({key, {level.levelKey}});
                this->sessionsOrder.grouping.insert({key, grouping});
            }
        });

        linkedLevelsData.push_back(level);
    }

    if (sessionSelector != nullptr)
        sessionSelector->setMaximumCount(sessionsOrder.grouping.size(), false);

    updateStaticGroupings();

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
    else{
        DTLayer::onClose(nullptr);
    }
}

void DTLayer::onClose(CCObject* sender){
    if (m_MyLevelStats.isOk() && Settings::getAutoBackupEnabled() && Settings::getAutoBackupAtDTExit()){
        if (m_MyLevelStats.unwrap().metadata.autoBackup){
            (void)StatsManager::addBackup(
                m_MyLevelStats.unwrap().levelKey,
                Settings::getAutoBackupGeneral(),
                Settings::getAutoBackupSessionAmount()
            );
        }
    }

    organizationListener.cancel();

    for (auto& [_, key] : specialStrings) {
        if (key)
        {
            key->setUpdateCompletedCallback(nullptr);
            key->setUpdateStartedCallback(nullptr);
            key->cancel();
        }
    }

    keyListeners.clear();

    instance = nullptr;

    Popup::onClose(sender);
}

void DTLayer::show(){
    Popup::show();
    this->setZOrder(100);

    m_mainLayer->stopAllActions();
    m_mainLayer->setScale(1);
}

void DTLayer::keyDown(enumKeyCodes key, double d){
    scrollLayer->keyDown(key, d);
}
void DTLayer::keyUp(enumKeyCodes key, double d){
    scrollLayer->keyUp(key, d);
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
        if (key->getKey() == "general" || key->getKey() == "s0" || key->getKey() == "runs" || key->getKey() == "sruns")
            key->updateContent();
    }
}

void DTLayer::updateStaticGroupings(){
    daySGroup.groupName = "Day";
    daySGroup.grouping.clear();
    weekSGroup.groupName = "Week";
    weekSGroup.grouping.clear();
    monthSGroup.groupName = "Month";
    monthSGroup.grouping.clear();

    for (const auto& [date, group] : sessionsOrder.grouping)
    {
        for (const auto& [date, lvls] : group.group)
        {
            auto coolerDate = std::chrono::sys_seconds{std::chrono::seconds(date)};

            auto startOfDay = std::chrono::system_clock::to_time_t(std::chrono::floor<std::chrono::days>(coolerDate));
            auto startOfWeek = std::chrono::system_clock::to_time_t(std::chrono::floor<std::chrono::weeks>(coolerDate));
            auto startOfMonth = std::chrono::system_clock::to_time_t(std::chrono::floor<std::chrono::months>(coolerDate));
            
            if (!daySGroup.grouping.contains(startOfDay)){
                SessionGrouping grouping{};
                daySGroup.grouping.insert({startOfDay, grouping});
            }
            daySGroup.grouping[startOfDay].group.insert({date, lvls});

            if (!weekSGroup.grouping.contains(startOfWeek)){
                SessionGrouping grouping{};
                weekSGroup.grouping.insert({startOfWeek, grouping});
            }
            weekSGroup.grouping[startOfWeek].group.insert({date, lvls});

            if (!monthSGroup.grouping.contains(startOfMonth)){
                SessionGrouping grouping{};
                monthSGroup.grouping.insert({startOfMonth, grouping});
            }
            monthSGroup.grouping[startOfMonth].group.insert({date, lvls});
        }
    }  
}

bool DTLayer::createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests, const ccColor3B& newBestColoring, bool ignoreExtraSettings){
    out = "";
    if (m_MyLevelStats.isErr()) return false;

    auto toReturn = createDeathsString(deaths, m_MyLevelStats.unwrap().metadata, custom, out, newBests, newBestColoring, ignoreExtraSettings);

    return toReturn;
}

bool DTLayer::createDeathsString(const Deaths& deaths, const LevelMetadeta& meta, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests, const ccColor3B& newBestColoring, bool ignoreExtraSettings){
    out = "";
    
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

    int prevStart = -2;

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

        if (includeRunStart && !ignoreExtraSettings && !meta.showAnyRun){
            if (meta.runsToShow.contains(runSplit.start)){
                if (meta.runsToShow.at(runSplit.start) > runSplit.end)
                    continue;
            }
            else{
                continue;
            }
        }
        else if (!includeRunStart && !ignoreExtraSettings){
            if (meta.hideUpto > runSplit.end)
                continue;

            if (newBests.has_value() && newBests.value().contains(runSplit.end)){
                nbDeColor = "</wave></color>";
                nbColor = fmt::format("<color={}><wave>", cc3bToHexString(newBestColoring));
            }
        }

        if (prevStart != runSplit.start){
            if (prevStart != -2){
                out += "----------{nl}";
            }
            prevStart = runSplit.start;
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
        out += fmt::format("{}{}{}{}", nbColor, format, nbDeColor, custom.seperator);
        // auto toAdd = fmt::format("{}{}", format, custom.seperator);
        // out += toAdd;
    }

    if (out == "")
        out = "No Deaths Found!";
    else {
        out.erase(out.length() - custom.seperator.length());
    }

    return true;
}

int DTLayer::getCurrentSelectedSession(){
    return sessionSelector->getCurrentCount();
}

void DTLayer::onSessionSelected(int sessionNum, bool updateContent){
    auto it = getCurrentGrouping().grouping.begin();
    std::advance(it, sessionNum - 1);

    if (it == getCurrentGrouping().grouping.end()) return;

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

int DTLayer::stoi(std::string_view str) {
    int result = 0;
    std::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
float DTLayer::stof(std::string_view str) {
    return utils::numFromString<float>(str).unwrapOr(0);
}

float DTLayer::timeForLevelString(const std::string& levelString) {
    struct SpeedPortalObject {
        int id;
        float xPos;
        bool checked;
    };

    try {
        auto a = timeInMs();

        auto decompressString = decodeBase64Gzip(levelString);
        auto c = timeInMs();
        std::stringstream responseStream(decompressString);
        std::string currentObject;
        std::string currentKey;
        std::string keyID;
        std::vector<SpeedPortalObject> speedPortals;

        //std::stringstream objectStream;
        float prevPortalX = 0;
        int prevPortalId = 0;

        float timeFull = 0;

        float maxPos = 0;
        while(getline(responseStream, currentObject, ';')){
            size_t i = 0;
            int objID = 0;
            float xPos = 0;
            bool checked = false;

            /*objectStream.str("");
            objectStream.clear();
            objectStream << currentObject;
            objectStream.seekp(0);
            objectStream.seekg(0);*/
            std::stringstream objectStream(currentObject);
            while(getline(objectStream, currentKey, ',')) {
                if(i % 2 == 0) keyID = currentKey;
                else {
                    if(keyID == "1") objID = DTLayer::stoi(currentKey);
                    else if(keyID == "2") xPos = DTLayer::stof(currentKey);
                    else if(keyID == "13") checked = DTLayer::stoi(currentKey);
                    else if(keyID == "kA4") prevPortalId = speedToPortalId(DTLayer::stoi(currentKey));
                }
                i++;

                if(xPos != 0 && objID != 0 && checked == true) break;
            }

            if(maxPos < xPos) maxPos = xPos;
            if(!checked || !objectIDIsSpeedPortal(objID)) continue;

            speedPortals.push_back({objID, xPos, checked});
        }

        std::sort(speedPortals.begin(), speedPortals.end(), [](const SpeedPortalObject& a, const SpeedPortalObject& b) {
            return a.xPos < b.xPos;
        });

        for(const auto& portal : speedPortals) {
            //log::info("Object ID: {}, X Position: {}, Portal ID: {}", portal.id, portal.xPos, prevPortalId);
            timeFull += (portal.xPos - prevPortalX) / travelForPortalId(prevPortalId);
            prevPortalId = portal.id;
            prevPortalX = portal.xPos;
        }

        //log::info("Last portal ID: {}, Last X Position: {}", prevPortalId, prevPortalX);
        timeFull += (maxPos - prevPortalX) / travelForPortalId(prevPortalId);
        auto b = timeInMs() - a;
        //log::info("Time for levelString: {}ms, decompress: {}ms, parse: {}ms, maxPos {}", b, c - a, timeInMs() - c, maxPos);
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
    if (!canOrganize) return;
    organizationListener.spawn(
        organizeLayoutTask(),
        [this](organizationFuture::Output result){
            if (scrollLayer->isAtMinZoom())
                scrollLayer->zoomBy(0.001f);


            float fixedhighest = result.highestColumn - (isEditingLayout ? 0 : LayoutColumn::topHeight + LayoutColumn::addNewBtnOffset * 2);
            for (const auto& column : columns){
                column->setContentHeight(fixedhighest);
            }

            float cappedHeight = std::max(fixedhighest, ogLimits.height);

            float oldHeightLimits = scrollLayer->content->getContentHeight();

            float oldTop = scrollLayer->getMaximumPosition(false);
            float oldBottom = scrollLayer->getMinimumPosition(false);
            float oldHeight = oldBottom - oldTop;

            float offsetFromTop = scrollLayer->content->getPositionY() - oldTop;

            scrollLayer->setLimitsHeight(cappedHeight);
            columnHolder->setPositionY(cappedHeight);
            columnHolder->updateLayout();
            labelsHolder->setPosition(columnHolder->getPosition());

            float delta = 0;

            delta = oldHeightLimits - scrollLayer->content->getContentHeight();

            scrollLayer->moveBy(ccp(0, delta / 2));
            float newTop = scrollLayer->getMaximumPosition(false);
            float newBottom = scrollLayer->getMinimumPosition(false);
            float newHeight = newBottom - newTop;

            float newOffset = offsetFromTop * (newHeight / oldHeight);

            scrollLayer->content->setPositionY(newTop + newOffset);

            for (const auto& [label, newPos, newWidth] : result.labelData)
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

            for (const auto& [target, _] : onOrganizationCompleteEvent)
            {
                onOrganizationCompleteEvent[target](delta);
            }

            if (cornerOnNextOrganization){
                cornerOnNextOrganization = false;
                scrollLayer->moveToCorner(true, false);
            }

            if (firstTime == 0){
                firstTime = 1;
                scrollLayer->setVisible(true);
                lc->removeMeAndCleanup();
                cornerOnNextOrganization = false;

                this->organizeLayout();
            }
            else if (firstTime == 1){
                firstTime = 2;

                scrollLayer->moveToCorner(true, false);
            }
        }
    );
}

organizationFuture DTLayer::organizeLayoutTask(){
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

    co_await arc::yield();

    std::set<DTLabel*> allLabels{};
    std::map<DTLabel*, std::set<LayoutColumn*>> labelHolders{};

    for (const auto& colData : columnSnapshots)
    {
        for (const auto& [layer, label] : colData.labels)
        {
            co_await arc::yield();
            if (allLabels.contains(label)) continue;

            allLabels.insert(label);
            label->tempPos = ccp(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            label->tempWidth = 0;
        }

        co_await arc::yield();
    }

    for (const auto& [label, data] : labelSnapshots)
    {
        labelHolders[label] = data.holders;

        co_await arc::yield();
    }

    std::map<LayoutColumn*, DTLabel*> lastVisitedLabelForColumn{};
    std::map<DTLabel*, std::map<LayoutColumn*, std::optional<int>>> labelAwaitingColumnValues{};
    std::set<DTLabel*> processedLabels{};

    auto UpdateTempPos = [&](LayoutColumn* column, DTLabel* label, DTLabel* prevLabel) -> arc::Future<> {
        if (column == nullptr || label == nullptr || label->getParent() == nullptr) co_return;
        auto startPosInLabelSpace = label->getParent()->convertToNodeSpace(column->convertToWorldSpace(column->bgSpr->getPosition()));

        float prevHeight = startPosInLabelSpace.y;

        if (label->info.layer != 0 && prevLabel != nullptr) prevHeight = prevLabel->tempPos.y - prevLabel->getContentHeight();

        if (prevHeight < label->tempPos.y) label->tempPos.y = prevHeight;

        float newX = startPosInLabelSpace.x;

        if (newX < label->tempPos.x) label->tempPos.x = newX;
    };

    auto UpdateTempWidth = [](DTLabel* label) -> arc::Future<> {
        label->tempWidth = 0;
        for (const auto& labelColumn : label->getHolders())
        {
            label->tempWidth += labelColumn->getContentWidth();

            co_await arc::yield();
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
                    co_await UpdateTempPos(column, label, prevLabel);
                    prevLabel = label;
                    continue;
                }
                else foundLastLabel  = true;

                if (processedLabels.contains(label)){
                    co_await UpdateTempPos(column, label, prevLabel);
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

                        for (const auto& holder : holdersBack){
                            mapToSet.insert({holder, std::nullopt});
                            co_await arc::yield();
                        }

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

                    co_await UpdateTempPos(column, label, prevLabel);

                    // log::info("checking conclusion..");

                    for (const auto& [column, optLayer] : labelAwaitingColumnValues[label]){
                        if (!optLayer.has_value()){
                            wereAllLayersFound = false;
                            co_await arc::yield();
                            break;
                        }

                        highestOptLayer = std::max(highestOptLayer, optLayer.value());
                        co_await arc::yield();
                    }

                    if (!wereAllLayersFound){
                        // log::info("invalid labels were found");
                        co_await arc::yield();
                        break;
                    }

                    // log::info("all layer values were found for label");

                    label->info.layer = highestOptLayer;
                    processedLabels.insert(label);

                    co_await UpdateTempWidth(label);

                    // log::info("combo found at {}", highestOptLayer);

                    prevLabel = label;
                    continue;
                }

                // log::info("non double found! adding..");

                label->info.layer = newLayer;
                processedLabels.insert(label);

                co_await UpdateTempPos(column, label, prevLabel);
                co_await UpdateTempWidth(label);

                // log::info("single found at {}", newLayer);

                prevLabel = label;
            }
            co_await arc::yield();
        }

        // log::info("res: {} | {}", processedLabels.size(), allLabels.size());

        if (processedLabels.size() == allLabels.size()) break;
        co_await arc::yield();
    }

    for (const auto& colData : columnSnapshots){
        colData.column->refreshAllLabelsLayer();
        co_await arc::yield();
    }

    float heighestHeight = 0;

    for (const auto& colData : columnSnapshots){
        for (const auto& [_, label] : colData.labels)
        {
            auto height = std::abs(label->tempPos.y) + label->getContentHeight() + LayoutColumn::addNewBtnOffset * 2;
            if (heighestHeight < height) heighestHeight = height;
            co_await arc::yield();
        }
        co_await arc::yield();
    }

    organizationResult data{};
    data.highestColumn = heighestHeight;

    for (const auto& label : allLabels)
    {
        auto targetPosition = label->tempPos;
        auto targetWidth = label->tempWidth - LayoutColumn::borderWidth;

        data.labelData.push_back({label, targetPosition, targetWidth});
        co_await arc::yield();
    }

    co_await arc::yield();
    co_return data;
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

std::set<LayoutColumn*, ColumnComperator> DTLayer::getColumnsBetween(CCPoint a, CCPoint b){

    std::set<LayoutColumn*, ColumnComperator> toReturn{};

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


void DTLayer::subscribeToOrganizationEvent(CCNode* target, geode::Function<void(float)> callback){
    if (onOrganizationCompleteEvent.contains(target)) return;

    onOrganizationCompleteEvent.insert({target, std::move(callback)});
}
void DTLayer::unsubscribeToOrganizationEvent(CCNode* target){
    if (!onOrganizationCompleteEvent.contains(target)) return;

    onOrganizationCompleteEvent.erase(target);
}

void DTLayer::setLayoutBy(const DTLayoutV3& layout)
{
    for (auto* column : columns)
    {
        column->destroyColumnAndCleanup();
    }
    columns.clear();

    for (const auto& column : layout.columns)
    {
        addColumn(column);
    }

    fixUpColumnPositions();
    columnHolder->updateLayout();

    std::map<LayoutColumn*, std::vector<DTLabel*>> labelsForColumns;
    std::vector<DTLabel*> allLabels;
    allLabels.reserve(layout.labels.size());

    for (const auto& label : layout.labels)
    {
        DTLabel* labelNode = createNewLabel(label);
        allLabels.push_back(labelNode);

        const int minPos = labelNode->info.minPlacementRange;
        const int maxPos = labelNode->info.maxPlacementRange;

        for (auto* column : columns)
        {
            const int pos = column->info.orderPos;

            if (pos < minPos)
                continue;

            if (pos > maxPos)
                break;

            labelsForColumns[column].push_back(labelNode);
        }
    }

    for (auto& [column, labels] : labelsForColumns)
    {
        for (auto* label : labels)
        {
            column->addLabel(label);
        }
    }

    for (auto* label : allLabels)
    {
        if (label->isAlone())
        {
            label->removeMeAndCleanup();
        }
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

    for (const auto& [_, otherKey] : specialStrings)
    {
        if (otherKey->doesRefreshWith(key->getKey()))
            otherKey->updateContent();
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

    bottomLeftMenu->setEnabled(true);
    bottomLeftMenu->stopAllActions();
    bottomLeftMenu->runAction(CCFadeTo::create(.15f, 255));

    bottomRightMenu->setEnabled(true);
    bottomRightMenu->stopAllActions();
    bottomRightMenu->runAction(CCFadeTo::create(.15f, 255));

    m_buttonMenu->setEnabled(true);
    m_buttonMenu->stopAllActions();
    m_buttonMenu->runAction(CCFadeTo::create(.15f, 255));
    editLayoutBtnSpr->stopAllActions();
    editLayoutBtnSpr->runAction(CCFadeTo::create(.15f, 255));

    editLayoutMenu->setEnabled(false);
    layoutInfo->stopAllActions();
    layoutInfo->runAction(CCFadeTo::create(.15f, 0));
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
        log::info("{} | {} | {}", amount, data[runStr], newNum);

        if (newNum <= 0){
            data.erase(runStr);
            return true;
        }

        data[runStr] = newNum;

        return true;
    };

    if (sessionNumber.has_value()){
        auto it = getCurrentGrouping().grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()){
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.data.deaths.contains(std::to_string(startPer))) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()){
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        if (!processRun(session.value().data.deaths)) return;

        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processRun(from0Stats.deaths)){
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        //log::info("pros linked");
        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr() || linkedLevel.levelKey == stats.levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();

            if (processRun(linkedLevelFrom0Stats.deaths)){
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
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
        auto it = getCurrentGrouping().grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()){
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.data.runs.contains(fmt::format("{}-{}", startPer, endPer))) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()){
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        if (!processRun(session.value().data.runs)) return;

        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processRun(from0Stats.runs)){
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr() || linkedLevel.levelKey == stats.levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();

            if (processRun(linkedLevelFrom0Stats.runs)){
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
                return;
            }
        }
    }
}

void DTLayer::modifyNewBest(int percent, bool makeTrue, std::optional<int> sessionNumber){
    if (m_MyLevelStats.isErr()) return;

    auto processBest = [&, percent, makeTrue](NewBests& bests) -> bool {
        if (makeTrue){
            if (bests.contains(percent)) return true;

            bests.insert(percent);
        }
        else{
            if (!bests.contains(percent)) return true;

            bests.erase(percent);
        }

        return true;
    };

    if (sessionNumber.has_value()){
        auto it = getCurrentGrouping().grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()){
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.data.newBests.contains(percent)) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()){
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        processBest(session.value().data.newBests);

        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processBest(from0Stats.newBests)){
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr() || linkedLevel.levelKey == stats.levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();

            if (processBest(linkedLevelFrom0Stats.newBests)){
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
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

UpdateFuture DTLayer::onNLKey(){
    co_return Ok(std::string("\n"));
}
UpdateFuture DTLayer::onATTKey(){
    long long totalAttempts = 0;
    for (const auto& lebel : linkedLevelsData)
        totalAttempts += lebel.metadata.attempts;

    co_return Ok(std::to_string(totalAttempts));
}
UpdateFuture DTLayer::onLVLNKey(){
    co_return Ok(std::string(m_Level->m_levelName));
}
UpdateFuture DTLayer::onGeneralKey(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to create from0 deaths string (no save)");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    auto linkedLevelsCopy = linkedLevelsData;

    Deaths sharedDeaths;
    StatsManager::mergeMapsAdd(sharedDeaths, myFrom0Stats.deaths);
    NewBests sharedNBs = myFrom0Stats.newBests;

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();

        StatsManager::mergeMapsAdd(sharedDeaths, levelFrom0Stats.deaths);
        sharedNBs.insert(levelFrom0Stats.newBests.begin(), levelFrom0Stats.newBests.end());
    }

    std::string out;
    if (!createDeathsString(sharedDeaths, Save::getFrom0Customazations(), out, sharedNBs, Save::getNewBestColor()))
        co_return Err("Failed to create from0 deaths string");

    co_return Ok(out);
}
UpdateFuture DTLayer::onDTATTKey(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to create death tracker attempts string");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    auto linkedLevelsCopy = linkedLevelsData;

    unsigned long long attempts = 0;

    auto deaths = [&attempts](const Deaths& d) -> arc::Future<> {
        for (const auto& [_, count] : d){
            attempts += count;
            co_await arc::yield();
        }
    };

    co_await deaths(myFrom0Stats.deaths);
    co_await deaths(myFrom0Stats.runs);

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();

        co_await deaths(levelFrom0Stats.deaths);
        co_await deaths(levelFrom0Stats.runs);
    }

    co_return Ok(std::to_string(attempts));
}
UpdateFuture DTLayer::onRUNSKey(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to create run deaths string");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    auto linkedLevelsCopy = linkedLevelsData;

    Deaths sharedRuns;
    StatsManager::mergeMapsAdd(sharedRuns, myFrom0Stats.runs);

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();
        StatsManager::mergeMapsAdd(sharedRuns, levelFrom0Stats.runs);
    }

    std::string out;
    if (!createDeathsString(sharedRuns, Save::getRunsCustomazations(), out))
        co_return Err("Failed to create run deaths string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onS0Key(){
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err("{}", sessionRes.unwrapErr());

    auto session = sessionRes.unwrap();

    co_await arc::yield();

    std::string out;
    if (!createDeathsString(session.data.deaths, Save::getSessionF0Customazations(), out, session.data.newBests, Save::getSessionBestColor()))
        co_return Err("Failed to create session from0 deaths string");

    co_return Ok(out);
}
UpdateFuture DTLayer::onSRUNSKey(){
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err("{}", sessionRes.unwrapErr());

    auto session = sessionRes.unwrap();

    co_await arc::yield();

    std::string out;
    if (!createDeathsString(session.data.runs, Save::getRunsCustomazations(), out))
        co_return Err("Failed to create session run deaths string");

    co_return Ok(out);
}

long long DTLayer::calcPlaytime(const Deaths& deaths){
    long double playtime = 0;

    // auto __calc_start_ms = timeInMs();

    float wt;
    if (!cachedLevelLength.has_value()){
        wt = m_Level->m_timestamp
            ? m_Level->m_timestamp / 240
            : m_Level->isPlatformer() ? 0 : std::ceil(timeForLevelString(m_Level->m_levelString));

        cachedLevelLength = wt;
    }
    else{
        wt = cachedLevelLength.value();
    }

    for (const auto& death : deaths)
    {
        auto runSplitRes = StatsManager::splitRunKey(death.first);
        if (runSplitRes.isErr()) continue;

        float runLength;
        if (runSplitRes.unwrap().start == -1)
            runLength = runSplitRes.unwrap().end;
        else
            runLength = (runSplitRes.unwrap().end - runSplitRes.unwrap().start);
        // log::info("{} | {} | {}", runLength, runSplitRes.unwrap().end, runSplitRes.unwrap().start);
        // log::info("{}", death.second);

        long double runOverallPlaytime = 0;

        if (runLength != 100)
            runLength += 0.5f;

        runOverallPlaytime = wt * (runLength / 100.0f) * death.second;

        playtime += runOverallPlaytime;
    }

    // auto __calc_elapsed_ms = timeInMs() - __calc_start_ms;
    // log::info("calcPlaytime took {} ms", __calc_elapsed_ms);

    return playtime;
}

UpdateFuture DTLayer::onAPTALLSKey(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate playtime");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    auto linkedLevelsCopy = linkedLevelsData;

    Deaths deaths{};
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.deaths);
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.runs);

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();
        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.deaths);
        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.runs);
    }

    auto pt = StatsManager::workingTime(calcPlaytime(deaths));
    co_return Ok(pt);
}

UpdateFuture DTLayer::onAPTF0SKey(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate from 0 playtime");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    auto linkedLevelsCopy = linkedLevelsData;

    Deaths deaths{};
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.deaths);

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();
        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.deaths);
    }

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}
UpdateFuture DTLayer::onAPTRUNSKey(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate runs playtime");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    auto linkedLevelsCopy = linkedLevelsData;

    Deaths deaths{};
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.runs);

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();
        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.runs);
    }

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}

UpdateFuture DTLayer::onAPTSALLSKey(){
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err("{}", sessionRes.unwrapErr());
    auto session = sessionRes.unwrap();

    Deaths deaths;
    StatsManager::mergeMapsAdd(deaths, session.data.deaths);
    StatsManager::mergeMapsAdd(deaths, session.data.runs);

    co_await arc::yield();

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}
UpdateFuture DTLayer::onAPTSF0Key(){
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err("{}", sessionRes.unwrapErr());
    auto session = sessionRes.unwrap();

    co_await arc::yield();

    co_return Ok(StatsManager::workingTime(calcPlaytime(session.data.deaths)));
}
UpdateFuture DTLayer::onAPTSRUNSKey(){
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err("{}", sessionRes.unwrapErr());
    auto session = sessionRes.unwrap();

    co_await arc::yield();

    co_return Ok(StatsManager::workingTime(calcPlaytime(session.data.runs)));
}

Result<Session> DTLayer::loadSessionFromSave(std::optional<int> sessionIndex){
    if (!getCurrentGrouping().grouping.size()) return Err("No sessions saved!");
    int i = sessionIndex.has_value() ? sessionIndex.value() : sessionSelector->getCurrentCount();

    if (i == 0 || i > getCurrentGrouping().grouping.size())
        return Err("Failed to get session, not in range");

    auto it = getCurrentGrouping().grouping.begin();
    std::advance(it, i - 1);

    Result<Session> sess = Err("");

    for (const auto& [SDate, lvlKeys] : it->second.group)
    {
        for (const auto& lvlKey : lvlKeys)
        {
            auto sessionRes = StatsManager::getSession(lvlKey, SDate);
            if (sessionRes.isErr()){
                sess = Err(sessionRes.unwrapErr());
                break;
            }

            if (sess.isErr()){
                sess = Ok(sessionRes.unwrap());
            }
            else{
                auto& overallRef = sess.unwrap();

                overallRef.data += sessionRes.unwrap().data;
            }
        }
    }

    if (sess.isOk()){
        auto& sessRef = sess.unwrap();
        sessRef.groupID = it->first;
    }
        
    return sess;
}

UpdateFuture DTLayer::onRunsTo100Key(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate runs playtime");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    auto linkedLevelsCopy = linkedLevelsData;

    Deaths deaths{};
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.runs);
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.deaths);

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();

        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.runs);
        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.deaths);
    }

    Deaths to100Deaths{};

    for (const auto& death : deaths)
    {
        co_await arc::yield();
        auto splitRunRes = StatsManager::splitRunKey(death.first);
        if (splitRunRes.isErr()) continue;
        auto splitRun = splitRunRes.unwrap();

        if (splitRun.end != 100) continue;

        to100Deaths.insert(death);
    }

    std::string out;
    if (!createDeathsString(to100Deaths, Save::getRunsCustomazations(), out))
        co_return Err("Failed to create runs to 100 string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onBestRunsKey(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate runs playtime");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    auto linkedLevelsCopy = linkedLevelsData;

    Deaths deaths{};
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.runs);
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.deaths);

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();

        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.runs);
        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.deaths);
    }

    std::map<int, int> bestRuns{};

    for (const auto& death : deaths)
    {
        co_await arc::yield();
        auto splitRunRes = StatsManager::splitRunKey(death.first);
        if (splitRunRes.isErr()) continue;
        auto splitRun = splitRunRes.unwrap();

        if (!bestRuns.contains(splitRun.start))
            bestRuns.insert({splitRun.start, splitRun.end});
        else if (bestRuns[splitRun.start] < splitRun.end){
            bestRuns[splitRun.start] = splitRun.end;
        }
    }

    Deaths bestRunDeaths{};

    for (const auto& [bestRunStart, bestRunEnd] : bestRuns)
    {
        co_await arc::yield();
        auto runStringRes = StatsManager::createRunKey(Run{bestRunStart, bestRunEnd});
        if (runStringRes.isErr()) continue;
        auto runString = runStringRes.unwrap();
        if (!deaths.contains(runString)) continue;

        bestRunDeaths.insert({runString, deaths[runString]});
    }

    std::string out;
    if (!createDeathsString(bestRunDeaths, Save::getRunsCustomazations(), out))
        co_return Err("Failed best runs string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onSAttKey(){
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err("{}", sessionRes.unwrapErr());
    auto session = sessionRes.unwrap();

    unsigned long long attempts = 0;

    auto deathsCalc = [&attempts](const Deaths& d) -> arc::Future<> {
        for (const auto& [_, count] : d){
            attempts += count;
            co_await arc::yield();
        }
    };

    co_await deathsCalc(session.data.deaths);
    co_await deathsCalc(session.data.runs);
    
    co_return Ok(std::to_string(attempts));
}

void DTLayer::foreachLinkedLevel(geode::Function<void(LevelData&)> onLevelVisit){
    if (m_MyLevelStats.isErr()) return;
    auto& myStats = m_MyLevelStats.unwrap();

    for (auto& levelData : linkedLevelsData)
    {
        if (levelData.levelKey == myStats.levelKey) continue;
        onLevelVisit(levelData);
    }
}

UpdateFuture DTLayer::onPTALLSKey(){
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeF0 + data.playtimeGeneral.playtimeRuns;
    }, false);
}

// currently same as PTALLSKey since theres no difference between playtime on runs and playtime from 0
// TODO: make distinguishment between runs and from 0 playtime? remove key?
UpdateFuture DTLayer::onPTF0SKey() {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeF0;
    }, false);
}

// same thing here, no distinction between playtime from 0 and playtime from runs
UpdateFuture DTLayer::onPTRUNSKey() {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPTSALLSKey() {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeF0 + data.playtimeGeneral.playtimeRuns;
    }, true);
}

// again, no difference between playtime from 0 and playtime from runs, consider removing?
UpdateFuture DTLayer::onPTSF0Key() {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeF0;
    }, true);
}

// again, no difference between playtime from 0 and playtime from runs, consider removing?
UpdateFuture DTLayer::onPTSRUNSKey() {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onSectionKey(){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate runs playtime");
    auto myStats = m_MyLevelStats.unwrap();
    if (myStats.from0.isErr()) co_return Err("No deaths saved!");
    auto myFrom0Stats = myStats.from0.unwrap();

    std::vector<Section> validSections{};
    for (const auto& section : myStats.metadata.sections)
    {
        if (!section.isValid()) continue;

        validSections.push_back(section);
    }
    
    if (validSections.size() <= 1) co_return Err("Not enough sections!");

    auto linkedLevelsCopy = linkedLevelsData;

    Deaths deaths{};
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.runs);
    StatsManager::mergeMapsAdd(deaths, myFrom0Stats.deaths);

    for (const auto& levelData : linkedLevelsCopy)
    {
        co_await arc::yield();
        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();
        
        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.runs);
        StatsManager::mergeMapsAdd(deaths, levelFrom0Stats.deaths);
    }

    std::unordered_map<std::string, int> deathsPerSection{};

    auto CreateSectioIDForSectionPair = [&](const Run& splitDeath, const std::optional<Section>& startingSection, int deaths){
        Section endingSection;
        for (const auto& section : validSections)
        {
            if (!section.isPercentInSection(splitDeath.end)) continue;

            endingSection = section;
            break;
        }

        std::string sectionID = "";
        if (!startingSection.has_value()){
            sectionID = fmt::format("{}", endingSection.name);
        }
        else
            sectionID = fmt::format("{}-{}", startingSection.value().name, endingSection.name);
        if (!deathsPerSection.contains(sectionID))
            deathsPerSection.insert({sectionID, deaths});
        else
            deathsPerSection[sectionID] += deaths;
    };

    for (const auto& death : deaths){
        auto splitDeathRes = StatsManager::splitRunKey(death.first);
        if (splitDeathRes.isErr()) continue;
        auto splitDeath = splitDeathRes.unwrap();

        if (splitDeath.start == -1){
            CreateSectioIDForSectionPair(splitDeath, std::nullopt, death.second);
            continue;
        }
        
        std::vector<Section> startingSectionsForDeath{};

        for (const auto& section : validSections)
        {
            if (!section.isPercentInSection(splitDeath.start)) continue;

            startingSectionsForDeath.push_back(section);
        }

        for (const auto& startingSection : startingSectionsForDeath)
        {
            CreateSectioIDForSectionPair(splitDeath, startingSection, death.second);
        }
    }

    std::string out;

    std::vector<int> order;
    order.reserve(validSections.size());
    for (size_t i = 0; i < validSections.size(); ++i) order.push_back(i);
    std::sort(order.begin(), order.end(), [&](int a, int b){
        return validSections[a].startPercent < validSections[b].startPercent;
    });

    auto custom = Save::getFrom0Customazations();

    for (const auto& startIdx : order)
    {
        for (const auto& endIdx : order)
        {
            auto sectionID = (startIdx == endIdx) ? validSections[startIdx].name : fmt::format("{}-{}", validSections[startIdx].name, validSections[endIdx].name);
            auto it = deathsPerSection.find(sectionID);
            if (it == deathsPerSection.end()) continue;

            auto format = custom.format;
            format = std::regex_replace(format, std::regex("\\{per\\}"), sectionID);
            format = std::regex_replace(format, std::regex("\\{d\\}"), std::to_string(it->second));

            out += fmt::format("{}{}", format, custom.seperator);
        }

        if (startIdx != order.back())
            out += "----------{nl}";
    }

    if (!out.empty())
        out.erase(out.length() - custom.seperator.length());

    co_return Ok(out);
}

void DTLayer::onCalculator(CCObject*){
    CalculatorPopup::create()->show();
}

UpdateFuture DTLayer::getPlaytimeFor(geode::Function<uint64_t(GeneralData const&)>&& dataGetter, bool session){
    if (!session){
        if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate playtime");
        auto myStats = m_MyLevelStats.unwrap();
        if (myStats.from0.isErr()) co_return Err("No deaths saved!");
        auto myFrom0Stats = myStats.from0.unwrap();

        auto linkedLevelsCopy = linkedLevelsData;

        uint64_t alltime = dataGetter(myFrom0Stats);

        for (const auto& levelData : linkedLevelsCopy)
        {
            co_await arc::yield();
            if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
            auto levelFrom0Stats = levelData.from0.unwrap();
            alltime += dataGetter(levelFrom0Stats);
        }

        co_return Ok(StatsManager::workingTime(alltime));
    }
    else{
        auto sessionRes = loadSessionFromSave();
        if (sessionRes.isErr()) co_return Err("{}", sessionRes.unwrapErr());
        auto session = sessionRes.unwrap();

        co_await arc::yield();

        co_return Ok(StatsManager::workingTime(
            dataGetter(session.data)
        ));
    }
}

SessionCategory& DTLayer::getCurrentGrouping(){
    if (m_MyLevelStats.isErr()) return sessionsOrder;
    auto& myStats = m_MyLevelStats.unwrap();

    if (currentGrouping == -3) return daySGroup;
    if (currentGrouping == -2) return weekSGroup;
    if (currentGrouping == -1) return monthSGroup;

    if (!myStats.metadata.sessionGroups.size() || currentGrouping < 0 || currentGrouping >= myStats.metadata.sessionGroups.size()) return sessionsOrder;

    return myStats.metadata.sessionGroups[currentGrouping];
}

UpdateFuture DTLayer::onSessionDateKey(){
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err("{}", sessionRes.unwrapErr());

    auto session = sessionRes.unwrap();

    co_await arc::yield();

    auto tp = std::chrono::system_clock::from_time_t(session.groupID);

    std::string dateStr = fmt::format("{:%m/%d/%Y} {:%I:%M%p}", tp, tp);

    co_return Ok(dateStr);
}

void DTLayer::onGroups(CCObject*){
    if (groupsList->isOpened())
        groupsList->close();
    else
        groupsList->open();
}

void DTLayer::onGroupSelected(int const& id){
    groupsList->close();

    currentGrouping = id;
    specialStrings["s0"]->updateContent();
    specialStrings["sruns"]->updateContent();

    auto opt = groupsList->getItemForID(id);

    auto newSpr = ButtonSprite::create(
        opt.value().text.c_str(),
        100,
        100,
        1,
        false,
        opt.value().font.c_str(), 
        opt.value().BGTexture.c_str()
    );
    newSpr->setCascadeOpacityEnabled(true);
    newSpr->setScale(.45f);

    groupsBtn->setSprite(newSpr);

    bottomLeftMenu->updateLayout();
    bottomRightMenu->updateLayout();

    sessionSelector->setMaximumCount(getCurrentGrouping().grouping.size(), true);
}

void DTLayer::CleanGetStats(){
    m_MyLevelStats = StatsManager::getLevelData(m_Level);
    if (m_MyLevelStats.isErr() && m_MyLevelStats.unwrapErr().size() && m_MyLevelStats.unwrapErr()[0] == '1'){
        LevelData newData;
        newData.levelKey = StatsManager::getLevelKey(m_Level).unwrap();
        m_MyLevelStats = Ok(newData);
    }
    else if (m_MyLevelStats.isErr()){
        auto notif = geode::Notification::create(fmt::format("Failed to load DT level data! {}", m_MyLevelStats.unwrapErr()), NotificationIcon::Error, 3);
        notif->show();
        notif->setZOrder(101);
    }

    StatsManager::transferPlaytimeFromPT(m_MyLevelStats, m_Level);

    if (m_MyLevelStats.isOk()){
        auto stats = m_MyLevelStats.unwrap();
        stats.metadata.levelName = m_Level->m_levelName;
        stats.metadata.attempts = m_Level->m_attempts;
        stats.metadata.difficulty = StatsManager::getDifficulty(m_Level);
        (void)StatsManager::setMetadata(stats.metadata, stats.levelKey);
        m_MyLevelStats = Ok(stats);
    }

    StatsManager::setCurrentLevel(m_Level);

    DTLayer::UpdateSharedStats();
}