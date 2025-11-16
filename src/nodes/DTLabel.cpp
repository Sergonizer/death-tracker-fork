#include "DTLabel.hpp"
#include <nodes/LayoutColumn.hpp>

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

    this->setContentHeight(20);
    this->setContentWidth(0);
    this->setPosition({0, 0});

    bg = CCScale9Sprite::create("GJ_button_05.png");
    bg->setScale(.3f);
    this->addChild(bg);

    this->scheduleUpdate();

    return true;
}

void DTLabel::update(float dt){
    bg->setContentSize(this->getContentSize() / bg->getScale());
    bg->setPosition(this->getContentSize() / 2);
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