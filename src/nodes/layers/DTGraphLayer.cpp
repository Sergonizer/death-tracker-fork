#include <nodes/layers/DTGraphLayer.hpp>
#include <utils/Settings.hpp>

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
    // auto overallInfoBS = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    // overallInfoBS->setScale(0.8f);
    // auto overallInfoButton = CCMenuItemSpriteExtra::create(
    //     overallInfoBS,
    //     nullptr,
    //     this,
    //     menu_selector(DTGraphLayer::onOverallInfo)
    // );
    // overallInfoButton->setPosition(m_size.width - 8.5f, m_size.height - 8.5f);
    // this->m_buttonMenu->addChild(overallInfoButton);

    noGraphLabel = CCLabelBMFont::create("No Progress\nFor Graph", "bigFont.fnt");
    noGraphLabel->setZOrder(1);
    noGraphLabel->setVisible(false);
    noGraphLabel->setPosition({46, 3});
    m_mainLayer->addChild(noGraphLabel);

    auto graph = GraphHolder::create(ccp(m_size.height - 50, m_size.height - 50));
    graph->setPosition({m_size.width - graph->getContentWidth() / 2 - 20, m_size.height / 2 + 5});
    this->m_mainLayer->addChild(graph);

    // CCScale9Sprite* FontTextDisplayBG = CCScale9Sprite::create("square02b_001.png", {0,0, 80, 80});
    // FontTextDisplayBG->setPosition(graph->getPosition());
    // FontTextDisplayBG->setContentSize(graph->getContentSize());
    // FontTextDisplayBG->setColor({0,0,0});
    // FontTextDisplayBG->setOpacity(125);
    // m_mainLayer->addChild(FontTextDisplayBG);

    auto SessionSelectCont = CCNode::create();
    SessionSelectCont->setID("Session-Select-Container");
    SessionSelectCont->setPosition({-215, 50});
    SessionSelectCont->setScale(0.85f);
    m_mainLayer->addChild(SessionSelectCont);

    // auto m_SessionSelectMenu = CCMenu::create();
    // m_SessionSelectMenu->setPosition({0, 0});
    // SessionSelectCont->addChild(m_SessionSelectMenu);

    // m_SessionSelectionInput = TextInput::create(120, "Session");
    // m_SessionSelectionInput->getInputNode()->setDelegate(this);
    // m_SessionSelectionInput->setCommonFilter(CommonFilter::Uint);
    // m_SessionSelectionInput->setScale(0.45f);
    // SessionSelectCont->addChild(m_SessionSelectionInput);

    // auto SessionSelectionRightS = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    // SessionSelectionRightS->setScaleX(0.35f);
    // SessionSelectionRightS->setScaleY(0.2f);
    // auto SessionSelectionRight = CCMenuItemSpriteExtra::create(
    //     SessionSelectionRightS,
    //     nullptr,
    //     this,
    //     menu_selector(DTGraphLayer::switchedSessionRight)
    // );
    // SessionSelectionRight->setPosition({34, 0});
    // m_SessionSelectMenu->addChild(SessionSelectionRight);

    // auto SessionSelectionLeftS = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    // SessionSelectionLeftS->setScaleX(0.35f);
    // SessionSelectionLeftS->setScaleY(0.2f);
    // auto SessionSelectionLeft = CCMenuItemSpriteExtra::create(
    //     SessionSelectionLeftS,
    //     nullptr,
    //     this,
    //     menu_selector(DTGraphLayer::switchedSessionLeft)
    // );
    // SessionSelectionLeft->setPosition({-34, 0});
    // SessionSelectionLeft->setRotation(180);
    // m_SessionSelectMenu->addChild(SessionSelectionLeft);

    // auto SessionSelectionLabel = CCLabelBMFont::create("Session", "bigFont.fnt");
    // SessionSelectionLabel->setPosition({0, 16});
    // SessionSelectionLabel->setScale(0.45f);
    // SessionSelectCont->addChild(SessionSelectionLabel);

    // m_RunSelectInput = TextInput::create(120, "Run %");
    // m_RunSelectInput->getInputNode()->setDelegate(this);
    // m_RunSelectInput->setCommonFilter(CommonFilter::Uint);
    // m_RunSelectInput->setScale(0.45f);
    // m_RunSelectInput->setPosition({-215, 34});
    // alignmentNode->addChild(m_RunSelectInput);

    return true;
}

void DTGraphLayer::OnPointSelected(cocos2d::CCNode* point){
    // pointToDisplay = static_cast<GraphPoint*>(point);

    // PointInfoLabel->setVisible(true);
    // npsLabel->setVisible(false);
    // std::string typeText = "Passrate";

    // PointInfoLabel->setText(fmt::format("Run:\n{}\n \n{}:\n{:.2f}%", pointToDisplay->m_Run, typeText, pointToDisplay->m_Passrate));
}

void DTGraphLayer::OnPointDeselected(cocos2d::CCNode* point){
    // if (pointToDisplay != point)
    //     return;

    // npsLabel->setVisible(true);
    // PointInfoLabel->setVisible(false);
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