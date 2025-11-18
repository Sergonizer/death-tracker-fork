#include <Geode/Geode.hpp>
using namespace geode::prelude;
#include "utils/Save.hpp"
#include "managers/StatsManager.hpp"
#include "utils/Settings.hpp"

$execute {
    auto _ = file::createDirectory(Mod::get()->getSaveDir() / "levels");

    if (Save::getLayout().isEmpty()){
        Save::setOldLayout({});

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
