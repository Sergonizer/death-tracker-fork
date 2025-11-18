#include "DTLabel.hpp"
#include <nodes/LayoutColumn.hpp>
#include <nodes/layers/DTLayer.hpp>

#include <utils/CCResizeHeightTo.hpp>

float DTLabel::labelTitleHeight = 15;
float DTLabel::moveThreshold = 5;

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
    bg->setAnchorPoint({0, 1});
    this->addChild(bg);
    
    labelTitleBG = CCScale9Sprite::create("GJ_button_05.png");
    labelTitleBG->setScale(.3f);
    labelTitleBG->setAnchorPoint({0, 1});
    labelTitleBG->setContentHeight(labelTitleHeight / labelTitleBG->getScale());
    this->addChild(labelTitleBG);

    labelTitleArea = SimpleTextArea::create("Label name", "bigFont.fnt", .35f);
    labelTitleArea->setAnchorPoint({0, 1});
    labelTitleArea->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    labelTitleArea->setPositionX(15);
    labelTitleArea->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    labelTitleArea->setMaxLines(1);
    this->addChild(labelTitleArea);

    menu = CCMenu::create();
    menu->setPosition({0,0});
    this->addChild(menu);

    auto expandBtnSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    expandBtnSpr->setScale(.20f);
    expandBtn = CCMenuItemSpriteExtra::create(
        expandBtnSpr,
        this,
        menu_selector(DTLabel::toggleExpand)
    );
    expandBtn->setPosition({5.0f, -labelTitleHeight / 2});
    menu->addChild(expandBtn);

    this->scheduleUpdate();

    return true;
}

void DTLabel::update(float dt){
    labelTitleBG->setContentWidth(this->getContentWidth() / labelTitleBG->getScale());
    bg->setContentWidth(this->getContentWidth() / bg->getScale());
    labelTitleBG->setPositionY(this->getContentHeight());
    bg->setPositionY(this->getContentHeight());
    menu->setPositionY(this->getContentHeight());

    labelTitleArea->setWidth(this->getContentWidth() - labelTitleArea->getPositionX() - 4);
    labelTitleArea->setPositionY(this->getContentHeight());
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

    expandBtn->stopAllActions();
    expandBtn->runAction(CCEaseInOut::create(CCRotateTo::create(.5f, isExpanded ? 90 : 0), 2));
    expandBtn->runAction(CCEaseInOut::create(CCMoveTo::create(.5f, isExpanded ? ccp(8, expandBtn->getPositionY()) : ccp(5, expandBtn->getPositionY())), 2));

    bg->stopAllActions();
    bg->runAction(CCEaseInOut::create(CCResizeHeightTo::create(.5f, this->getContentHeight() / bg->getScale()), 2));

    DTLayer::get()->organizeLayout();
}

void DTLabel::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
}

bool DTLabel::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    auto touchInSpace = labelTitleBG->getParent()->convertTouchToNodeSpace(touch);

    if (labelTitleBG->boundingBox().containsPoint(touchInSpace)) {
        touchStartPoint = touch->getLocation();
        isBeingTouched = true;
        return true;
    }

    return false;
}

void DTLabel::ccTouchEnded(CCTouch* touch, CCEvent*) {
    if (!isBeingTouched) return;
    if (!isMovingLabel){
        auto touchInSpace = labelTitleBG->getParent()->convertTouchToNodeSpace(touch);

        if (labelTitleBG->boundingBox().containsPoint(touchInSpace)) {
            onSettings();
        }
    }
    else onMoveEnded();

    isMovingLabel = false;
    isBeingTouched = false;
}

void DTLabel::ccTouchCancelled(CCTouch*, CCEvent*) {
    if (!isBeingTouched) return;
    if (isMovingLabel) onMoveEnded();
    isMovingLabel = false;
    isBeingTouched = false;
}

void DTLabel::ccTouchMoved(CCTouch* touch, CCEvent*){
    if (!isBeingTouched) return;

    if (!isMovingLabel){
        auto touchCurrentLocation = touch->getLocation();
    
        if (std::abs((touchStartPoint - touchCurrentLocation).getLength()) >= moveThreshold){
            isMovingLabel = true;
            onMoveBegan();
        }
    }
    
}

void DTLabel::onSettings(){
    log::info("opened settings for label {}", name);
}

void DTLabel::onMoveBegan(){
    log::info("started moving label {}", name);
}

void DTLabel::onMoveEnded(){
    log::info("ended moving label {}", name);
}