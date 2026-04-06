#include "BeforeCalcMode.hpp"

BeforeCalcMode* BeforeCalcMode::create(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate) {
    auto ret = new BeforeCalcMode();
    if (ret && ret->init(size, std::move(onVariablesChanged), std::move(onCalculate))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool BeforeCalcMode::init(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate){
    if (!CalculatorMode::init(size, std::move(onVariablesChanged), std::move(onCalculate))) return false;



    return true;
}

void BeforeCalcMode::calculate(){

}

std::string BeforeCalcMode::getName() const{
    return "Before";
}
std::string BeforeCalcMode::getDescription() const{
    return "The amount of times you didnt get to a percentage from the target run";
}

void BeforeCalcMode::open(){

}
void BeforeCalcMode::close(){

}