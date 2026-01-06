#pragma once

#include <Geode/Geode.hpp>
#include <nodes/layers/DTTutorialLayer.hpp>

using namespace geode::prelude;

class TutorialButton : public CCMenuItemSpriteExtra {
    protected:
        bool init(float size, const std::function<void(DTTutorialLayer*)>& initilizeTutorial);
        
    public:
        static TutorialButton* create(float size, const std::function<void(DTTutorialLayer*)>& initilizeTutorial);

    private:

        std::function<void(DTTutorialLayer*)> initilizeTutorial = NULL;

        void onClicked(CCObject*);
};