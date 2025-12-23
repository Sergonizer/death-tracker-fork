#include <Geode/Geode.hpp>
using namespace geode::prelude;
#include "utils/Save.hpp"
#include "managers/StatsManager.hpp"
#include "utils/Settings.hpp"
#include <geode.custom-keybinds/include/Keybinds.hpp>
using namespace keybinds;

$execute {
    BindManager::get()->registerBindable({ "enter-new-line"_spr, "Enter new line",
        "Enters a new line when editing a labels text",
        { 
            Keybind::create(KEY_Enter, Modifier::None) 
        }
    });
    BindManager::get()->registerBindable({ "enter-new-run-per"_spr, "Enter new run percentage",
        "Enters a new run percent when inputting for adding a new percent to your \"runs to track\"",
        { 
            Keybind::create(KEY_Enter, Modifier::None) 
        }
    });
    BindManager::get()->registerBindable({ "add-deaths"_spr, "Add deaths",
        "Adds deaths while you input a percent to change deaths to your level",
        { 
            Keybind::create(KEY_Enter, Modifier::None)
        }
    });
    BindManager::get()->registerBindable({ "remove-deaths"_spr, "Remove deaths",
        "Removes deaths while you input a percent to change deaths to your level",
        { 
            Keybind::create(KEY_Enter, Modifier::Shift)
        }
    });

    auto _ = file::createDirectory(Mod::get()->getSaveDir() / "levels");

    auto oldLayout = Save::getOldLayout();

    if (oldLayout.size()){
        DTLayoutV3 v3Layout{};

        v3Layout.columns.push_back(DTColumnInfo{
            .orderPos = 0,
            .currentWidth = 205
        });
        v3Layout.columns.push_back(DTColumnInfo{
            .orderPos = 1,
            .currentWidth = 205
        });
        
        std::map<V2LabelLayout*, std::pair<bool, bool>> labelsOrganized{};

        std::map<int, std::pair<V2LabelLayout*, V2LabelLayout*>> labelsInLine{};

        for (int i = 0; i < oldLayout.size(); i++)
        {
            if (!labelsInLine.contains(oldLayout[i].line)){
                labelsInLine.insert({oldLayout[i].line, {&oldLayout[i], nullptr}});
                continue;
            }
            else{
                labelsInLine[oldLayout[i].line].second = &oldLayout[i];
            }
        }

        for (const auto& [line, lineLabels] : labelsInLine)
        {
            if (lineLabels.second == nullptr){
                labelsOrganized.insert({lineLabels.first, {true, true}});
            }
            else{
                bool isFirstOnLeft = lineLabels.first->position == 0;
                
                labelsOrganized.insert({lineLabels.first, {isFirstOnLeft, !isFirstOnLeft}});
                labelsOrganized.insert({lineLabels.second, {!isFirstOnLeft, isFirstOnLeft}});
            }
        }
        
        for (const auto& [label, whichColumn] : labelsOrganized)
        {
            DTLabelInfo labelInfo{};

            labelInfo.layer = label->line;
            labelInfo.textColor = label->color;
            labelInfo.labelColor = label->color;
            labelInfo.font = StatsManager::getFont(label->font);
            labelInfo.labelName = label->labelName;
            labelInfo.scale = label->fontSize;
            labelInfo.text = label->text;

            if (!whichColumn.first && whichColumn.second){
                labelInfo.minPlacementRange = 1;
                labelInfo.maxPlacementRange = 1;
            }
            else if (whichColumn.first && whichColumn.second){
                labelInfo.maxPlacementRange = 1;
            }   

            v3Layout.labels.push_back(labelInfo);
        }
        
        Save::setLayout(v3Layout);
        Save::setOldLayout({});
    }

    if (Save::getLayout().isEmpty()){
        //Save::setLayout(defaultLayout);

        //put on some default layout (will make later)

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