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

        CCPoint tempPos = CCPoint{0, 0};
        float tempWidth = 0;

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

        std::set<LayoutColumn*, ColumnComperator> getHolders();

        void setLabelName(const std::string& newName);
        void setFontSize(float newSize);
        void setFont(const std::string& fnt);
        void setLabelColor(const ccColor4B& newColor);
        void setTextColor(const ccColor4B& newColor);
        void setTextAlignment(CCTextAlignment alignment);
        void setTextWrapping(WrappingMode wrapMode);

        float textCornerOffset = 5;

        void setLabelText(const std::string& text);

        arc::Future<std::optional<std::string>> modifyKeys();

        void setLoading(const std::shared_ptr<SpecialKey>& key);
        void completeLoading(const std::shared_ptr<SpecialKey>& key);
        
        std::set<std::string> keysUsed{};

        void toggleExpand(CCObject*);

        void onBeingEdited();
        void onBeingEditedEnded();

        void setEditable(bool editable);

        void setExpandable(bool enabled);

        void fadeTextColorTo(ccColor4B newColor, float time);
        void fadeTitleColorTo(ccColor4B newColor, float time);

        ~DTLabel();
    private:
        bool init(const DTLabelInfo& info);

        void update(float dt) override;

        async::TaskHolder<std::optional<std::string>> modifyListener;

        bool isEditable;

        bool isExpandable = true;

        std::set<std::shared_ptr<SpecialKey>> currentlyLoadingFor{};
        
        std::set<LayoutColumn*, ColumnComperator> holders{};

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

        void registerWithTouchDispatcher() override;
        bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
        void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

        bool isBeingTouched;
        CCPoint touchStartPoint;
        bool isMovingLabel;

        CCPoint currentTouchPosition;

        std::set<LayoutColumn*, ColumnComperator> holdersSave{};

        void onSettings();
        void onMoveBegan();
        void onMoveUpdate(float dt);
        void onMoveEnded();

        bool currentlyExpandingLeft;
        bool currentlyExpandingRight;

        void updateInfoWithColumnData();

        LoadingCircle* loadingCircle;

        CCSprite* glow;

        arc::Future<std::string> modifyStrRecursive(const std::string& str);

        bool isBeingEdited = false;
        bool isHovering;
};