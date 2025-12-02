#include "DTLabel.hpp"
#include <nodes/LayoutColumn.hpp>
#include <nodes/layers/DTLayer.hpp>

#include <utils/CCResizeHeightTo.hpp>
#include <utils/CCResizeWidthTo.hpp>

float DTLabel::labelTitleHeight = 15;
float DTLabel::moveThreshold = 5;
float DTLabel::labelLerpSpeed = 10;

bool DTLabel::ColumnComperator::operator()(LayoutColumn* a, LayoutColumn* b) const {
    return a->info.orderPos < b->info.orderPos;
}

DTLabel* DTLabel::create(const DTLabelInfo& info) {
    auto ret = new DTLabel();
    if (ret && ret->init(info)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool DTLabel::init(const DTLabelInfo& info){
    if (!CCMenu::init()) return false;

    this->info = info;

    this->setAnchorPoint({0, 1});

    labelTextContainer = CCNode::create();
    labelTextContainer->setID("text-container");
    labelTextContainer->setZOrder(1);
    this->addChild(labelTextContainer);

    labelText = SimpleTextArea::create(info.text, info.font, info.scale);
    labelText->setID("text");
    labelText->setAnchorPoint({.5f, 1});
    labelText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    labelText->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    labelText->setWidth(this->getContentWidth() - textCornerOffset);
    labelTextContainer->addChild(labelText);

    auto textHeight = 0.0f;
    if (!info.isExpanded) textHeight = 0;
    else textHeight = labelText->getContentHeight() + textCornerOffset;

    this->setContentHeight(textHeight + labelTitleHeight);
    this->setContentWidth(0);
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    labelTextContainer->setScale(info.isExpanded ? 1 : 0);

    bg = CCScale9Sprite::create("GJ_button_05.png");
    bg->setID("text-bg");
    bg->setOpacity(normalBGOpacity);
    bg->setScale(.3f);
    bg->setAnchorPoint({0, 1});
    this->addChild(bg);
    
    labelTitleBG = CCScale9Sprite::create("GJ_button_05.png");
    labelTitleBG->setID("title-bg");
    labelTitleBG->setScale(.3f);
    labelTitleBG->setAnchorPoint({0, 1});
    labelTitleBG->setContentHeight(labelTitleHeight / labelTitleBG->getScale());
    this->addChild(labelTitleBG);

    labelTitleArea = SimpleTextArea::create(info.labelName, "bigFont.fnt", .35f);
    labelTitleArea->setID("title-text");
    labelTitleArea->setAnchorPoint({0, 1});
    labelTitleArea->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    labelTitleArea->setPositionX(15);
    labelTitleArea->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    labelTitleArea->setMaxLines(1);
    this->addChild(labelTitleArea);

    float expandLineWidth = 3;

    leftExpandLine = CCScale9Sprite::create("pixel.png");
    leftExpandLine->setID("expand-line-left");
    leftExpandLine->setAnchorPoint({0, 1});
    leftExpandLine->setContentWidth(expandLineWidth);
    leftExpandLine->setContentHeight(labelTitleHeight);
    leftExpandLine->setVisible(false);
    this->addChild(leftExpandLine);

    rightExpandLine = CCScale9Sprite::create("pixel.png");
    rightExpandLine->setID("expand-line-right");
    rightExpandLine->setAnchorPoint({1, 1});
    rightExpandLine->setContentWidth(expandLineWidth);
    rightExpandLine->setContentHeight(labelTitleHeight);
    rightExpandLine->setVisible(false);
    this->addChild(rightExpandLine);

    menu = CCMenu::create();
    menu->setID("btn-menu");
    menu->setPosition({0,0});
    this->addChild(menu);

    auto expandBtnSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    expandBtnSpr->setScale(.20f);
    expandBtn = CCMenuItemSpriteExtra::create(
        expandBtnSpr,
        this,
        menu_selector(DTLabel::toggleExpand)
    );
    expandBtn->setID("expand-btn");
    expandBtn->setPosition({5.0f, -labelTitleHeight / 2});
    menu->addChild(expandBtn);

    this->scheduleUpdate();

    setLabelColor(info.labelColor);

    labelTitleArea->setAlignment(info.horizontalAlignment);

    return true;
}

void DTLabel::update(float dt){

    labelTextContainer->setPosition(bg->getPosition() + ccp(this->getContentWidth() / 2, -labelTitleHeight));
    labelText->setWidth(this->getContentWidth() - textCornerOffset);

    auto textHeight = 0.0f;
    if (!info.isExpanded) textHeight = 0;
    else textHeight = labelText->getContentHeight() + textCornerOffset;

    float targetHeight = textHeight + labelTitleHeight;
    bool doUpdateLayout = false;

    if (targetHeight != this->getContentHeight()) doUpdateLayout = true;

    this->setContentHeight(targetHeight);

    if (doUpdateLayout) {
        DTLayer::get()->organizeLayout();

        bg->stopAllActions();
        bg->runAction(CCEaseInOut::create(CCResizeHeightTo::create(DTLayer::transitionTime, targetHeight / bg->getScale()), 2));
    }

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
        column->labels.erase(info.layer);
    info.layer++;
    for (const auto& column : holders)
    {
        column->updateLabelPosition(this);
    }
}

void DTLabel::toggleExpand(CCObject*){
    info.isExpanded = !info.isExpanded;

    expandBtn->stopAllActions();
    expandBtn->runAction(CCEaseInOut::create(CCRotateTo::create(DTLayer::transitionTime, info.isExpanded ? 90 : 0), 2));
    expandBtn->runAction(CCEaseInOut::create(CCMoveTo::create(DTLayer::transitionTime, info.isExpanded ? ccp(8, expandBtn->getPositionY()) : ccp(5, expandBtn->getPositionY())), 2));

    labelTextContainer->stopAllActions();
    labelTextContainer->runAction(CCEaseInOut::create(CCScaleTo::create(DTLayer::transitionTime, info.isExpanded ? 1 : 0), 2));
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
        auto worldPos = this->convertToWorldSpace(ccp(5, 0));
        auto touchPos = touch->getLocation();

        if (touchPos.x <= worldPos.x) return;

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
}

void DTLabel::onSettings(){
    // log::info("opened settings for label {}", name);
    DTLayer::get()->setOptionsLayerTo(this);
}

void DTLabel::onMoveBegan(){
    // log::info("started moving label {}", name);

    holdersSave = holders;

    removeFromColumns();

    this->runAction(CCEaseInOut::create(CCResizeWidthTo::create(DTLayer::transitionTime, DTColumnInfo::minWidth), 2));
    if (info.isExpanded) toggleExpand(nullptr);
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
    // log::info("ended moving label {}", name);

    auto res = DTLayer::get()->getColumnLayerFromPosition(this->getParent()->convertToWorldSpace(currentTouchPosition + ccp(-this->getContentWidth(), this->getContentHeight()) / 2));

    if (res.first == nullptr){
        for (const auto& container : holdersSave){
            container->addLabel(this);
        }

        DTLayer::get()->organizeLayout();
    }
    else{
        this->info.layer = res.second;
        res.first->addLabel(this);

        DTLayer::get()->organizeLayout();
    }
}

void DTLabel::removeFromColumns(){
    for (const auto& container : holders){
        container->removeLabel(this);
    }
}

void DTLabel::addColumnAsHolder(LayoutColumn* column){
    holders.insert(column);

    updateInfoWithColumnData();
}
void DTLabel::removeColumnAsHolder(LayoutColumn* column){
    holders.erase(column);

    updateInfoWithColumnData();
}

bool DTLabel::isPartOfColumn(LayoutColumn* column){
    return holders.contains(column);
}

bool DTLabel::isAlone(){
    return !holders.size();
}

void DTLabel::updateInfoWithColumnData(){
    int min = std::numeric_limits<int>::max();
    int max = std::numeric_limits<int>::min();

    for (const auto& column : holders)
    {
        min = std::min(min, column->info.orderPos);
        max = std::max(max, column->info.orderPos);
    }

    info.minPlacementRange = min;
    info.maxPlacementRange = max;
}

std::multiset<LayoutColumn*, DTLabel::ColumnComperator> DTLabel::getHolders(){
    return holders;
}

void DTLabel::setFontSize(float newSize){
    newSize = std::clamp(newSize, DTLabelInfo::MIN_MAX_SCALE.x, DTLabelInfo::MIN_MAX_SCALE.y);
    info.scale = newSize;

    labelText->setScale(newSize);
}

void DTLabel::setLabelName(const std::string& newName){
    info.labelName = newName;

    labelTitleArea->setText(newName);
}

void DTLabel::setLabelText(const std::string& newText){
    info.text = newText;

    labelText->setText(newText);
}

void DTLabel::setLabelColor(const ccColor4B& newColor){
    info.labelColor = newColor;

    bg->setColor({info.labelColor.r, info.labelColor.g, info.labelColor.b});
    bg->setOpacity(normalBGOpacity * (info.labelColor.a / 255));
    labelTitleBG->setColor({info.labelColor.r, info.labelColor.g, info.labelColor.b});
    labelTitleBG->setOpacity(info.labelColor.a);
}

void DTLabel::setTextColor(const ccColor4B& newColor){
    info.textColor = newColor;

    labelText->setColor(newColor);
}

void DTLabel::setTextAlignment(CCTextAlignment alignment){
    info.horizontalAlignment = alignment;

    labelText->setAlignment(alignment);
}