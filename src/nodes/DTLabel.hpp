#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace cocos2d;
using namespace geode;

class DTLabel : public CCNode {
    public:
        static DTLabel* create(const DTLabelInfo& info, float gridSize);
        
        DTLabelInfo labelInfo;
        float gridSize;

    private:
        bool init(const DTLabelInfo& info, float gridSize);

        void updateState();

        CCPoint localToGridPosition(CCPoint localPosition);
        CCPoint gridToLocalPosition(int x, int y);

        std::string modifyText(const std::string& str);

        SimpleTextArea* textArea;
};