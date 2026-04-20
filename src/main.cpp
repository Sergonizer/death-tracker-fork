#include <Geode/Geode.hpp>
using namespace geode::prelude;
#include "utils/Save.hpp"
#include "managers/StatsManager.hpp"
#include "utils/Settings.hpp"

$execute {
    (void)file::createDirectory(Mod::get()->getSaveDir() / "levels");

    auto oldLayout = Save::getOldLayout();

    if (oldLayout.size()){
        // DTLayoutV3 v3Layout{};

        // v3Layout.columns.push_back(DTColumnInfo{
        //     .orderPos = 0,
        //     .currentWidth = 205
        // });
        // v3Layout.columns.push_back(DTColumnInfo{
        //     .orderPos = 1,
        //     .currentWidth = 205
        // });
        
        // std::map<V2LabelLayout*, std::pair<bool, bool>> labelsOrganized{};

        // std::map<int, std::pair<V2LabelLayout*, V2LabelLayout*>> labelsInLine{};

        // for (int i = 0; i < oldLayout.size(); i++)
        // {
        //     if (!labelsInLine.contains(oldLayout[i].line)){
        //         labelsInLine.insert({oldLayout[i].line, {&oldLayout[i], nullptr}});
        //         continue;
        //     }
        //     else{
        //         labelsInLine[oldLayout[i].line].second = &oldLayout[i];
        //     }
        // }

        // for (const auto& [line, lineLabels] : labelsInLine)
        // {
        //     if (lineLabels.second == nullptr){
        //         labelsOrganized.insert({lineLabels.first, {true, true}});
        //     }
        //     else{
        //         bool isFirstOnLeft = lineLabels.first->position == 0;
                
        //         labelsOrganized.insert({lineLabels.first, {isFirstOnLeft, !isFirstOnLeft}});
        //         labelsOrganized.insert({lineLabels.second, {!isFirstOnLeft, isFirstOnLeft}});
        //     }
        // }
        
        // for (const auto& [label, whichColumn] : labelsOrganized)
        // {
        //     DTLabelInfo labelInfo{};

        //     labelInfo.layer = label->line;
        //     labelInfo.textColor = label->color;
        //     labelInfo.labelColor = label->color;
        //     labelInfo.font = StatsManager::getFont(label->font);
        //     labelInfo.labelName = label->labelName;
        //     labelInfo.scale = label->fontSize;
        //     labelInfo.text = label->text;

        //     if (!whichColumn.first && whichColumn.second){
        //         labelInfo.minPlacementRange = 1;
        //         labelInfo.maxPlacementRange = 1;
        //     }
        //     else if (whichColumn.first && whichColumn.second){
        //         labelInfo.maxPlacementRange = 1;
        //     }   

        //     v3Layout.labels.push_back(labelInfo);
        // }
        
        // Save::setLayout(v3Layout);
        // Save::setOldLayout({});
    }

    if (Save::getLayout().isEmpty()){
        Save::setLayout(Save::getDefaultLayout());

        auto graphs = std::vector<DTGraphInfo>{
            DTGraphInfo{
                .isEnabled = true,
                .coverage = static_cast<DTGraphCoverage>(3),
                .type = static_cast<DTGraphType>(0),
                .orderPos = 0,
                .thickness = 1.0f,
                .outlineThickness = 0.75f,
                .pointScale = 0.10000000149011612f,
                .color = {122, 74, 0, 255},
                .outlineColor = {101, 50, 0, 255},
                .pointColor = {164, 76, 0, 255},
                .name = "session runs"
            },
            DTGraphInfo{
                .isEnabled = true,
                .coverage = static_cast<DTGraphCoverage>(2),
                .type = static_cast<DTGraphType>(0),
                .orderPos = 1,
                .thickness = 1.0f,
                .outlineThickness = 0.75f,
                .pointScale = 0.10000000149011612f,
                .color = {255, 145, 0, 255},
                .outlineColor = {170, 104, 16, 255},
                .pointColor = {114, 66, 3, 255},
                .name = "sess from0"
            },
            DTGraphInfo{
                .isEnabled = true,
                .coverage = static_cast<DTGraphCoverage>(1),
                .type = static_cast<DTGraphType>(0),
                .orderPos = 2,
                .thickness = 0.75f,
                .outlineThickness = 0.5f,
                .pointScale = 0.07999999821186066f,
                .color = {14, 138, 37, 253},
                .outlineColor = {11, 96, 32, 255},
                .pointColor = {5, 181, 48, 255},
                .name = "general runs"
            },
            DTGraphInfo{
                .isEnabled = true,
                .coverage = static_cast<DTGraphCoverage>(0),
                .type = static_cast<DTGraphType>(0),
                .orderPos = 3,
                .thickness = 1.0f,
                .outlineThickness = 0.75f,
                .pointScale = 0.10000000149011612f,
                .color = {0, 255, 55, 255},
                .outlineColor = {18, 162, 53, 255},
                .pointColor = {15, 105, 41, 255},
                .name = "general from0"
            }
        };

        Save::setGraphs(graphs);

        Save::setNewBestColor({255, 255, 0});
        Save::setSessionBestColor({ 255, 136, 0 });
    }

    StatsManager::m_savesFolderPath = Settings::getSavePath();
    
};

$on_mod(Loaded){
    //load fonts
    auto allFonts = StatsManager::getAllFonts();

    for (int i = 0; i < allFonts.size(); i++)
    {
        auto label = CCLabelBMFont::create("abcdefghijclmnopqrstuvwxyz{}-", allFonts[i].c_str());
        CCScene::get()->addChild(label);
        label->removeMeAndCleanup();
    }
};