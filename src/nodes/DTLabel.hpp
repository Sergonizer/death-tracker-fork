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

        struct ColumnComperator {
            bool operator() (LayoutColumn* a, LayoutColumn* b) const;
        };
        std::multiset<LayoutColumn*, ColumnComperator> holders{};

        CCPoint tempPos;
        float tempWidth;

        static float labelTitleHeight;
        static float moveThreshold;
        static float labelLerpSpeed;

        void removeFromColumns();
        
    private:
        bool init() override;

        void update(float dt) override;

        CCScale9Sprite* bg;
        CCScale9Sprite* labelTitleBG;
        CCMenu* menu;
        CCMenuItemSpriteExtra* expandBtn;
        SimpleTextArea* labelTitleArea;

        CCScale9Sprite* leftExpandLine;
        CCScale9Sprite* rightExpandLine;

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

        CCPoint currentTouchPosition;

        std::multiset<LayoutColumn*, ColumnComperator> holdersSave{};

        void onSettings();
        void onMoveBegan();
        void onMoveUpdate(float dt);
        void onMoveEnded();

        bool currentlyExpandingLeft;
        bool currentlyExpandingRight;
};