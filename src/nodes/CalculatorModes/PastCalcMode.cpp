#include "PastCalcMode.hpp"

#include <nodes/layers/DTLayer.hpp>

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

    passPercentInput = TextInput::create(size.width / 1.2f, "pass%");
    passPercentInput->setCommonFilter(CommonFilter::Uint);
    passPercentInput->setCallback([&](auto newStr){
        sendVars();
    });
    passPercentInput->setPosition(size / 2);
    this->addChild(passPercentInput);

    auto passPercentInputLabel = CCLabelBMFont::create("Passed percent", "bigFont.fnt");
    passPercentInputLabel->setScale(.45f);
    passPercentInputLabel->setPosition(passPercentInput->getPosition() + ccp(1.5f, passPercentInput->getScaledContentHeight() / 2));
    passPercentInputLabel->setAnchorPoint({.5f, 0});
    this->addChild(passPercentInputLabel);

    return true;
}

void PastCalcMode::calculate(){
    if (passPercentInput->getString() == "" || DTLayer::get()->m_MyLevelStats.isErr()) return;

    auto numRes = geode::utils::numFromString<int>(passPercentInput->getString());
    if (numRes.isErr()) return;
    auto num = numRes.unwrap();

    auto& meStats = DTLayer::get()->m_MyLevelStats.unwrap();

    if (meStats.from0.isErr()) return; 
    auto& meGen = meStats.from0.unwrap();

    Deaths deads{};
    StatsManager::mergeMapsAdd(deads, meGen.deaths);

    for (const auto& levelData : DTLayer::get()->linkedLevelsData)
    {
        if (levelData.from0.isErr() || levelData.levelKey == meStats.levelKey) continue;
        auto levelFrom0Stats = levelData.from0.unwrap();
        
        StatsManager::mergeMapsAdd(deads, levelFrom0Stats.deaths);
    }

    int totalPastDeaths = 0;

    for (const auto& death : deads)
    {
        auto run = StatsManager::splitRunKey(death.first);
        if (run.isErr()) continue;

        if (run.unwrap().end > num)
            totalPastDeaths += death.second;
    }
    
    onCalculate(fmt::format("{}x", totalPastDeaths));
}

std::string PastCalcMode::getName() const{
    return "Passes";
}

std::string PastCalcMode::getDescription() const{
    return "The amount of times you have passed a percentage from the target run";
}

void PastCalcMode::open(){
    passPercentInput->setEnabled(true);
    this->setVisible(true);

    sendVars();
}
void PastCalcMode::close(){
    passPercentInput->setEnabled(false);
    this->setVisible(false);
}

void PastCalcMode::sendVars(){
    onVariablesChanged(passPercentInput->getString() != "" ? fmt::format(
        "{}>",
        passPercentInput->getString()
    ) : "");
}