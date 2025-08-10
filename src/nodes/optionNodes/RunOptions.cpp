#include <nodes/optionNodes/RunOptions.hpp>

#include <nodes/SimpleToggler.hpp>
#include <nodes/layers/DTLayer.hpp>

RunOptions* RunOptions::create(const CCSize& size) {
    auto ret = new RunOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool RunOptions::setup(){

    auto TARLabel = CCLabelBMFont::create("Track any run", "gjFont17.fnt");
    TARLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    TARLabel->setAnchorPoint({0, .5f});
    this->addChild(TARLabel);

    auto dtlayer = DTLayer::get();
    bool startState = dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().trackAnyRun : false);

    auto toggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        .75f,
        startState
    );
    toggler->setCallback([&](bool isToggled){
        
    });
    this->addChild(toggler);

    this->setOpacity(0);

    return true;
}

void RunOptions::onOpened(){
    this->runAction(CCFadeIn::create(.5f));
}
void RunOptions::onClosed(){
    this->runAction(CCFadeOut::create(.5f));
}