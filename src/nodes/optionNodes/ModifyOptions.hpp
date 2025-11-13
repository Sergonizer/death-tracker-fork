#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>
#include <nodes/SimpleToggler.hpp>

using namespace geode::prelude;


class ModifyOptions : public OptionsNode {
    public:
        static ModifyOptions* create(const CCSize& size);
    private:
        virtual bool setup();

        SimpleToggler* TypeToggler;

        void onLocalView(CCObject*);
        void onSharedView(CCObject*);
        CCMenuItemSpriteExtra* localViewBtn;
        CCMenuItemSpriteExtra* sharedViewBtn;
    public:
        virtual void onOpened();
        virtual void onClosed();
};