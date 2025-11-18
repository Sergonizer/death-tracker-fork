#include "DTLabel.hpp"
#include <nodes/LayoutColumn.hpp>
#include <nodes/layers/DTLayer.hpp>

#include <utils/CCResizeHeightTo.hpp>
#include <utils/CCResizeWidthTo.hpp>

float DTLabel::labelTitleHeight = 15;
float DTLabel::moveThreshold = 5;
float DTLabel::labelLerpSpeed = 10;

bool DTLabel::ColumnComperator::operator()(LayoutColumn* a, LayoutColumn* b) const {
    return a->orderPos < b->orderPos;
}

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

    float expandLineWidth = 3;

    leftExpandLine = CCScale9Sprite::create("pixel.png");
    leftExpandLine->setAnchorPoint({0, 1});
    leftExpandLine->setContentWidth(expandLineWidth);
    leftExpandLine->setContentHeight(labelTitleHeight);
    leftExpandLine->setVisible(false);
    this->addChild(leftExpandLine);

    rightExpandLine = CCScale9Sprite::create("pixel.png");
    rightExpandLine->setAnchorPoint({1, 1});
    rightExpandLine->setContentWidth(expandLineWidth);
    rightExpandLine->setContentHeight(labelTitleHeight);
    rightExpandLine->setVisible(false);
    this->addChild(rightExpandLine);

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

    leftExpandLine->setPositionY(this->getContentHeight());

    rightExpandLine->setPositionY(this->getContentHeight());
    rightExpandLine->setPositionX(this->getContentWidth());

    if (isMovingLabel)
        onMoveUpdate(dt);
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
    auto touchInSpace = this->convertTouchToNodeSpace(touch);

    if (leftExpandLine->boundingBox().containsPoint(touchInSpace)){
        leftExpandLine->setVisible(true);
        currentlyExpandingLeft = true;

        return true;
    }
    else if (rightExpandLine->boundingBox().containsPoint(touchInSpace)){
        rightExpandLine->setVisible(true);
        currentlyExpandingRight = true;

        return true;
    }
    else if (labelTitleBG->boundingBox().containsPoint(touchInSpace)) {
        touchStartPoint = touch->getLocation();
        isBeingTouched = true;

        currentTouchPosition = this->getParent()->convertTouchToNodeSpace(touch);

        return true;
    }

    return false;
}

void DTLabel::ccTouchEnded(CCTouch* touch, CCEvent*) {
    if (isBeingTouched){
        if (!isMovingLabel){
            auto touchInSpace = labelTitleBG->getParent()->convertTouchToNodeSpace(touch);

            if (labelTitleBG->boundingBox().containsPoint(touchInSpace)) {
                onSettings();
            }
        }
        else {
            currentTouchPosition = this->getParent()->convertTouchToNodeSpace(touch);
            onMoveEnded();
        }
    }

    if (currentlyExpandingLeft){
        leftExpandLine->setVisible(false);
    }
    if (currentlyExpandingRight){
        rightExpandLine->setVisible(false);
    }

    isMovingLabel = false;
    isBeingTouched = false;
    currentlyExpandingLeft = false;
    currentlyExpandingRight = false;
}

void DTLabel::ccTouchCancelled(CCTouch* touch, CCEvent*) {
    if (isBeingTouched){
        if (isMovingLabel){
            currentTouchPosition = this->getParent()->convertTouchToNodeSpace(touch);
            onMoveEnded();
        }
    }

    if (currentlyExpandingLeft){
        leftExpandLine->setVisible(false);
    }
    if (currentlyExpandingRight){
        rightExpandLine->setVisible(false);
    }

    isMovingLabel = false;
    isBeingTouched = false;
    currentlyExpandingLeft = false;
    currentlyExpandingRight = false;
}

void DTLabel::ccTouchMoved(CCTouch* touch, CCEvent*){
    if (isBeingTouched){
        if (!isMovingLabel){
            auto touchCurrentLocation = touch->getLocation();
        
            if (std::abs((touchStartPoint - touchCurrentLocation).getLength()) >= moveThreshold){
                isMovingLabel = true;
                onMoveBegan();
            }
        }
        else currentTouchPosition = this->getParent()->convertTouchToNodeSpace(touch);
    }

    if (currentlyExpandingLeft){
        auto worldPos = this->convertToWorldSpace(ccp(this->getContentWidth(), 0));
        auto touchPos = touch->getLocation();

        if (touchPos.x >= worldPos.x) return;

        auto columns = DTLayer::get()->getColumnsBetween(worldPos, touchPos);

        // log::info("found {} columns", columns.size());

        int oldColumnsSize = holders.size();

        removeFromColumns();

        for (const auto& column : columns)
        {
            column->addLabel(this);
        }

        if (holders.size() != oldColumnsSize)
            DTLayer::get()->organizeLayout();
    }

    if (currentlyExpandingRight){
        auto worldPos = this->convertToWorldSpace(ccp(0, 0));
        auto touchPos = touch->getLocation();

        if (touchPos.x <= worldPos.x) return;

        auto columns = DTLayer::get()->getColumnsBetween(touchPos, worldPos);

        // log::info("found {} columns", columns.size());

        int oldColumnsSize = holders.size();

        removeFromColumns();

        for (const auto& column : columns)
        {
            column->addLabel(this);
        }

        if (holders.size() != oldColumnsSize)
            DTLayer::get()->organizeLayout();
    }
}

void DTLabel::onSettings(){
    log::info("opened settings for label {}", name);
}

void DTLabel::onMoveBegan(){
    log::info("started moving label {}", name);

    holdersSave = holders;

    removeFromColumns();

    this->runAction(CCEaseInOut::create(CCResizeWidthTo::create(.5f, LayoutColumn::minWidth), 2));
    if (isExpanded) toggleExpand(nullptr);
    else DTLayer::get()->organizeLayout();
}

void DTLabel::onMoveUpdate(float dt){
    auto targetPosition = currentTouchPosition + ccp(-this->getContentWidth(), this->getContentHeight()) / 2;
    CCPoint currentLerp;
    currentLerp.x = std::lerp(this->getPositionX(), targetPosition.x, dt * DTLabel::labelLerpSpeed);
    currentLerp.y = std::lerp(this->getPositionY(), targetPosition.y, dt * DTLabel::labelLerpSpeed);
    this->setPosition(currentLerp);
}

void DTLabel::onMoveEnded(){
    log::info("ended moving label {}", name);

    auto res = DTLayer::get()->getColumnLayerFromPosition(this->getParent()->convertToWorldSpace(currentTouchPosition + ccp(-this->getContentWidth(), this->getContentHeight()) / 2));

    if (res.first == nullptr){
        for (const auto& container : holdersSave){
            container->addLabel(this);
        }

        DTLayer::get()->organizeLayout();
    }
    else{
        this->layer = res.second;
        res.first->addLabel(this);

        DTLayer::get()->organizeLayout();
    }
}

void DTLabel::removeFromColumns(){
    for (const auto& container : holders){
        container->removeLabel(this);
    }
}