#include <nodes/optionNodes/ModifyOptions.hpp>
#include <nodes/SessionSelector.hpp>
#include <nodes/layers/DTLayer.hpp>

ModifyOptions* ModifyOptions::create(const CCSize& size) {
    auto ret = new ModifyOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ModifyOptions::setup(){

    this->setEnabled(false);
    this->setOpacity(0);

    float topeToggleHightPadding = 5;

    auto TypLevelBtnSpr = ButtonSprite::create("Level");
    auto TypeSessionBtnSpr = ButtonSprite::create("Session");
    TypeToggler = SimpleToggler::create(
        TypLevelBtnSpr,
        TypeSessionBtnSpr,
        .75f
    );
    TypeToggler->setOpacity(0);
    TypeToggler->setPosition({
        size.width / 2.5f / 2 - size.width / 2.5f / 2 / 2,
        size.height - TypeSessionBtnSpr->getScaledContentSize().height / 2 - topeToggleHightPadding
    });
    this->addChild(TypeToggler);

    auto dtLayer = DTLayer::get();

    int sessionAmount = 1;
    if  (dtLayer != nullptr){
        sessionAmount = dtLayer->sessionsOrder.size();
    }

    auto sessionSelector = SessionSelector::create(sessionAmount);
    sessionSelector->setScale(.7f);
    sessionSelector->ignoreAnchorPointForPosition(true);
    sessionSelector->setPosition(TypeToggler->getPosition() + ccp(size.width / 2.5f / 2, 0) - sessionSelector->getContentSize() / 2);
    this->addChild(sessionSelector);

    float localSharedMargin = 10;
    float localSharedTopPadding = 10;
    
    auto localViewBtnSpr = ButtonSprite::create("local", "gjFont17.fnt", "GJ_button_04.png");
    localViewBtnSpr->setScale(.5f);
    localViewBtnSpr->setID("enabled");
    localViewBtn = CCMenuItemSpriteExtra::create(
        localViewBtnSpr,
        this,
        menu_selector(ModifyOptions::onLocalView)
    );
    localViewBtn->setPosition({
        size.width / 2.5f / 2 - localViewBtn->getContentWidth() / 2 - localSharedMargin,
        TypeToggler->getPositionY() - localViewBtn->getContentHeight() / 2 - TypeToggler->getContentHeight() / 2 - localSharedTopPadding
    });

    auto localViewBtnSprDisabled = ButtonSprite::create("local", "gjFont17.fnt", "GJ_button_01.png");
    localViewBtnSprDisabled->setScale(.5f);
    localViewBtnSprDisabled->setPosition(localViewBtnSpr->getPosition());
    localViewBtnSprDisabled->setID("disabled");
    localViewBtnSprDisabled->setVisible(false);
    localViewBtn->addChild(localViewBtnSprDisabled);

    auto sharedViewBtnSpr = ButtonSprite::create("shared", "gjFont17.fnt", "GJ_button_04.png");
    sharedViewBtnSpr->setID("enabled");
    sharedViewBtnSpr->setScale(.5f);
    sharedViewBtn = CCMenuItemSpriteExtra::create(
        sharedViewBtnSpr,
        this,
        menu_selector(ModifyOptions::onSharedView)
    );
    sharedViewBtnSpr->setVisible(false);
    sharedViewBtn->setPosition({
        size.width / 2.5f / 2 + sharedViewBtn->getContentWidth() / 2 + localSharedMargin,
        TypeToggler->getPositionY() - sharedViewBtn->getContentHeight() / 2 - TypeToggler->getContentHeight() / 2 - localSharedTopPadding
    });

    auto sharedViewBtnSprDisabled = ButtonSprite::create("shared", "gjFont17.fnt", "GJ_button_01.png");
    sharedViewBtnSprDisabled->setScale(.5f);
    sharedViewBtnSprDisabled->setPosition(sharedViewBtnSpr->getPosition());
    sharedViewBtnSprDisabled->setID("disabled");
    sharedViewBtn->addChild(sharedViewBtnSprDisabled);

    sharedViewBtn->setEnabled(false);

    this->addChild(localViewBtn);
    this->addChild(sharedViewBtn);

    float previewBGMargin = 10;

    auto previewBG = CCScale9Sprite::create("square02_001.png");
    previewBG->setContentSize({
        size.width / 2.5f - previewBGMargin,
        size.height - TypeToggler->getContentHeight() - localViewBtn->getContentHeight() - localSharedTopPadding - topeToggleHightPadding - previewBGMargin
    });
    previewBG->setPosition({
        size.width / 2.5f / 2 + previewBGMargin / 2,
        0 + previewBGMargin / 2
    });
    previewBG->setAnchorPoint({0.5f, 0});
    previewBG->setOpacity(150);
    this->addChild(previewBG);

    auto runSeperatorInput = TextInput::create(60, "Run Sep", "gjFont17.fnt");
    runSeperatorInput->setPosition({
        previewBG->getPositionX() + previewBG->getContentWidth() / 2 + runSeperatorInput->getContentWidth() / 2 + previewBGMargin,
        runSeperatorInput->getContentHeight() / 2 + previewBGMargin / 2
    });
    runSeperatorInput->setCommonFilter(CommonFilter::Any);
    this->addChild(runSeperatorInput);

    auto f0SeperatorInput = TextInput::create(60, "F0 Sep", "gjFont17.fnt");
    f0SeperatorInput->setPosition({
        previewBG->getPositionX() + previewBG->getContentWidth() / 2 + f0SeperatorInput->getContentWidth() / 2 + previewBGMargin,
        previewBGMargin / 2 + previewBG->getContentHeight() / 2
    });
    f0SeperatorInput->setCommonFilter(CommonFilter::Any);
    this->addChild(f0SeperatorInput);

    auto deathFormatInput = TextInput::create(60, "DFormat", "gjFont17.fnt");
    deathFormatInput->setPosition({
        previewBG->getPositionX() + previewBG->getContentWidth() / 2 + deathFormatInput->getContentWidth() / 2 + previewBGMargin,
        previewBGMargin / 2 + previewBG->getContentHeight() - deathFormatInput->getContentHeight() / 2
    });
    deathFormatInput->setCommonFilter(CommonFilter::Any);
    this->addChild(deathFormatInput);

    auto seperator1 = CCScale9Sprite::create("pixel.png");
    seperator1->setContentSize({2, size.height - (deathFormatInput->getContentHeight() / 2 + deathFormatInput->getPositionY() + 10)});
    seperator1->setPositionX(size.width / 2 - 20);
    seperator1->setPositionY(deathFormatInput->getScaledContentHeight() / 2 + deathFormatInput->getPositionY() + 10 + seperator1->getContentHeight() / 2);
    this->addChild(seperator1);

    auto seperator2 = CCScale9Sprite::create("pixel.png");
    seperator2->setContentSize({deathFormatInput->getScaledContentWidth(), 2});
    seperator2->setAnchorPoint({0, 0});
    seperator2->setPositionX(seperator1->getPositionX());
    seperator2->setPositionY(seperator1->getPositionY() - seperator1->getContentHeight() / 2);
    this->addChild(seperator2);

    auto seperator3 = CCScale9Sprite::create("pixel.png");
    seperator3->setContentSize({2, size.height - seperator1->getContentHeight()});
    seperator3->setAnchorPoint({1, 1});
    seperator3->setPositionX(seperator2->getPositionX() + seperator2->getContentWidth());
    seperator3->setPositionY(seperator2->getPositionY());
    this->addChild(seperator3);

    return true;
}

void ModifyOptions::onOpened(){
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));
    TypeToggler->runAction(CCFadeIn::create(fadeTime));

    this->setEnabled(true);
}
void ModifyOptions::onClosed(){
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));
    TypeToggler->runAction(CCFadeOut::create(fadeTime));

    this->setEnabled(false);
}

void ModifyOptions::onLocalView(CCObject*){
    sharedViewBtn->setEnabled(true);
    localViewBtn->setEnabled(false);

    sharedViewBtn->getChildByID("enabled")->setVisible(true);
    sharedViewBtn->getChildByID("disabled")->setVisible(false);

    localViewBtn->getChildByID("enabled")->setVisible(false);
    localViewBtn->getChildByID("disabled")->setVisible(true);
}
void ModifyOptions::onSharedView(CCObject*){
    sharedViewBtn->setEnabled(false);
    localViewBtn->setEnabled(true);

    sharedViewBtn->getChildByID("enabled")->setVisible(false);
    sharedViewBtn->getChildByID("disabled")->setVisible(true);

    localViewBtn->getChildByID("enabled")->setVisible(true);
    localViewBtn->getChildByID("disabled")->setVisible(false);
}