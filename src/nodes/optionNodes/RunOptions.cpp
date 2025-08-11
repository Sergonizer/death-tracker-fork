#include <nodes/optionNodes/RunOptions.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <utils/Dev.hpp>

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
    TARLabel->setScale(.65f);
    TARLabel->setPosition({size.width / 4, size.height - TARLabel->getScaledContentHeight() / 2});
    this->addChild(TARLabel);

    auto dtlayer = DTLayer::get();

    TARToggler = SimpleToggler::createWithDefaults(
        .75f,
        dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().trackAnyRun : false)
    );
    TARToggler->setCallback([&](bool isToggled){
        auto dtlayer = DTLayer::get();
        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr()) return;

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.trackAnyRun = isToggled;
    });
    float offset = TARToggler->getContentWidth() / 4 + 5;
    TARToggler->setPosition(TARLabel->getPosition() - ccp(TARLabel->getScaledContentWidth() / 2 + offset, 0));
    TARLabel->setPositionX(TARLabel->getPositionX() + offset);
    this->addChild(TARToggler);

    runAdditionInput = TextInput::create(size.width / 4.f, "<c-FFFF00>St</c>art %");
    runAdditionInput->setPosition({size.width / 4, TARLabel->getPositionY() - runAdditionInput->getContentHeight()});
    runAdditionInput->setCommonFilter(CommonFilter::Uint);
    runAdditionInput->setCallback([&](const std::string& newText){
        auto numRes = geode::utils::numFromString<int>(newText);

        if (numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        runAdditionInput->setString(std::to_string(num));
    });
    this->addChild(runAdditionInput);

    auto plusBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    auto plusBtn = CCMenuItemSpriteExtra::create(
        plusBtnSpr,
        this,
        menu_selector(RunOptions::addNewRun)
    );
    plusBtn->setPosition(runAdditionInput->getPosition() + ccp(runAdditionInput->getScaledContentWidth() / 2 + plusBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(plusBtn);

    auto runsMenuBG = CCScale9Sprite::create("GJ_square06.png");
    runsMenuBG->setContentSize({size.width / 2 / 1.1f, runAdditionInput->getPositionY() - runAdditionInput->getContentHeight() / 2});
    runsMenuBG->setPosition({abs(runsMenuBG->getContentWidth() / 2 - runAdditionInput->getPositionX()), 0});
    runsMenuBG->setAnchorPoint({0, 0});
    runsMenuBG->setColor({75, 75, 75});
    runsMenuBG->setOpacity(100);
    this->addChild(runsMenuBG);

    runsMenu = CCMenu::create();
    auto runsMenuOffset = ccp(15, 15);
    runsMenu->setPosition(runsMenuBG->getPosition() + runsMenuOffset / 2);
    runsMenu->setContentSize(runsMenuBG->getContentSize() - runsMenuOffset);
    runsMenu->setAnchorPoint(runsMenuBG->getAnchorPoint());
    this->addChild(runsMenu);
    runsMenu->setLayout(RowLayout::create()
        ->setCrossAxisOverflow(false)
        ->setGrowCrossAxis(true)
        ->setAxisAlignment(AxisAlignment::Start)
        ->setCrossAxisAlignment(AxisAlignment::End)
    );

    if (dtlayer != nullptr && dtlayer->m_SharedLevelStats.isOk()){
        auto& stats = dtlayer->m_SharedLevelStats.unwrap();
        for (const auto& startPercent : stats.RunsToSave)
        {
            auto percentCell = PercentCell::create(
                runsMenu->getContentWidth() / 2,
                startPercent,
                CCSprite::createWithSpriteFrameName("minus_button.png"_spr),
                [&](PercentCell* cell){ RunOptions::PercentCellClicked(cell); }
            );
            runsMenu->addChild(percentCell);
        }
    }

    runsMenu->updateLayout();

    auto seperator = CCScale9Sprite::create("square.png");
    seperator->setScale(.5f);
    seperator->setPosition(size / 2);
    seperator->setContentSize(ccp(1, size.height / 1.15f) / seperator->getScale());
    this->addChild(seperator);

    float rightOffset = 5;

    HideByLenInput = TextInput::create(45, "Len");
    HideByLenInput->setPosition({size.width - HideByLenInput->getContentWidth() / 2 - rightOffset, size.height / 8 * 7});
    HideByLenInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().hideRunLength) : ""));
    HideByLenInput->setCommonFilter(CommonFilter::Uint);
    HideByLenInput->setCallback([&](const std::string& newText){
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);

        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        HideByLenInput->setString(std::to_string(num));

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.hideRunLength = num;
    });
    this->addChild(HideByLenInput);

    auto HideByLenLabel = CCLabelBMFont::create("Hide by Length", "gjFont17.fnt");
    HideByLenLabel->setScale(.65f);
    HideByLenLabel->setWidth(size.width / 2 - HideByLenInput->getContentWidth() - rightOffset);
    HideByLenLabel->setPosition(HideByLenInput->getPosition() - ccp(HideByLenInput->getContentWidth() / 2 + rightOffset, 0));
    HideByLenLabel->setAnchorPoint({1, .5f});
    this->addChild(HideByLenLabel);

    HidUpToInput = TextInput::create(45, "%");
    HidUpToInput->setPosition({size.width - HidUpToInput->getContentWidth() / 2 - rightOffset, size.height / 8 * 5});
    HidUpToInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().hideUpto) : ""));
    HidUpToInput->setCommonFilter(CommonFilter::Uint);
    HidUpToInput->setCallback([&](const std::string& newText){
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);

        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        HidUpToInput->setString(std::to_string(num));

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.hideUpto = num;
    });
    this->addChild(HidUpToInput);

    auto HideUpToLabel = CCLabelBMFont::create("Hide Up To", "gjFont17.fnt");
    HideUpToLabel->setScale(.65f);
    HideUpToLabel->setWidth(size.width / 2 - HidUpToInput->getContentWidth() - rightOffset);
    HideUpToLabel->setPosition(HidUpToInput->getPosition() - ccp(HidUpToInput->getContentWidth() / 2 + rightOffset, 0));
    HideUpToLabel->setAnchorPoint({1, .5f});
    this->addChild(HideUpToLabel);

    RealEndPerInput = TextInput::create(45, "E %");
    RealEndPerInput->setPosition({size.width - RealEndPerInput->getContentWidth() / 2 - rightOffset, size.height / 8 * 3});
    RealEndPerInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().realEndPercent) : ""));
    RealEndPerInput->setCommonFilter(CommonFilter::Uint);
    RealEndPerInput->setCallback([&](const std::string& newText){
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);

        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        RealEndPerInput->setString(std::to_string(num));

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.realEndPercent = num;
    });
    this->addChild(RealEndPerInput);

    auto RealEndPerLabel = CCLabelBMFont::create("Real End %", "gjFont17.fnt");
    RealEndPerLabel->setScale(.65f);
    RealEndPerLabel->setWidth(size.width / 2 - RealEndPerInput->getContentWidth() - rightOffset);
    RealEndPerLabel->setPosition(RealEndPerInput->getPosition() - ccp(RealEndPerInput->getContentWidth() / 2 + rightOffset, 0));
    RealEndPerLabel->setAnchorPoint({1, .5f});
    this->addChild(RealEndPerLabel);

    ResetAsDeathToggler = SimpleToggler::createWithDefaults(
        .75f,
        dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().resetAsDeath : false)
    );
    ResetAsDeathToggler->setCallback([&](bool isToggled){
        auto dtlayer = DTLayer::get();
        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr()) return;

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.resetAsDeath = isToggled;
    });
    ResetAsDeathToggler->setPosition({size.width - ResetAsDeathToggler->getContentWidth() / 2 - rightOffset, size.height / 8 * 1});
    this->addChild(ResetAsDeathToggler);

    auto ResetAsDeathLabel = CCLabelBMFont::create("Reset as Death", "gjFont17.fnt");
    ResetAsDeathLabel->setScale(.65f);
    ResetAsDeathLabel->setWidth(size.width / 2 - ResetAsDeathToggler->getContentWidth() - rightOffset);
    ResetAsDeathLabel->setPosition(ResetAsDeathToggler->getPosition() - ccp(ResetAsDeathToggler->getContentWidth() / 2 + rightOffset, 0));
    ResetAsDeathLabel->setAnchorPoint({1, .5f});
    this->addChild(ResetAsDeathLabel);

    this->setOpacity(0);

    return true;
}

void RunOptions::onOpened(){
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));
    TARToggler->runAction(CCFadeIn::create(fadeTime));
    ResetAsDeathToggler->runAction(CCFadeIn::create(fadeTime));

    runAdditionInput->setEnabled(true);
    runAdditionInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(runAdditionInput, true, fadeTime);
    HideByLenInput->setEnabled(true);
    HideByLenInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(HideByLenInput, true, fadeTime);
    RealEndPerInput->setEnabled(true);
    RealEndPerInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(RealEndPerInput, true, fadeTime);
    HidUpToInput->setEnabled(true);
    HidUpToInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(HidUpToInput, true, fadeTime);

    for (const auto& child : CCArrayExt<CCMenu*>(runsMenu->getChildren())){
        child->setEnabled(true);
    }

    this->setEnabled(true);
}
void RunOptions::onClosed(){
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));
    TARToggler->runAction(CCFadeOut::create(fadeTime));
    ResetAsDeathToggler->runAction(CCFadeOut::create(fadeTime));

    runAdditionInput->setEnabled(false);
    Dev::fadeTextInput(runAdditionInput, false, fadeTime);
    HideByLenInput->setEnabled(false);
    Dev::fadeTextInput(HideByLenInput, false, fadeTime);
    RealEndPerInput->setEnabled(false);
    Dev::fadeTextInput(RealEndPerInput, false, fadeTime);
    HidUpToInput->setEnabled(false);
    Dev::fadeTextInput(HidUpToInput, false, fadeTime);

    for (const auto& child : CCArrayExt<CCMenu*>(runsMenu->getChildren())){
        child->setEnabled(false);
    }

    this->setEnabled(false);
}

void RunOptions::addNewRun(CCObject*){
    auto dtlayer = DTLayer::get();
    auto numRes = geode::utils::numFromString<int>(runAdditionInput->getString());

    if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

    int num = numRes.unwrap();

    auto& stats = dtlayer->m_MyLevelStats.unwrap();
    if (stats.RunsToSave.contains(num)) return;
    
    stats.RunsToSave.insert(num);

    auto percentCell = PercentCell::create(
        runsMenu->getContentWidth() / 2,
        num,
        CCSprite::createWithSpriteFrameName("minus_button.png"_spr),
        [&](PercentCell* cell){ RunOptions::PercentCellClicked(cell); }
    );
    runsMenu->addChild(percentCell);

    runsMenu->updateLayout();
}

void RunOptions::PercentCellClicked(PercentCell* cell){
    auto dtlayer = DTLayer::get();

    if (dtlayer != nullptr){
        int percent = cell->getPercent();

        dtlayer->UpdateOnAllShared([&, percent](LevelStats& stats){
            if (stats.RunsToSave.contains(percent))
                stats.RunsToSave.erase(percent);
        });
    }

    cell->removeMeAndCleanup();
    runsMenu->updateLayout();
}