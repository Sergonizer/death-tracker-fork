#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class LevelLinkCell : public CCMenu {
    public:
        static LevelLinkCell* create(float width, const std::string& levelKey, const LevelMetadeta& data);

        void setAlignment(bool left);

        void setCallback(const std::function<void(const std::string& levelKey, bool isLeftAligned)>& callback);

    private:
        bool init(float width, const std::string& levelKey, const LevelMetadeta& data);

        std::string levelKey;

        std::function<void(const std::string& levelKey, bool isLeftAligned)> callback = NULL;

        bool isAlignedLeft;

        void onArrow(CCObject*);

        CCMenuItemSpriteExtra* arrowBtn;
        GJDifficultySprite* diffSpr;
        CCLabelBMFont* title;
        float titleWidth;
        CCLabelBMFont* att;
        float attWidth;
};