#include "TutorialButton.hpp"

TutorialButton* TutorialButton::create(float size, geode::Function<void(DTTutorialLayer*)> initilizeTutorial) {
    auto ret = new TutorialButton();
    if (ret && ret->init(size, std::move(initilizeTutorial))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool TutorialButton::init(float size, geode::Function<void(DTTutorialLayer*)> initilizeTutorial){
    auto spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    spr->setScale(size);

    if (!CCMenuItemSpriteExtra::init(spr, nullptr, this, menu_selector(TutorialButton::onClicked))) return false;

    this->initilizeTutorial = std::move(initilizeTutorial);

    return true;
}

void TutorialButton::onClicked(CCObject*){
    auto layer = DTTutorialLayer::create();

    initilizeTutorial(layer);

    layer->show();
}
