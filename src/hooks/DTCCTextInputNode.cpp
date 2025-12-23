#include "DTCCTextInputNode.hpp"

void DTCCTextInputNode::setCallback(const std::function<void(const std::string&)>& callback){
    m_fields->callback = callback;
}

bool DTCCTextInputNode::onTextFieldInsertText(cocos2d::CCTextFieldTTF* pSender, char const* text, int nLen, cocos2d::enumKeyCodes keyCodes){
    if (m_fields->callback != NULL)
        m_fields->callback(text);
    
    return CCTextInputNode::onTextFieldInsertText(pSender, text, nLen, keyCodes);
}