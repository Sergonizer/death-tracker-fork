#pragma once

#include <Geode/Geode.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/GraphPoint.hpp>
#include <nodes/GraphHolder.hpp>
#include <nodes/GraphCell.hpp>

using namespace geode::prelude;

class DTGraphLayer : public Popup<>, public TextInputDelegate, public GraphPointDelegate {
    protected:
        bool setup() override;
    public:
        static DTGraphLayer* create();

    private:
        //graph stuff

        CCLabelBMFont* noGraphLabel;

        //change the displayed point
        void OnPointSelected(cocos2d::CCNode* point) override;
        //have no point displayed if the deselected point was the was selected prior
        void OnPointDeselected(cocos2d::CCNode* point) override;

        //handle the layer closing and refresh the text
        void onClose(cocos2d::CCObject*) override;

        void update(float dt) override;

        GraphHolder* graph;

        ScrollLayer* graphsScroll;
        TextInput* runSelectInput;

        void onAddGraph(CCObject*);

        void addGraph();
        void addGraph(const DTGraphInfo& info);
        void removeGraph(const std::string& graphName);

        void openOptionsFor(GraphCell* cell);
        CCMenu* graphsPage;
        CCMenu* graphOptionsPage;

        void onBaseColor(CCObject*);
        void onOutlineColor(CCObject*);
};