#pragma once

#include <Geode/Geode.hpp>
#include <nodes/SwitcherButton.hpp>

using namespace geode::prelude;

struct CalcMode{
    std::string modeName;
    std::string modeDescription;
};

class CalculatorPopup : public Popup {
    protected:
        bool init() override;

    public:
        static CalculatorPopup* create();

    private:

        std::vector<CalcMode> modes;

        SwitcherButton* modeButton;

        void onModeInfo(CCObject*);
};