#include <nodes/optionNodes/ModifyOptions.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <utils/Dev.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>
#include <hooks/DTCCTextInputNode.hpp>

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
    TypLevelBtnSpr->setCascadeOpacityEnabled(true);
    TypeSessionBtnSpr->setCascadeOpacityEnabled(true);
    TypeToggler = SimpleToggler::create(
        TypLevelBtnSpr,
        TypeSessionBtnSpr,
        .75f
    );
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

    sessionSelector = SessionSelector::create(sessionAmount);
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
    from0ViewBtnSpr->setCascadeOpacityEnabled(true);
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
    from0ViewBtnSprDisabled->setCascadeOpacityEnabled(true);
    from0ViewBtnSprDisabled->setScale(.5f);
    from0ViewBtnSprDisabled->setPosition(from0ViewBtnSpr->getPosition());
    from0ViewBtnSprDisabled->setID("disabled");
    from0ViewBtnSprDisabled->setVisible(false);
    from0ViewBtn->addChild(from0ViewBtnSprDisabled);

    auto runViewBtnSpr = ButtonSprite::create("runs", "gjFont17.fnt", "GJ_button_04.png");
    runViewBtnSpr->setCascadeOpacityEnabled(true);
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
    runViewBtnSprDisabled->setCascadeOpacityEnabled(true);
    runViewBtnSprDisabled->setScale(.5f);
    runViewBtnSprDisabled->setPosition(runViewBtnSpr->getPosition());
    runViewBtnSprDisabled->setID("disabled");
    runViewBtn->addChild(runViewBtnSprDisabled);

    runViewBtn->setEnabled(false);

    this->addChild(from0ViewBtn);
    this->addChild(runViewBtn);

    float previewBGMargin = 10;

    auto previewBGHolder = CCNode::create();
    this->addChild(previewBGHolder);

    previewBG = CCScale9Sprite::create("square02_001.png");
    previewBG->setContentSize({
        size.width / 2.5f - previewBGMargin,
        size.height - TypeToggler->getContentHeight() - from0ViewBtn->getContentHeight() - localSharedTopPadding - topeToggleHightPadding - previewBGMargin
    });
    previewBG->setPosition({
        size.width / 2.5f / 2 + previewBGMargin / 2,
        0 + previewBGMargin / 2
    });
    previewBG->setAnchorPoint({0.5f, 0});
    previewBG->setOpacity(0);
    previewBGHolder->addChild(previewBG);

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
    previewScroll->setMouseEnabled(false);
    this->addChild(previewScroll);

    scrollbar = Scrollbar::create(previewScroll);
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
    runSeperatorInput->setString(Save::getRunsCustomazations().seperator);
    runSeperatorInput->setCallback([&](auto newStr){
        auto curr = Save::getRunsCustomazations();
        curr.seperator = newStr;

        Save::setRunCustomazations(curr);

        DTLayer::get()->specialStrings["runs"]->updateContent();
        DTLayer::get()->specialStrings["sruns"]->updateContent();
    });
    runSeperatorInput->setCommonFilter(CommonFilter::Any);
    this->addChild(runSeperatorInput);

    auto f0SeperatorInput = TextInput::create(60, "F0 Sep", "gjFont17.fnt");
    f0SeperatorInput->setPosition({
        previewBG->getPositionX() + previewBG->getContentWidth() / 2 + f0SeperatorInput->getContentWidth() / 2 + previewBGMargin,
        previewBGMargin / 2 + previewBG->getContentHeight() / 2
    });
    f0SeperatorInput->setString(Save::getFrom0Customazations().seperator);
    f0SeperatorInput->setCallback([&](auto newStr){
        auto curr = Save::getFrom0Customazations();
        curr.seperator = newStr;

        Save::setFrom0Customazations(curr);

        DTLayer::get()->specialStrings["f0"]->updateContent();
        DTLayer::get()->specialStrings["s0"]->updateContent();
    });
    f0SeperatorInput->setCommonFilter(CommonFilter::Any);
    this->addChild(f0SeperatorInput);

    auto deathFormatInput = TextInput::create(60, "DFormat", "gjFont17.fnt");
    deathFormatInput->setPosition({
        previewBG->getPositionX() + previewBG->getContentWidth() / 2 + deathFormatInput->getContentWidth() / 2 + previewBGMargin,
        previewBGMargin / 2 + previewBG->getContentHeight() - deathFormatInput->getContentHeight() / 2
    });
    deathFormatInput->setString(Save::getFrom0Customazations().format);
    deathFormatInput->setCallback([&](auto newStr){
        auto currF0 = Save::getFrom0Customazations();
        auto currRuns = Save::getRunsCustomazations();
        currF0.format = newStr;
        currRuns.format = newStr;

        Save::setFrom0Customazations(currF0);
        Save::setRunCustomazations(currRuns);

        DTLayer::get()->specialStrings["f0"]->updateContent();
        DTLayer::get()->specialStrings["s0"]->updateContent();
        DTLayer::get()->specialStrings["runs"]->updateContent();
        DTLayer::get()->specialStrings["sruns"]->updateContent();
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
    amountInput->setString(std::to_string(currAmount));
    amountInput->setCallback([&, amountInput](auto newstr){
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        currAmount = numRes.unwrap();

        if (currAmount > 100) {
            currAmount = 100;
            amountInput->setString("100");
        }
    });
    amountInput->setCommonFilter(CommonFilter::Uint);
    this->addChild(amountInput);

    auto addPercentInput = TextInput::create(75, "percent", "gjFont17.fnt");
    addPercentInput->setPosition({
        size.width - size.width / 5,
        size.height - size.height / 5
    });
    addPercentInput->setCallback([addPercentInput](auto newstr){
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            addPercentInput->setString("100");
        }
    });
    addPercentInput->setCommonFilter(CommonFilter::Uint);
    this->addChild(addPercentInput);

    addPlusMinusBtns(addPercentInput, false, [&, addPercentInput, amountInput](bool isPlus){
        auto amountRes = utils::numFromString<int>(amountInput->getString());
        if (amountRes.isErr()) return;
        auto amount = amountRes.unwrap();
        auto addPerRes = utils::numFromString<int>(addPercentInput->getString());
        if (addPerRes.isErr()) return;
        auto addPer = addPerRes.unwrap();

        std::optional<int> sessionNum = std::nullopt;
        if (TypeToggler->isToggled())
            sessionNum = sessionSelector->getCurrentCount();

        DTLayer::get()->modifyRun(addPer, amount * (isPlus ? 1 : -1), sessionNum);
        
        if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["f0"]->updateContent();
        else DTLayer::get()->specialStrings["s0"]->updateContent();
    }, .9f);

    auto addPercentInputLabel = CCLabelBMFont::create("From 0", "bigFont.fnt");
    addPercentInputLabel->setPosition(addPercentInput->getPosition() + ccp(0, addPercentInput->getContentHeight() / 2));
    addPercentInputLabel->setAnchorPoint({.5f, 0});
    addPercentInputLabel->setScale(.5f);
    this->addChild(addPercentInputLabel);

    auto addRunSeperator = CCLabelBMFont::create("-", "bigFont.fnt");
    addRunSeperator->setPosition({
        size.width - size.width / 5,
        size.height - (size.height / 5 * 2.25f)
    });
    addRunSeperator->setScale(.5f);
    this->addChild(addRunSeperator);

    auto addRunStartInput = TextInput::create(40, "sta%", "gjFont17.fnt");
    addRunStartInput->setPosition(addRunSeperator->getPosition() + ccp(-addRunSeperator->getScaledContentWidth() / 2 - addRunStartInput->getContentWidth() / 2 - 5, 0));
    addRunStartInput->setCommonFilter(CommonFilter::Uint);
    this->addChild(addRunStartInput);

    auto addRunEndInput = TextInput::create(40, "end%", "gjFont17.fnt");
    addRunEndInput->setPosition(addRunSeperator->getPosition() + ccp(addRunSeperator->getScaledContentWidth() / 2 + addRunEndInput->getContentWidth() / 2 + 5, 0));
    addRunEndInput->setCommonFilter(CommonFilter::Uint);
    this->addChild(addRunEndInput);

    static_cast<DTCCTextInputNode*>(addRunStartInput->getInputNode())->setCallback([addRunStartInput, addRunEndInput, addRunSeperator](const std::string& newText){
        if (newText == "-" && addRunStartInput->getInputNode()->m_selected){
            addRunStartInput->defocus();
            addRunEndInput->focus();
            addRunEndInput->setString("");
            addRunSeperator->setColor({ 0, 255, 0 });
            addRunSeperator->runAction(CCTintTo::create(.5f, 255, 255, 255));
        }
    });

    static_cast<DTCCTextInputNode*>(addRunEndInput->getInputNode())->setCallback([addRunStartInput, addRunEndInput, addRunSeperator](const std::string& newText){
        if (newText == "-" && addRunEndInput->getInputNode()->m_selected){
            addRunEndInput->defocus();
            addRunStartInput->focus();
            addRunStartInput->setString("");
            addRunSeperator->setColor({ 0, 255, 0 });
            addRunSeperator->runAction(CCTintTo::create(.5f, 255, 255, 255));
        }
    });

    addRunStartInput->setCallback([&, addRunStartInput, addRunEndInput](auto newstr){
        auto numStartRes = geode::utils::numFromString<int>(newstr);
        if (numStartRes.isErr()) return;
        int numStart = numStartRes.unwrap();
        auto numEndRes = geode::utils::numFromString<int>(addRunEndInput->getString());
        int numEnd;
        if (numEndRes.isErr()) numEnd = 0;
        else numEnd = numEndRes.unwrap();

        if (numStart > 100) {
            numStart = 100;
            addRunStartInput->setString("100");
        }
    });

    addRunEndInput->setCallback([&, addRunStartInput, addRunEndInput](auto newstr){
        auto numStartRes = geode::utils::numFromString<int>(addRunStartInput->getString());
        int numStart;
        if (numStartRes.isErr()) numStart = 0;
        else numStart = numStartRes.unwrap();
        auto numEndRes = geode::utils::numFromString<int>(newstr);
        if (numEndRes.isErr()) return;
        int numEnd = numEndRes.unwrap();

        if (numEnd > 100) {
            numEnd = 100;
            addRunEndInput->setString("100");
        }
    });

    auto addRunOverallNode = CCNode::create();
    addRunOverallNode->setPosition(addRunSeperator->getPosition());
    addRunOverallNode->setAnchorPoint({.5f, .5f});
    addRunOverallNode->setContentWidth(addRunSeperator->getScaledContentWidth() + addRunStartInput->getContentWidth() + addRunEndInput->getContentWidth() + 10);
    this->addChild(addRunOverallNode);

    addPlusMinusBtns(addRunOverallNode, false, [&, amountInput, addRunStartInput, addRunEndInput](bool isPlus){
        auto amountRes = utils::numFromString<int>(amountInput->getString());
        if (amountRes.isErr()) return;
        auto amount = amountRes.unwrap();
        auto startPerRes = utils::numFromString<int>(addRunStartInput->getString());
        if (startPerRes.isErr()) return;
        auto startPer = startPerRes.unwrap();
        auto endPerRes = utils::numFromString<int>(addRunEndInput->getString());
        if (endPerRes.isErr()) return;
        auto endPer = endPerRes.unwrap();

        if (startPer > endPer){
            auto tint = CCTintTo::create(.2f, 255, 0, 0);
            auto seq = CCSequence::create(
                CCTintTo::create(.2f, 255, 0, 0),
                CCTintTo::create(.2f, 255, 255, 255),
                nullptr
            );
            seq->setTag(8);
            addRunStartInput->getInputNode()->m_textLabel->stopActionByTag(8);
            addRunStartInput->getInputNode()->m_textLabel->runAction(seq);
            return;
        }

        std::optional<int> sessionNum = std::nullopt;
        if (TypeToggler->isToggled())
            sessionNum = sessionSelector->getCurrentCount();

        DTLayer::get()->modifyRun(startPer, endPer, amount * (isPlus ? 1 : -1), sessionNum);
        
        if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["runs"]->updateContent();
        else DTLayer::get()->specialStrings["sruns"]->updateContent();
    }, .9f);

    scheduleUpdate();
    onLevelView(nullptr);

    auto addRunLabel = CCLabelBMFont::create("Runs", "bigFont.fnt");
    addRunLabel->setPosition(addRunSeperator->getPosition() + ccp(0, addRunStartInput->getContentHeight() / 2));
    addRunLabel->setAnchorPoint({.5f, 0});
    addRunLabel->setScale(.5f);
    this->addChild(addRunLabel);

    auto seperator4 = CCScale9Sprite::create("pixel.png");
    seperator4->setContentSize({size.width / 3, 1.5f});
    seperator4->setPosition({
        size.width - size.width / 5,
        size.height - (size.height / 5 * 3)
    });
    this->addChild(seperator4);

    auto addNewBestInput = TextInput::create(75, "NB perce", "gjFont17.fnt");
    addNewBestInput->setPosition({
        size.width - size.width / 5,
        size.height - (size.height / 5 * 4)
    });
    addNewBestInput->setCallback([addNewBestInput](auto newstr){
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            addNewBestInput->setString("100");
        }
    });
    addNewBestInput->setCommonFilter(CommonFilter::Uint);
    this->addChild(addNewBestInput);

    addPlusMinusBtns(addNewBestInput, false, [&, amountInput, addNewBestInput](bool isPlus){
        auto amountRes = utils::numFromString<int>(amountInput->getString());
        if (amountRes.isErr()) return;
        auto amount = amountRes.unwrap();
        auto nbPercentRes = utils::numFromString<int>(addNewBestInput->getString());
        if (nbPercentRes.isErr()) return;
        auto nbPercent = nbPercentRes.unwrap();

        std::optional<int> sessionNum = std::nullopt;
        if (TypeToggler->isToggled())
            sessionNum = sessionSelector->getCurrentCount();

        DTLayer::get()->modifyNewBest(nbPercent, isPlus, sessionNum);

        if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["f0"]->updateContent();
        else DTLayer::get()->specialStrings["s0"]->updateContent();
    }, .9f);

    auto addNewBestInputLabel = CCLabelBMFont::create("New Bests", "bigFont.fnt");
    addNewBestInputLabel->setPosition(addNewBestInput->getPosition() + ccp(0, addNewBestInput->getContentHeight() / 2));
    addNewBestInputLabel->setAnchorPoint({.5f, 0});
    addNewBestInputLabel->setScale(.5f);
    this->addChild(addNewBestInputLabel);

    for (const auto& child : CCArrayExt<CCNode*>(this->getChildren()))
    {
        if (auto textInput = typeinfo_cast<TextInput*>(child)){
            Dev::fadeTextInput(textInput, false, 0);
        }
    }
    this->setOpacity(0);

    myLabel->setOpacity(0);
    myLabel->setTextColor({255, 255, 255, 0});
    myLabel->fadeTitleColorTo({255, 255, 255, 0}, 0);
    Dev::fadeTextInput(sessionSelector->getTextInput(), false, 0);

    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(0))->setOpacity(0);
    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(1))->setOpacity(0);
    scrollbar->setTouchEnabled(false);

    sessionSelector->setEnabled(false);

    addEventListener<keybinds::InvokeBindFilter>([&, addNewBestInput, addRunOverallNode, addRunStartInput, addRunEndInput, addPercentInput](keybinds::InvokeBindEvent* event) {
        if (event->isDown()) {
            if (addNewBestInput->getInputNode()->m_selected) {
                plusMinusCallbacks[holdersOfPlusMinusBtns[addNewBestInput].first](true);
            }
            else if (addRunStartInput->getInputNode()->m_selected || addRunEndInput->getInputNode()->m_selected){
                plusMinusCallbacks[holdersOfPlusMinusBtns[addRunOverallNode].first](true);
            }
            else if (addPercentInput->getInputNode()->m_selected) {
                plusMinusCallbacks[holdersOfPlusMinusBtns[addPercentInput].first](true);
            }
        }
        return ListenerResult::Propagate;
    }, "add-deaths"_spr);

    addEventListener<keybinds::InvokeBindFilter>([&, addNewBestInput, addRunOverallNode, addRunStartInput, addRunEndInput, addPercentInput](keybinds::InvokeBindEvent* event) {
        if (event->isDown()) {
            if (addNewBestInput->getInputNode()->m_selected) {
                plusMinusCallbacks[holdersOfPlusMinusBtns[addNewBestInput].second](false);
            }
            else if (addRunStartInput->getInputNode()->m_selected || addRunEndInput->getInputNode()->m_selected){
                plusMinusCallbacks[holdersOfPlusMinusBtns[addRunOverallNode].second](false);
            }
            else if (addPercentInput->getInputNode()->m_selected) {
                plusMinusCallbacks[holdersOfPlusMinusBtns[addPercentInput].second](false);
            }
        }
        return ListenerResult::Propagate;
    }, "remove-deaths"_spr);

    return true;
}

void ModifyOptions::onOpened(){
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));
    previewBG->runAction(CCFadeTo::create(fadeTime, 150));

    for (const auto& child : CCArrayExt<CCNode*>(this->getChildren()))
    {
        if (auto textInput = typeinfo_cast<TextInput*>(child)){
            Dev::fadeTextInput(textInput, true, fadeTime);
        }   
    }

    sessionSelector->setMaximumCount(DTLayer::get()->sessionSelector->getMaximumCount(), false);

    myLabel->runAction(CCFadeIn::create(fadeTime));
    myLabel->fadeTextColorTo({255, 255, 255, 255}, fadeTime);
    myLabel->fadeTitleColorTo({255, 255, 255, 255}, fadeTime);
    Dev::fadeTextInput(sessionSelector->getTextInput(), true, fadeTime);
    sessionSelector->setEnabled(true);

    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(0))->runAction(CCFadeTo::create(fadeTime, 150));
    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(1))->runAction(CCFadeTo::create(fadeTime, 255));
    scrollbar->setTouchEnabled(true);

    this->setEnabled(true);
    previewScroll->setMouseEnabled(true);  
}
void ModifyOptions::onClosed(){
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));
    previewBG->runAction(CCFadeTo::create(fadeTime, 0));

    for (const auto& child : CCArrayExt<CCNode*>(this->getChildren()))
    {
        if (auto textInput = typeinfo_cast<TextInput*>(child)){
            Dev::fadeTextInput(textInput, false, fadeTime);
        }   
    }

    myLabel->runAction(CCFadeOut::create(fadeTime));
    myLabel->fadeTextColorTo({255, 255, 255, 0}, fadeTime);
    myLabel->fadeTitleColorTo({255, 255, 255, 0}, fadeTime);
    Dev::fadeTextInput(sessionSelector->getTextInput(), false, fadeTime);
    sessionSelector->setEnabled(false);

    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(0))->runAction(CCFadeTo::create(fadeTime, 0));
    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(1))->runAction(CCFadeTo::create(fadeTime, 0));
    scrollbar->setTouchEnabled(false);

    this->setEnabled(false);
    previewScroll->setMouseEnabled(false);
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

void ModifyOptions::addPlusMinusBtns(CCNode* around, bool flip, const std::function<void(bool isPlus)>& callback, float scale){
    auto plusBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    plusBtnSpr->setScale(scale);
    auto plusBtn = CCMenuItemSpriteExtra::create(
        plusBtnSpr,
        this,
        menu_selector(ModifyOptions::onPlusMinusBtn)
    );
    plusBtn->setTag(1);
    this->addChild(plusBtn);
    
    auto minusBtnSpr = CCSprite::createWithSpriteFrameName("minus_button.png"_spr);
    minusBtnSpr->setScale(scale);
    auto minusBtn = CCMenuItemSpriteExtra::create(
        minusBtnSpr,
        this,
        menu_selector(ModifyOptions::onPlusMinusBtn)
    );
    minusBtn->setTag(2);
    this->addChild(minusBtn);

    plusBtn->setPosition(around->getPosition() +
        ccp((flip ? -1 : 1) * (around->getScaledContentWidth() * around->getAnchorPoint().x + plusBtn->getContentWidth() / 2 + 5), 0)
    );

    minusBtn->setPosition(around->getPosition() +
        ccp((flip ? 1 : -1) * (around->getScaledContentWidth() * around->getAnchorPoint().x + minusBtn->getContentWidth() / 2 + 5), 0)
    );

    plusMinusCallbacks.insert({plusBtn, callback});
    plusMinusCallbacks.insert({minusBtn, callback});

    holdersOfPlusMinusBtns.insert({around, std::make_pair(plusBtn, minusBtn)});
}
void ModifyOptions::onPlusMinusBtn(CCObject* sender){
    if (auto btnSender = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)){
        if (!plusMinusCallbacks.contains(btnSender)) return;

        plusMinusCallbacks[btnSender](btnSender->getTag() == 1);
    }
}