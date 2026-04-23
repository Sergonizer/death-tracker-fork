#include <Geode/modify/EditLevelLayer.hpp>

using namespace geode::prelude;

class $modify(DTEditLevelLayer, EditLevelLayer) {
    bool init(GJGameLevel* level);

    void openDTLayer(CCObject*);
};
