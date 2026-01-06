#include "DTTutorialLayer.hpp"
#include <hooks/DTDialogLayer.hpp>

DTTutorialLayer* DTTutorialLayer::create() {
    auto ret = new DTTutorialLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTTutorialLayer::init() {
    if (!CCLayer::init()) return false;

    auto winSize = CCDirector::get()->getWinSize();

    lightRT = CCRenderTexture::create(winSize.width, winSize.height, CCTexture2DPixelFormat::kCCTexture2DPixelFormat_RGBA8888);
    lightRT->setPosition(winSize / 2);
    this->addChild(lightRT);
    //lightRT->retain();

    darknessRT = CCRenderTexture::create(winSize.width, winSize.height, CCTexture2DPixelFormat::kCCTexture2DPixelFormat_RGBA8888);
    darknessRT->setPosition(winSize / 2);
    //this->addChild(darknessRT);

    highlightsHolder = CCNode::create();
    this->addChild(highlightsHolder);

    return true;
}

DTTutorialLayer* DTTutorialLayer::appendDialogue(DialogObject* dialogue, DialogChatPlacement alignment){
    TutorialSegment segment{
        .dialogue = dialogue,
        .alignment = alignment
    };
    
    allSegments.push_back(segment);

    return this;
}
DTTutorialLayer* DTTutorialLayer::joinHighlight(CCNode* targetObject){
    if (!allSegments.size()) return this;

    allSegments[allSegments.size() - 1].targetObjects.insert(targetObject);

    return this;
}

void DTTutorialLayer::show(){
    // shadow->runAction(CCFadeTo::create(.5f, 180));

    auto highestZ = CCScene::get()->getHighestChildZ();

    this->setZOrder(highestZ + 1);
    CCScene::get()->addChild(this);

    auto dialogueArray = CCArray::create();
    int i = 0;
    for (const auto& segment : allSegments)
    {
        dialogueArray->addObject(segment.dialogue);

        dialogueSegmentIndexes.insert({segment.dialogue, i});
        i++;
    }
    
    dialogueLayer = DialogLayer::createWithObjects(dialogueArray, 2);
    static_cast<DTDialogLayer*>(dialogueLayer)->setProgressCallback(std::bind(&DTTutorialLayer::onProgress, this, std::placeholders::_1));
    dialogueLayer->addToMainScene();
    dialogueLayer->m_delegate = this;
    dialogueLayer->setOpacity(0);
}

void DTTutorialLayer::onProgress(DialogObject* dObject){
    auto segmentIndex = dialogueSegmentIndexes[dObject];

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    CCEaseInOut* movement;

    float time = segmentIndex == 0.1f ? 0 : 0.5f;

    dialogueLayer->m_mainLayer->stopActionByTag(10);

    switch (allSegments[segmentIndex].alignment) {
        case DialogChatPlacement::Center:
            movement = CCEaseInOut::create(CCMoveTo::create(time, {winSize.width * 0.5F, winSize.height * 0.5F}), 2);
        break;
    case DialogChatPlacement::Top:
            movement = CCEaseInOut::create(CCMoveTo::create(time, {winSize.width * 0.5F, (winSize.height - 50.F) - 20.F}), 2);
        break;
    case DialogChatPlacement::Bottom:
            movement = CCEaseInOut::create(CCMoveTo::create(time, {winSize.width * 0.5F, 70.F}), 2);
        break;
    }

    movement->setTag(10);
    dialogueLayer->m_mainLayer->runAction(movement);

    for (const auto& glow : CCArrayExt<CCScale9Sprite*>(highlightsHolder->getChildren())){
        glow->runAction(CCFadeTo::create(.25f, 0));
    }

    lightRT->beginWithClear(0, 0, 0, 0);

    for (const auto& highlightTarget : allSegments[segmentIndex].targetObjects)
    {
        auto glow = CCScale9Sprite::createWithSpriteFrameName("squareGlow.png"_spr);
        glow->setContentSize(highlightTarget->getContentSize() + ccp(15, 15));
        glow->setPosition(
            highlightsHolder->convertToNodeSpace(
                highlightTarget->getParent()->convertToWorldSpace(
                    highlightTarget->getPosition()
                )
            )
        );

        glow->visit();
    }
    
    lightRT->end();
    
    auto tempSpr = CCSprite::createWithTexture(lightRT->getSprite()->getTexture());
    tempSpr->setFlipY(true);
    tempSpr->setBlendFunc({GL_ONE, GL_ONE});
    lightRT->beginWithClear(0, 0, 0, 0);
    tempSpr->visit();
    lightRT->end();
    
    auto temp2Spr = CCSprite::createWithTexture(lightRT->getSprite()->getTexture());
    temp2Spr->setFlipY(true);
    temp2Spr->setBlendFunc({GL_ZERO, GL_ONE_MINUS_SRC_ALPHA});
    
    darknessRT->beginWithClear(0, 0, 0, 1);
    temp2Spr->visit();
    darknessRT->end();
}

void DTTutorialLayer::dialogClosed(DialogLayer* layer){
    // shadow->runAction(CCSequence::create(
    //     CCFadeTo::create(.25f, 0),
    //     CCCallFunc::create(this, callfunc_selector(DTTutorialLayer::close)),
    //     nullptr
    // ));

    for (const auto& glow : CCArrayExt<CCScale9Sprite*>(highlightsHolder->getChildren())){
        glow->runAction(CCFadeTo::create(.25f, 0));
    }
}

void DTTutorialLayer::close(){
    this->removeMeAndCleanup();
}