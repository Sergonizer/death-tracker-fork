#include <nodes/optionNodes/ModifyOptions.hpp>

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

    


    return true;
}

void ModifyOptions::onOpened(){
    
}
void ModifyOptions::onClosed(){
    
}