#include "ImportCompletePopup.hpp"
#include <nodes/layers/DTLayer.hpp>

ImportCompletePopup* ImportCompletePopup::create(LevelData&& data, std::vector<Session> const& sessions) {
    auto ret = new ImportCompletePopup();
    if (ret->init(std::move(data), std::move(sessions))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ImportCompletePopup::init(LevelData&& _data, std::vector<Session> const& _sessions) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (!Popup::init(250, 250, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Import Complete");

    this->data = std::move(_data);
    this->sessions = std::move(_sessions);

    generalView = ScrollLayer::create({m_size.width / 2.25f, 150});
    generalView->ignoreAnchorPointForPosition(false);
    generalView->setAnchorPoint({0, 1});
    generalView->setZOrder(1);
    generalView->setPosition({
        5,
        m_size.height - (m_size.height - m_title->getPositionY()) - m_title->getScaledContentHeight()
    });
    generalView->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(generalView->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    m_mainLayer->addChild(generalView);

    auto generalViewBG = CCScale9Sprite::create("square02_small.png");
    generalViewBG->setAnchorPoint(generalView->getAnchorPoint());
    generalViewBG->setPosition(generalView->getPosition() - ccp(5, -5) / 2);
    generalViewBG->setContentSize(generalView->getContentSize() + ccp(5, 5));
    generalViewBG->setOpacity(120);
    m_mainLayer->addChild(generalViewBG);

    auto sessionView = ScrollLayer::create({m_size.width / 2.25f, 150});
    sessionView->ignoreAnchorPointForPosition(false);
    sessionView->setAnchorPoint({1, 1});
    sessionView->setZOrder(1);
    sessionView->setPosition({
        m_size.width - 5,
        m_size.height - (m_size.height - m_title->getPositionY()) - m_title->getScaledContentHeight()
    });
    m_mainLayer->addChild(sessionView);

    auto sessionViewBG = CCScale9Sprite::create("square02_small.png");
    sessionViewBG->setAnchorPoint(sessionView->getAnchorPoint());
    sessionViewBG->setPosition(sessionView->getPosition() - ccp(-5, -5) / 2);
    sessionViewBG->setContentSize(sessionView->getContentSize() + ccp(5, 5));
    sessionViewBG->setOpacity(120);
    m_mainLayer->addChild(sessionViewBG);

    DTLabelInfo info;
    info.labelName = "General Data";
    info.scale = .3f;
    info.wrapping = WrappingMode::SPACE_WRAP;
    info.isExpanded = true;

    if (data.from0.isErr()){
        info.text = "No found general data!";
    }
    else{
        auto& f0 = data.from0.unwrap();

        std::string str = "From 0:{nl}";

        std::string f0Str;

        DTLayer::get()->createDeathsString(
            f0.deaths,
            data.metadata,
            Save::getFrom0Customazations(),
            f0Str,
            f0.newBests,
            Save::getNewBestColor(),
            true
        );

        str += f0Str;

        std::string runsStr;

        DTLayer::get()->createDeathsString(
            f0.runs,
            data.metadata,
            Save::getRunsCustomazations(),
            runsStr
        );

        str += "{nl} {nl}Runs:{nl}";

        str += runsStr;

        info.text = str;
    }

    auto label = DTLabel::create(info);
    label->setExpandable(false);
    label->setContentWidth(generalView->getContentWidth());
    generalView->m_contentLayer->addChild(label);

    scheduleUpdate();

    return true;
}

void ImportCompletePopup::update(float dt){
    float prevHeight = generalView->m_contentLayer->getContentHeight();
    generalView->m_contentLayer->updateLayout();
    if (generalView->m_contentLayer->getContentHeight() != prevHeight)
        generalView->moveToTop();
}