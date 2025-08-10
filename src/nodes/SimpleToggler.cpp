#include <nodes/SimpleToggler.hpp>

SimpleToggler* SimpleToggler::create(CCNode* offSprite, CCNode* onSprite, float scale, bool startState){
    auto ret = new SimpleToggler();
    if (ret && ret->init(offSprite, onSprite, scale, startState)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SimpleToggler::init(CCNode* offSprite, CCNode* onSprite, float scale, bool startState){
    offSprite->setScale(scale);
    onSprite->setScale(scale);

    if (!CCMenuItemToggler::init(offSprite, onSprite, this, menu_selector(SimpleToggler::onToggled))) return false;

    this->toggle(startState);

    return true;
}

void SimpleToggler::setCallback(const std::function<void(bool)>& callback){
    this->callback = callback;
}

void SimpleToggler::onToggled(CCObject* sender){
    if (this->callback != NULL)
        this->callback(!static_cast<CCMenuItemToggler*>(sender)->isToggled());
}