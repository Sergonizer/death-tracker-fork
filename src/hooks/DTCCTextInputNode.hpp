#include <Geode/modify/CCTextInputNode.hpp>

using namespace geode::prelude;

class $modify(DTCCTextInputNode, CCTextInputNode) {
    struct Fields{
        std::function<void(const std::string&)> callback = NULL;
    };

    public:
        void setCallback(const std::function<void(const std::string&)>& callback);

        bool onTextFieldInsertText(cocos2d::CCTextFieldTTF* pSender, char const* text, int nLen, cocos2d::enumKeyCodes keyCodes);
};