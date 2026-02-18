#include <nodes/optionNodes/RunOptions.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <utils/Dev.hpp>
#if !defined(GEODE_IS_IOS)
// #include <geode.custom-keybinds/include/Keybinds.hpp>
#endif
#include <nodes/TutorialButton.hpp>

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

    HidUpToInput = TextInput::create(45, "%");
    HidUpToInput->setPosition({size.width - HidUpToInput->getContentWidth() / 2 - rightOffset, size.height / 8 * 6});
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
        auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);

        dtlayer->specialStrings["general"]->updateContent();
    });
    this->addChild(HidUpToInput);

    auto HideUpToLabel = CCLabelBMFont::create("Hide Up To", "gjFont17.fnt");
    HideUpToLabel->setScale(.65f);
    HideUpToLabel->setWidth(size.width / 2 - HidUpToInput->getContentWidth() - rightOffset);
    HideUpToLabel->setPosition(HidUpToInput->getPosition() - ccp(HidUpToInput->getContentWidth() / 2 + rightOffset, 0));
    HideUpToLabel->setAnchorPoint({1, .5f});
    this->addChild(HideUpToLabel);

    // RealEndPerInput = TextInput::create(45, "E %");
    // RealEndPerInput->setPosition({size.width - RealEndPerInput->getContentWidth() / 2 - rightOffset, size.height / 8 * 3});
    // RealEndPerInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().metadata.realEndPercent) : ""));
    // RealEndPerInput->setCommonFilter(CommonFilter::Uint);
    // RealEndPerInput->setCallback([&](const std::string& newText){
    //     auto dtlayer = DTLayer::get();
    //     auto numRes = geode::utils::numFromString<int>(newText);

    //     if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

    //     int num = numRes.unwrap();
    //     num = std::min(num, 100);
    //     RealEndPerInput->setString(std::to_string(num));

    //     auto& stats = dtlayer->m_MyLevelStats.unwrap();
    //     stats.metadata.realEndPercent = num;
    // });
    // this->addChild(RealEndPerInput);

    // auto RealEndPerLabel = CCLabelBMFont::create("Real End %", "gjFont17.fnt");
    // RealEndPerLabel->setScale(.65f);
    // RealEndPerLabel->setWidth(size.width / 2 - RealEndPerInput->getContentWidth() - rightOffset);
    // RealEndPerLabel->setPosition(RealEndPerInput->getPosition() - ccp(RealEndPerInput->getContentWidth() / 2 + rightOffset, 0));
    // RealEndPerLabel->setAnchorPoint({1, .5f});
    // this->addChild(RealEndPerLabel);

    auto ResetAsDeathToggler = SimpleToggler::createWithDefaults(
        .75f,
        dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().metadata.resetAsDeath : false)
    );
    ResetAsDeathToggler->setCallback([&](bool isToggled){
        auto dtlayer = DTLayer::get();
        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr()) return;

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.metadata.resetAsDeath = isToggled;
        auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);
    });
    ResetAsDeathToggler->setPosition({size.width - ResetAsDeathToggler->getContentWidth() / 2 - rightOffset, size.height / 8 * 2});
    this->addChild(ResetAsDeathToggler);

    auto ResetAsDeathLabel = CCLabelBMFont::create("Reset as Death", "gjFont17.fnt");
    ResetAsDeathLabel->setScale(.65f);
    ResetAsDeathLabel->setWidth(size.width / 2 - ResetAsDeathToggler->getContentWidth() - rightOffset);
    ResetAsDeathLabel->setPosition(ResetAsDeathToggler->getPosition() - ccp(ResetAsDeathToggler->getContentWidth() / 2 + rightOffset, 0));
    ResetAsDeathLabel->setAnchorPoint({1, .5f});
    this->addChild(ResetAsDeathLabel);

    auto runsHidingTutorial = TutorialButton::create(.75f, [&, TARToggler, TARLabel, plusBtn](DTTutorialLayer* tutorial){
        int randomPer = 7 + CCRANDOM_0_1() * (95 - 7);
        int randomMaxToHide = (randomPer + 2) + CCRANDOM_0_1() * (93 - (randomPer + 2));

        auto emptyPer = PercentCell::create(
            runsScrollLayer->m_contentLayer->getContentWidth(), 
            randomPer, 
            randomMaxToHide, 
            CCSprite::createWithSpriteFrameName("minus_button.png"_spr),
            [](PercentCell* _){}
        );
        emptyPer->setPositionX(this->getContentWidth() / 2);
        emptyPer->setAnchorPoint({.5f, 0});
        emptyPer->hideInstant();
        emptyPer->setScale(1.5f);
        this->addChild(emptyPer);

        tutorial->appendDialogue("In here you can manage which runs you want to see! to clear up clutter!", TutorialCharacterFace::TCFNormal)
            ->appendDialogue("First you have \"Show any run\", this will tell death tracker to display all the runs you have done", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)
            ->joinHighlight(TARToggler)
            ->joinHighlight(TARLabel)
            ->appendDialogue("if this is enabled, the runs you pick below will not effect anything.", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(TARToggler)
            ->joinHighlight(TARLabel)
            ->appendDialogue("Now for addding runs you have this text input and button", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottomRight, .75f)
            ->joinHighlight(runAdditionInput)
            ->joinHighlight(plusBtn)
            ->appendDialogue("You write the percentage you wanna see into the text input", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(runAdditionInput)
            ->appendDialogue("And click the plus button to confirm and add that percent to your list of showen percentages!", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(plusBtn)
            ->appendDialogue("If you have \"Show any run\" disabled, you will only see the runs that start from any percentage on this list!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPTopRight, .75f)
            ->joinHighlight(runsScrollLayer)
            ->appendDialogue("In this list you have a few options per percent", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(runsScrollLayer)
            ->appendDialogue("You can remove the percent from the list", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPTop, .75f)
            ->joinHighlight(emptyPer->getChildByID("btn"))
            ->joinCallback([emptyPer](){emptyPer->show();}, true)
            ->appendDialogue("And you can edit the \"Max to Hide\" percent.", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(emptyPer->getSilderBG())
            ->joinHighlight(emptyPer->getSilderHandle())
            ->joinHighlight(emptyPer->getChildByID("mth-label"))
            ->joinHighlight(emptyPer->getChildByID("mth-input"))
            ->appendDialogue("The number you input here will hide any run thats between that number and the original percentage.", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(emptyPer->getSilderBG())
            ->joinHighlight(emptyPer->getSilderHandle())
            ->joinHighlight(emptyPer->getChildByID("mth-label"))
            ->joinHighlight(emptyPer->getChildByID("mth-input"))
            ->appendDialogue(fmt::format("For example, in this case it will hide all runs starting from {} and ending below {}", randomPer, randomMaxToHide), TutorialCharacterFace::TCFNormal)
            ->joinHighlight(emptyPer->getChildByID("percent-label"))
            ->joinHighlight(emptyPer->getChildByID("mth-input"), 1.5f)
            ->appendDialogue(fmt::format("So {}-{} wont be seen but {}-{} will be!", randomPer, randomMaxToHide - 2, randomPer, randomMaxToHide), TutorialCharacterFace::TCFNormal)
            ->joinHighlight(emptyPer->getChildByID("percent-label"))
            ->joinHighlight(emptyPer->getChildByID("mth-input"))
            ->appendDialogue("Hope this feature helps you improve clutter in your death tracker! :D", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1)
            ->joinCallback([emptyPer](){emptyPer->hide();}, true)
            ->joinCallback([emptyPer](){emptyPer->removeMeAndCleanup();}, false);
    });
    runsHidingTutorial->setPosition(TARLabel->getPosition() + ccp(15 + TARLabel->getScaledContentWidth() / 2, 0));
    this->addChild(runsHidingTutorial);

    this->setOpacity(0);

    #if !defined(GEODE_IS_IOS)
    // addEventListener<keybinds::InvokeBindFilter>([&](keybinds::InvokeBindEvent* event) {
    //     if (event->isDown() && runAdditionInput->getInputNode()->m_selected) {
    //         addNewRun(nullptr);
    //     }
    //     return ListenerResult::Propagate;
    // }, "enter-new-run-per"_spr);
    #endif

    return true;
}

void RunOptions::onOpened(){
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));

    runAdditionInput->getInputNode()->m_textLabel->setOpacity(0);
    Dev::fadeTextInput(runAdditionInput, true, fadeTime);
    // RealEndPerInput->getInputNode()->m_textLabel->setOpacity(0);
    // Dev::fadeTextInput(RealEndPerInput, true, fadeTime);
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
    // Dev::fadeTextInput(RealEndPerInput, false, fadeTime);
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

    dtlayer->foreachLinkedLevel([&](auto& lvlData){
        if (lvlData.metadata.RunsToShow.contains(num)) return;

        lvlData.metadata.RunsToShow.insert({num, num});
        auto _ = StatsManager::setMetadata(lvlData.metadata, lvlData.levelKey);
    });

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

        dtlayer->foreachLinkedLevel([&](auto& lvlData){
            if (!lvlData.metadata.RunsToShow.contains(percent)) return;

            lvlData.metadata.RunsToShow.erase(percent);
            auto _ = StatsManager::setMetadata(lvlData.metadata, lvlData.levelKey);
        });
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

        dtlayer->foreachLinkedLevel([&](auto& lvlData){
            lvlData.metadata.RunsToShow[percent] = cell->getMaxToHide();
            auto _ = StatsManager::setMetadata(lvlData.metadata, lvlData.levelKey);
        });
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