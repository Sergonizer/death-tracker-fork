#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>

using namespace cocos2d;
using namespace geode;

class SaveOptions : public OptionsNode {
    public:
        static SaveOptions* create(const CCSize& size);
    private:
        virtual bool setup();
    public:
        virtual void onOpened();
        virtual void onClosed();
};