#pragma once

#include <Geode/Geode.hpp>
#include <unordered_map>
#include <functional>
#include <types/DTTypes.hpp>
#include <utils/SpecialKey.hpp>

class LayoutColumn;

using namespace geode::prelude;

class DTLabel : public CCMenu {
    public:
        static DTLabel* create(const DTLabelInfo& info);

        DTLabelInfo info;

        void moveUpLayer();

        CCPoint tempPos;
        float tempWidth;

        static float labelTitleHeight;
        static float moveThreshold;
        static float labelLerpSpeed;

        void removeFromColumns();

        void addColumnAsHolder(LayoutColumn* column);
        void removeColumnAsHolder(LayoutColumn* column);
        bool isPartOfColumn(LayoutColumn* column);
        bool isAlone();

        struct ColumnComperator {
            bool operator() (LayoutColumn* a, LayoutColumn* b) const;
        };

        std::multiset<LayoutColumn*, ColumnComperator> getHolders();

        void setLabelName(const std::string& newName);
        void setFontSize(float newSize);
        void setLabelColor(const ccColor4B& newColor);
        void setTextColor(const ccColor4B& newColor);
        void setTextAlignment(CCTextAlignment alignment);

        float textCornerOffset = 5;

        void setLabelText(const std::string& text);

        void modifyKeys();

        void setLoading(SpecialKey* key);
        void completeLoading(SpecialKey* key);

        std::set<std::string> keysUsed{};
        
    private:
        bool init(const DTLabelInfo& info);

        void update(float dt) override;

        std::set<SpecialKey*> currentlyLoadingFor{};
        
        std::multiset<LayoutColumn*, ColumnComperator> holders{};

        CCScale9Sprite* bg;
        CCScale9Sprite* labelTitleBG;
        CCMenu* menu;
        CCMenuItemSpriteExtra* expandBtn;
        SimpleTextArea* labelTitleArea;

        CCScale9Sprite* leftExpandLine;
        CCScale9Sprite* rightExpandLine;

        CCNode* labelTextContainer;
        SimpleTextArea* labelText;
        float currentHeight;

        float normalBGOpacity = 150;

        void toggleExpand(CCObject*);

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

        void updateInfoWithColumnData();
};