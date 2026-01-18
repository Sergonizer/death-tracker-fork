#include <nodes/optionNodes/RunOptions.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <utils/Dev.hpp>
#if !defined(GEODE_IS_IOS)
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif

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
    auto TARLabel = CCLabelBMFont::create("Show any run", "gjFont17.fnt");
    TARLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    TARLabel->setAnchorPoint({.5f, .5f});
    TARLabel->setScale(.6f);
    TARLabel->setPosition({size.width / 4, size.height - TARLabel->getScaledContentHeight() / 2});
    this->addChild(TARLabel);

    auto dtlayer = DTLayer::get();

    auto TARToggler = SimpleToggler::createWithDefaults(
        .75f,
        dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().metadata.showAnyRun : false)
    );
    TARToggler->setCallback([&](bool isToggled){
        auto dtlayer = DTLayer::get();
        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr()) return;

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.metadata.showAnyRun = isToggled;
        auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);

        dtlayer->foreachLinkedLevel([&isToggled](auto& lvlData){
            lvlData.metadata.showAnyRun = isToggled;
            auto _ = StatsManager::setMetadata(lvlData.metadata, lvlData.levelKey);
        });

        DTLayer::get()->specialStrings["runs"]->updateContent();
        DTLayer::get()->specialStrings["sruns"]->updateContent();
    });
    float offset = TARToggler->getContentWidth() / 4 + 5;
    TARToggler->setPosition(TARLabel->getPosition() - ccp(TARLabel->getScaledContentWidth() / 2 + offset, 0));
    TARLabel->setPositionX(TARLabel->getPositionX() + offset);
    this->addChild(TARToggler);

    runAdditionInput = TextInput::create(size.width / 4.f, "Start %");
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

    runsScrollLayer = ScrollLayer::create(runsMenuBG->getContentSize() - ccp(10, 10));
    runsScrollLayer->setAnchorPoint({0, 0});
    runsScrollLayer->setPosition(runsMenuBG->getPosition());
    runsScrollLayer->setPosition(runsScrollLayer->getPosition() + ccp(5, 5));
    runsScrollLayer->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(runsScrollLayer->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    this->addChild(runsScrollLayer);

    if (dtlayer != nullptr && dtlayer->m_MyLevelStats.isOk()){
        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        for (const auto& [startPercent, maxToShow] : stats.metadata.RunsToShow)
        {
            createRunCell(startPercent, maxToShow);
        }
    }

    auto seperator = CCScale9Sprite::create("square.png");
    seperator->setScale(.5f);
    seperator->setPosition(size / 2);
    seperator->setContentSize(ccp(1, size.height / 1.15f) / seperator->getScale());
    this->addChild(seperator);

    float rightOffset = 5;

    HideByLenInput = TextInput::create(45, "Len");
    HideByLenInput->setPosition({size.width - HideByLenInput->getContentWidth() / 2 - rightOffset, size.height / 8 * 7});
    HideByLenInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().metadata.hideRunLength) : ""));
    HideByLenInput->setCommonFilter(CommonFilter::Uint);
    HideByLenInput->setCallback([&](const std::string& newText){
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);

        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        HideByLenInput->setString(std::to_string(num));

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.metadata.hideRunLength = num;
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
    HidUpToInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().metadata.hideUpto) : ""));
    HidUpToInput->setCommonFilter(CommonFilter::Uint);
    HidUpToInput->setCallback([&](const std::string& newText){
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);

        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        HidUpToInput->setString(std::to_string(num));

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.metadata.hideUpto = num;
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
    RealEndPerInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().metadata.realEndPercent) : ""));
    RealEndPerInput->setCommonFilter(CommonFilter::Uint);
    RealEndPerInput->setCallback([&](const std::string& newText){
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);

        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        RealEndPerInput->setString(std::to_string(num));

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.metadata.realEndPercent = num;
    });
    this->addChild(RealEndPerInput);

    auto RealEndPerLabel = CCLabelBMFont::create("Real End %", "gjFont17.fnt");
    RealEndPerLabel->setScale(.65f);
    RealEndPerLabel->setWidth(size.width / 2 - RealEndPerInput->getContentWidth() - rightOffset);
    RealEndPerLabel->setPosition(RealEndPerInput->getPosition() - ccp(RealEndPerInput->getContentWidth() / 2 + rightOffset, 0));
    RealEndPerLabel->setAnchorPoint({1, .5f});
    this->addChild(RealEndPerLabel);

    auto ResetAsDeathToggler = SimpleToggler::createWithDefaults(
        .75f,
        dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().metadata.resetAsDeath : false)
    );
    ResetAsDeathToggler->setCallback([&](bool isToggled){
        auto dtlayer = DTLayer::get();
        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr()) return;

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.metadata.resetAsDeath = isToggled;
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

    #if !defined(GEODE_IS_IOS)
    addEventListener<keybinds::InvokeBindFilter>([&](keybinds::InvokeBindEvent* event) {
        if (event->isDown() && runAdditionInput->getInputNode()->m_selected) {
            addNewRun(nullptr);
        }
        return ListenerResult::Propagate;
    }, "enter-new-run-per"_spr);
    #endif

    return true;
}

void RunOptions::onOpened(){
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));

    runAdditionInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(runAdditionInput, true, fadeTime);
    HideByLenInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(HideByLenInput, true, fadeTime);
    RealEndPerInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(RealEndPerInput, true, fadeTime);
    HidUpToInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(HidUpToInput, true, fadeTime);

    for (const auto& child : CCArrayExt<PercentCell*>(runsScrollLayer->m_contentLayer->getChildren())){
        child->show();
    }

    this->setEnabled(true);
}
void RunOptions::onClosed(){
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));

    Dev::fadeTextInput(runAdditionInput, false, fadeTime);
    Dev::fadeTextInput(HideByLenInput, false, fadeTime);
    Dev::fadeTextInput(RealEndPerInput, false, fadeTime);
    Dev::fadeTextInput(HidUpToInput, false, fadeTime);

    for (const auto& child : CCArrayExt<PercentCell*>(runsScrollLayer->m_contentLayer->getChildren())){
        child->hide();
    }

    this->setEnabled(false);
}

void RunOptions::addNewRun(CCObject*){
    auto dtlayer = DTLayer::get();
    //log::info("{}", dtlayer->specialStrings["totalLocalDeaths"]);
    auto numRes = geode::utils::numFromString<int>(runAdditionInput->getString());

    if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

    int num = numRes.unwrap();

    auto& stats = dtlayer->m_MyLevelStats.unwrap();
    if (stats.metadata.RunsToShow.contains(num)) return;
    
    stats.metadata.RunsToShow.insert({num, num});

    auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);

    DTLayer::get()->specialStrings["runs"]->updateContent();
    DTLayer::get()->specialStrings["sruns"]->updateContent();

    createRunCell(num, num);
}

void RunOptions::PercentCellClicked(PercentCell* cell){
    auto dtlayer = DTLayer::get();

    if (dtlayer != nullptr){
        int percent = cell->getPercent();

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        if (stats.metadata.RunsToShow.contains(percent)){
            stats.metadata.RunsToShow.erase(percent);
            auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);

            DTLayer::get()->specialStrings["runs"]->updateContent();
            DTLayer::get()->specialStrings["sruns"]->updateContent();
        }
    }

    cell->removeMeAndCleanup();
    runsScrollLayer->m_contentLayer->updateLayout();
}

void RunOptions::PercentMaxHideValChanged(PercentCell* cell){
    auto dtlayer = DTLayer::get();

    if (dtlayer != nullptr){
        int percent = cell->getPercent();

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        if (stats.metadata.RunsToShow.contains(percent)){
            stats.metadata.RunsToShow[percent] = cell->getMaxToHide();
            auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);

            DTLayer::get()->specialStrings["runs"]->updateContent();
            DTLayer::get()->specialStrings["sruns"]->updateContent();
        }
    }
}

void RunOptions::createRunCell(int percent, int maxToHide){
    auto percentCell = PercentCell::create(
        runsScrollLayer->m_contentLayer->getContentWidth(),
        percent,
        maxToHide,
        CCSprite::createWithSpriteFrameName("minus_button.png"_spr),
        [&](PercentCell* cell){ RunOptions::PercentCellClicked(cell); }
    );
    percentCell->onMaxToHideChanged = [&](PercentCell* cell){ RunOptions::PercentMaxHideValChanged(cell); };
    runsScrollLayer->m_contentLayer->addChild(percentCell);

    runsScrollLayer->m_contentLayer->updateLayout();
}