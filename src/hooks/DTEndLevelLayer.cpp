#include <hooks/DTEndLevelLayer.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <utils/Settings.hpp>

void DTEndLevelLayer::customSetup(){
    EndLevelLayer::customSetup();

    if (!Settings::getCompleteMenuEnabled()) return;

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    auto btn = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTEndLevelLayer::openDTLayer)
    );
    btn->setID("dt-skull-button");

    auto hideLMenu = this->getChildByID("hide-layer-menu");
    hideLMenu->addChild(btn);

    hideLMenu->updateLayout();
}
void DTEndLevelLayer::openDTLayer(CCObject*){
    auto dtLayer = DTLayer::create(this->m_playLayer->m_level);
    dtLayer->show();
}