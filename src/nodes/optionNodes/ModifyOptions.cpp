#include <nodes/optionNodes/ModifyOptions.hpp>
#include <nodes/SessionSelector.hpp>
#include <nodes/layers/DTLayer.hpp>

ModifyOptions* ModifyOptions::create(const CCSize& size) {
    auto ret = new ModifyOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ModifyOptions::setup(){

    this->setEnabled(false);
    this->setOpacity(0);

    auto TypLevelBtnSpr = ButtonSprite::create("Level");
    auto TypeSessionBtnSpr = ButtonSprite::create("Session");
    TypeToggler = SimpleToggler::create(
        TypLevelBtnSpr,
        TypeSessionBtnSpr,
        .75f
    );
    TypeToggler->setOpacity(0);
    this->addChild(TypeToggler);

    auto dtLayer = DTLayer::get();

    int sessionAmount = 1;
    if  (dtLayer != nullptr && dtLayer->m_SharedLevelStats.isOk()){
        auto& stats = dtLayer->m_SharedLevelStats.unwrap();
        sessionAmount = stats.sessions.size();
    }

    auto sessionSelector = SessionSelector::create(sessionAmount);

    return true;
}

void ModifyOptions::onOpened(){
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));
    TypeToggler->runAction(CCFadeIn::create(fadeTime));

    this->setEnabled(true);
}
void ModifyOptions::onClosed(){
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));
    TypeToggler->runAction(CCFadeOut::create(fadeTime));

    this->setEnabled(false);
}