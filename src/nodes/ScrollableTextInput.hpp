#pragma once

#include <Geode/Geode.hpp>
#include <nodes/layers/DTTutorialLayer.hpp>

using namespace geode::prelude;

class ScrollableTextInput : public CCNode, public TextInputDelegate {
    protected:
        bool init(float width, const std::string& placeholder, const std::string& font);
        
    public:
        static ScrollableTextInput* create(float width, const std::string& placeholder, const std::string& font = "bigFont.fnt");

        void setCommonFilter(CommonFilter filter);
        void setCallback(geode::Function<void(std::string const&)> onInput);
        void setString(std::string const& str, bool triggerCallback = false);
        std::string getString() const { return textInput->getString(); }
        bool isSelected() const { return isFocused; }
        TextInput* getTextInput() const { return textInput; }
        void setEnabled(bool enabled);
        bool isEnabled() const { return enabled; }

    private:
        void updateTextInput();

        TextInput* textInput = nullptr;
        std::string placeholder;
        std::string font;
        float width;
        std::optional<float> prevXursorPos = std::nullopt;
        Scrollbar* bar;

        bool isFocused = false;

        void recreateTextInput(float extraWidth);

        virtual void textChanged(CCTextInputNode* node) override;

        virtual void textInputOpened(CCTextInputNode* node) override;

        virtual void textInputClosed(CCTextInputNode* node) override;

        CCScrollLayerExt* scrollLayer = nullptr;

        void update(float dt) override;

        CommonFilter filter;

        geode::Function<void(std::string const&)> onInput = NULL;

        bool enabled = true;

        int androidTextChangedCounter = -1;
        bool isEmpty = false;
};