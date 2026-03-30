#include "CalculatorPopup.hpp"

CalculatorPopup* CalculatorPopup::create() {
    auto ret = new CalculatorPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CalculatorPopup::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(200, 250, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Calculator");

    modes = {
        CalcMode{
            .modeName = "Passes",
            .modeDescription = "The amount of times you have passed a percentage from the target run"
        },
        CalcMode{
            .modeName = "before",
            .modeDescription = "The amount of times you didnt get to a percentage from the target run"
        }
    };

    auto calcBG = TextInput::create(m_size.width / 1.2f, "NA");
    calcBG->setPosition({
        m_size.width / 2,
        m_title->getPositionY() - m_title->getScaledContentHeight() / 2 - calcBG->getScaledContentHeight() / 2 - 5
    });
    calcBG->getInputNode()->setTouchEnabled(false);
    m_mainLayer->addChild(calcBG);

    auto modeLabel = CCLabelBMFont::create("Mode:", "bigFont.fnt");
    modeLabel->setScale(.5f);
    modeLabel->setPosition({
        m_size.width / 2,
        calcBG->getPositionY() - calcBG->getScaledContentHeight() / 2 - modeLabel->getScaledContentHeight() / 2 - 5
    });
    m_mainLayer->addChild(modeLabel);

    auto menu = CCMenu::create();
    menu->setContentSize({0, 0});
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);

    std::vector<std::string> modesTexts{};
    for (const auto& mode : modes)
    {
        modesTexts.push_back(mode.modeName);
    }

    modeButton = SwitcherButton::create({m_size.width / 1.5f, 25}, "GJ_button_01.png", modesTexts);
    modeButton->setPosition({
        m_size.width / 2,
        modeLabel->getPositionY() - modeLabel->getScaledContentHeight() / 2 - modeButton->getScaledContentHeight() / 2 - 5
    });
    modeButton->setOptionChangedCallback([&](auto currOption){

    });
    menu->addChild(modeButton);

    auto modeInfoBtnSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    modeInfoBtnSpr->setScale(.65f);
    auto modeInfobtn = CCMenuItemSpriteExtra::create(
        modeInfoBtnSpr,
        this,
        menu_selector(CalculatorPopup::onModeInfo)
    );
    modeInfobtn->setPosition({
        modeButton->getPositionX() + modeButton->getScaledContentWidth() / 2 + modeInfobtn->getScaledContentWidth() / 2 + 5,
        modeButton->getPositionY()
    });
    menu->addChild(modeInfobtn);

    return true;
}

void CalculatorPopup::onModeInfo(CCObject*){
    auto modeID = modeButton->getCurrentOptionIndex();

    FLAlertLayer::create(modes[modeID].modeName.c_str(), modes[modeID].modeDescription.c_str(), "OK")->show();
}