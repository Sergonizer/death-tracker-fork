#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;


class PercentCell : public CCMenu {
    protected:
        bool init(float width, int Percent, CCNode* sideButtonSprite, const std::function<void(PercentCell*)>& callback);
    public:
        static PercentCell* create(float width, int Percent, CCNode* sideButtonSprite, const std::function<void(PercentCell*)>& callback = NULL);
        
        int getPercent();

        CCScale9Sprite* BGSprite;

    private:
        void RunCallback(CCObject*);

        int m_Percent;
        std::function<void(PercentCell*)> m_Callback;
};