#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;


class SessionSelector : public CCMenu, public TextInputDelegate {
    public:
        static SessionSelector* create(int count);

        void setCallback(const std::function<void(int)>& callback);

        void setMaximumCount(int count);
        void setCurrentCount(int count, bool ignoreIfUnchanged = true, bool runCallback = false);

        int getCurrentCount();

        void setEnabled(bool value);
        
    private:
        bool init(int count);

        void leftArrowClicked(CCObject*);
        void rightArrowClicked(CCObject*);

        void textInputOpened(CCTextInputNode* input);
        void textInputClosed(CCTextInputNode* input);
        void textChanged(CCTextInputNode* input);

        geode::TextInput* inputNode;
        bool isInputOpened;

        std::function<void(int)> callback = NULL;

        int maxCount = 0;
        int currentCount = 0;
};