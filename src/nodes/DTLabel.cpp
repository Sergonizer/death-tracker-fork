#include "DTLabel.hpp"
#include <nodes/LayoutColumn.hpp>
#include <nodes/layers/DTLayer.hpp>

#include <utils/CCResizeHeightTo.hpp>
#include <utils/CCResizeWidthTo.hpp>
#include <utils/CCTextAreaTintTo.hpp>
#include <utils/CCTextAreaFadeTo.hpp>

#include <regex>

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

DTLabel::~DTLabel(){
    if (DTLayer::get() != nullptr){
        DTLayer::get()->unsubscribeToOrganizationEvent(this);
        DTLayer::get()->unsubscribeKeyListener(this);
    }
}

bool DTLabel::init(const DTLabelInfo& info){
    if (DTLayer::get() != nullptr){
        DTLayer::get()->subscribeToOrganizationEvent(this, [&](auto _){
            bg->stopAllActions();
            bg->runAction(CCEaseInOut::create(CCResizeHeightTo::create(DTLayer::transitionTime, this->getContentHeight() / bg->getScale()), 2));
        });
        DTLayer::get()->subscribeKeyListener(this);
    }
    if (!CCMenu::init()) return false;

    this->setCascadeOpacityEnabled(true);

    this->info = info;

    this->setAnchorPoint({0, 1});

    auto circleLayer = CCLayer::create();
    circleLayer->setID("circle-layer");
    circleLayer->setZOrder(5);
    this->addChild(circleLayer);

    loadingCircle = LoadingCircle::create();
    loadingCircle->setParentLayer(circleLayer);
    loadingCircle->show();
    loadingCircle->ignoreAnchorPointForPosition(false);
    loadingCircle->setVisible(false);
    loadingCircle->setScale(.4f);

    labelTextContainer = CCNode::create();
    labelTextContainer->setID("text-container");
    labelTextContainer->setZOrder(1);
    this->addChild(labelTextContainer);

    labelText = SimpleTextArea::create("", info.font, info.scale);
    labelText->setID("text");
    labelText->setAnchorPoint({.5f, 1});
    labelText->setAlignment(info.horizontalAlignment);
    labelText->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    labelText->setWidth(this->getContentWidth() - textCornerOffset);
    setLabelText(info.text);
    labelTextContainer->addChild(labelText);

    auto textHeight = 0.0f;
    if (!info.isExpanded) textHeight = 0;
    else{
        textHeight = labelText->getContentHeight() + textCornerOffset;   
    }

    this->setContentHeight(textHeight + labelTitleHeight);
    this->setContentWidth(0);
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    labelTextContainer->setScale(info.isExpanded ? 1 : 0);

    bg = CCScale9Sprite::create("GJ_squareB_01.png");
    bg->setID("text-bg");
    bg->setOpacity(normalBGOpacity);
    bg->setScale(.2f);
    bg->setAnchorPoint({0, 1});
    bg->setContentHeight(labelTitleHeight / bg->getScale());
    if (info.isExpanded) bg->setContentHeight(this->getContentHeight() / bg->getScale());
    this->addChild(bg);
    
    labelTitleBG = CCScale9Sprite::create("GJ_squareB_01.png");
    labelTitleBG->setID("title-bg");
    labelTitleBG->setScale(.15f);
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
    menu->setTouchPriority(-503);
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

    if (info.isExpanded){
        expandBtn->setRotation(90);
        expandBtn->setPositionX(8);
    }

    this->scheduleUpdate();

    setLabelColor(info.labelColor);
    setTextColor(info.textColor);
    setTextWrapping(info.wrapping);

    labelTitleArea->setAlignment(info.horizontalAlignment);

    glow = CCSprite::create("pixel.png");
    glow->setAnchorPoint({0, 0});
    ccBlendFunc bf = { GL_ONE, GL_ONE };
    glow->setBlendFunc(bf);
    glow->setColor({0, 0, 0});
    glow->setZOrder(15);
    this->addChild(glow);

    return true;
}

void DTLabel::registerWithTouchDispatcher(){
    CCTouchDispatcher::get()->addPrioTargetedDelegate(this, -502, true);
}

void DTLabel::update(float dt){

    labelTextContainer->setPosition(bg->getPosition() + ccp(this->getContentWidth() / 2, -labelTitleHeight));
    if (labelText->getWidth() != this->getContentWidth() - textCornerOffset && info.isExpanded){
        labelText->setWidth(this->getContentWidth() - textCornerOffset);
        labelText->setWidth(labelText->getWidth());
    }
    if (labelText->getWidth() != this->getContentWidth() - textCornerOffset && info.isExpanded){
        labelText->setWidth(this->getContentWidth() - textCornerOffset);
        labelText->setWidth(labelText->getWidth());
    }

    glow->setScaleX(this->getContentWidth() * 2);
    glow->setScaleY(this->getContentHeight() * 2);

    auto textHeight = 0.0f;
    if (!info.isExpanded) textHeight = 0;
    else textHeight = labelText->getContentHeight() + textCornerOffset;

    float targetHeight = textHeight + labelTitleHeight;
    bool doUpdateLayout = false;

    if (targetHeight != this->getContentHeight()) doUpdateLayout = true;

    this->setContentHeight(targetHeight);

    if (doUpdateLayout){
        if (!DTLayer::get()->cornerOnNextOrganization){
            DTLayer::get()->organizeLayout();
            
        }
        else{

        }
    }

    labelTitleBG->setContentWidth(this->getContentWidth() / labelTitleBG->getScale());
    bg->setContentWidth(this->getContentWidth() / bg->getScale());
    labelTitleBG->setPositionY(this->getContentHeight());
    bg->setPositionY(this->getContentHeight());
    menu->setPositionY(this->getContentHeight());

    loadingCircle->setPosition({this->getContentWidth() / 2, this->getContentHeight() - labelTitleHeight / 2});

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
    if (!isExpandable) return;

    info.isExpanded = !info.isExpanded;

    expandBtn->stopAllActions();
    expandBtn->runAction(
        CCEaseInOut::create(
            CCRotateTo::create(
                DTLayer::transitionTime, 
                info.isExpanded ? 90 : 0
            ), 
            2
        )
    );
    expandBtn->runAction(
        CCEaseInOut::create(
            CCMoveTo::create(
                DTLayer::transitionTime, 
                info.isExpanded ? ccp(8, expandBtn->getPositionY()) : ccp(5, expandBtn->getPositionY())
            ),
            2
        )
    );

    labelTextContainer->stopAllActions();
    labelTextContainer->runAction(
        CCEaseInOut::create(
            CCScaleTo::create(
                DTLayer::transitionTime, 
                info.isExpanded ? 1 : 0
            ), 
            2
        )
    );

    if (!DTLayer::get()->isEditingLayout)
        DTLayer::get()->saveCurrentLayout();
}

// void DTLabel::registerWithTouchDispatcher() {
//     //CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -500, true);
//     //if (auto handler = CCTouchDispatcher::get()->findHandler(this))
//     //    CCTouchDispatcher::get()->setPriority(-500, handler->getDelegate());
// }

bool DTLabel::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!isEditable) return false;
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

    this->runAction(
        CCEaseInOut::create(
            CCResizeWidthTo::create(
                DTLayer::transitionTime, 
                DTColumnInfo::minWidth
            ), 
            2
        )
    );
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

void DTLabel::setLabelText(const std::string& text){
    info.text = text;    
    labelText->setText(text);

    modifyKeys();
}

void DTLabel::modifyKeys(){
    if (currentlyLoadingFor.size() != 0) return;
    auto text = labelText->getText();
    
    keysUsed.clear();

    //log::info("modifying {}", info.labelName);
    
    auto modifiedText = modifyStrRecursive(text);
    if (modifiedText.length() != 0 && modifiedText[modifiedText.length() - 1] == '\n') modifiedText.push_back(' ');
    labelText->setText(modifiedText);
    //log::info("ended, {} keys exist | {}", keysUsed.size(), info.labelName);
}

void DTLabel::setLoading(const std::shared_ptr<SpecialKey>& key){
    if (currentlyLoadingFor.contains(key) || !keysUsed.contains(key->getKey())) return;
    currentlyLoadingFor.insert(key);

    //log::info("started loading for {}", key->getKey());

    loadingCircle->setVisible(true);
}
void DTLabel::completeLoading(const std::shared_ptr<SpecialKey>& key){
    if (!currentlyLoadingFor.contains(key)) return;
    currentlyLoadingFor.erase(key);

    //log::info("ended loading for {}", key->getContent());

    if (currentlyLoadingFor.size() == 0){
        loadingCircle->setVisible(false);
        labelText->setText(info.text);
        modifyKeys();
    }
}

/*
- when a specialKey is being updated, all labels using that key will load until finished and update their text accordingly
- when text is changed update the text with the specialKeys values if not in loading mode



*/

void DTLabel::setFontSize(float newSize){
    newSize = std::clamp(newSize, DTLabelInfo::MIN_MAX_SCALE.x, DTLabelInfo::MIN_MAX_SCALE.y);
    info.scale = newSize;

    labelText->setScale(newSize);
}

void DTLabel::setFont(const std::string& fnt){
    info.font = fnt;
    
    labelText->setFont(fnt);
}

void DTLabel::setLabelName(const std::string& newName){
    info.labelName = newName;

    labelTitleArea->setText(newName);
}

void DTLabel::setLabelColor(const ccColor4B& newColor){
    info.labelColor = newColor;

    bg->setColor({info.labelColor.r, info.labelColor.g, info.labelColor.b});
    bg->setOpacity(normalBGOpacity * (info.labelColor.a / 255));
    labelTitleBG->setColor({info.labelColor.r, info.labelColor.g, info.labelColor.b});
    labelTitleBG->setOpacity(info.labelColor.a);
}

void DTLabel::setTextColor(const ccColor4B& newColor){
    labelText->stopActionByTag(10);
    labelText->stopActionByTag(11);
    info.textColor = newColor;

    labelText->setColor(newColor);
}

void DTLabel::setTextAlignment(CCTextAlignment alignment){
    info.horizontalAlignment = alignment;

    labelText->setAlignment(alignment);
}

void DTLabel::setTextWrapping(WrappingMode wrapMode){
    if (wrapMode == WrappingMode::NO_WRAP) wrapMode = WrappingMode::CUTOFF_WRAP;
    info.wrapping = wrapMode;

    labelText->setWrappingMode(wrapMode);
}

void DTLabel::onBeingEdited(){
    glow->stopAllActions();
    glow->runAction(CCTintTo::create(.1f, 35, 35, 35));
}
void DTLabel::onBeingEditedEnded(){
    glow->stopAllActions();
    glow->runAction(CCTintTo::create(.1f, 0, 0, 0));
}

void DTLabel::setEditable(bool editable){
    isEditable = editable;
}

std::string DTLabel::modifyStrRecursive(const std::string& str){
    std::regex specialKeyRegex(R"(\{([A-Za-z0-9_\\]+)(?:-([^{}]*))?\})");

    std::sregex_iterator begin(str.begin(), str.end(), specialKeyRegex);
    std::sregex_iterator end;

    std::string result = "";

    int lastPos = 0;
    for (auto it = begin; it != end; ++it) {
        const auto& match = *it;

        int matchStart = match.position();
        int matchEnd = matchStart + match.length();

        result += str.substr(lastPos, matchStart - lastPos);

        auto dtLayer = DTLayer::get();

        std::string key = match.str(1);
        if (!keysUsed.contains(key)){
            keysUsed.insert(key);
        }
        
        bool isCancellation = false;
        if (key.size() && key[0] == '\\'){
            isCancellation = true;
            key = key.erase(0, 1);
        }

        std::string value = match.size() > 2 ? match.str(2) : "";

        if (dtLayer->specialStrings.contains(key)) {
            result += modifyStrRecursive(dtLayer->specialStrings.at(key)->getContent());
        }
        else {
            result += match.str();
        }

        lastPos = matchEnd;
    }

    result += str.substr(lastPos);

    return result;
}
void DTLabel::setExpandable(bool enabled){
    isExpandable = enabled;
    expandBtn->setVisible(enabled);
}

void DTLabel::fadeTextColorTo(ccColor4B newColor, float time){
    info.textColor = newColor;

    labelText->stopActionByTag(10);
    labelText->stopActionByTag(11);

    auto faceAction = CCTextAreaFadeTo::create(time, newColor.a);
    faceAction->setTag(10);

    auto colorAction = CCTextAreaTintTo::create(time, {newColor.r, newColor.g, newColor.b});
    colorAction->setTag(11);

    labelText->runAction(faceAction);
    labelText->runAction(colorAction);
}

void DTLabel::fadeTitleColorTo(ccColor4B newColor, float time){
    labelTitleArea->stopActionByTag(10);
    labelTitleArea->stopActionByTag(11);

    auto faceAction = CCTextAreaFadeTo::create(time, newColor.a);
    faceAction->setTag(10);

    auto colorAction = CCTextAreaTintTo::create(time, {newColor.r, newColor.g, newColor.b});
    colorAction->setTag(11);

    labelTitleArea->runAction(faceAction);
    labelTitleArea->runAction(colorAction);
}