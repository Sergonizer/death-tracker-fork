#pragma once

#include <Geode/Geode.hpp>

using namespace cocos2d;
using namespace geode;

class SessionSelector : public CCMenu {
    public:
        static SessionSelector* create(int count);

        void setCallback(const std::function<void(int)>& callback);

        void setMaximumCount(int count);
        void setCurrentCount(int count);
        
    private:
        bool init(int count);

        void leftArrowClicked(CCObject*);
        void rightArrowClicked(CCObject*);

        std::function<void(int)> callback = NULL;

        int maxCount = 0;
        int currentCount = 0;
};