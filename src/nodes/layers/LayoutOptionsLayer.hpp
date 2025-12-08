#pragma once

#include <Geode/Geode.hpp>
#include <nodes/DTLabel.hpp>
#include <nodes/LayoutColumn.hpp>
#include <nodes/FontSelectionCell.hpp>

using namespace geode::prelude;

class LayoutOptionsLayer : public CCLayer, public geode::ColorPickPopupDelegate {
    public:
        static LayoutOptionsLayer* create(const CCSize& size);

        void setEditedNodeTo(DTLabel* label);
        void setEditedNodeTo(LayoutColumn* column);

        void close();

        bool isEditingNode() const {
            return editedLabel.has_value() || editedColumn.has_value();
        }

        std::function<void()> onBackedOut = NULL;

    private:
        bool init(const CCSize& size);

        std::optional<DTLabel*> editedLabel = std::nullopt;
        std::optional<LayoutColumn*> editedColumn = std::nullopt;

        void switchToMenu(uint8_t menuID);

        CCMenu* labelSettingsNode = nullptr;
        CCMenu* columnSettingsNode = nullptr;
        CCMenu* fontSelectionNode = nullptr;

        uint8_t currentPage = 0;

        TextInput* labelNameInput;
        TextInput* labelTextInput;
        TextInput* fontSizeInput;

        void onTextColorBtnClicked(CCObject*);
        void onLabelColorBtnClicked(CCObject*);

        void updateColor(cocos2d::ccColor4B const& color);

        CCSprite* textColorBtnSprite;
        CCSprite* labelColorBtnSprite;

        void scaleSliderChanged(CCObject*);
        Slider* scaleSlider;

        std::function<void(cocos2d::ccColor4B const&)> colorChangeFunc = NULL;

        void onAlignmentChanged(CCObject* sender);
        CCMenu* alignmentMenu;

        void onFontSelection(CCObject*);
        SimpleTextArea* fontSelectedIndicatorLabel;

        void onBack(CCObject*);
        void onDelete(CCObject*);

        std::map<std::string, FontSelectionCell*> allFontCells{};
        FontSelectionCell* currentlySelectedFontCell = nullptr;
        void onFontSelected(FontSelectionCell* cell);
        ScrollLayer* fontsScroll;
};