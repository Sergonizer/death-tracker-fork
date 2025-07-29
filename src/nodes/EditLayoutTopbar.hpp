#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/DTLabel.hpp>

using namespace cocos2d;
using namespace geode;

class EditLayoutTopbar : public CCNode, public geode::ColorPickPopupDelegate {
    public:
        static EditLayoutTopbar* create();

        std::function<void(bool)> onExit = NULL;

        void setTarget(DTLabel* target);

        void keyBackClicked();

    private:
        bool init();

        DTLabel* targetLabel = nullptr;

        void onApplyClicked(CCObject*);
        void onExitClicked(CCObject*);
        void onNewLabelClicked(CCObject*);

        void onFontSelClicked(CCObject*);
        void onHAlignmentClicked(CCObject*);
        void onVAlignmentClicked(CCObject*);
        void onColorClicked(CCObject*);
        void onInfGrowClicked(CCObject*);
        void onDeselectClicked(CCObject*);
        void onDeleteClicked(CCObject*);
        void onTextClicked(CCObject*);

        void onConfirmTextClicked(CCObject*);
        void onSpecialKeysClicked(CCObject*);

        void updateColor(cocos2d::ccColor4B const& color);

        ButtonSprite* HAlignmentBtnSpr;
        ButtonSprite* VAlignmentBtnSpr;
        CCSprite* colorBtnSpr;
        TextInput* scaleInput;
        TextInput* contentWidthInput;
        TextInput* contentHeightInput;
        CCMenuItemToggler* infGrowBtn;

        CCNode* createSpacingNode(float spacing);

        CCMenu* idleMenu;
        CCMenu* targetMenu;
        CCMenu* textMenu;
};