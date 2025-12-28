#pragma once

#include <Geode/Geode.hpp>
#include "../managers/StatsManager.hpp"
#include "../nodes/GraphPoint.hpp"

using namespace geode::prelude;


class DTGraphNode : public CCNode {
    protected:
        bool init();
        
    public:
        static DTGraphNode* create();

        enum GraphType{
            PassRate,
            ReachRate
        };

        void addGraphForDeaths(const std::string& graphName, const std::vector<DeathInfo>& deaths, GraphType type, float thickness, const ccColor4B& color, bool clearOther = false);
        void setGraphColorByName(const std::string& graphName, const ccColor4B& newColor);
        void eraseGraphByName(const std::string& graphName);

    private:

        struct GraphLine {
            CCDrawNode* lineNode = nullptr;
            std::vector<CCPoint> points{};
            float thickness = 1;
            CCMenu* GraphPointsContainer = nullptr;
        };
};