#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;


class PercentCell : public CCMenu, public SliderDelegate {
    protected:
        bool init(float width, int Percent, int maxToHide, CCNode* sideButtonSprite, const std::function<void(PercentCell*)>& callback);
    public:
        static PercentCell* create(float width, int Percent, int maxToHide, CCNode* sideButtonSprite, const std::function<void(PercentCell*)>& callback = NULL);
        
        int getPercent() const {
            return percent;
        }
        int getMaxToHide() const {
            return maxToHide;
        }

        CCScale9Sprite* BGSprite;
        std::function<void(PercentCell*)> onMaxToHideChanged = NULL;

        void hide();
        void show();
        
    private:
        void RunCallback(CCObject*);
        void mthScroll(CCObject*);

        Slider* MTHScroll;
        TextInput* MTHInput;

        int percent;
        int maxToHide;
        std::function<void(PercentCell*)> callback;

        void sliderEnded(Slider* slider);
};