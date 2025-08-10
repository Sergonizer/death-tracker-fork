#include <nodes/optionNodes/LinkOptions.hpp>

LinkOptions* LinkOptions::create(const CCSize& size) {
    auto ret = new LinkOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool LinkOptions::setup(){

    


    return true;
}

void LinkOptions::onOpened(){
    
}
void LinkOptions::onClosed(){
    
}