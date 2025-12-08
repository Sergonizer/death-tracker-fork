#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FontSelectionCell : public CCMenu {
    public:
        static FontSelectionCell* create(const std::string& font, const std::function<void(FontSelectionCell* onSelected)>& onFontChosen);

        void deselect();
        void select();

        std::string font;
    private:
        bool init(const std::string& font, const std::function<void(FontSelectionCell* onSelected)>& onFontChosen);

        std::function<void(FontSelectionCell* onSelected)> onFontChosen = NULL;

        void onUsedClicked(CCObject*);
        CCMenuItemSpriteExtra* useBtn;
};