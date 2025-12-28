#pragma once

#include <Geode/Geode.hpp>
#include "../layers/DTLayer.hpp"
#include "../GraphPoint.hpp"
#include "../GraphHolder.hpp"

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
};