#include <nodes/optionNodes/SectionOptions.hpp>

SectionOptions* SectionOptions::create(const CCSize& size) {
    auto ret = new SectionOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SectionOptions::setup(){

    


    return true;
}

void SectionOptions::onOpened(){
    
}
void SectionOptions::onClosed(){
    
}