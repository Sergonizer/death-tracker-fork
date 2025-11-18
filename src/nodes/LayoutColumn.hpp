#pragma once

#include <Geode/Geode.hpp>
#include <nodes/DTLabel.hpp>

using namespace geode::prelude;

class LayoutColumn : public CCMenu {
    public:
        static LayoutColumn* create(float topHeight, float minHeight);

        void setColor(ccColor3B color);

        void addLabel(DTLabel* label);
        void removeLabel(DTLabel* label);

        void updateLabelPositions();

        std::map<int, DTLabel*> labels{};

        int orderPos;

        void updateLabelPosition(DTLabel* label);

        static float borderWidth;

        void setContentHeight(float height);

        float tempHeight;

        static float minWidth;
        
    private:
        bool init(float topHeight, float minHeight);

        float minHeight;
        
        float topHeight;

        float bgOpacity = 125;

        CCScale9Sprite* topSpr;
        CCScale9Sprite* bgSpr;

        CCScale9Sprite* topBorder1;
        CCScale9Sprite* topBorder2;
        CCScale9Sprite* topBorder3;
        CCScale9Sprite* bgSideBorder;

        void updateSizesByContent();

        void registerWithTouchDispatcher() override;
        bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
        void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

        bool isTouchingSide;
};