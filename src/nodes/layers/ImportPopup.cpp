#include "ImportPopup.hpp"

ImportPopup* ImportPopup::create() {
    auto ret = new ImportPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ImportPopup::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (!Popup::init(250, 150, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Import");

    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    menu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setMainAxisAlignment(MainAxisAlignment::Between)
        ->setCrossAxisScaling(AxisScaling::Fit)
    );
    menu->setContentWidth(175);
    menu->setPosition(m_size / 2);
    menu->setZOrder(1);
    m_mainLayer->addChild(menu);

    auto fromDTBtnSpr = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("geode.loader/file.png"),
        50,
        50,
        50,
        1,
        false,
        "GJ_button_02.png",
        false
    );
    auto fromDTBtn = CCMenuItemSpriteExtra::create(
        fromDTBtnSpr,
        this,
        menu_selector(ImportPopup::onFromDT)
    );
    menu->addChild(fromDTBtn);
    
    auto fromTextBtnSpr = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("geode.loader/changelog.png"),
        50,
        50,
        50,
        1,
        false,
        "GJ_button_02.png",
        false
    );
    auto fromTextBtn = CCMenuItemSpriteExtra::create(
        fromTextBtnSpr,
        this,
        menu_selector(ImportPopup::onFromText)
    );
    menu->addChild(fromTextBtn);

    menu->updateLayout();

    auto fromDTBtnC1 = m_mainLayer->convertToNodeSpace(fromDTBtn->convertToWorldSpace({0, 0}));
    auto fromDTBtnC2 = m_mainLayer->convertToNodeSpace(fromDTBtn->convertToWorldSpace(fromDTBtn->getContentSize()));

    auto fromDTLabel = CCLabelBMFont::create("To DT File", "bigFont.fnt");
    fromDTLabel->setScale(.5f);
    fromDTLabel->setPosition((fromDTBtnC1 + fromDTBtnC2) / 2 + ccp(0, fromDTBtnC2.y - fromDTBtnC1.y) / 2);
    fromDTLabel->setAnchorPoint({.5f, 0});
    m_mainLayer->addChild(fromDTLabel);

    auto fromTextBtnC1 = m_mainLayer->convertToNodeSpace(fromTextBtn->convertToWorldSpace({0, 0}));
    auto fromTextBtnC2 = m_mainLayer->convertToNodeSpace(fromTextBtn->convertToWorldSpace(fromTextBtn->getContentSize()));

    auto fromTextLabel = CCLabelBMFont::create("From Text", "bigFont.fnt");
    fromTextLabel->setScale(.5f);
    fromTextLabel->setPosition((fromTextBtnC1 + fromTextBtnC2) / 2 + ccp(0, fromTextBtnC2.y - fromTextBtnC1.y) / 2);
    fromTextLabel->setAnchorPoint({.5f, 0});
    m_mainLayer->addChild(fromTextLabel);

    return true;
}

void ImportPopup::onFromDT(CCObject*){

}
void ImportPopup::onFromText(CCObject*){
    
}