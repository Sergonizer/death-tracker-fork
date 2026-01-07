#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class GraphPointDisplay : public CCNode {
    protected:
        bool init();
        
    public:
        static GraphPointDisplay* create();

        void setContent(std::string run, float ratePercent, DTGraphType type);
    
    private:

        CCLabelBMFont* percentLabel;
        CCLabelBMFont* rateLabel;
};