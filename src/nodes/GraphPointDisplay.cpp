#include "GraphPointDisplay.hpp"
#include <managers/StatsManager.hpp>

GraphPointDisplay* GraphPointDisplay::create() {
    auto ret = new GraphPointDisplay();
    if (ret && ret->init()) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool GraphPointDisplay::init(){

    this->setContentSize({100, 35});

    auto bg = CCScale9Sprite::create("square01_001.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.4f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    this->addChild(bg);

    percentLabel = CCLabelBMFont::create("", "bigFont.fnt");
    percentLabel->setScale(.4f);
    percentLabel->setPosition({this->getContentWidth() / 2, this->getContentHeight() - 5});
    percentLabel->setAnchorPoint({.5f, 1});
    this->addChild(percentLabel);
    
    rateLabel = CCLabelBMFont::create("", "bigFont.fnt");
    rateLabel->setScale(.25f);
    rateLabel->setPosition({this->getContentWidth() / 2, 5});
    rateLabel->setAnchorPoint({.5f, 0});
    this->addChild(rateLabel);

    return true;
}

void GraphPointDisplay::setContent(std::string run, float ratePercent, DTGraphType type){
    auto runInfo = StatsManager::splitRunKey(run).unwrapOr(Run{-1, -1});

    std::string runSprFormatted;
    if (runInfo.start == -1 && runInfo.end == -1)
        runSprFormatted = "Error";
    else if (runInfo.start == -1)
        runSprFormatted = fmt::format("{}%", runInfo.end);
    else
        runSprFormatted = fmt::format("{}%-{}%", runInfo.start, runInfo.end);

    percentLabel->setString(runSprFormatted.c_str());

    std::string rateType;

    switch (type)
    {
    case DTGraphType::Passrate :
        rateType = "Passrate";
        break;
    case DTGraphType::Reachrate :
        rateType = "Reachrate";
        break;
    
    default:
        rateType = "NA";
        break;
    }

    rateLabel->setString(fmt::format("{}: {}%", rateType, ratePercent).c_str());
}