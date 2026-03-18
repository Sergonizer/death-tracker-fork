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
        auto defaultLayout = DTLayoutV3{
            .columns = std::vector<DTColumnInfo>{
                DTColumnInfo{ .orderPos = 0, .currentWidth = 120.0f, .color = {255, 255, 255} },
                DTColumnInfo{ .orderPos = 1, .currentWidth = 80.0f, .color = {255, 255, 255} },
                DTColumnInfo{ .orderPos = 2, .currentWidth = 80.0f, .color = {255, 255, 255} },
                DTColumnInfo{ .orderPos = 3, .currentWidth = 80.0f, .color = {255, 255, 255} },
                DTColumnInfo{ .orderPos = 4, .currentWidth = 80.0f, .color = {255, 255, 255} },
            },
            .labels = std::vector<DTLabelInfo>{
                DTLabelInfo{
                    .minPlacementRange = 0,
                    .maxPlacementRange = 2,
                    .layer = 0,
                    .labelName = "level name",
                    .text = "{lvln}:",
                    .font = "goldFont.fnt",
                    .horizontalAlignment = static_cast<CCTextAlignment>(1),
                    .labelColor = {255, 213, 118, 255},
                    .textColor = {255, 255, 255, 255},
                    .scale = 0.5f,
                    .isExpanded = true,
                    .wrapping = static_cast<WrappingMode>(2)
                },
                DTLabelInfo{
                    .minPlacementRange = 0,
                    .maxPlacementRange = 0,
                    .layer = 1,
                    .labelName = "attempts",
                    .text = "gd - {att}{nl}death tracker - {dtatt}",
                    .font = "chatFont.fnt",
                    .horizontalAlignment = static_cast<CCTextAlignment>(0),
                    .labelColor = {170, 170, 170, 255},
                    .textColor = {255, 255, 255, 255},
                    .scale = 0.6417998f,
                    .isExpanded = true,
                    .wrapping = static_cast<WrappingMode>(3)
                },
                DTLabelInfo{
                    .minPlacementRange = 0,
                    .maxPlacementRange = 0,
                    .layer = 2,
                    .labelName = "playtime",
                    .text = "overall - {ptgen}{nl}from 0: {ptf0}{nl}runs: {ptruns}",
                    .font = "gjFont17.fnt",
                    .horizontalAlignment = static_cast<CCTextAlignment>(0),
                    .labelColor = {170, 170, 170, 255},
                    .textColor = {255, 255, 255, 255},
                    .scale = 0.28f,
                    .isExpanded = true,
                    .wrapping = static_cast<WrappingMode>(3)
                },
                DTLabelInfo{
                    .minPlacementRange = 1,
                    .maxPlacementRange = 1,
                    .layer = 1,
                    .labelName = "from 0",
                    .text = "{general}",
                    .font = "bigFont.fnt",
                    .horizontalAlignment = static_cast<CCTextAlignment>(0),
                    .labelColor = {64, 138, 98, 255},
                    .textColor = {123, 255, 184, 255},
                    .scale = 0.34999999f,
                    .isExpanded = true,
                    .wrapping = static_cast<WrappingMode>(2)
                },
                DTLabelInfo{
                    .minPlacementRange = 2,
                    .maxPlacementRange = 2,
                    .layer = 1,
                    .labelName = "runs",
                    .text = "{runs}",
                    .font = "bigFont.fnt",
                    .horizontalAlignment = static_cast<CCTextAlignment>(0),
                    .labelColor = {37, 86, 70, 255},
                    .textColor = {42, 167, 102, 255},
                    .scale = 0.34999999f,
                    .isExpanded = true,
                    .wrapping = static_cast<WrappingMode>(2)
                },
                DTLabelInfo{
                    .minPlacementRange = 3,
                    .maxPlacementRange = 4,
                    .layer = 0,
                    .labelName = "session playtime",
                    .text = "overall: {ptsgen}{nl}from 0: {ptsf0} runs: {ptsruns}",
                    .font = "gjFont17.fnt",
                    .horizontalAlignment = static_cast<CCTextAlignment>(1),
                    .labelColor = {176, 150, 110, 255},
                    .textColor = {255, 255, 255, 255},
                    .scale = 0.30000001f,
                    .isExpanded = true,
                    .wrapping = static_cast<WrappingMode>(2)
                },
                DTLabelInfo{
                    .minPlacementRange = 3,
                    .maxPlacementRange = 3,
                    .layer = 1,
                    .labelName = "session",
                    .text = "{s0}",
                    .font = "bigFont.fnt",
                    .horizontalAlignment = static_cast<CCTextAlignment>(0),
                    .labelColor = {160, 109, 60, 255},
                    .textColor = {255, 174, 77, 255},
                    .scale = 0.34999999f,
                    .isExpanded = true,
                    .wrapping = static_cast<WrappingMode>(2)
                },
                DTLabelInfo{
                    .minPlacementRange = 4,
                    .maxPlacementRange = 4,
                    .layer = 1,
                    .labelName = "session runs",
                    .text = "{sruns}",
                    .font = "bigFont.fnt",
                    .horizontalAlignment = static_cast<CCTextAlignment>(0),
                    .labelColor = {102, 68, 31, 255},
                    .textColor = {206, 127, 42, 255},
                    .scale = 0.34999999f,
                    .isExpanded = true,
                    .wrapping = static_cast<WrappingMode>(2)
                },
            }
        };

        Save::setLayout(defaultLayout);

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