#pragma once

#include <Geode/Geode.hpp>
#include <nodes/DTLabel.hpp>

using namespace geode::prelude;

class LayoutColumn : public CCMenu {
    public:
        static LayoutColumn* create(const DTColumnInfo& info, float topHeight, float minHeight);

        DTColumnInfo info;

        void setColor(ccColor3B color);

        void addLabel(DTLabel* label);
        void removeLabel(DTLabel* label);

        std::map<int, DTLabel*> labels{};

        void updateLabelPosition(DTLabel* label);

        static float borderWidth;

        void setContentHeight(float height);
        void setContentWidth(float width);

        float tempHeight;

        static float addNewBtnOffset;

        void destroyColumnAndCleanup();

        void refreshAllLabelsLayer();
        CCScale9Sprite* bgSpr;

        float topHeight;
        
    private:
        bool init(const DTColumnInfo& info, float topHeight, float minHeight);

        float minHeight;

        float bgOpacity = 125;

        CCScale9Sprite* topSpr;
        

        CCScale9Sprite* topBorder1;
        CCScale9Sprite* topBorder2;
        CCScale9Sprite* topBorder3;
        CCScale9Sprite* bgSideBorder;

        CCMenu* menu;
        CCPoint tempMenuPos;

        void updateSizesByContent();

        void registerWithTouchDispatcher() override;
        bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
        void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

        bool isTouchingSide;
        bool isTouchingTopSpr;
        CCPoint originalTopSprTouchPos;
        float maxMovementOffset = 3;

        void onAddLabelBtnClicked(CCObject*);

        void update(float dt) override;

        void onOrganized(float deltaMove);
};