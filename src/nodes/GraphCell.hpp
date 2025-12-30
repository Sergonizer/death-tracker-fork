#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

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

        void setOrderPos(int pos);

    private:
        bool init(float width, const DTGraphInfo& graphInfo);

        DTGraphInfo graphInfo;

        void onArrowUp(CCObject*);
        void onArrowDown(CCObject*);
        void onOptions(CCObject*);
};