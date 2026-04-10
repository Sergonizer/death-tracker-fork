#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

class $modify(DTEndLevelLayer, EndLevelLayer) {
    
    void customSetup();

    void openDTLayer(CCObject*);
};