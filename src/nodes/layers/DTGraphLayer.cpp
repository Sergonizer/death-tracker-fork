#include <nodes/layers/DTGraphLayer.hpp>
#include <utils/Settings.hpp>
#include <utils/Save.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>
#include <nodes/TutorialButton.hpp>

DTGraphLayer* DTGraphLayer::create() {
    auto ret = new DTGraphLayer();
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (ret && ret->initAnchored(winSize.width - 120, winSize.height - 30, "square01_001.png", {0.f, 0.f, 94.f, 94.f})) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTGraphLayer::setup() {

    noGraphLabel = CCLabelBMFont::create("No Graphs\nEnabled!", "bigFont.fnt");
    noGraphLabel->setZOrder(1);
    noGraphLabel->setVisible(false);
    noGraphLabel->setPosition({46, 3});
    m_mainLayer->addChild(noGraphLabel);

    graph = GraphHolder::create(ccp(m_size.width - 140, m_size.height - 50));
    graph->setPosition({m_size.width - graph->getContentWidth() / 2 - 20, m_size.height / 2 + 5});
    graph->delegate = this;
    this->m_mainLayer->addChild(graph);

    auto graphTutorial = TutorialButton::create(.6f, [&](DTTutorialLayer* tl){
        tl->appendDialogue("Welcome to the graph view!", TutorialCharacterFace::TCFNormal)
            ->appendDialogue("This is the main graphs view", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(graph)
            ->joinTransform(TutorialBoxPlacement::TBPBottomLeft, .5f)
            ->appendDialogue("You can hold <cg>control</c> and <cp>scroll</c> to zoom in! and <cc>shift</c> and <cp>scroll</c> to move side to side.", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(graph)
            ->appendDialogue("You can also hover on any of the points in the graph to see their details!", TutorialCharacterFace::TCFNormal);
        for (const auto& graphNode : graph->getAllGraphNodes())
        {
            if (!graphNode->isVisible()) continue;

            for (const auto& child : CCArrayExt<CCNode*>(graphNode->pointHolder->getChildren())){
                tl->joinHighlight(child)
                    ->joinTextToHighlight("V", .5f);
            }
        }
        tl->appendDialogue("Feel free to check out how to customize graphs on the left menu!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter);
    });
    graphTutorial->setPosition(graph->getPosition() + graph->getContentSize() / 2 + ccp(10, 10));
    this->m_buttonMenu->addChild(graphTutorial);

    graphsPage = CCMenu::create();
    graphsPage->setPositionX(55);
    graphsPage->setPositionY(m_size.height - 20);
    graphsPage->ignoreAnchorPointForPosition(false);
    graphsPage->setContentSize({0,0});
    m_mainLayer->addChild(graphsPage);

    graphOptionsPage = CCMenu::create();
    graphOptionsPage->setPositionX(55);
    graphOptionsPage->setPositionY(m_size.height - 20);
    graphOptionsPage->ignoreAnchorPointForPosition(false);
    graphOptionsPage->setContentSize({0,0});
    graphOptionsPage->setScaleY(0);
    m_mainLayer->addChild(graphOptionsPage);

    graphsScroll = ScrollLayer::create({70, m_size.height - 90});
    graphsScroll->ignoreAnchorPointForPosition(false);
    graphsScroll->setAnchorPoint({.5f, 1});
    graphsScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(graphsScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    graphsPage->addChild(graphsScroll);

    auto scrollBG = CCScale9Sprite::create("square02_001.png");
    scrollBG->setContentSize(graphsScroll->getContentSize() + ccp(5, 5));
    scrollBG->setAnchorPoint({.5f,1});
    scrollBG->setPosition(graphsScroll->getPosition());
    scrollBG->setOpacity(100);
    scrollBG->setZOrder(-1);
    graphsPage->addChild(scrollBG);

    auto SessionSelectionLabel = CCLabelBMFont::create("Session", "bigFont.fnt");
    SessionSelectionLabel->setScale(0.3f);
    SessionSelectionLabel->setPositionY(-graphsScroll->getContentHeight() - 5 - SessionSelectionLabel->getScaledContentHeight() / 2);
    graphsPage->addChild(SessionSelectionLabel);

    sessionSelector = SessionSelector::create(DTLayer::get()->sessionsOrder.size());
    sessionSelector->setCurrentCount(DTLayer::get()->getCurrentSelectedSession());
    sessionSelector->setScale(0.45f);
    sessionSelector->setPositionY(SessionSelectionLabel->getPositionY() - SessionSelectionLabel->getScaledContentHeight() / 2 - sessionSelector->getScaledContentHeight() / 2);
    sessionSelector->setCallback([&](auto _){
        graph->sendUpdateToGraphOfType(DTGraphCoverage::SessionCover);
        graph->sendUpdateToGraphOfType(DTGraphCoverage::SessionRunsCover);
    });
    graphsPage->addChild(sessionSelector);

    graph->sessionSelector = sessionSelector;

    auto runSelectInputLabel = CCLabelBMFont::create("Run Percent", "bigFont.fnt");
    runSelectInputLabel->setScale(0.3f);
    runSelectInputLabel->setPositionY(sessionSelector->getPositionY() - 5 - runSelectInputLabel->getScaledContentHeight() / 2 - sessionSelector->getScaledContentHeight() / 2);
    graphsPage->addChild(runSelectInputLabel);

    runSelectInput = TextInput::create(120, "Run %");
    runSelectInput->setCommonFilter(CommonFilter::Uint);
    runSelectInput->setScale(0.45f);
    runSelectInput->setPositionY(runSelectInputLabel->getPositionY() - runSelectInput->getScaledContentHeight() / 2 - runSelectInputLabel->getScaledContentHeight() / 2);
    runSelectInput->setCallback([&](const auto& newStr){
        auto toNumRes = utils::numFromString<int>(newStr);
        if (toNumRes.isErr()) return;
        int num = toNumRes.unwrap();

        if (num > 100){
            runSelectInput->setString("100");
            num = 100;
        }

        graph->setToAllGraphs([num](DTGraphNode* node){
            node->runPercent = num;
        });

        graph->sendUpdateToGraphOfType(DTGraphCoverage::GeneralRunsCover);
        graph->sendUpdateToGraphOfType(DTGraphCoverage::SessionRunsCover);
    });
    graphsPage->addChild(runSelectInput);

    auto addGraphBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    auto addGraphBtn = CCMenuItemSpriteExtra::create(
        addGraphBtnSpr,
        this,
        menu_selector(DTGraphLayer::onAddGraph)
    );
    addGraphBtn->setPositionX(graphsScroll->getContentWidth() / 2 - 7.5f);
    graphsPage->addChild(addGraphBtn);

    auto graphsInfo = Save::getGraphs();

    for (const auto& currGraph : graphsInfo)
    {
        addGraph(currGraph);
    }

    auto sideTutorial = TutorialButton::create(.6f, [&, runSelectInputLabel, SessionSelectionLabel](DTTutorialLayer* tl){
        tl->appendDialogue("This is where you can manage your graphs!", TutorialCharacterFace::TCFNormal)
            ->appendDialogue("These are the graphs you have available", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)
            ->joinHighlight(graphsScroll)
            ->appendDialogue("You can show/hide graphs by clicking the circle colored button right there", TutorialCharacterFace::TCFNormal);
        CCNode* firstBtn = nullptr;
        for (const auto& cell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
        {
            if (firstBtn == nullptr)
                firstBtn = cell->getChildByID("toggle-btn");
            tl->joinHighlight(cell->getChildByID("toggle-btn"));
        }
        tl->appendDialogue("If you toggle one ON, all others will be disabled", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(firstBtn)
            ->joinTextToHighlight(">", .5f, TutorialTextPlacement::TTLeft);
        tl->appendDialogue("Unless you hold SHIFT while toggling to enable multiple graphs!", TutorialCharacterFace::TCFNormal);
        int index = 0;
        for (const auto& cell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
        {
            tl->joinHighlight(cell->getChildByID("toggle-btn"), index * .3f);
            tl->joinTextToHighlight(std::to_string(index + 1), .5f, TutorialTextPlacement::TTLeft);
            index++;
        }
        tl->appendDialogue("You can edit each graphs settings", TutorialCharacterFace::TCFNormal);
        for (const auto& cell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
        {
            tl->joinHighlight(cell->getChildByID("settings-btn"));
        }
        tl->appendDialogue("And you can also change their layering order using the arrows on the right!", TutorialCharacterFace::TCFNormal);
        for (const auto& cell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
        {
            tl->joinHighlight(cell->getChildByID("up-arrow"));
            tl->joinHighlight(cell->getChildByID("down-arrow"));
        }
        tl->appendDialogue("You also have quick access to change the graphs type, more on that in the graph settings page!", TutorialCharacterFace::TCFNormal);
        for (const auto& cell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
        {
            tl->joinHighlight(cell->getChildByID("type-switcher"));
            tl->joinHighlight(cell->getChildByID("type-label"));
        }
        tl->appendDialogue("You can choose which session the session graphs show using this session switcher", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(sessionSelector)
            ->joinHighlight(SessionSelectionLabel)
            ->joinTransform(TutorialBoxPlacement::TBPBottomRight, .75f)
            ->appendDialogue("And choose which run the run graphs would display using the run input!", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(runSelectInput)
            ->joinHighlight(runSelectInputLabel)
            ->appendDialogue("The number you input would be the starting percent of the run you wanna see!", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(runSelectInput)
            ->joinHighlight(runSelectInputLabel)
            ->appendDialogue("Thats about it! Enjoy using graphs!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1);
    });
    sideTutorial->setPositionX(-graphsScroll->getContentWidth() / 2 + 2.5f);
    graphsPage->addChild(sideTutorial);


    auto nameLabel = CCLabelBMFont::create("Name", "bigFont.fnt");
    nameLabel->setScale(.3f);
    graphOptionsPage->addChild(nameLabel);

    nameInput = TextInput::create(90, "Name", "bigFont.fnt");
    nameInput->setAnchorPoint({.5f, 1});
    nameInput->setScale(.75f);
    nameInput->setPositionY(-nameLabel->getScaledContentHeight() / 2);
    nameInput->setCallback([&](const std::string& newStr){
        if (!editedGraph.has_value()) return;

        editedGraph.value()->setName(newStr);
    });
    graphOptionsPage->addChild(nameInput);

    auto coverageLabel = CCLabelBMFont::create("Coverage", "bigFont.fnt");
    coverageLabel->setScale(.3f);
    coverageLabel->setAnchorPoint({.5f, 1});
    coverageLabel->setPositionY(nameInput->getPositionY() - nameInput->getScaledContentHeight() - 5);
    graphOptionsPage->addChild(coverageLabel);

    coverageSwitcher = OptionSwitcher<DTGraphCoverage>::create(90, {
        {DTGraphCoverage::GeneralCover, "General"},
        {DTGraphCoverage::GeneralRunsCover, "General Runs"},
        {DTGraphCoverage::SessionCover, "Session"},
        {DTGraphCoverage::SessionRunsCover, "Session Runs"},
        {DTGraphCoverage::SectionCover, "Section"},
    });
    coverageSwitcher->setScale(.75f);
    coverageSwitcher->setAnchorPoint({.5f, 1});
    coverageSwitcher->setPositionY(coverageLabel->getPositionY() - coverageLabel->getScaledContentHeight());
    coverageSwitcher->setCallback([&](auto value){
        if (!editedGraph.has_value()) return;

        editedGraph.value()->setCoverage(value);
    });
    graphOptionsPage->addChild(coverageSwitcher);

    auto typeLabel = CCLabelBMFont::create("Type", "bigFont.fnt");
    typeLabel->setScale(.3f);
    typeLabel->setAnchorPoint({.5f, 1});
    typeLabel->setPositionY(coverageSwitcher->getPositionY() - coverageSwitcher->getScaledContentHeight() - 5);
    graphOptionsPage->addChild(typeLabel);

    typeSwitcher = OptionSwitcher<DTGraphType>::create(90, {
        {DTGraphType::Passrate, "Passrate"},
        {DTGraphType::Reachrate, "Reachrate"}
    });
    typeSwitcher->setScale(.75f);
    typeSwitcher->setAnchorPoint({.5f, 1});
    typeSwitcher->setPositionY(typeLabel->getPositionY() - typeLabel->getScaledContentHeight());
    typeSwitcher->setCallback([&](auto value){
        if (!editedGraph.has_value()) return;

        editedGraph.value()->setType(value);
    });
    graphOptionsPage->addChild(typeSwitcher);

    auto thicknessLabel = CCLabelBMFont::create("Thickness", "bigFont.fnt");
    thicknessLabel->setScale(.3f);
    thicknessLabel->setAnchorPoint({.5f, 1});
    thicknessLabel->setPositionY(typeSwitcher->getPositionY() - typeSwitcher->getScaledContentHeight() - 5);
    graphOptionsPage->addChild(thicknessLabel);

    auto thicknessBaseLabel = CCLabelBMFont::create("Base", "bigFont.fnt");
    thicknessBaseLabel->setScale(.25f);
    thicknessBaseLabel->setAnchorPoint({1, 1});
    thicknessBaseLabel->setPositionY(thicknessLabel->getPositionY() - thicknessLabel->getScaledContentHeight());
    thicknessBaseLabel->setPositionX(-10);
    graphOptionsPage->addChild(thicknessBaseLabel);

    auto thicknessOutlineLabel = CCLabelBMFont::create("Outline", "bigFont.fnt");
    thicknessOutlineLabel->setScale(.25f);
    thicknessOutlineLabel->setAnchorPoint({0, 1});
    thicknessOutlineLabel->setPositionY(thicknessLabel->getPositionY() - thicknessLabel->getScaledContentHeight());
    thicknessOutlineLabel->setPositionX(0);
    graphOptionsPage->addChild(thicknessOutlineLabel);

    thicknessBaseInput = TextInput::create(40, "X", "bigFont.fnt");
    thicknessBaseInput->setScale(.75f);
    thicknessBaseInput->setAnchorPoint({.5f, 1});
    thicknessBaseInput->setPositionY(thicknessBaseLabel->getPositionY() - thicknessBaseLabel->getScaledContentHeight());
    thicknessBaseInput->setPositionX(thicknessBaseLabel->getPositionX() - thicknessBaseLabel->getScaledContentWidth() / 2);
    thicknessBaseInput->setCommonFilter(CommonFilter::Float);
    thicknessBaseInput->setCallback([&](auto value){
        if (!editedGraph.has_value()) return;

        auto numRes = utils::numFromString<float>(value);
        if (numRes.isErr()) return;

        float num = numRes.unwrap();

        if (num < 0) num = 0;

        editedGraph.value()->setThickness(num);
    });
    graphOptionsPage->addChild(thicknessBaseInput);

    thicknessOutlineInput = TextInput::create(40, "X", "bigFont.fnt");
    thicknessOutlineInput->setScale(.75f);
    thicknessOutlineInput->setAnchorPoint({.5f, 1});
    thicknessOutlineInput->setPositionY(thicknessOutlineLabel->getPositionY() - thicknessOutlineLabel->getScaledContentHeight());
    thicknessOutlineInput->setPositionX(thicknessOutlineLabel->getPositionX() + thicknessOutlineLabel->getScaledContentWidth() / 2);
    thicknessOutlineInput->setCommonFilter(CommonFilter::Float);
    thicknessOutlineInput->setCallback([&](auto value){
        if (!editedGraph.has_value()) return;

        auto numRes = utils::numFromString<float>(value);
        if (numRes.isErr()) return;

        float num = numRes.unwrap();

        if (num < 0) num = 0;

        editedGraph.value()->setOutlineThickness(num);
    });
    graphOptionsPage->addChild(thicknessOutlineInput);

    auto colorLabel = CCLabelBMFont::create("Color", "bigFont.fnt");
    colorLabel->setScale(.3f);
    colorLabel->setAnchorPoint({.5f, 1});
    colorLabel->setPositionY(thicknessOutlineInput->getPositionY() - thicknessOutlineInput->getScaledContentHeight() - 5);
    graphOptionsPage->addChild(colorLabel);

    auto colorBaseLabel = CCLabelBMFont::create("Base", "bigFont.fnt");
    colorBaseLabel->setScale(.25f);
    colorBaseLabel->setAnchorPoint({1, 1});
    colorBaseLabel->setPositionY(colorLabel->getPositionY() - colorLabel->getScaledContentHeight());
    colorBaseLabel->setPositionX(-10);
    graphOptionsPage->addChild(colorBaseLabel);

    auto colorOutlineLabel = CCLabelBMFont::create("Outline", "bigFont.fnt");
    colorOutlineLabel->setScale(.25f);
    colorOutlineLabel->setAnchorPoint({0, 1});
    colorOutlineLabel->setPositionY(colorLabel->getPositionY() - colorLabel->getScaledContentHeight());
    colorOutlineLabel->setPositionX(0);
    graphOptionsPage->addChild(colorOutlineLabel);

    colorBaseBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    colorBaseBtnSpr->setScale(.6f);
    auto colorBaseBtn = CCMenuItemSpriteExtra::create(
        colorBaseBtnSpr,
        this,
        menu_selector(DTGraphLayer::onColor)
    );
    colorBaseBtn->setPositionY(colorBaseLabel->getPositionY() - colorBaseLabel->getScaledContentHeight() - colorBaseBtn->getContentHeight() / 2);
    colorBaseBtn->setPositionX(colorBaseLabel->getPositionX() - colorBaseLabel->getScaledContentWidth() / 2);
    colorBaseBtn->setID("line");
    colorBaseBtn->setTag(1);
    graphOptionsPage->addChild(colorBaseBtn);

    colorOutlineBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    colorOutlineBtnSpr->setScale(.6f);
    auto colorOutlineBtn = CCMenuItemSpriteExtra::create(
        colorOutlineBtnSpr,
        this,
        menu_selector(DTGraphLayer::onColor)
    );
    colorOutlineBtn->setPositionY(colorOutlineLabel->getPositionY() - colorOutlineLabel->getScaledContentHeight() - colorOutlineBtn->getContentHeight() / 2);
    colorOutlineBtn->setPositionX(colorOutlineLabel->getPositionX() + colorOutlineLabel->getScaledContentWidth() / 2);
    colorOutlineBtn->setID("line");
    colorOutlineBtn->setTag(2);
    graphOptionsPage->addChild(colorOutlineBtn);

    auto pointLabel = CCLabelBMFont::create("Point", "bigFont.fnt");
    pointLabel->setScale(.3f);
    pointLabel->setAnchorPoint({.5f, 1});
    pointLabel->setPositionY(colorOutlineBtn->getPositionY() - colorOutlineBtn->getScaledContentHeight() / 2 - 5);
    graphOptionsPage->addChild(pointLabel);

    auto pointColorLabel = CCLabelBMFont::create("Color", "bigFont.fnt");
    pointColorLabel->setScale(.25f);
    pointColorLabel->setAnchorPoint({1, 1});
    pointColorLabel->setPositionY(pointLabel->getPositionY() - pointLabel->getScaledContentHeight());
    pointColorLabel->setPositionX(-10);
    graphOptionsPage->addChild(pointColorLabel);

    auto pointScaleLabel = CCLabelBMFont::create("Scale", "bigFont.fnt");
    pointScaleLabel->setScale(.25f);
    pointScaleLabel->setAnchorPoint({0, 1});
    pointScaleLabel->setPositionY(pointLabel->getPositionY() - pointLabel->getScaledContentHeight());
    pointScaleLabel->setPositionX(0);
    graphOptionsPage->addChild(pointScaleLabel);

    pointScaleInput = TextInput::create(40, "X", "bigFont.fnt");
    pointScaleInput->setScale(.75f);
    pointScaleInput->setAnchorPoint({.5f, 1});
    pointScaleInput->setPositionY(pointScaleLabel->getPositionY() - pointScaleLabel->getScaledContentHeight());
    pointScaleInput->setPositionX(pointScaleLabel->getPositionX() + pointScaleLabel->getScaledContentWidth() / 2);
    pointScaleInput->setCommonFilter(CommonFilter::Float);
    pointScaleInput->setCallback([&](auto value){
        if (!editedGraph.has_value()) return;

        auto numRes = utils::numFromString<float>(value);
        if (numRes.isErr()) return;

        float num = numRes.unwrap();

        if (num < 0) num = 0;

        editedGraph.value()->setPointSize(num);
    });
    graphOptionsPage->addChild(pointScaleInput);

    pointColorBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    pointColorBtnSpr->setScale(.6f);
    auto pointColorBtn = CCMenuItemSpriteExtra::create(
        pointColorBtnSpr,
        this,
        menu_selector(DTGraphLayer::onColor)
    );
    pointColorBtn->setPositionY(pointColorLabel->getPositionY() - pointColorLabel->getScaledContentHeight() - pointColorBtn->getContentHeight() / 2);
    pointColorBtn->setPositionX(pointColorLabel->getPositionX() - pointColorLabel->getScaledContentWidth() / 2);
    pointColorBtn->setID("point");
    graphOptionsPage->addChild(pointColorBtn);

    auto okBtnSpr = ButtonSprite::create("OK");
    okBtnSpr->setScale(.6f);
    auto okBtn = CCMenuItemSpriteExtra::create(
        okBtnSpr,
        this,
        menu_selector(DTGraphLayer::onOk)
    );
    okBtn->setPositionY(pointColorBtn->getPositionY() - pointColorBtn->getScaledContentHeight() / 2 - okBtn->getContentHeight() / 2 - 5);
    okBtn->setPositionX(pointColorBtn->getPositionX());
    graphOptionsPage->addChild(okBtn);

    auto DeleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    DeleteBtnSpr->setScale(.6f);
    auto DeleteBtn = CCMenuItemSpriteExtra::create(
        DeleteBtnSpr,
        this,
        menu_selector(DTGraphLayer::onDelete)
    );
    DeleteBtn->setPositionY(pointScaleInput->getPositionY() - pointScaleInput->getScaledContentHeight() - DeleteBtn->getContentHeight() / 4 - 10);
    DeleteBtn->setPositionX(pointScaleInput->getPositionX());
    graphOptionsPage->addChild(DeleteBtn);

    graphOptionsPage->setEnabled(false);
    nameInput->setEnabled(false);
    thicknessBaseInput->setEnabled(false);
    thicknessOutlineInput->setEnabled(false);
    pointScaleInput->setEnabled(false);
    
    scheduleUpdate();

    return true;
}

void DTGraphLayer::keyDown(enumKeyCodes key){
    if (key == enumKeyCodes::KEY_LeftShift){
        holdingShift = true;
    }

    graph->sendKeyStuff(false, key);
    
    CCLayer::keyDown(key);
}

void DTGraphLayer::keyUp(enumKeyCodes key){
    if (key == enumKeyCodes::KEY_LeftShift){
        holdingShift = false;
    }

    graph->sendKeyStuff(true, key);
}

void DTGraphLayer::OnPointSelected(GraphPoint* point){
    if (displaysForPoints.contains(point)) return;

    auto display = GraphPointDisplay::create();

    auto pointPosConverted = m_mainLayer->convertToNodeSpace(point->convertToWorldSpace({point->getContentWidth() / 2, 0}));

    display->setPosition(pointPosConverted - ccp(display->getContentWidth() / 2, display->getContentHeight()));
    display->setContent(point->m_Run, point->m_Rate, point->relatedGraph->getInfo().value().type);
    m_mainLayer->addChild(display);

    displaysForPoints.insert({point, display});
}

void DTGraphLayer::OnPointDeselected(GraphPoint* point){
    if (!displaysForPoints.contains(point)) return;

    displaysForPoints[point]->removeMeAndCleanup();
    displaysForPoints.erase(point);
}

void DTGraphLayer::onClose(cocos2d::CCObject*) {
    if (editedGraph.has_value()){
        onOk(nullptr);
        return;
    }
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void DTGraphLayer::update(float dt){
    auto mousePos = getMousePos();

    CCRect graphsScrollRect = {graphsScroll->getPositionX(), graphsScroll->getPositionY(), graphsScroll->getScaledContentSize().width, graphsScroll->getScaledContentSize().height};

    if (graphsScrollRect.containsPoint(graphsScroll->getParent()->convertToNodeSpace(mousePos))){
        graphsScroll->setMouseEnabled(true);
    }
    else {
        graphsScroll->setMouseEnabled(false);
    }
}

void DTGraphLayer::onAddGraph(CCObject*){
    addGraph();
}

void DTGraphLayer::addGraph(){
    std::string name = "New-Graph";
    int highestnewGraphNum = 0;

    int highestZ = -1;
    for (const auto& cell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
    {
        if (highestZ < cell->getZOrder()) highestZ = cell->getZOrder();

        auto splitName = StatsManager::splitStr(cell->getinfo().name, " ");
        if (!splitName.size()) continue;

        if (splitName[0] == name){
            if (splitName.size() == 1 && highestnewGraphNum == 0){
                highestnewGraphNum = 1;
            }
            else if (splitName.size() == 2){
                auto numRes = utils::numFromString<int>(splitName[1]);
                if (numRes.isErr()) continue;
                auto num = numRes.unwrap();

                if (highestnewGraphNum <= num) highestnewGraphNum = num + 1;
            }
        }
    }

    if (highestnewGraphNum != 0)
        name += " " + std::to_string(highestnewGraphNum);

    DTGraphInfo GInfo;
    GInfo.name = name;
    GInfo.orderPos = highestZ + 1;

    addGraph(GInfo);
    
    graphsScroll->m_contentLayer->setPositionY(0);
}

void DTGraphLayer::addGraph(const DTGraphInfo& info){
    auto graphCell = GraphCell::create(graphsScroll->getContentWidth(), info);

    graphCell->onArrowCallback = [&](GraphCell* cell, bool isUp){
        if (!isUp){
            int highestZ = 0;
            GraphCell* existingCellWithZ = nullptr;

            for (const auto& otherCell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
            {
                if (highestZ < otherCell->getZOrder()) highestZ = otherCell->getZOrder();

                if (otherCell->getZOrder() == cell->getZOrder() + 1) existingCellWithZ = otherCell;
            }

            if (cell->getZOrder() == highestZ) return;

            cell->setOrderPos(cell->getZOrder() + 1);
            if (existingCellWithZ != nullptr) existingCellWithZ->setOrderPos(existingCellWithZ->getZOrder() - 1);
        }
        else{
            if (cell->getZOrder() == 0) return;

            GraphCell* existingCellWithZ = nullptr;

            for (const auto& otherCell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
            {
                if (otherCell->getZOrder() == cell->getZOrder() - 1) existingCellWithZ = otherCell;
            }

            cell->setOrderPos(cell->getZOrder() - 1);
            if (existingCellWithZ != nullptr) existingCellWithZ->setOrderPos(existingCellWithZ->getZOrder() + 1);
        }

        graphsScroll->m_contentLayer->updateLayout();

        saveAllGraphs();
    };
    graphCell->onOptionsCallback = [&](GraphCell* cell){
        openOptionsFor(cell);
    };
    graphCell->onEnabledChanged = [&](GraphCell* cell){
        graph->getGraphNode(cell->getinfo().name)->setInfo(cell->getinfo());

        if (!holdingShift && cell->getinfo().isEnabled){
            for (const auto& graphCell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren())){
                if (graphCell == cell) continue;
                graphCell->setEnabledInfo(false, true, false);
            }
        }

        saveAllGraphs();
    };
    graphCell->onInfoChangedCallback = [&](GraphCell* cell){
        auto graphNode = graph->getGraphNode(cell->getinfo().name);
        bool changeName = false;

        if (graphNode == nullptr){
            graphNode = graph->getGraphNode(cell->oldName);
            changeName = true;
            
        }

        if (graphNode == nullptr) return;

        if (changeName)
            graph->changeGraphName(cell->oldName, cell->getinfo().name);

        graphNode->setInfo(cell->getinfo());
        saveAllGraphs();
    };

    graphCell->canChangeNameTo = [&](const std::string& to, GraphCell* cell) -> bool {
        GraphCell* cellWithName = nullptr;

        for (const auto& child : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
        {
            if (child->getinfo().name == to){
                cellWithName = child;
                break;
            }
        }
        
        return cellWithName == nullptr || cellWithName == cell;
    };

    graphCell->onDeleted = [&](DTGraphInfo info){
        int indexZ = 0;
        for (const auto& child : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
        {
            child->setOrderPos(indexZ);
            indexZ++;
        }

        graphsScroll->m_contentLayer->updateLayout();

        graph->removeGraph(info.name);
        
        saveAllGraphs();
    };

    graphsScroll->m_contentLayer->addChild(graphCell);

    graph->addGraph(info);
    
    graphsScroll->m_contentLayer->updateLayout();

    saveAllGraphs();
}

void DTGraphLayer::removeGraph(const std::string& graphName){
    for (const auto& cell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren())){
        if (cell->getinfo().name == graphName){
            cell->removeMeAndCleanup();
            graph->removeGraph(graphName);
            graphsScroll->m_contentLayer->updateLayout();
            break;
        }
    }
}

void DTGraphLayer::openOptionsFor(GraphCell* cell){
    auto exitEasing = [](bool open, float finalSize = 1) -> CCEaseExponentialOut* {
        return CCEaseExponentialOut::create(CCScaleTo::create(.2f, finalSize, open ? finalSize : 0));
    };

    graphsPage->stopAllActions();
    graphsPage->runAction(CCSequence::create(
        exitEasing(false),
        nullptr
    ));

    graphOptionsPage->stopAllActions();
    graphOptionsPage->runAction(CCSequence::create(
        exitEasing(false),
        CCDelayTime::create(.1f),
        exitEasing(true, .97f),
        nullptr
    ));

    m_closeBtn->setEnabled(false);
    m_closeBtn->runAction(CCFadeTo::create(.2f, 0));

    auto cellInfo = cell->getinfo();

    nameInput->setString(cellInfo.name);
    coverageSwitcher->setValue(cellInfo.coverage, true);
    typeSwitcher->setValue(cellInfo.type, true);
    thicknessBaseInput->setString(fmt::format("{:.2f}", cellInfo.thickness));
    thicknessOutlineInput->setString(fmt::format("{:.2f}", cellInfo.outlineThickness));
    colorBaseBtnSpr->setColor({cellInfo.color.r, cellInfo.color.g, cellInfo.color.b});
    colorBaseBtnSpr->setOpacity(cellInfo.color.a);
    colorOutlineBtnSpr->setColor({cellInfo.outlineColor.r, cellInfo.outlineColor.g, cellInfo.outlineColor.b});
    colorOutlineBtnSpr->setOpacity(cellInfo.outlineColor.a);
    pointScaleInput->setString(fmt::format("{:.2f}", cellInfo.pointScale));
    pointColorBtnSpr->setColor({cellInfo.pointColor.r, cellInfo.pointColor.g, cellInfo.pointColor.b});
    pointColorBtnSpr->setOpacity(cellInfo.pointColor.a);

    editedGraph = cell;
    graphsScroll->setTouchEnabled(false);
    graphsScroll->setMouseEnabled(false);
    sessionSelector->setEnabled(false);
    runSelectInput->setEnabled(false);
    for (const auto& child : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
    {
        child->setEnabled(false);
    }
    

    graphOptionsPage->setEnabled(true);
    nameInput->setEnabled(true);
    thicknessBaseInput->setEnabled(true);
    thicknessOutlineInput->setEnabled(true);
    pointScaleInput->setEnabled(true);
}


void DTGraphLayer::onColor(CCObject* sender){
    auto nodeSender = static_cast<CCNode*>(sender);

    ColorPickPopup* popup = nullptr;

    if (nodeSender->getID() == "line"){
        ccColor4B color;
        

        if (nodeSender->getTag() == 1){
            color = {
                colorBaseBtnSpr->getColor().r,
                colorBaseBtnSpr->getColor().g,
                colorBaseBtnSpr->getColor().b,
                colorBaseBtnSpr->getOpacity(),
            };
        }
        else{
            color = {
                colorOutlineBtnSpr->getColor().r,
                colorOutlineBtnSpr->getColor().g,
                colorOutlineBtnSpr->getColor().b,
                colorOutlineBtnSpr->getOpacity(),
            };
        }

        popup = ColorPickPopup::create(color);
        popup->setColorTarget(nodeSender->getTag() == 1 ? colorBaseBtnSpr : colorOutlineBtnSpr);

        if (nodeSender->getTag() == 1){
            callbacksForColorPopups = [&](const auto& color){
                colorBaseBtnSpr->setColor({color.r, color.g, color.b});
                colorBaseBtnSpr->setOpacity(color.a);

                if (!editedGraph.has_value()) return;

                editedGraph.value()->setColor(color);
            };
        }
        else{
            callbacksForColorPopups = [&](const auto& color){
                colorOutlineBtnSpr->setColor({color.r, color.g, color.b});
                colorOutlineBtnSpr->setOpacity(color.a);

                if (!editedGraph.has_value()) return;

                editedGraph.value()->setOutlineColor(color);
            };
        }
    }
    else if (nodeSender->getID() == "point"){
        popup = ColorPickPopup::create({
            pointColorBtnSpr->getColor().r,
            pointColorBtnSpr->getColor().g,
            pointColorBtnSpr->getColor().b,
            pointColorBtnSpr->getOpacity(),
        });

        popup->setColorTarget(pointColorBtnSpr);

        callbacksForColorPopups = [&](const auto& color){
            pointColorBtnSpr->setColor({color.r, color.g, color.b});
            pointColorBtnSpr->setOpacity(color.a);

            if (!editedGraph.has_value()) return;

            editedGraph.value()->setPointColor(color);
        };
    }

    if (popup != nullptr) {
        popup->setDelegate(this);
        popup->show();
    }
}

void DTGraphLayer::updateColor(cocos2d::ccColor4B const& color){
    if (callbacksForColorPopups.has_value())
        callbacksForColorPopups.value()(color);
}

void DTGraphLayer::saveAllGraphs(){
    std::vector<DTGraphInfo> allInfos{};

    for (const auto& graphCell : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
    {
        allInfos.push_back(graphCell->getinfo());
    }
    
    Save::setGraphs(allInfos);
}

void DTGraphLayer::onOk(CCObject*){
    if (!editedGraph.has_value()) return;

    if (!editedGraph.value()->setName(nameInput->getString())){
        FLAlertLayer::create("Cant save graph!", "A graph with this name already exists! please change the name you selected!", "OK")->show();
        return;
    }

    closeOptionsTab();
}
void DTGraphLayer::onDelete(CCObject*){
    if (!editedGraph.has_value()) return;
    deleteNotification = FLAlertLayer::create(this, "WARNING!", "This action will delete the selected graph! are you sure you want to do this?", "CANCEL", "DELETE");
    deleteNotification->show();
}

void DTGraphLayer::closeOptionsTab(){
    auto exitEasing = [](bool open, float finalSize = 1) -> CCEaseExponentialOut* {
        return CCEaseExponentialOut::create(CCScaleTo::create(.2f, finalSize, open ? finalSize : 0));
    };

    graphOptionsPage->stopAllActions();
    graphOptionsPage->runAction(CCSequence::create(
        exitEasing(false),
        nullptr
    ));

    graphsPage->stopAllActions();
    graphsPage->runAction(CCSequence::create(
        exitEasing(false),
        CCDelayTime::create(.1f),
        exitEasing(true, .97f),
        nullptr
    ));

    graphsScroll->setTouchEnabled(true);
    sessionSelector->setEnabled(true);
    runSelectInput->setEnabled(true);
    for (const auto& child : CCArrayExt<GraphCell*>(graphsScroll->m_contentLayer->getChildren()))
    {
        child->setEnabled(true);
    }
    
    m_closeBtn->setEnabled(true);
    m_closeBtn->runAction(CCFadeTo::create(.2f, 255));
    editedGraph = std::nullopt;

    graphOptionsPage->setEnabled(false);
    nameInput->setEnabled(false);
    thicknessBaseInput->setEnabled(false);
    thicknessOutlineInput->setEnabled(false);
    pointScaleInput->setEnabled(false);
}

void DTGraphLayer::FLAlert_Clicked(FLAlertLayer* layer, bool btn2){
    if (layer == deleteNotification && btn2){
        editedGraph.value()->deleteMe();
        editedGraph = std::nullopt;

        closeOptionsTab();
    }
}