#include "DTLabel.hpp"
#include <nodes/LayoutColumn.hpp>
#include <nodes/layers/DTLayer.hpp>

float DTLabel::labelTitleHeight = 15;

DTLabel* DTLabel::create() {
    auto ret = new DTLabel();
    if (ret && ret->init()) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool DTLabel::init(){
    if (!CCMenu::init()) return false;

    this->setAnchorPoint({0, 1});

    this->setContentHeight(labelTitleHeight);
    this->setContentWidth(0);
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    bg = CCScale9Sprite::create("GJ_button_05.png");
    bg->setOpacity(150);
    bg->setScale(.3f);
    bg->setAnchorPoint({0, 0});
    followContentHeight.insert({bg, [&](){ return 1.0f / bg->getScale(); }});
    followContentWidth.insert({bg, [&](){ return 1.0f / bg->getScale(); }});
    this->addChild(bg);
    
    auto labelTitleBG = CCScale9Sprite::create("GJ_button_05.png");
    labelTitleBG->setScale(.3f);
    labelTitleBG->setAnchorPoint({0, 1});
    alwaysSetPosition.insert({labelTitleBG, [&, labelTitleBG](){ return ccp(0, this->getContentHeight()); }});
    followContentWidth.insert({labelTitleBG, [labelTitleBG](){ return 1.0f / labelTitleBG->getScale(); }});
    labelTitleBG->setContentHeight(labelTitleHeight / labelTitleBG->getScale());
    this->addChild(labelTitleBG);

    auto menu = CCMenu::create();
    menu->setPosition({0,0});
    this->addChild(menu);

    auto expandBtnSpr = CCSprite::create("dialogIcon_018.png");
    expandBtnSpr->setScale(.25f);
    auto expandBtn = CCMenuItemSpriteExtra::create(
        expandBtnSpr,
        this,
        menu_selector(DTLabel::toggleExpand)
    );
    menu->addChild(expandBtn);

    this->scheduleUpdate();

    return true;
}

void DTLabel::update(float dt){
    for (const auto& [node, multiplier] : followContentWidth)
    {
        float mult = 1;
        if (multiplier != NULL) mult = multiplier();

        node->setContentWidth(this->getContentWidth() * mult);
    }

    for (const auto& [node, multiplier] : followContentHeight)
    {
        float mult = 1;
        if (multiplier != NULL) mult = multiplier();

        node->setContentHeight(this->getContentHeight() * mult);
    }

    for (const auto& [node, posFunc] : alwaysSetPosition)
    {
        CCPoint pos = {0, 0};
        if (posFunc != NULL) pos = posFunc();

        node->setPosition(pos);
    }
}


void DTLabel::moveUpLayer(){
    for (const auto& column : holders)
        column->labels.erase(layer);
    layer++;
    for (const auto& column : holders)
    {
        column->updateLabelPosition(this);
    }
}

void DTLabel::toggleExpand(CCObject*){
    isExpanded = !isExpanded;
    this->setContentHeight(this->getContentHeight() + 100 * (isExpanded ? 1 : -1));

    DTLayer::get()->organizeLayout();
}