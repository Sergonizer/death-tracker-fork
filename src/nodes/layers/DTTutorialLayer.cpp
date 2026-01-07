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

    scheduleUpdate();

    return true;
}

DTTutorialLayer* DTTutorialLayer::appendDialogue(const std::string& text, TutorialCharacterFace face, const ccColor3B& textColor, float textSize){
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

    return appendDialogue(dObj);
}

DTTutorialLayer* DTTutorialLayer::appendDialogue(DialogObject* dialogue){
    TutorialSegment segment{
        .dialogue = dialogue
    };
    
    allSegments.push_back(segment);

    return this;
}

DTTutorialLayer* DTTutorialLayer::joinTransform(TutorialBoxPlacement alignment, float boxScale){
    if (!allSegments.size()) return this;

    allSegments[allSegments.size() - 1].alignment = alignment;
    allSegments[allSegments.size() - 1].boxScale = boxScale;

    return this;
}

DTTutorialLayer* DTTutorialLayer::joinHighlight(CCNode* targetObject, float delayTime){
    if (targetObject == nullptr) return this;
    if (!allSegments.size()) return this;

    allSegments[allSegments.size() - 1].targetObjects.insert({targetObject, delayTime});

    allSegments[allSegments.size() - 1].lastAddedHighlight = targetObject;

    return this;
}

DTTutorialLayer* DTTutorialLayer::joinTextToHighlight(const std::string& text, float size, TutorialTextPlacement alignment){
    if (!allSegments.size()) return this;
    auto& targetObjects = allSegments[allSegments.size() - 1].targetObjects;
    if (!targetObjects.size()) return this;

    allSegments[allSegments.size() - 1].textForTargets[allSegments[allSegments.size() - 1].lastAddedHighlight] = HighlightText{
        .text = text,
        .placement = alignment,
        .size = size
    };

    return this;
}

void DTTutorialLayer::show(){
    if (!allSegments.size()) return;
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

    bool firstRN = false;

    if (!firstDialogue){
        firstDialogue = true;
        firstRN = true;
        lightRT->getSprite()->runAction(CCEaseInOut::create(CCFadeTo::create(.25f, 255), 2));
    }
    
    if (allSegments[segmentIndex].alignment.has_value()){
        dialogueLayer->m_mainLayer->stopActionByTag(10);
        CCPoint toMoveTo;

        auto bg = static_cast<CCScale9Sprite*>(dialogueLayer->m_mainLayer->getChildren()->objectAtIndex(0));

        switch (allSegments[segmentIndex].alignment.value()) {
            case TutorialBoxPlacement::TBPCenter:
                toMoveTo = CCPoint{winSize.width * 0.5F, winSize.height * 0.5F};
            break;
        case TutorialBoxPlacement::TBPTop:
                toMoveTo = CCPoint{winSize.width * 0.5F, (winSize.height - 50.F) - 20.F};
            break;
        case TutorialBoxPlacement::TBPBottom:
                toMoveTo = CCPoint{winSize.width * 0.5F, 70.F};
            break;
        case TutorialBoxPlacement::TBPLeft:
                toMoveTo = CCPoint{bg->getContentWidth() / 2, winSize.height * 0.5F};
            break;
        case TutorialBoxPlacement::TBPRight:
                toMoveTo = CCPoint{winSize.width - bg->getContentWidth() / 2, winSize.height * 0.5F};
            break;
        case TutorialBoxPlacement::TBPTopRight:
                toMoveTo = CCPoint{winSize.width - bg->getContentWidth() / 2, (winSize.height - 50.F) - 20.F};
            break;
        case TutorialBoxPlacement::TBPTopLeft:
                toMoveTo = CCPoint{bg->getContentWidth() / 2, (winSize.height - 50.F) - 20.F};
            break;
        case TutorialBoxPlacement::TBPBottomLeft:
                toMoveTo = CCPoint{bg->getContentWidth() / 2, 70.F};
            break;
        case TutorialBoxPlacement::TBPBottomRight:
                toMoveTo = CCPoint{winSize.width - bg->getContentWidth() / 2, 70.F};
            break;
        }

        if (firstRN){
            dialogueLayer->m_mainLayer->setPosition(toMoveTo);
        }
        else{
            auto movement = CCEaseInOut::create(CCMoveTo::create(time, toMoveTo), 2);
            movement->setTag(10);
            dialogueLayer->m_mainLayer->runAction(movement);
        }
    }

    if (allSegments[segmentIndex].boxScale.has_value()){
        dialogueLayer->m_mainLayer->stopActionByTag(11);
        
        if (firstRN){
            dialogueLayer->m_mainLayer->setScale(allSegments[segmentIndex].boxScale.value());
        }
        else{
            auto scaleEase = CCEaseInOut::create(CCScaleTo::create(time, allSegments[segmentIndex].boxScale.value()), 2);
            scaleEase->setTag(11);
            dialogueLayer->m_mainLayer->runAction(scaleEase);
        }   
    }

    for (const auto& glow : retainedSprites){
        if (prevHighlights.contains(glow) && allSegments[segmentIndex].targetObjects.contains(prevHighlights[glow])) continue;
        glow->stopActionByTag(14);
        glow->runAction(CCFadeTo::create(.25f, 0));

        if (textsForHighlights.contains(glow)){
            textsForHighlights[glow]->stopActionByTag(14);
            textsForHighlights[glow]->runAction(CCFadeTo::create(.25f, 0));
        }
    }

    prevHighlights.clear();

    for (const auto& [highlightTarget, delayTime] : allSegments[segmentIndex].targetObjects)
    {
        auto glow = CCScale9Sprite::createWithSpriteFrameName("squareGlow.png"_spr);

        auto cornerMin = this->convertToNodeSpace(highlightTarget->convertToWorldSpace({0, 0}));
        auto cornerMax = this->convertToNodeSpace(highlightTarget->convertToWorldSpace(highlightTarget->getContentSize()));

        auto realContentSize = CCSize{
            std::abs(cornerMax.x - cornerMin.x),
            std::abs(cornerMax.y - cornerMin.y)
        };

        glow->setContentSize(realContentSize + ccp(15, 15));
        glow->setPosition(
            this->convertToNodeSpace(
                highlightTarget->convertToWorldSpace(
                    {0, 0}
                )
            ) + realContentSize / 2
        );

        glow->_scale9Image->setBlendFunc({GL_ZERO, GL_ONE_MINUS_SRC_ALPHA});
        glow->retain();
        glow->onEnter();
        retainedSprites.insert(glow);

        glow->setOpacity(0);
        auto fadeSeq = CCSequence::create(
            CCDelayTime::create(delayTime),
            CCFadeTo::create(.5f, 255),
            nullptr
        );
        fadeSeq->setTag(14);
        glow->runAction(fadeSeq);

        prevHighlights.insert({glow, highlightTarget});

        if (!allSegments[segmentIndex].textForTargets.contains(highlightTarget)) continue;

        auto& textInfo = allSegments[segmentIndex].textForTargets[highlightTarget];

        auto label = CCLabelBMFont::create(textInfo.text.c_str(), "bigFont.fnt");
        label->setAlignment(textInfo.placement == TutorialTextPlacement::TTLeft ? 
            CCTextAlignment::kCCTextAlignmentLeft :
            (textInfo.placement == TutorialTextPlacement::TTRight ? 
                CCTextAlignment::kCCTextAlignmentRight : 
                CCTextAlignment::kCCTextAlignmentCenter
            ) 
        );
        auto anchotPoint = CCPoint{.5f, .5f};
        if (textInfo.placement == TutorialTextPlacement::TTRight) anchotPoint.x = 0;
        if (textInfo.placement == TutorialTextPlacement::TTLeft) anchotPoint.x = 1;
        if (textInfo.placement == TutorialTextPlacement::TTTop) anchotPoint.y = 0;
        if (textInfo.placement == TutorialTextPlacement::TTBottom) anchotPoint.y = 1;
        label->setAnchorPoint(anchotPoint);
        label->setPosition(this->convertToNodeSpace(
                highlightTarget->convertToWorldSpace(
                    {0, 0}
                )
            ) + realContentSize * ccp(std::abs(anchotPoint.x - 1), std::abs(anchotPoint.y - 1))
        );
        label->setScale(textInfo.size);
        label->setOpacity(0);
        fadeSeq = CCSequence::create(
            CCDelayTime::create(delayTime),
            CCFadeTo::create(.5f, 255),
            nullptr
        );
        fadeSeq->setTag(14);
        label->runAction(fadeSeq);
        this->addChild(label);

        textsForHighlights.insert({glow, label});
    }

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

void DTTutorialLayer::update(float dt){
    lightRT->beginWithClear(0, 0, 0, 0.85f);

    for (const auto& glow : retainedSprites)
    {
        glow->visit();
    }
    
    lightRT->end();
}

DTTutorialLayer::~DTTutorialLayer(){
    for (const auto& glow : retainedSprites){
        glow->release();
    }
}