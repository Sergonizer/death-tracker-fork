#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace cocos2d;
using namespace geode;

class DTLabel : public CCNode {
    public:
        static DTLabel* create(const DTLabelInfo& info, float gridSize);
        
        DTLabelInfo labelInfo;
        bool wasCreatedThisEdit;
        float gridSize;
        
        void enterEditMode();
        void exitEditMode();
        
        void updateState();
        void updatePosition();
        void updateAlignment();
        void updateContentSize();
        void updateText();
        void updateScale();
        
        std::function<void(DTLabel*)> onClicked = NULL;
        
        CCPoint localToGridPosition(CCPoint localPosition);
        CCPoint gridToLocalPosition(int x, int y);
        
        bool touchMoved(CCTouch* touch);
        void touchEnded(CCTouch* touch);
        
        void onDeselected();
        void onSelected(CCObject*);

        void softDelete();

        void revert();
        
    private:
        DTLabelInfo labelInfoBackup;
        bool init(const DTLabelInfo& info, float gridSize);

        std::string modifyText(const std::string& str);

        SimpleTextArea* textArea;

        CCNode* editVisualsContainer;
        CCScale9Sprite* contentOutline;
        CCScale9Sprite* selectedShadow;

        CCMenuItem* clickHitbox;

        bool isTouching = false;
        CCPoint touchStartGridPoint;
        bool isTouchlocked = false;

        bool isSelected = false;

        bool markedForDeletion = false;
        CCMenu* menu;
};

/*
    TODO:

    add font select menu

    add text editing (somehow) [maybe have a text btn that changes the entire bottom bar to be a giant text field :O thats a good idea]

    add actual level data to move to labels
*/