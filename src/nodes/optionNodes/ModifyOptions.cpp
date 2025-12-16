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
    TypeToggler->setCallback([&](bool res){
        if (res){
            updatePreviewName(res);
        }
        else{
            updatePreviewName(res);
        }
    });
    this->addChild(TypeToggler);

    auto dtLayer = DTLayer::get();

    int sessionAmount = 1;
    if  (dtLayer != nullptr){
        sessionAmount = dtLayer->sessionsOrder.size();
    }

    auto sessionSelector = SessionSelector::create(sessionAmount);
    sessionSelector->setCallback([&](int newSession) {
        DTLayer::get()->onSessionSelected(newSession, true);
        if (TypeToggler->isToggled())
            updatePreviewName(TypeToggler->isToggled());
    });
    sessionSelector->setScale(.7f);
    sessionSelector->ignoreAnchorPointForPosition(true);
    sessionSelector->setCurrentCount(DTLayer::get()->getCurrentSelectedSession(), true);
    sessionSelector->setPosition(TypeToggler->getPosition() + ccp(size.width / 2.5f / 2, 0) - sessionSelector->getContentSize() / 2);
    this->addChild(sessionSelector);

    float localSharedMargin = 10;
    float localSharedTopPadding = 10;
    
    auto from0ViewBtnSpr = ButtonSprite::create("from 0", "gjFont17.fnt", "GJ_button_04.png");
    from0ViewBtnSpr->setScale(.5f);
    from0ViewBtnSpr->setID("enabled");
    from0ViewBtn = CCMenuItemSpriteExtra::create(
        from0ViewBtnSpr,
        this,
        menu_selector(ModifyOptions::onLevelView)
    );
    from0ViewBtn->setPosition({
        size.width / 2.5f / 2 - from0ViewBtn->getContentWidth() / 2 - localSharedMargin,
        TypeToggler->getPositionY() - from0ViewBtn->getContentHeight() / 2 - TypeToggler->getContentHeight() / 2 - localSharedTopPadding
    });

    auto from0ViewBtnSprDisabled = ButtonSprite::create("from 0", "gjFont17.fnt", "GJ_button_01.png");
    from0ViewBtnSprDisabled->setScale(.5f);
    from0ViewBtnSprDisabled->setPosition(from0ViewBtnSpr->getPosition());
    from0ViewBtnSprDisabled->setID("disabled");
    from0ViewBtnSprDisabled->setVisible(false);
    from0ViewBtn->addChild(from0ViewBtnSprDisabled);

    auto runViewBtnSpr = ButtonSprite::create("runs", "gjFont17.fnt", "GJ_button_04.png");
    runViewBtnSpr->setID("enabled");
    runViewBtnSpr->setScale(.5f);
    runViewBtn = CCMenuItemSpriteExtra::create(
        runViewBtnSpr,
        this,
        menu_selector(ModifyOptions::onRunsView)
    );
    runViewBtnSpr->setVisible(false);
    runViewBtn->setPosition({
        size.width / 2.5f / 2 + runViewBtn->getContentWidth() / 2 + localSharedMargin,
        TypeToggler->getPositionY() - runViewBtn->getContentHeight() / 2 - TypeToggler->getContentHeight() / 2 - localSharedTopPadding
    });

    auto runViewBtnSprDisabled = ButtonSprite::create("runs", "gjFont17.fnt", "GJ_button_01.png");
    runViewBtnSprDisabled->setScale(.5f);
    runViewBtnSprDisabled->setPosition(runViewBtnSpr->getPosition());
    runViewBtnSprDisabled->setID("disabled");
    runViewBtn->addChild(runViewBtnSprDisabled);

    runViewBtn->setEnabled(false);

    this->addChild(from0ViewBtn);
    this->addChild(runViewBtn);

    float previewBGMargin = 10;

    auto previewBG = CCScale9Sprite::create("square02_001.png");
    previewBG->setContentSize({
        size.width / 2.5f - previewBGMargin,
        size.height - TypeToggler->getContentHeight() - from0ViewBtn->getContentHeight() - localSharedTopPadding - topeToggleHightPadding - previewBGMargin
    });
    previewBG->setPosition({
        size.width / 2.5f / 2 + previewBGMargin / 2,
        0 + previewBGMargin / 2
    });
    previewBG->setAnchorPoint({0.5f, 0});
    previewBG->setOpacity(150);
    this->addChild(previewBG);

    previewScroll = ScrollLayer::create(previewBG->getContentSize() - ccp(5, 5));
    previewScroll->setPosition(previewBG->getPosition() + ccp(0, 2.5f));
    previewScroll->ignoreAnchorPointForPosition(false);
    previewScroll->setAnchorPoint({.5f, 0});
    previewScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(previewScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    this->addChild(previewScroll);

    auto scrollbar = Scrollbar::create(previewScroll);
    scrollbar->ignoreAnchorPointForPosition(false);
    scrollbar->setPosition(previewScroll->getPosition() + previewScroll->getContentSize() / 2);
    this->addChild(scrollbar);

    DTLabelInfo info;
    info.scale = .5f;
    info.wrapping = WrappingMode::SPACE_WRAP;
    info.isExpanded = true;

    myLabel = DTLabel::create(info);
    myLabel->setExpandable(false);
    myLabel->setContentWidth(previewScroll->getContentWidth());
    previewScroll->m_contentLayer->addChild(myLabel);

    TypeToggler->toggleWithCallback(true);
    TypeToggler->toggleWithCallback(false);

    previewScroll->m_contentLayer->updateLayout();
    previewScroll->moveToTop();

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
    seperator1->setContentSize({1.5f, size.height - (deathFormatInput->getContentHeight() / 2 + deathFormatInput->getPositionY() + 10)});
    seperator1->setPositionX(size.width / 2 - 20);
    seperator1->setPositionY(deathFormatInput->getScaledContentHeight() / 2 + deathFormatInput->getPositionY() + 10 + seperator1->getContentHeight() / 2);
    this->addChild(seperator1);

    auto seperator2 = CCScale9Sprite::create("pixel.png");
    seperator2->setContentSize({deathFormatInput->getScaledContentWidth(), 1.5f});
    seperator2->setAnchorPoint({0, 0});
    seperator2->setPositionX(seperator1->getPositionX());
    seperator2->setPositionY(seperator1->getPositionY() - seperator1->getContentHeight() / 2);
    this->addChild(seperator2);

    auto seperator3 = CCScale9Sprite::create("pixel.png");
    seperator3->setContentSize({1.5f, size.height - seperator1->getContentHeight()});
    seperator3->setAnchorPoint({1, 1});
    seperator3->setPositionX(seperator2->getPositionX() + seperator2->getContentWidth());
    seperator3->setPositionY(seperator2->getPositionY());
    this->addChild(seperator3);

    auto amountInput = TextInput::create(60, "amount", "gjFont17.fnt");
    amountInput->setPosition({
        seperator1->getPositionX() + amountInput->getScaledContentWidth() / 2 + 15,
        seperator1->getPositionY()
    });
    amountInput->setCommonFilter(CommonFilter::Uint);
    this->addChild(amountInput);

    scheduleUpdate();
    onLevelView(nullptr);

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

void ModifyOptions::onLevelView(CCObject*){
    runViewBtn->setEnabled(true);
    from0ViewBtn->setEnabled(false);

    runViewBtn->getChildByID("enabled")->setVisible(true);
    runViewBtn->getChildByID("disabled")->setVisible(false);

    from0ViewBtn->getChildByID("enabled")->setVisible(false);
    from0ViewBtn->getChildByID("disabled")->setVisible(true);

    updatePreviewName(TypeToggler->isToggled());
}
void ModifyOptions::onRunsView(CCObject*){
    runViewBtn->setEnabled(false);
    from0ViewBtn->setEnabled(true);

    runViewBtn->getChildByID("enabled")->setVisible(false);
    runViewBtn->getChildByID("disabled")->setVisible(true);

    from0ViewBtn->getChildByID("enabled")->setVisible(true);
    from0ViewBtn->getChildByID("disabled")->setVisible(false);

    updatePreviewName(TypeToggler->isToggled());
}

void ModifyOptions::update(float dt){
    float prevHeight = previewScroll->m_contentLayer->getContentHeight();
    previewScroll->m_contentLayer->updateLayout();
    if (previewScroll->m_contentLayer->getContentHeight() != prevHeight)
        previewScroll->moveToTop();
}

void ModifyOptions::updatePreviewName(bool categotyIsSession){
    std::string category = !categotyIsSession ?
        "level" :
        fmt::format("session {}", DTLayer::get()->getCurrentSelectedSession());
    std::string type = !from0ViewBtn->isEnabled() ?
        "from 0" :
        "runs";

    myLabel->setLabelName(fmt::format("preview ({}, {})", category, type));

    std::string text;

    if (!categotyIsSession){
        if (!from0ViewBtn->isEnabled()) text = "f0";
        else text = "runs";
    }
    else{
        if (!from0ViewBtn->isEnabled()) text = "s0";
        else text = "sruns";
    }

    myLabel->setLabelText(fmt::format("{{{}}}", text));
}