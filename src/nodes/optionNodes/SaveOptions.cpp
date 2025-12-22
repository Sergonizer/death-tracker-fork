#include <nodes/optionNodes/SaveOptions.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/layers/DTLevelSpecificSettingsLayer.hpp>

SaveOptions* SaveOptions::create(const CCSize& size) {
    auto ret = new SaveOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SaveOptions::setup(){

    auto deleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSpr,
        this,
        menu_selector(SaveOptions::onDelete)
    );
    this->addChild(deleteBtn);

    auto backupBtnSpr = CCSprite::createWithSpriteFrameName("GJ_savedBtn_001.png");
    auto backupBtn = CCMenuItemSpriteExtra::create(
        backupBtnSpr,
        this,
        menu_selector(SaveOptions::onBackup)
    );
    this->addChild(backupBtn);


    this->setEnabled(false);
    this->setOpacity(0);

    return true;
}

void SaveOptions::onOpened(){
    float fadeTime = .2f;
    this->setEnabled(true);
    this->runAction(CCFadeTo::create(fadeTime, 255));
}
void SaveOptions::onClosed(){
    float fadeTime = .2f;
    this->setEnabled(false);
    this->runAction(CCFadeTo::create(fadeTime, 0));
}

void SaveOptions::onDelete(CCObject*){
    createChoiceAlert("WARNING!", "Doing this will delete ALL saved progress from this levels death tracker save.\nAre you sure you want to do this?", "No", "Yes", [&](bool btn2){
        if (btn2){
            if (DTLayer::get()->DeleteSave())
                settingsLayer->keyBackClicked();
        }
    });
}

void SaveOptions::onBackup(CCObject*){
    if (DTLayer::get()->m_MyLevelStats.isErr()) return;
    auto _ = StatsManager::addBackup(DTLayer::get()->m_MyLevelStats.unwrap().levelKey, true, -1);
    if (_.isErr()) log::error("{}", _.unwrapErr());
}




void SaveOptions::FLAlert_Clicked(FLAlertLayer* layer, bool btn2){
    if (!choiceAlertsMap.contains(layer)) return;

    choiceAlertsMap[layer](btn2);

    choiceAlertsMap.erase(layer);
}

void SaveOptions::createChoiceAlert(const std::string& title, const std::string& desc, const std::string& btn1, const std::string& btn2, const std::function<void(bool)>& callback){
    auto alert = FLAlertLayer::create(this, title.c_str(), desc, btn1.c_str(), btn2.c_str());
    alert->show();
    choiceAlertsMap.insert({alert, callback});
}

