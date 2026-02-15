#pragma once

#include <Geode/Geode.hpp>
#include "../layers/DTLayer.hpp"
#include "../utils/Save.hpp"

using namespace geode::prelude;

class confirmImportLayer : public Popup {
    protected:
        bool init(DTLayer* const& layer, const Deaths& ds, const Runs& rs);
    public:
        static confirmImportLayer* create(DTLayer* const& layer, const Deaths& ds, const Runs& rs);

    private:
        //makes the text of the scroll layer always visble
        void update(float delta) override;
        
        //runs when clicking yes on the popup
        //
        //will add the found deaths and runs into this levels main save
        void yesClicked(CCObject*);
        

        DTLayer* m_DTLayer;
        Deaths deaths;
        Runs runs;
        SimpleTextArea* text;
};