#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>
#include <nodes/SimpleToggler.hpp>
#include <nodes/cells/PercentCell.hpp>

using namespace geode::prelude;


class RunOptions : public OptionsNode {
    public:
        static RunOptions* create(const CCSize& size);
    private:
        virtual bool setup();

        void addNewRun(CCObject*);

        TextInput* runAdditionInput;
        TextInput* HideByLenInput;
        TextInput* RealEndPerInput;
        TextInput* HidUpToInput;

        CCMenu* runsMenu;

        void PercentCellClicked(PercentCell* cell);

    public:
        virtual void onOpened();
        virtual void onClosed();
};