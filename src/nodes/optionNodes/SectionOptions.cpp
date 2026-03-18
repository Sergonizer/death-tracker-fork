#include <nodes/optionNodes/SectionOptions.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/SectionCell.hpp>

SectionOptions* SectionOptions::create(const CCSize& size) {
    auto ret = new SectionOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SectionOptions::setup(){

    auto dtlayer = DTLayer::get();

    mainScroll = ScrollLayer::create(size / 1.2f);
    mainScroll->ignoreAnchorPointForPosition(false);
    mainScroll->setPosition({size / 2 - ccp(0, 20)});
    
    mainScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(mainScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    this->addChild(mainScroll);

    auto sectionsLabel = CCLabelBMFont::create("Sections", "bigFont.fnt");
    sectionsLabel->setPosition(mainScroll->getPosition() + ccp(0, mainScroll->getContentSize().height / 2 + sectionsLabel->getScaledContentHeight() / 2 + 5));
    sectionsLabel->setScale(.75f);
    this->addChild(sectionsLabel);

    if (dtlayer->m_MyLevelStats.isErr()){

    }
    else{
        auto myStats = dtlayer->m_MyLevelStats.unwrap();
        for (const auto& section : myStats.metadata.sections)
        {
            addSection(section);
        }
    }

    auto addSectionBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    //addSectionBtnSpr->setScale(.75f);
    auto addSectionBtn = CCMenuItemSpriteExtra::create(
        addSectionBtnSpr,
        this,
        menu_selector(SectionOptions::onAddSection)
    );
    addSectionBtn->setPosition(mainScroll->getPosition() + ccp(mainScroll->getScaledContentWidth() / 2, mainScroll->getScaledContentHeight() / 2));
    this->addChild(addSectionBtn);

    for (const auto& cell : mainScroll->m_contentLayer->getChildrenExt<SectionCell*>())
    {
        cell->fade(false, 0);
    }
    
    this->setOpacity(0);
    this->setEnabled(false);

    mainScroll->setMouseEnabled(false);

    mainScroll->m_contentLayer->updateLayout();

    return true;
}

void SectionOptions::onOpened(){
    float fadeTime = .2f;
    this->setEnabled(true);
    this->runAction(CCFadeTo::create(fadeTime, 255));
    mainScroll->moveToTop();
    mainScroll->setMouseEnabled(true);

    for (const auto& cell : mainScroll->m_contentLayer->getChildrenExt<SectionCell*>())
    {
        cell->fade(true, fadeTime);
    }
}
void SectionOptions::onClosed(){
    float fadeTime = .2f;
    this->setEnabled(false);
    this->runAction(CCFadeTo::create(fadeTime, 0));

    mainScroll->setMouseEnabled(false);

    for (const auto& cell : mainScroll->m_contentLayer->getChildrenExt<SectionCell*>())
    {
        cell->fade(false, fadeTime);
    }
}

void SectionOptions::saveSections(){
    auto dtlayer = DTLayer::get();
    if (dtlayer->m_MyLevelStats.isErr()) return;

    auto& myStats = dtlayer->m_MyLevelStats.unwrap();

    std::vector<Section> newSections;
    for (const auto& cell : mainScroll->m_contentLayer->getChildrenExt<SectionCell*>()) {
        newSections.push_back(cell->getSection());
    }

    myStats.metadata.sections = newSections;
    (void)StatsManager::setMetadata(myStats.metadata, myStats.levelKey);

    dtlayer->foreachLinkedLevel([&](auto& lvlData){
        lvlData.metadata.sections = newSections;
        (void)StatsManager::setMetadata(lvlData.metadata, lvlData.levelKey);
    });

    DTLayer::get()->specialStrings["section"]->updateContent();
}

void SectionOptions::onAddSection(CCObject* sender){
    addSection(Section{});
    mainScroll->m_contentLayer->updateLayout();
    saveSections();
}

void SectionOptions::addSection(const Section& section){
    auto cell = SectionCell::create(mainScroll->getContentWidth(), section);
    cell->onDeleted = [&](){
        mainScroll->m_contentLayer->updateLayout();
        this->saveSections();
    };
    cell->onChanged = [&](){
        this->saveSections();
    };
    mainScroll->m_contentLayer->addChild(cell);
    mainScroll->m_contentLayer->updateLayout();
    mainScroll->m_contentLayer->setPositionY(0);
}