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
    TARLabel->setAnchorPoint({.5f, .5f});
    TARLabel->setScale(.75f);
    TARLabel->setPosition({size.width / 4, size.height - TARLabel->getScaledContentHeight() / 2});
    this->addChild(TARLabel);

    auto dtlayer = DTLayer::get();
    bool startState = dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().trackAnyRun : false);

    auto toggler = SimpleToggler::createWithDefaults(
        .75f,
        startState
    );
    toggler->setCallback([&](bool isToggled){
        auto dtlayer = DTLayer::get();
        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.trackAnyRun = isToggled;
    });
    float offset = toggler->getContentWidth() / 4 + 5;
    toggler->setPosition(TARLabel->getPosition() - ccp(TARLabel->getScaledContentWidth() / 2 + offset, 0));
    TARLabel->setPositionX(TARLabel->getPositionX() + offset);
    this->addChild(toggler);

    auto runAdditionInput = TextInput::create(size.width / 4.f, "Start %");
    runAdditionInput->setPosition({size.width / 4, TARLabel->getPositionY() - runAdditionInput->getContentHeight()});
    this->addChild(runAdditionInput);

    auto plusBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    auto plusBtn = CCMenuItemSpriteExtra::create(
        plusBtnSpr,
        this,
        menu_selector(RunOptions::addNewRun)
    );
    plusBtn->setPosition(runAdditionInput->getPosition() + ccp(runAdditionInput->getScaledContentWidth() / 2 + plusBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(plusBtn);

    auto runsScroll = ScrollLayer::create({size.width / 2 / 1.1f, runAdditionInput->getPositionY() - runAdditionInput->getContentHeight() / 2});
    runsScroll->setPosition({abs(runsScroll->getContentWidth() / 2 - runAdditionInput->getPositionX()), 0});
    this->addChild(runsScroll);

    auto seperator = CCScale9Sprite::create("square.png");
    seperator->setScale(.5f);
    seperator->setPosition(size / 2);
    seperator->setContentSize(ccp(1, size.height / 1.15f) / seperator->getScale());
    this->addChild(seperator);

    auto runAdditionInput = TextInput::create(30, "Len");
    runAdditionInput->setPosition({size.width, 0});
    this->addChild(runAdditionInput);

    this->setOpacity(0);

    return true;
}

void RunOptions::onOpened(){
    this->runAction(CCFadeIn::create(.5f));
}
void RunOptions::onClosed(){
    this->runAction(CCFadeOut::create(.5f));
}

void RunOptions::addNewRun(CCObject*){

}