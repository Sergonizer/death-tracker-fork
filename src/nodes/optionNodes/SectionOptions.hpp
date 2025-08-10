#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>

using namespace cocos2d;
using namespace geode;

class SectionOptions : public OptionsNode {
    public:
        static SectionOptions* create(const CCSize& size);
    private:
        virtual bool setup();
    public:
        virtual void onOpened();
        virtual void onClosed();
};