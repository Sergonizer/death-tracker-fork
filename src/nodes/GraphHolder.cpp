#include "GraphHolder.hpp"

GraphHolder* GraphHolder::create(const CCSize& scaling){
    auto ret = new GraphHolder();
    if (ret && ret->init(scaling)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool GraphHolder::init(const CCSize& scaling){
    if (!CCNode::init()) return false;

    LabelsContainer = CCNode::create();
    LabelsContainer->setID("labels-container");
    this->addChild(LabelsContainer);

    LabelsVerticalContainer = CCNode::create();
    LabelsVerticalContainer->setID("labels-vertical-container");
    LabelsContainer->addChild(LabelsVerticalContainer);

    LabelsHorizontalContainer = CCNode::create();
    LabelsHorizontalContainer->setID("labels-horizontal-container");
    LabelsContainer->addChild(LabelsHorizontalContainer);
    
    this->setContentSize(scaling);
    this->setAnchorPoint({.5f, .5f});

    labelEvery = labelEveryBase;

    scrollLayer = AdvancedScrollLayer::create(scaling, scaling + ccp(.01f,.01f));
    scrollLayer->setHorizontalScrollbarPosition(true);
    this->addChild(scrollLayer);

    GraphHolder::refreshBackground(fillColor, outlineThickness, outlineColor);

    scheduleUpdate();

    return true;
}

void GraphHolder::setBGFillColor(const ccColor4B& color){
    GraphHolder::refreshBackground(color, outlineThickness, outlineColor);
}

void GraphHolder::setOutlineThickness(float thickness){
    GraphHolder::refreshBackground(fillColor, thickness, outlineColor);
}

void GraphHolder::setOutlineColor(const ccColor4B& color){
    GraphHolder::refreshBackground(fillColor, outlineThickness, color);
}

void GraphHolder::refreshBackground(const ccColor4B& newFillColor, float newOutlineThickness, const ccColor4B& newOutlineColor){
    if (fillNode) fillNode->clear();
    else fillNode = CCDrawNode::create();

    if (outlineNode) outlineNode->clear();
    else outlineNode = CCDrawNode::create();

    fillColor = newFillColor;
    outlineThickness = newOutlineThickness;
    outlineColor = newOutlineColor;

    auto size = scrollLayer->content->getContentSize() * scrollLayer->getCurrentZoom();
    
    CCPoint MaskShape[4] = {
        ccp(0, 0),
        ccp(size.width, 0),
        ccp(size.width, size.height),
        ccp(0, size.height)
    };

    if (!mask){
        mask = CCClippingNode::create();
        mask->setID("mask");
        this->addChild(mask);
    }

    fillNode->drawPolygon(MaskShape, 4, ccc4FFromccc4B(newFillColor), newOutlineThickness, ccc4FFromccc4B(newOutlineColor));
    fillNode->setID("fill");
    fillNode->setZOrder(-1);
    mask->setStencil(fillNode);
    mask->addChild(fillNode);

    outlineNode->drawPolygon(MaskShape, 4, ccc4FFromccc4B({0, 0, 0, 0}), newOutlineThickness, ccc4FFromccc4B(newOutlineColor));
    outlineNode->setID("outline");
    outlineNode->setZOrder(2);
    mask->addChild(outlineNode);
}


void GraphHolder::refreshGrid(){

    if (boldGridNode) boldGridNode->clear();
    else {
        boldGridNode = CCDrawNode::create();
        boldGridNode->setZOrder(-1);
        boldGridNode->m_bUseArea = false;
        this->addChild(boldGridNode);
    }

    auto devidedSize = (scrollLayer->content->getContentSize() * scrollLayer->getCurrentZoom() + this->getContentSize()) / 100;

    for (int i = 0; i <= 100; i++){
        if (floor(static_cast<float>(i) / labelEvery) == static_cast<float>(i) / labelEvery){
            boldGridNode->drawSegment(ccp(0, i * devidedSize.height), ccp(100 * devidedSize.width, i * devidedSize.height), 0.2f, ccc4FFromccc4B(boldGridColor));
            boldGridNode->drawSegment(ccp(i * devidedSize.width, 0), ccp(i * devidedSize.width, 100 * devidedSize.height), 0.2f, ccc4FFromccc4B(boldGridColor));
        }
    }
}

void GraphHolder::updateLabels(){
    auto tempT = CCLabelBMFont::create("100", "chatFont.fnt");
    tempT->setScale(0.4f);
    float XForPr = tempT->getScaledContentSize().width;

    smallLines.clear();
    boldLines.clear();
    labels.clear();

    LabelsHorizontalContainer->removeAllChildrenWithCleanup(true);
    LabelsVerticalContainer->removeAllChildrenWithCleanup(true);

    auto devidedSize = (scrollLayer->content->getContentSize() * scrollLayer->getCurrentZoom() + this->getContentSize()) / 100;

    for (int i = 0; i <= 100; i++)
    {
        auto labelPr = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPr->setPositionX(i * devidedSize.width);
        labelPr->setRotation(90);
        labelPr->setColor({smallLineColor.r, smallLineColor.g, smallLineColor.b});
        labelPr->setOpacity(smallLineColor.a);
        LabelsHorizontalContainer->addChild(labelPr);

        float closestBigLabel = static_cast<float>(i) / labelEvery;
        
        if (floor(closestBigLabel) == closestBigLabel){
            labelPr->setScaleX(0.2f);

            labelPr->setColor({boldLineColor.r, boldLineColor.g, boldLineColor.b});
            labelPr->setOpacity(boldLineColor.a);

            auto labelPrText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPrText->setPositionX(i * devidedSize.width);
            labelPrText->setScale(0.4f);
            labelPrText->setPositionY(-labelPr->getScaledContentSize().width - labelPrText->getScaledContentSize().height);
            labelPrText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelPrText->setOpacity(labelColor.a);
            LabelsHorizontalContainer->addChild(labelPrText);

            boldLines.insert(labelPr);
            labels.insert(labelPrText);
        }
        else{
            labelPr->setScaleX(0.1f);
            labelPr->setScaleY(0.8f);

            smallLines.insert(labelPr);
        }
        labelPr->setPositionY(-labelPr->getScaledContentSize().width);

        //

        auto labelPS = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPS->setPositionY(i * devidedSize.height);
        labelPS->setColor({smallLineColor.r, smallLineColor.g, smallLineColor.b});
        labelPS->setOpacity(smallLineColor.a);
        LabelsVerticalContainer->addChild(labelPS);

        if (floor(closestBigLabel) == closestBigLabel){
            labelPS->setScaleX(0.2f);

            labelPS->setColor({boldLineColor.r, boldLineColor.g, boldLineColor.b});
            labelPS->setOpacity(boldLineColor.a);

            auto labelPSText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPSText->setPositionY(i * devidedSize.height);
            labelPSText->setScale(0.4f);
            labelPSText->setPositionX(-labelPS->getScaledContentSize().width - XForPr);
            labelPSText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelPSText->setOpacity(labelColor.a);
            LabelsVerticalContainer->addChild(labelPSText);

            boldLines.insert(labelPS);
            labels.insert(labelPSText);
        }
        else{
            labelPS->setScaleX(0.1f);
            labelPS->setScaleY(0.8f);

            smallLines.insert(labelPS);
        }

        labelPS->setPositionX(-labelPS->getScaledContentSize().width);
    }
}

// void GraphHolder::setSmallLinesColor(const ccColor4B& newColor){
//     smallLineColor = newColor;
//     for (auto line : smallLines){
//         line->setColor({newColor.r, newColor.g, newColor.b});
//         line->setOpacity(newColor.a);
//     }
// }

// void GraphHolder::setBoldLinesColor(const ccColor4B& newColor){
//     boldLineColor = newColor;
//     for (auto line : boldLines){
//         line->setColor({newColor.r, newColor.g, newColor.b});
//         line->setOpacity(newColor.a);
//     }
// }

// void GraphHolder::setLabelsColor(const ccColor4B& newColor){
//     labelColor = newColor;
//     for (auto label : labels){
//         label->setColor({newColor.r, newColor.g, newColor.b});
//         label->setOpacity(newColor.a);
//     }
// }

void GraphHolder::update(float dt){
    if (prevZoom != scrollLayer->getCurrentZoom()){
        prevZoom = scrollLayer->getCurrentZoom();

        labelEvery = labelEveryBase / scrollLayer->getCurrentZoom();

        GraphHolder::refreshGrid();
        GraphHolder::updateLabels();
    }

    auto posInSpace = LabelsHorizontalContainer->getParent()->convertToNodeSpace(scrollLayer->content->convertToWorldSpace(scrollLayer->content->getPosition()));

    auto zoomModification = this->getContentSize() * ((scrollLayer->getCurrentZoom() - 1) * 0.5f);

    LabelsHorizontalContainer->setPositionX(posInSpace.x - zoomModification.width);
    LabelsVerticalContainer->setPositionY(posInSpace.y - zoomModification.height);
    boldGridNode->setPosition(posInSpace - zoomModification);
}