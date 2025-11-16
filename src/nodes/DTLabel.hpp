#pragma once

#include <Geode/Geode.hpp>

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
        
    private:
        bool init();

        void update(float dt);

        CCScale9Sprite* bg;

        void toggleExpand(CCObject*);
};