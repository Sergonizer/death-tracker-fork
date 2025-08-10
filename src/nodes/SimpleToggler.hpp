#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SimpleToggler : public CCMenuItemToggler {
    public:
        static SimpleToggler* create(CCNode* offSprite, CCNode* onSprite, float scale = 1, bool startState = false);

        void setCallback(const std::function<void(bool)>& callback);
    private:
        bool init(CCNode* offSprite, CCNode* onSprite, float scale, bool startState);

        void onToggled(CCObject* sender);

        std::function<void(bool)> callback = NULL;
};