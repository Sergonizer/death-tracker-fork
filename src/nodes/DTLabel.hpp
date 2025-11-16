#pragma once

#include <Geode/Geode.hpp>
#include <unordered_map>
#include <functional>

class LayoutColumn;

using namespace geode::prelude;

class DTLabel : public CCMenu {
    public:
        static DTLabel* create();

        int layer = 0;

        void moveUpLayer();

        std::string name;

        std::vector<LayoutColumn*> holders{};

        CCPoint tempPos;
        float tempWidth;

        static float labelTitleHeight;
        
    private:
        bool init();

        void update(float dt);

        CCScale9Sprite* bg;

        void toggleExpand(CCObject*);

        bool isExpanded;

        std::unordered_map<CCNode*, std::function<float()>> followContentWidth{};
        std::unordered_map<CCNode*, std::function<float()>> followContentHeight{};

        std::unordered_map<CCNode*, std::function<CCPoint()>> alwaysSetPosition{};
};