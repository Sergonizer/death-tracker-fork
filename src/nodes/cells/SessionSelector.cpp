#include "../cells/SessionSelector.hpp"

SessionSelector* SessionSelector::create(int count) {
    auto ret = new SessionSelector();
    if (ret && ret->init(count)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SessionSelector::init(int count){
    if (!CCMenu::init()) return false;

    this->setContentSize({ 180, 30 });

    auto inputNode = geode::TextInput::create(120, "Session");
    inputNode->setPosition(this->getContentSize() / 2);
    this->addChild(inputNode);

    auto leftArrowSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    leftArrowSpr->setScale(.4f);
    auto leftArrowBtn = CCMenuItemSpriteExtra::create(
        leftArrowSpr,
        nullptr,
        this,
        menu_selector(SessionSelector::leftArrowClicked)
    );
    leftArrowBtn->setPosition(inputNode->getPosition() - ccp(inputNode->getContentWidth() / 2 + leftArrowSpr->getContentSize().width / 2, 0));
    leftArrowBtn->setRotation(180);
    leftArrowSpr->setScaleY(-.4f);
    this->addChild(leftArrowBtn);

    auto rightArrowSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    rightArrowSpr->setScale(.4f);
    auto rightArrowBtn = CCMenuItemSpriteExtra::create(
        rightArrowSpr,
        nullptr,
        this,
        menu_selector(SessionSelector::rightArrowClicked)
    );
    rightArrowBtn->setPosition(inputNode->getPosition() + ccp(inputNode->getContentWidth() / 2 + rightArrowSpr->getContentSize().width / 2, 0));
    this->addChild(rightArrowBtn);

    return true;
}

void SessionSelector::leftArrowClicked(CCObject*){

}
void SessionSelector::rightArrowClicked(CCObject*){

}

void SessionSelector::setCallback(const std::function<void(int)>& callback){
    this->callback = callback;
}

void SessionSelector::setMaximumCount(int count){
    count = std::max(count, 1);
    maxCount = count;
}
void SessionSelector::setCurrentCount(int count){
    currentCount = std::clamp(count, 1, maxCount);
}