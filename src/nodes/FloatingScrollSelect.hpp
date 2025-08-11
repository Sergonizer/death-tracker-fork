#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;


class FloatingScrollSelect : public CCNode {
    public:
        static FloatingScrollSelect* create(CCSize size, const std::vector<std::string>& options);
        
    private:
        bool init(CCSize size, const std::vector<std::string>& options);

        ScrollLayer* scrollLayer;
};