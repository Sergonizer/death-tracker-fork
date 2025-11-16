#include "LayoutColumn.hpp"

#include <nodes/layers/DTLayer.hpp>

float LayoutColumn::borderWidth = 2;

LayoutColumn* LayoutColumn::create(float topHeight, float minHeight, float minWidth) {
    auto ret = new LayoutColumn();
    if (ret && ret->init(topHeight, minHeight, minWidth)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool LayoutColumn::init(float topHeight, float minHeight, float minWidth){
    if (!CCMenu::init()) return false;

    this->minHeight = minHeight;
    this->minWidth = minWidth;
    this->topHeight = topHeight;

    topSpr = CCScale9Sprite::create("pixel.png");
    topSpr->setID("top-spr");
    topSpr->setAnchorPoint({0, 1});
    this->addChild(topSpr);

    bgSpr = CCScale9Sprite::create("pixel.png");
    bgSpr->setID("bg-spr");
    bgSpr->setAnchorPoint({0, 1});
    bgSpr->setOpacity(bgOpacity);
    this->addChild(bgSpr);

    topBorder1 = CCScale9Sprite::create("pixel.png");
    topBorder1->setID("top-border-1");
    topBorder1->setAnchorPoint({0, 1});
    topBorder1->setColor({0,0,0});
    this->addChild(topBorder1);

    topBorder2 = CCScale9Sprite::create("pixel.png");
    topBorder2->setID("top-border-2");
    topBorder2->setAnchorPoint({1, 1});
    topBorder2->setColor({0,0,0});
    this->addChild(topBorder2);

    topBorder3 = CCScale9Sprite::create("pixel.png");
    topBorder3->setID("top-border-3");
    topBorder3->setAnchorPoint({0, 0});
    topBorder3->setColor({0,0,0});
    this->addChild(topBorder3);

    bgSideBorder = CCScale9Sprite::create("pixel.png");
    bgSideBorder->setID("bg-side-border");
    bgSideBorder->setAnchorPoint({1, 1});
    bgSideBorder->setColor({0,0,0});
    bgSideBorder->setOpacity(125);
    this->addChild(bgSideBorder);

    this->setAnchorPoint({0, 1});
    this->ignoreAnchorPointForPosition(false);
    this->setContentSize({minWidth, minHeight});
    
    updateSizesByContent();

    return true;
}

void LayoutColumn::updateSizesByContent(){
    if (this->getContentHeight() < minHeight) this->setContentHeight(minHeight);
    
    topSpr->setContentSize({this->getContentWidth(), topHeight});
    topSpr->setPosition({0, this->getContentHeight()});

    topBorder1->setContentSize({topSpr->getContentWidth(), borderWidth});
    topBorder1->setPosition(topSpr->getPosition());

    topBorder2->setContentSize({borderWidth, topSpr->getContentHeight()});
    topBorder2->setPosition(topSpr->getPosition() + ccp(topSpr->getContentWidth(), 0));

    topBorder3->setContentSize({topSpr->getContentWidth(), borderWidth});
    topBorder3->setPosition(topSpr->getPosition() - ccp(0, topSpr->getContentHeight()));

    bgSpr->setContentSize({this->getContentWidth(), this->getContentHeight() - topHeight});
    bgSpr->setPosition({0, bgSpr->getContentHeight()});
    
    bgSideBorder->setContentSize({borderWidth, bgSpr->getContentHeight()});
    bgSideBorder->setPosition(topBorder2->getPosition() - ccp(0, topBorder2->getContentHeight()));
}

void LayoutColumn::setContentHeight(float height){
    CCMenu::setContentHeight(height);
    updateSizesByContent();
}

void LayoutColumn::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
}

bool LayoutColumn::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    auto touchInParentSpace = topBorder2->getParent()->convertTouchToNodeSpace(touch);

    if (topBorder2->boundingBox().containsPoint(touchInParentSpace)) {
        isTouchingSide = true;
        return true;
    }

    isTouchingSide = false;
    return false;
}

void LayoutColumn::ccTouchEnded(CCTouch*, CCEvent*) {
    isTouchingSide = false;

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::ccTouchCancelled(CCTouch*, CCEvent*) {
    isTouchingSide = false;

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::ccTouchMoved(CCTouch* touch, CCEvent*){
    if (!isTouchingSide) return;

    auto currentWidth = this->getContentWidth();

    auto spacedTouchLoc = this->getParent()->convertToNodeSpace(touch->getLocation());
    auto spacedPrevTouchLoc = this->getParent()->convertToNodeSpace(touch->m_prevPoint);

    currentWidth += (spacedTouchLoc - spacedPrevTouchLoc).x;

    this->setContentWidth(std::max(minWidth, currentWidth));

    updateSizesByContent();

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::setColor(ccColor3B color){
    topSpr->setColor(color);
    bgSpr->setColor(color);
}

void LayoutColumn::addLabel(DTLabel* label){
    label->holders.push_back(this);
    if (labels.contains(label->layer)){
        log::info("same layer found {} | {} - {}", label->name, labels[label->layer]->name, label->layer);
        labels[label->layer]->moveUpLayer();
    }
    
    labels.insert({label->layer, label});
}

void LayoutColumn::updateLabelPositions(){

    float diff = 0;
    DTLabel* prevLabel = nullptr;

    log::info("column {}, running sorting", orderPos);

    for (const auto& [layer, label] : labels)
    {
        auto startPosInLabelSpace = label->getParent()->convertToNodeSpace(this->convertToWorldSpace(bgSpr->getPosition()));

        if (prevLabel != nullptr){
            float normalOffset = startPosInLabelSpace.y;

            float emptySpaceDifference = prevLabel->tempPos.y + diff;

            log::info("start diff: {}", diff);
            log::info("normalOffset: {}", normalOffset);
            log::info("emptySpaceDifference: {}", emptySpaceDifference);
            log::info("res: {}", normalOffset - emptySpaceDifference);

            diff += normalOffset - emptySpaceDifference;
            
            diff += prevLabel->getContentHeight();
        }

        log::info("iterating label {}, ({}) - {}", label->name, label->tempPos, label->tempWidth);
        log::info("dif is {}", diff);

        float newY = startPosInLabelSpace.y - diff;

        if (newY < label->tempPos.y) label->tempPos.y = newY;

        float newX = startPosInLabelSpace.x;

        if (newX < label->tempPos.x) label->tempPos.x = newX;
        
        //border might cause offset
        label->tempWidth = label->tempWidth + this->getContentWidth();

        log::info("finished iterating label {}, ({}) - {}", label->name, label->tempPos, label->tempWidth);
        
        prevLabel = label;
    }

    log::info("----");

    if (prevLabel != nullptr)
        diff += prevLabel->getContentHeight();
    
    tempHeight = diff + topHeight;
}

void LayoutColumn::updateLabelPosition(DTLabel* label){
    if (labels.contains(label->layer)){
        labels[label->layer]->moveUpLayer();
    }

    // log::info("inserting revamp {}, {}", label->layer, label->name);

    labels.insert({label->layer, label});
}