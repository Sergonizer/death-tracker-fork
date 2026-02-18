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
        //TextInput* RealEndPerInput;
        TextInput* HidUpToInput;

        ScrollLayer* runsScrollLayer;

        void PercentCellClicked(PercentCell* cell);
        void PercentMaxHideValChanged(PercentCell* cell);

        void createRunCell(int percent, int maxToHide);

    public:
        virtual void onOpened();
        virtual void onClosed();
};