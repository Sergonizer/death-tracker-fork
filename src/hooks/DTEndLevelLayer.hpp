#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

class $modify(DTEndLevelLayer, EndLevelLayer) {
    struct Fields{
        CCMenuItemSpriteExtra* skullBtn = nullptr;
    };
    
    void customSetup();

    void openDTLayer(CCObject*);
    void onHideLayer(cocos2d::CCObject* sender);
};