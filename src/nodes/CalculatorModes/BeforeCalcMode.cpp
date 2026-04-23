#include "BeforeCalcMode.hpp"

#include <nodes/layers/DTLayer.hpp>

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

    beforePercentInput = TextInput::create(size.width / 1.2f, "before%");
    beforePercentInput->setCommonFilter(CommonFilter::Uint);
    beforePercentInput->setCallback([&](auto newStr){
        sendVars();
    });
    beforePercentInput->setPosition(size / 2);
    this->addChild(beforePercentInput);

    auto beforePercentInputLabel = CCLabelBMFont::create("Before percent", "bigFont.fnt");
    beforePercentInputLabel->setScale(.45f);
    beforePercentInputLabel->setPosition(beforePercentInput->getPosition() + ccp(1.5f, beforePercentInput->getScaledContentHeight() / 2));
    beforePercentInputLabel->setAnchorPoint({.5f, 0});
    this->addChild(beforePercentInputLabel);

    return true;
}

void BeforeCalcMode::calculate(){
    if (beforePercentInput->getString() == "" || DTLayer::get()->m_MyLevelStats.isErr()) return;

    auto numRes = geode::utils::numFromString<int>(beforePercentInput->getString());
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

        if (run.unwrap().end < num)
            totalPastDeaths += death.second;
    }
    
    onCalculate(fmt::format("{}x", totalPastDeaths));
}

std::string BeforeCalcMode::getName() const{
    return "Before";
}
std::string BeforeCalcMode::getDescription() const{
    return "The amount of times you didnt get to a percentage from the target run";
}

void BeforeCalcMode::open(){
    beforePercentInput->setEnabled(true);
    this->setVisible(true);

    sendVars();
}
void BeforeCalcMode::close(){
    beforePercentInput->setEnabled(false);
    this->setVisible(false);
}

void BeforeCalcMode::sendVars(){
    onVariablesChanged(beforePercentInput->getString() != "" ? fmt::format(
        "{}<",
        beforePercentInput->getString()
    ) : "");
}