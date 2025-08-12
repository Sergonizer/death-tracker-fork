#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;


using textUpdateTask = Task<bool, int>;

class DTLabel : public CCNode {
    public:
        static DTLabel* create(const DTLabelInfo& info, float gridSize);
        
        DTLabelInfo labelInfo;
        bool wasCreatedThisEdit;
        float gridSize;
        
        void enterEditMode();
        void exitEditMode();
        
        void updateState();
        void updateTransform();
        void updateText();
        
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

        EventListener<textUpdateTask> textUpdateListener;
        textUpdateTask runningTask;
        LoadingCircle* loadingCircle;

        std::set<std::string> usedKeys{};
        std::map<int, std::optional<ccColor3B>> colorData{};
        std::optional<ccColor3B> currentColor = std::nullopt;
};

/*
    TODO:

    add font select menu

    add text editing (somehow) [maybe have a text btn that changes the entire bottom bar to be a giant text field :O thats a good idea]

    add actual level data to move to labels
*/