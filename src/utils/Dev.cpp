#include <utils/Dev.hpp>

// TODO: set this to false before builing
const bool Dev::ENABLED = false;

/* global flags
================ */
const bool Dev::MINIFY_SAVE_FILE = Dev::ENABLED
    ? false
    : true;

void Dev::fadeTextInput(TextInput* const input, bool fadeIn, float time, bool changeEnabled){
    input->getBGSprite()->runAction(fadeIn ? static_cast<CCAction*>(CCFadeTo::create(time, 90)) : static_cast<CCAction*>(CCFadeTo::create(time, 0)));

    if (fadeIn){
        if (changeEnabled)
            input->setEnabled(true);
        input->getInputNode()->m_textLabel->runAction(input->getInputNode()->isTouchEnabled() ? 
            static_cast<CCAction*>(CCFadeTo::create(time, 255)) :
            static_cast<CCAction*>(CCFadeTo::create(time, 150))
        );
    }
    else{
        if (changeEnabled)
            input->setEnabled(false);
        input->getInputNode()->m_textLabel->runAction(CCFadeTo::create(time, 0));
    }
    
}