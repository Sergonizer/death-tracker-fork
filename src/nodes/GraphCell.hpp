#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/SimpleToggler.hpp>
#include <nodes/OptionSwitcher.hpp>

using namespace geode::prelude;

class GraphCell : public CCMenu {
    public:
        static GraphCell* create(float width, const DTGraphInfo& graphInfo);

        DTGraphInfo getinfo() const{
            return graphInfo;
        };

        std::function<void(GraphCell* cell, bool up)> onArrowCallback = NULL;
        std::function<void(GraphCell* cell)> onOptionsCallback = NULL;
        std::function<void(GraphCell* cell)> onEnabledChanged = NULL;
        std::function<void(GraphCell* cell)> onInfoChangedCallback = NULL;

        void setName(const std::string name);
        void setCoverage(DTGraphCoverage coverage);
        void setType(DTGraphType type);
        void setOrderPos(int pos);
        void setThickness(float thickness);
        void setOutlineThickness(float outlineThickness);
        void setPointSize(float pointSize);
        void setColor(ccColor4B color);
        void setOutlineColor(ccColor4B color);
        void setPointColor(ccColor4B color);

        void onInfoChanged(bool updateGraph);

        void setEnabledInfo(bool b, bool changeToggler, bool callback);
        
        private:
        bool init(float width, const DTGraphInfo& graphInfo);
        
        DTGraphInfo graphInfo;        

        OptionSwitcher<DTGraphType>* typeSwitcher;
        SimpleToggler* enableToggleBtn;
        SimpleTextArea* label;
        CCSprite* outerColor;
        CCSprite* innerColor;

        void onArrowUp(CCObject*);
        void onArrowDown(CCObject*);
        void onOptions(CCObject*);
};