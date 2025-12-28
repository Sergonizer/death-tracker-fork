#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/DTGraphNode.hpp>
#include <nodes/layers/AdvancedScrollLayer.hpp>

class GraphHolder : public CCNode {
    public:
        static GraphHolder* create(const CCSize& scaling);


    private:
        bool init(const CCSize& scaling);

        void setBGFillColor(const ccColor4B& color);
        void setOutlineThickness(float thickness);
        void setOutlineColor(const ccColor4B& color);

        void refreshBackground(const ccColor4B& newFillColor, float newOutlineThickness, const ccColor4B& newOutlineColor);

        void refreshGrid();

        void updateLabels();

        void update(float dt);

        // void setSmallLinesColor(const ccColor4B& newColor);
        // void setBoldLinesColor(const ccColor4B& newColor);
        // void setLabelsColor(const ccColor4B& newColor);

        AdvancedScrollLayer* scrollLayer;

        ccColor4B fillColor = {0, 0, 0, 120};
        float outlineThickness = 0.2f;
        ccColor4B outlineColor = { 124, 124, 124, 255};

        int labelEvery = 5;

        CCDrawNode* boldGridNode = nullptr;
        ccColor4B boldGridColor = { 29, 29, 29, 255 };

        ccColor4B boldLineColor = {115, 115, 115, 255};
        ccColor4B smallLineColor = {115, 115, 115, 100};
        ccColor4B labelColor = { 202, 202, 202, 255};

        CCDrawNode* fillNode = nullptr;
        CCDrawNode* outlineNode = nullptr;
        CCClippingNode* mask = nullptr;

        std::set<CCSprite*> smallLines{};
        std::set<CCSprite*> boldLines{};
        std::set<CCLabelBMFont*> labels{};

        CCNode* LabelsContainer = nullptr;
        CCNode* LabelsVerticalContainer = nullptr;
        CCNode* LabelsHorizontalContainer = nullptr;

        CCNode* graphsContainer = nullptr;

        std::unordered_map<std::string, DTGraphNode*> allGraphs{};

        float prevZoom;
};