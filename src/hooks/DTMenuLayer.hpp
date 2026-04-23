#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(DTMenuLayer, MenuLayer) {
    bool init();

    arc::Future<> awaitOpening();
};