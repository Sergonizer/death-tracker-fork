#include <nodes/cells/PercentCell.hpp>

PercentCell* PercentCell::create(float width, int Percent, CCNode* sideButtonSprite, const std::function<void(PercentCell*)>& callback) {
    auto ret = new PercentCell();
    if (ret && ret->init(width, Percent, sideButtonSprite, callback)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool PercentCell::init(float width, int Percent, CCNode* sideButtonSprite, const std::function<void(PercentCell*)>& callback){
    if (!CCMenu::init()) return false;

    this->setContentSize({width, 40});

    BGSprite = CCScale9Sprite::create("GJ_squareB_01.png");
    BGSprite->setScale(.25f);
    BGSprite->setContentSize(this->getContentSize() / BGSprite->getScale());
    BGSprite->setAnchorPoint({0, 0});
    this->addChild(BGSprite);

    auto button = CCMenuItemSpriteExtra::create(
        sideButtonSprite,
        this,
        menu_selector(PercentCell::RunCallback)
    );
    button->setPosition({width - button->getContentWidth() / 2 - 5, this->getContentHeight() / 2});
    this->addChild(button);

    auto label = CCLabelBMFont::create(fmt::format("{}%", Percent).c_str(), "gjFont17.fnt");
    label->setWidth(width - button->getContentWidth() / 2);
    label->setScale(.75f);
    label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    label->setAnchorPoint({0, .5f});
    label->setPosition({5, this->getContentHeight() / 2});
    this->addChild(label);

    m_Percent = Percent;
    m_Callback = callback;

    return true;
}

void PercentCell::RunCallback(CCObject*){
    if (m_Callback != NULL)
        m_Callback(this);
}

int PercentCell::getPercent(){
    return m_Percent;
}
