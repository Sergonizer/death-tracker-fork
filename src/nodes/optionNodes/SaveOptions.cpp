#include <nodes/optionNodes/SaveOptions.hpp>

SaveOptions* SaveOptions::create(const CCSize& size) {
    auto ret = new SaveOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SaveOptions::setup(){

    


    return true;
}

void SaveOptions::onOpened(){
    
}
void SaveOptions::onClosed(){
    
}