#pragma once

#include <Geode/Geode.hpp>
#include <managers/StatsManager.hpp>
#include <nodes/GraphPoint.hpp>
#include <types/DTTypes.hpp>
#include <nodes/SessionSelector.hpp>

using namespace geode::prelude;


class DTGraphNode : public CCNode {
    protected:
        bool init();
        
    public:
        static DTGraphNode* create();

        void setInfo(const DTGraphInfo& info);
        void setScaling(const CCSize& scaling);

        std::optional<DTGraphInfo> getInfo() const{
            return info;
        };

        std::optional<Session> sessionToShow = std::nullopt;

        int runPercent;

        void updateDeaths();

        GraphPointDelegate* delegate;

        CCNode* pointHolder;

    private:

        void updateGraphContent();

        void getGeneralDeaths();
        void getGeneralRuns();
        void getSessionDeaths();
        void getSessionRuns();

        std::optional<DTGraphInfo> info = std::nullopt;
        Deaths deaths{};

        CCSize scaling;

        CCDrawNode* lineNode = nullptr;

        std::vector<CCPoint> points{};
};