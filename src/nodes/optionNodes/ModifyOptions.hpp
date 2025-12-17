#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>
#include <nodes/SimpleToggler.hpp>
#include <nodes/DTLabel.hpp>
#include <nodes/SessionSelector.hpp>

using namespace geode::prelude;


class ModifyOptions : public OptionsNode {
    public:
        static ModifyOptions* create(const CCSize& size);
    private:
        virtual bool setup();

        SimpleToggler* TypeToggler;

        void onLevelView(CCObject*);
        void onRunsView(CCObject*);
        CCMenuItemSpriteExtra* from0ViewBtn;
        CCMenuItemSpriteExtra* runViewBtn;

        DTLabel* myLabel;

        void update(float dt);
        ScrollLayer* previewScroll;
        void updatePreviewName(bool categotyIsSession);

        void addPlusMinusBtns(CCNode* around, bool flip, const std::function<void(bool isPlus)>& callback, float scale);
        void onPlusMinusBtn(CCObject* sender);
        std::map<CCMenuItemSpriteExtra*, std::function<void(bool isPlus)>> plusMinusCallbacks{};
        CCScale9Sprite* previewBG;

        std::set<ButtonSprite*> btnSprites{};
        SessionSelector* sessionSelector;
        Scrollbar* scrollbar;

        int currAmount = 1;

    public:
        virtual void onOpened();
        virtual void onClosed();
};