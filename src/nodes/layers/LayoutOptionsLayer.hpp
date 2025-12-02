#pragma once

#include <Geode/Geode.hpp>
#include <nodes/DTLabel.hpp>
#include <nodes/LayoutColumn.hpp>

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

    private:
        bool init(const CCSize& size);

        std::optional<DTLabel*> editedLabel = std::nullopt;
        std::optional<LayoutColumn*> editedColumn = std::nullopt;

        CCMenu* labelSettingsNode = nullptr;
        CCMenu* columnSettingsNode = nullptr;

        TextInput* labelNameInput;
        TextInput* labelTextInput;
        TextInput* fontSizeInput;

        void onTextColorBtnClicked(CCObject*);
        void onLabelColorBtnClicked(CCObject*);

        void updateColor(cocos2d::ccColor4B const& color);

        CCSprite* textColorBtnSprite;
        CCSprite* labelColorBtnSprite;

        std::function<void(cocos2d::ccColor4B const&)> colorChangeFunc = NULL;

        void onAlignmentChanged(CCObject* sender);
        CCMenu* alignmentMenu;
};