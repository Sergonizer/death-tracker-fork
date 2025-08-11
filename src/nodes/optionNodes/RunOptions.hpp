#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>
#include <nodes/SimpleToggler.hpp>

using namespace cocos2d;
using namespace geode;

class RunOptions : public OptionsNode {
    public:
        static RunOptions* create(const CCSize& size);
    private:
        virtual bool setup();

        void addNewRun(CCObject*);

        SimpleToggler* TARToggler;
        SimpleToggler* ResetAsDeathToggler;
        TextInput* runAdditionInput;
        TextInput* HideByLenInput;
        TextInput* RealEndPerInput;
        TextInput* HidUpToInput;

    public:
        virtual void onOpened();
        virtual void onClosed();
};