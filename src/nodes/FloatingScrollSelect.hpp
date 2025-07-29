#pragma once

#include <Geode/Geode.hpp>

using namespace cocos2d;
using namespace geode;

class FloatingScrollSelect : public CCNode {
    public:
        static FloatingScrollSelect* create(CCSize size, const std::vector<std::string>& options);
        
    private:
        bool init(CCSize size, const std::vector<std::string>& options);

        ScrollLayer* scrollLayer;
};