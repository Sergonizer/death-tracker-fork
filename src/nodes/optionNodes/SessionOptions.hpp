#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>

using namespace cocos2d;
using namespace geode;

class SessionOptions : public OptionsNode {
    public:
        static SessionOptions* create(const CCSize& size);
    private:
        virtual bool setup();
    public:
        virtual void onOpened();
        virtual void onClosed();
};