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
        static float moveThreshold;
        
    private:
        bool init() override;

        void update(float dt) override;

        CCScale9Sprite* bg;
        CCScale9Sprite* labelTitleBG;
        CCMenu* menu;
        CCMenuItemSpriteExtra* expandBtn;
        SimpleTextArea* labelTitleArea;

        void toggleExpand(CCObject*);

        bool isExpanded;

        void registerWithTouchDispatcher() override;
        bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
        void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

        bool isBeingTouched;
        CCPoint touchStartPoint;
        bool isMovingLabel;

        void onSettings();
        void onMoveBegan();
        void onMoveEnded();
};