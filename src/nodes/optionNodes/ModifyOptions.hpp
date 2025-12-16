#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>
#include <nodes/SimpleToggler.hpp>
#include <nodes/DTLabel.hpp>

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

    public:
        virtual void onOpened();
        virtual void onClosed();
};