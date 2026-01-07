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
    lightRT->getSprite()->setOpacity(0);
    this->addChild(lightRT);

    highlightsHolder = CCNode::create();
    this->addChild(highlightsHolder);

    return true;
}

DTTutorialLayer* DTTutorialLayer::appendDialogue(const std::string& text, TutorialCharacterFace face, float textSize, const ccColor3B& textColor, float boxScale, DialogChatPlacement alignment){
    int idOfFace = 0;

    switch (face)
    {
    case TutorialCharacterFace::TCFNormal :
        idOfFace = -204;
        break;
    
    default:
        break;
    }
    
    auto dObj = DialogObject::create("abb2k", text, 1, textSize, false, textColor);
    dObj->setTag(idOfFace);

    return appendDialogue(dObj, boxScale, alignment);
}

DTTutorialLayer* DTTutorialLayer::appendDialogue(DialogObject* dialogue, float boxScale, DialogChatPlacement alignment){
    TutorialSegment segment{
        .dialogue = dialogue,
        .alignment = alignment,
        .boxScale = boxScale
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
    
    dialogueLayer = DTDialogLayer::createWithTaggedSprites(dialogueArray, 1, {
        {"abb2k.png"_spr, -204, 2}
    });
    static_cast<DTDialogLayer*>(dialogueLayer)->setProgressCallback(std::bind(&DTTutorialLayer::onProgress, this, std::placeholders::_1));
    dialogueLayer->addToMainScene();
    dialogueLayer->m_delegate = this;
    dialogueLayer->setOpacity(0);
}

void DTTutorialLayer::onProgress(DialogObject* dObject){
    auto segmentIndex = dialogueSegmentIndexes[dObject];

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    float time = segmentIndex == 0.1f ? 0 : 0.5f;

    dialogueLayer->m_mainLayer->stopActionByTag(10);
    dialogueLayer->m_mainLayer->stopActionByTag(11);

    CCPoint toMoveTo;

    switch (allSegments[segmentIndex].alignment) {
        case DialogChatPlacement::Center:
            toMoveTo = CCPoint{winSize.width * 0.5F, winSize.height * 0.5F};
        break;
    case DialogChatPlacement::Top:
            toMoveTo = CCPoint{winSize.width * 0.5F, (winSize.height - 50.F) - 20.F};
        break;
    case DialogChatPlacement::Bottom:
            toMoveTo = CCPoint{winSize.width * 0.5F, 70.F};
        break;
    }

    if (!firstDialogue){
        firstDialogue = true;
        dialogueLayer->m_mainLayer->setScale(allSegments[segmentIndex].boxScale);

        dialogueLayer->m_mainLayer->setPosition(toMoveTo);

        lightRT->getSprite()->runAction(CCEaseInOut::create(CCFadeTo::create(.25f, 255), 2));
    }
    else{
        auto scaleEase = CCEaseInOut::create(CCScaleTo::create(time, allSegments[segmentIndex].boxScale), 2);
        scaleEase->setTag(11);
        dialogueLayer->m_mainLayer->runAction(scaleEase);

        auto movement = CCEaseInOut::create(CCMoveTo::create(time, toMoveTo), 2);
        movement->setTag(10);
        dialogueLayer->m_mainLayer->runAction(movement);
    }   

    for (const auto& glow : CCArrayExt<CCScale9Sprite*>(highlightsHolder->getChildren())){
        glow->runAction(CCFadeTo::create(.25f, 0));
    }

    lightRT->beginWithClear(0, 0, 0, 0.85f);

    for (const auto& highlightTarget : allSegments[segmentIndex].targetObjects)
    {
        auto glow = CCScale9Sprite::createWithSpriteFrameName("squareGlow.png"_spr);

        auto cornerMin = highlightsHolder->convertToNodeSpace(highlightTarget->convertToWorldSpace({0, 0}));
        auto cornerMax = highlightsHolder->convertToNodeSpace(highlightTarget->convertToWorldSpace(highlightTarget->getContentSize()));

        auto realContentSize = CCSize{
            std::abs(cornerMax.x - cornerMin.x),
            std::abs(cornerMax.y - cornerMin.y)
        };

        glow->setContentSize(realContentSize + ccp(25, 25));
        glow->setPosition(
            highlightsHolder->convertToNodeSpace(
                highlightTarget->getParent()->convertToWorldSpace(
                    highlightTarget->getPosition()
                )
            ) + (highlightTarget->isIgnoreAnchorPointForPosition() ? (realContentSize * highlightTarget->getAnchorPoint()) : CCSize{0,0})
        );

        glow->_scale9Image->setBlendFunc({GL_ZERO, GL_ONE_MINUS_SRC_ALPHA});
        glow->visit();
    }

    lightRT->end();
}

void DTTutorialLayer::dialogClosed(DialogLayer* layer){

    lightRT->getSprite()->runAction(CCSequence::create(
        CCEaseInOut::create(CCFadeTo::create(.25f, 0), 2),
        CCCallFunc::create(this, callfunc_selector(DTTutorialLayer::close)),
        nullptr
    ));
}

void DTTutorialLayer::close(){
    this->removeMeAndCleanup();
}