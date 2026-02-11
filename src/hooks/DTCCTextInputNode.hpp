#include <Geode/modify/CCTextInputNode.hpp>

using namespace geode::prelude;

class $modify(DTCCTextInputNode, CCTextInputNode) {
    struct Fields{
        geode::Function<void(const std::string&)> callback = NULL;
    };

    public:
        void setCallback(geode::Function<void(const std::string&)> callback);

        bool onTextFieldInsertText(cocos2d::CCTextFieldTTF* pSender, char const* text, int nLen, cocos2d::enumKeyCodes keyCodes);
};