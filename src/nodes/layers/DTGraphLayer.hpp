#pragma once

#include <Geode/Geode.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/GraphPoint.hpp>
#include <nodes/GraphHolder.hpp>
#include <nodes/GraphCell.hpp>
#include <nodes/OptionSwitcher.hpp>

using namespace geode::prelude;

class DTGraphLayer : public Popup<>, public TextInputDelegate, public GraphPointDelegate, public ColorPickPopupDelegate {
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
        SessionSelector* sessionSelector;

        void onAddGraph(CCObject*);

        void addGraph();
        void addGraph(const DTGraphInfo& info);
        void removeGraph(const std::string& graphName);

        void openOptionsFor(GraphCell* cell);
        CCMenu* graphsPage;
        CCMenu* graphOptionsPage;

        void onColor(CCObject*);

        TextInput* nameInput;
        OptionSwitcher<DTGraphCoverage>* coverageSwitcher;
        OptionSwitcher<DTGraphType>* typeSwitcher;
        TextInput* thicknessBaseInput;
        TextInput* thicknessOutlineInput;
        CCSprite* colorBaseBtnSpr;
        CCSprite* colorOutlineBtnSpr;
        TextInput* pointScaleInput;
        CCSprite* pointColorBtnSpr;

        std::optional<GraphCell*> editedGraph = std::nullopt;

        void updateColor(cocos2d::ccColor4B const& color) override;

        std::optional<std::function<void(const ccColor4B&)>> callbacksForColorPopups = std::nullopt;

        void saveAllGraphs();
    };