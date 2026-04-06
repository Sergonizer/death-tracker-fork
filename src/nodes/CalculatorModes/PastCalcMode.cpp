#include "PastCalcMode.hpp"

PastCalcMode* PastCalcMode::create(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate) {
    auto ret = new PastCalcMode();
    if (ret && ret->init(size, std::move(onVariablesChanged), std::move(onCalculate))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool PastCalcMode::init(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate){
    if (!CalculatorMode::init(size, std::move(onVariablesChanged), std::move(onCalculate))) return false;



    return true;
}

void PastCalcMode::calculate(){

}

std::string PastCalcMode::getName() const{
    return "Passes";
}

std::string PastCalcMode::getDescription() const{
    return "The amount of times you have passed a percentage from the target run";
}

void PastCalcMode::open(){
    
}
void PastCalcMode::close(){

}