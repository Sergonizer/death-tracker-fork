#include <nodes/optionNodes/SaveOptions.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/layers/DTLevelSpecificSettingsLayer.hpp>
#include <utils/Dev.hpp>

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
    deleteBtn->setPosition({deleteBtn->getContentWidth() / 2, deleteBtn->getContentHeight() / 2});
    this->addChild(deleteBtn);

    auto deleteBtnLabel = CCLabelBMFont::create("Delete Save", "bigFont.fnt");
    deleteBtnLabel->setScale(.6f);
    deleteBtnLabel->setWidth(size.width / 2 - deleteBtn->getContentWidth());
    deleteBtnLabel->setAnchorPoint({0, .5f});
    deleteBtnLabel->setPosition(deleteBtn->getPosition() + ccp(deleteBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(deleteBtnLabel);

    auto backupBtnSpr = CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
    backupBtnSpr->setScale(.7f);
    auto backupBtn = CCMenuItemSpriteExtra::create(
        backupBtnSpr,
        this,
        menu_selector(SaveOptions::onBackup)
    );
    backupBtn->setPosition({size.width / 2 + backupBtn->getContentWidth() / 2, size.height - backupBtn->getContentHeight() / 2});
    this->addChild(backupBtn);

    auto backupBtnLabel = CCLabelBMFont::create("Create backup", "bigFont.fnt");
    backupBtnLabel->setScale(.6f);
    backupBtnLabel->setWidth(size.width / 2 - backupBtn->getContentWidth());
    backupBtnLabel->setAnchorPoint({0, .5f});
    backupBtnLabel->setPosition(backupBtn->getPosition() + ccp(backupBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(backupBtnLabel);

    auto autoBackupToggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        .6f,
        DTLayer::get()->m_MyLevelStats.isOk() ? DTLayer::get()->m_MyLevelStats.unwrap().metadata.autoBackup : false
    );
    autoBackupToggler->setCallback([&](bool toggled){
        if (DTLayer::get()->m_MyLevelStats.isErr()) return;

        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();
        stats.metadata.autoBackup = toggled;
        auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);
    });
    autoBackupToggler->setPosition(backupBtn->getPosition() - ccp(0, backupBtn->getContentWidth() / 2 + autoBackupToggler->getContentWidth() / 2 + 5));
    this->addChild(autoBackupToggler);

    auto autoBackupsSettingsBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    autoBackupsSettingsBtnSpr->setScale(.5f);
    auto autoBackupsSettingsBtn = CCMenuItemSpriteExtra::create(
        autoBackupsSettingsBtnSpr,
        this,
        menu_selector(SaveOptions::onAutoBackupsSettings)
    );
    autoBackupsSettingsBtn->setPosition(autoBackupToggler->getPosition() - ccp(autoBackupToggler->getContentWidth() / 2 + autoBackupsSettingsBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(autoBackupsSettingsBtn);

    auto autoBackupsLabel = CCLabelBMFont::create("Auto backups", "bigFont.fnt");
    autoBackupsLabel->setScale(.6f);
    autoBackupsLabel->setWidth(size.width / 2 - autoBackupToggler->getContentWidth());
    autoBackupsLabel->setAnchorPoint({0, .5f});
    autoBackupsLabel->setPosition(autoBackupToggler->getPosition() + ccp(autoBackupToggler->getContentWidth() / 2 + 5, 0));
    this->addChild(autoBackupsLabel);

    auto maxBackupsLabel = CCLabelBMFont::create("Max allowed backups", "bigFont.fnt");
    maxBackupsLabel->setScale(.6f);
    maxBackupsLabel->setWidth(size.width / 2);
    maxBackupsLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    maxBackupsLabel->setPosition({size.width - size.width / 4, autoBackupToggler->getPositionY() - autoBackupToggler->getContentHeight() / 2 - maxBackupsLabel->getContentHeight() / 2});
    this->addChild(maxBackupsLabel);

    maxBackupsInput = TextInput::create(75, "amount", "gjFont17.fnt");
    maxBackupsInput->setPosition(maxBackupsLabel->getPosition() - ccp(0, maxBackupsLabel->getContentHeight() / 2 + maxBackupsInput->getContentHeight() / 2));
    maxBackupsInput->setWidth(75);
    maxBackupsInput->setCommonFilter(CommonFilter::Uint);
    maxBackupsInput->setString(
        (DTLayer::get()->m_MyLevelStats.isOk() ? 
            (DTLayer::get()->m_MyLevelStats.unwrap().metadata.maxBackupsAmount.has_value() ? 
                std::to_string(DTLayer::get()->m_MyLevelStats.unwrap().metadata.maxBackupsAmount.value())
                : "Unlimited"
            ) 
            : "0"
        ).c_str()
    );
    this->addChild(maxBackupsInput);

    ultimitedBackupsToggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        .6f,
        !DTLayer::get()->m_MyLevelStats.isErr() && !DTLayer::get()->m_MyLevelStats.unwrap().metadata.maxBackupsAmount.has_value()
    );
    ultimitedBackupsToggler->setPosition(maxBackupsInput->getPosition() + ccp(maxBackupsInput->getContentWidth() / 2 + ultimitedBackupsToggler->getContentWidth() / 2 + 5, 0));
    maxBackupsInput->setPositionX(maxBackupsInput->getPositionX() - ultimitedBackupsToggler->getContentWidth() / 2);
    ultimitedBackupsToggler->setPositionX(ultimitedBackupsToggler->getPositionX() - ultimitedBackupsToggler->getContentWidth() / 2);
    ultimitedBackupsToggler->setCallback([&](bool toggled){
        if (DTLayer::get()->m_MyLevelStats.isErr()) return;

        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();
        if (toggled){
            stats.metadata.maxBackupsAmount = std::nullopt;
            maxBackupsInput->setString("Unlimited");
            maxBackupsInput->setEnabled(false);

            auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);
        } else {
            maxBackupsInput->setString("2", true);
            maxBackupsInput->setEnabled(true);
        }
    });
    this->addChild(ultimitedBackupsToggler);

    maxBackupsInput->setCallback([&](auto newstr){
        if (DTLayer::get()->m_MyLevelStats.isErr()) return;

        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();

        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();
        stats.metadata.maxBackupsAmount = num;

        auto _ = StatsManager::setMetadata(stats.metadata, stats.levelKey);
    });

    if (maxBackupsInput->getString() == "Unlimited") maxBackupsInput->setEnabled(false);


    auto backupsScrollLabel = CCLabelBMFont::create("Backups", "bigFont.fnt");
    backupsScrollLabel->setScale(.6f);
    backupsScrollLabel->setWidth(size.width / 2);
    backupsScrollLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    backupsScrollLabel->setPosition({maxBackupsLabel->getPositionX(), maxBackupsInput->getPositionY() - maxBackupsInput->getContentHeight() / 2 - backupsScrollLabel->getContentHeight() / 2 - 5});
    this->addChild(backupsScrollLabel);

    auto backupsScrollLayer = ScrollLayer::create({size.width / 2 - 10, backupsScrollLabel->getPositionY() - backupsScrollLabel->getContentHeight() / 2 - 5});
    backupsScrollLayer->setPositionX(backupsScrollLabel->getPositionX());
    backupsScrollLayer->setPositionY(5 + backupsScrollLayer->getContentHeight() / 2);
    backupsScrollLayer->ignoreAnchorPointForPosition(false);
    this->addChild(backupsScrollLayer);

    this->setEnabled(false);
    this->setOpacity(0);

    Dev::fadeTextInput(maxBackupsInput, false, 0);

    return true;
}

void SaveOptions::onOpened(){
    float fadeTime = .2f;
    this->setEnabled(true);
    this->runAction(CCFadeTo::create(fadeTime, 255));

    Dev::fadeTextInput(maxBackupsInput, true, fadeTime);
}
void SaveOptions::onClosed(){
    float fadeTime = .2f;
    this->setEnabled(false);
    this->runAction(CCFadeTo::create(fadeTime, 0));

    Dev::fadeTextInput(maxBackupsInput, false, fadeTime);
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

void SaveOptions::onAutoBackupsSettings(CCObject*){

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

