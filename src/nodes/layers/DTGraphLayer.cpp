#include "../layers/DTGraphLayer.hpp"
#include "../cells/ChooseRunCell.hpp"
#include "../../utils/Settings.hpp"

DTGraphLayer* DTGraphLayer::create(DTLayer* const& layer) {
    auto ret = new DTGraphLayer();
    if (ret && ret->initAnchored(520, 280, layer, "square01_001.png", {0.f, 0.f, 94.f, 94.f})) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTGraphLayer::setup(DTLayer* const& layer) {

    m_DTLayer = layer;

    auto overallInfoBS = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    overallInfoBS->setScale(0.8f);
    auto overallInfoButton = CCMenuItemSpriteExtra::create(
        overallInfoBS,
        nullptr,
        this,
        menu_selector(DTGraphLayer::onOverallInfo)
    );
    overallInfoButton->setPosition(m_size.width - 8.5f, m_size.height - 8.5f);
    this->m_buttonMenu->addChild(overallInfoButton);

    alignmentNode = CCNode::create();
    alignmentNode->setPosition(m_buttonMenu->getPosition());
    m_mainLayer->addChild(alignmentNode);

    noGraphLabel = CCLabelBMFont::create("No Progress\nFor Graph", "bigFont.fnt");
    noGraphLabel->setZOrder(1);
    noGraphLabel->setVisible(false);
    noGraphLabel->setPosition({46, 3});
    alignmentNode->addChild(noGraphLabel);

    graph = DTGraphNode::create({4.75f, 2});
    graph->setPosition({268, 158});
    this->m_mainLayer->addChild(graph);

    DTGraphLayer::refreshGraph();

    CCScale9Sprite* FontTextDisplayBG = CCScale9Sprite::create("square02b_001.png", {0,0, 80, 80});
    FontTextDisplayBG->setPosition({35, 0});
    FontTextDisplayBG->setContentSize({430, 256});
    FontTextDisplayBG->setColor({0,0,0});
    FontTextDisplayBG->setOpacity(125);
    alignmentNode->addChild(FontTextDisplayBG);

    auto SessionSelectCont = CCNode::create();
    SessionSelectCont->setID("Session-Select-Container");
    SessionSelectCont->setPosition({-215, 50});
    SessionSelectCont->setScale(0.85f);
    alignmentNode->addChild(SessionSelectCont);

    auto m_SessionSelectMenu = CCMenu::create();
    m_SessionSelectMenu->setPosition({0, 0});
    SessionSelectCont->addChild(m_SessionSelectMenu);

    std::ranges::sort(m_DTLayer->m_SharedLevelStats.sessions, [](const Session a, const Session b) {
        return a.lastPlayed > b.lastPlayed;
    });

    m_SessionSelectionInput = TextInput::create(120, "Session");
    m_SessionSelectionInput->getInputNode()->setDelegate(this);
    m_SessionSelectionInput->setCommonFilter(CommonFilter::Uint);
    m_SessionSelectionInput->setScale(0.45f);
    SessionSelectCont->addChild(m_SessionSelectionInput);

    auto SessionSelectionRightS = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    SessionSelectionRightS->setScaleX(0.35f);
    SessionSelectionRightS->setScaleY(0.2f);
    auto SessionSelectionRight = CCMenuItemSpriteExtra::create(
        SessionSelectionRightS,
        nullptr,
        this,
        menu_selector(DTGraphLayer::switchedSessionRight)
    );
    SessionSelectionRight->setPosition({34, 0});
    m_SessionSelectMenu->addChild(SessionSelectionRight);

    auto SessionSelectionLeftS = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    SessionSelectionLeftS->setScaleX(0.35f);
    SessionSelectionLeftS->setScaleY(0.2f);
    auto SessionSelectionLeft = CCMenuItemSpriteExtra::create(
        SessionSelectionLeftS,
        nullptr,
        this,
        menu_selector(DTGraphLayer::switchedSessionLeft)
    );
    SessionSelectionLeft->setPosition({-34, 0});
    SessionSelectionLeft->setRotation(180);
    m_SessionSelectMenu->addChild(SessionSelectionLeft);

    auto SessionSelectionLabel = CCLabelBMFont::create("Session", "bigFont.fnt");
    SessionSelectionLabel->setPosition({0, 16});
    SessionSelectionLabel->setScale(0.45f);
    SessionSelectCont->addChild(SessionSelectionLabel);

    m_RunSelectInput = TextInput::create(120, "Run %");
    m_RunSelectInput->getInputNode()->setDelegate(this);
    m_RunSelectInput->setCommonFilter(CommonFilter::Uint);
    m_RunSelectInput->setScale(0.45f);
    m_RunSelectInput->setPosition({-215, 34});
    alignmentNode->addChild(m_RunSelectInput);

    CCArray* runsAllowed = CCArray::create();

    // for (int i = 0; i < m_DTLayer->m_MyLevelStats.RunsToSave.size(); i++)
    // {
    //     if (m_DTLayer->m_MyLevelStats.RunsToSave[i] != -1)
    //         runsAllowed->addObject(ChooseRunCell::create(m_DTLayer->m_MyLevelStats.RunsToSave[i], std::bind(&DTGraphLayer::RunChosen, this, std::placeholders::_1)));
    // }
    
    auto runsAllowedView = ListView::create(runsAllowed, 20, 65, 55);

    m_RunsList = GJListLayer::create(runsAllowedView, "", {0,0,0,75}, 65, 55, 1);
    m_RunsList->setPosition({-247, -30});
    alignmentNode->addChild(m_RunsList);

    CCObject* child;

    CCARRAY_FOREACH(m_RunsList->m_listView->m_tableView->m_cellArray, child){
        auto childCell = dynamic_cast<GenericListCell*>(child);
        if (childCell)
            childCell->m_backgroundLayer->setOpacity(30);
    }

    std::vector<CCSprite*> spritesToRemove;
    CCLabelBMFont* title;

    CCARRAY_FOREACH(m_RunsList->getChildren(), child){
        auto childSprite = dynamic_cast<CCSprite*>(child);
        if (childSprite)
            spritesToRemove.push_back(childSprite);
    }

    for (int i = 0; i < spritesToRemove.size(); i++)
    {
        spritesToRemove[i]->removeMeAndCleanup();
    }

    this->scheduleUpdate();

    return true;
}

void DTGraphLayer::OnPointSelected(cocos2d::CCNode* point){
    pointToDisplay = static_cast<GraphPoint*>(point);

    PointInfoLabel->setVisible(true);
    npsLabel->setVisible(false);
    std::string typeText = "Passrate";

    PointInfoLabel->setText(fmt::format("Run:\n{}\n \n{}:\n{:.2f}%", pointToDisplay->m_Run, typeText, pointToDisplay->m_Passrate));
}

void DTGraphLayer::OnPointDeselected(cocos2d::CCNode* point){
    if (pointToDisplay != point)
        return;

    npsLabel->setVisible(true);
    PointInfoLabel->setVisible(false);
}

void DTGraphLayer::refreshGraph(){
    //if (m_graph) m_graph->removeMeAndCleanup();
}

void DTGraphLayer::switchedSessionRight(CCObject*){
    //m_DTLayer->refreshSession();
    if (!ViewModeNormal)
        DTGraphLayer::refreshGraph();
}

void DTGraphLayer::switchedSessionLeft(CCObject*){
        
    //m_DTLayer->refreshSession();
    if (!ViewModeNormal)
        DTGraphLayer::refreshGraph();
}

void DTGraphLayer::textChanged(CCTextInputNode* input){

    if (input == m_RunSelectInput->getInputNode()){
        auto res = utils::numFromString<int>(input->getString());
        int selected = res.unwrapOr(0);

        if (selected > 100){
            selected = 100;
            input->setString("100");
        }

        m_SelectedRunPercent = selected;

        if (!RunViewModeFromZero)
            DTGraphLayer::refreshGraph();
    }
}

void DTGraphLayer::textInputOpened(CCTextInputNode* input){

}

void DTGraphLayer::textInputClosed(CCTextInputNode* input){

}

void DTGraphLayer::onViewModeButton(CCObject*){
    if (ViewModeNormal){
        ViewModeNormal = false;
        viewModeButtonS->m_label->setString("Session");
    }
    else{
        ViewModeNormal = true;
        viewModeButtonS->m_label->setString("Normal");
    }
    DTGraphLayer::refreshGraph();
}

void DTGraphLayer::onRunViewModeButton(CCObject*){
    if (RunViewModeFromZero){
        RunViewModeFromZero = false;
        runViewModeButtonS->m_label->setString("Runs");
    }
    else{
        RunViewModeFromZero = true;
        runViewModeButtonS->m_label->setString("From 0");
    }
    DTGraphLayer::refreshGraph();
}

void DTGraphLayer::onTypeViewModeButton(CCObject*){
    /*
    if (currentType == GraphType::PassRate){
        currentType = GraphType::ReachRate;
        typeViewModeButtonS->m_label->setString("ReachRate");
    }
    else if (currentType == GraphType::ReachRate){
        currentType = GraphType::PassRate;
        typeViewModeButtonS->m_label->setString("PassRate");
    }*/
    DTGraphLayer::refreshGraph();
}

void DTGraphLayer::RunChosen(const int& run){
    m_RunSelectInput->setString(std::to_string(run));
    m_SelectedRunPercent = run;
    if (!RunViewModeFromZero)
        DTGraphLayer::refreshGraph();
}

void DTGraphLayer::onOverallInfo(CCObject*){
    auto alert = FLAlertLayer::create("Help", "You can change what the graph represents using the buttons under the <cy>\"View Mode\"</c>\nwhen in run mode you choose what run you see using the input field below\n \nYou can click any point on the graph to see its info! the info is displayed on the bottom left.", "Ok");
    alert->setZOrder(150);
    this->addChild(alert);
}

void DTGraphLayer::onClose(cocos2d::CCObject*) {
    //m_DTLayer->refreshAll();
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

/*
- create graph feature:

- name
- color
- type
    - passrate
    - reachrate
- coverage
    - from 0
    - runs
        - specify wether to use quickset runs start % or specific one
            - allow to choose specific runs start % if chosen
    - section
        - specify wether to use quickset section or specific one
            - allow to choose specific section if chosen
- thickness


type can be quick swapped for each specific graph

- graph cell:

- graph name
- a dot in the color of the graph
- delete button
- hide/show button
- edit button
- type quickswap option

- overall graph layer:

- graph management
    - big space for the graph
    - a space for the list of graphs created
    - hide/show all button
    - create new graph button
- run start % input
    - a selection of preset % by user to set the input to
- section selection


//to add to the actual graph node//

- graph point functionality
    - hover/raw number displays

- zoom in/out
*/