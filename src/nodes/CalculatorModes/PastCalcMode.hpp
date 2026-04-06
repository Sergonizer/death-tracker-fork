#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;
#include <nodes/CalculatorMode.hpp>

class PastCalcMode : public CalculatorMode {
    protected:
        bool init(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate);

    public:
        static PastCalcMode* create(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate);
        
        std::string getName() const override;
        std::string getDescription() const override;

        void calculate() override;
        void open() override;
        void close() override;
};