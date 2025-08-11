#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;


class OptionsNode : public CCMenu {
    protected:
        bool initWithSetup(const CCSize& size);

        virtual bool setup() = 0;
    public:
        virtual void onOpened() = 0;
        virtual void onClosed() = 0;
    protected:
        CCSize size;
};