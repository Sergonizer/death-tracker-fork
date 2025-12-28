#include <nodes/optionNodes/SaveOptions.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/layers/DTLevelSpecificSettingsLayer.hpp>
#include <utils/Dev.hpp>
#include <nodes/layers/CreateBackupPopup.hpp>

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
    deleteBtn->setPosition({deleteBtn->getContentWidth() / 2 + 5, deleteBtn->getContentHeight() / 2 + 5});
    this->addChild(deleteBtn);

    auto deleteBtnLabel = CCLabelBMFont::create("Delete Save", "bigFont.fnt");
    deleteBtnLabel->setScale(.45f);
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

    auto backupBtnLabel = CCLabelBMFont::create("Create Backup", "bigFont.fnt");
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

    auto autoBackupsLabel = CCLabelBMFont::create("Auto Backups", "bigFont.fnt");
    autoBackupsLabel->setScale(.6f);
    autoBackupsLabel->setWidth(size.width / 2 - autoBackupToggler->getContentWidth());
    autoBackupsLabel->setAnchorPoint({0, .5f});
    autoBackupsLabel->setPosition(autoBackupToggler->getPosition() + ccp(autoBackupToggler->getContentWidth() / 2 + 5, 0));
    this->addChild(autoBackupsLabel);

    auto maxBackupsLabel = CCLabelBMFont::create("Max Allowed Backups", "bigFont.fnt");
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


    auto backupsScrollLabel = CCLabelBMFont::create("Backups", "bigFont.fnt");
    backupsScrollLabel->setScale(.6f);
    backupsScrollLabel->setWidth(size.width / 2);
    backupsScrollLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    backupsScrollLabel->setPosition({maxBackupsLabel->getPositionX(), maxBackupsInput->getPositionY() - maxBackupsInput->getContentHeight() / 2 - backupsScrollLabel->getContentHeight() / 2 - 5});
    this->addChild(backupsScrollLabel);

    backupsScrollLayer = ScrollLayer::create({size.width / 2 - 10, backupsScrollLabel->getPositionY() - backupsScrollLabel->getContentHeight() / 2 - 5});
    backupsScrollLayer->setPositionX(backupsScrollLabel->getPositionX());
    backupsScrollLayer->setPositionY(5 + backupsScrollLayer->getContentHeight() / 2);
    backupsScrollLayer->ignoreAnchorPointForPosition(false);
    auto backupsScrollBG = CCScale9Sprite::create("square02_small.png");
    backupsScrollBG->setContentSize(backupsScrollLayer->getContentSize());
    backupsScrollBG->setZOrder(-1);
    backupsScrollBG->ignoreAnchorPointForPosition(true);
    backupsScrollBG->setID("bg");
    backupsScrollLayer->addChild(backupsScrollBG);
    backupsScrollLayer->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(backupsScrollLayer->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisOverflow(false)
    );
    this->addChild(backupsScrollLayer);

    updateBackupsList();

    auto exportBtnSpr = CCSprite::createWithSpriteFrameName("GJ_shareBtn_001.png");
    exportBtnSpr->setScale(0.5f);
    auto exportBtn = CCMenuItemSpriteExtra::create(
        exportBtnSpr,
        this,
        menu_selector(SaveOptions::onExport)
    );
    exportBtn->setPosition({exportBtn->getContentWidth() / 2 + 5, size.height - exportBtn->getContentHeight() / 2});
    this->addChild(exportBtn);

    auto exportBtnLabel = CCLabelBMFont::create("Export Save", "bigFont.fnt");
    exportBtnLabel->setScale(.6f);
    exportBtnLabel->setWidth(size.width / 2 - exportBtn->getContentWidth());
    exportBtnLabel->setAnchorPoint({0, .5f});
    exportBtnLabel->setPosition(exportBtn->getPosition() + ccp(exportBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(exportBtnLabel);

    auto importBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    // @geode-ignore(unknown-resource)
    auto importBtnSpr1 = CCSprite::createWithSpriteFrameName("geode.loader/install.png");
    importBtnSpr1->setPosition(importBtnSpr->getContentSize() / 2);
    importBtnSpr1->setScale(1.2f);
    importBtnSpr->addChild(importBtnSpr1);
    importBtnSpr->setCascadeOpacityEnabled(true);
    importBtnSpr->setScale(0.75f);
    auto importBtn = CCMenuItemSpriteExtra::create(
        importBtnSpr,
        this,
        menu_selector(SaveOptions::onImport)
    );
    importBtn->setPosition(exportBtn->getPosition() - ccp(0, exportBtn->getContentHeight() / 2 + importBtn->getContentHeight() / 2 + 5));
    this->addChild(importBtn);

    auto importBtnLabel = CCLabelBMFont::create("Import Save", "bigFont.fnt");
    importBtnLabel->setScale(.6f);
    importBtnLabel->setWidth(size.width / 2 - importBtn->getContentWidth());
    importBtnLabel->setAnchorPoint({0, .5f});
    importBtnLabel->setPosition(importBtn->getPosition() + ccp(importBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(importBtnLabel);

    this->setEnabled(false);
    this->setOpacity(0);
    backupsScrollBG->setOpacity(0);
    for (const auto& child : CCArrayExt<BackupCell*>(backupsScrollLayer->m_contentLayer->getChildren())){
        child->setOpacity(0);
        child->setEnabled(false);
    }
    
    Dev::fadeTextInput(maxBackupsInput, false, 0);

    backupsScrollLayer->setMouseEnabled(false);

    return true;
}

void SaveOptions::onOpened(){
    float fadeTime = .2f;
    this->setEnabled(true);
    this->runAction(CCFadeTo::create(fadeTime, 255));
    backupsScrollLayer->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 100));
    for (const auto& child : CCArrayExt<BackupCell*>(backupsScrollLayer->m_contentLayer->getChildren())){
        child->runAction(CCFadeTo::create(fadeTime, 255));
        child->setEnabled(true);
    }

    backupsScrollLayer->setMouseEnabled(true);

    if (maxBackupsInput->getString() == "Unlimited") maxBackupsInput->setEnabled(false);
    else maxBackupsInput->setEnabled(true);

    Dev::fadeTextInput(maxBackupsInput, true, fadeTime, false);
}
void SaveOptions::onClosed(){
    float fadeTime = .2f;
    this->setEnabled(false);
    this->runAction(CCFadeTo::create(fadeTime, 0));
    backupsScrollLayer->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 0));
    for (const auto& child : CCArrayExt<BackupCell*>(backupsScrollLayer->m_contentLayer->getChildren())){
        child->runAction(CCFadeTo::create(fadeTime, 0));
        child->setEnabled(false);
    }

    backupsScrollLayer->setMouseEnabled(false);

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

    auto popup = CreateBackupPopup::create();
    popup->setCallback([this](bool general, std::optional<int> sessions) {
        auto _ = StatsManager::addBackup(DTLayer::get()->m_MyLevelStats.unwrap().levelKey, general, sessions);
        if (_.isErr()) {
            log::error("{}", _.unwrapErr());
            Notification::create("Failed to create backup!", NotificationIcon::Error)->show();
        }
        else Notification::create("Created backup successfully!", NotificationIcon::Success)->show();
        
        updateBackupsList();
    });

    popup->show();
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

void SaveOptions::onBackupRevert(BackupCell* cell){
    if (DTLayer::get()->m_MyLevelStats.isErr()) return;

    if (cell->getBackupData() == nullptr) return;

    createChoiceAlert("WARNING!", "Reverting to this backup will overwrite your current death tracker save for this level.\nAre you sure you want to do this?", "No", "Yes", [&, cell](bool btn2){
        if (btn2){
            Result<> generalRes = Ok();
            Result<> sessionsRes = Ok();

            if (cell->getBackupData()->from0.has_value())
                generalRes = StatsManager::setGeneral(cell->getBackupData()->from0.value(), cell->getLevelKey());
            if (cell->getBackupData()->sessionNames.has_value()){
                sessionsRes = StatsManager::reveretBackupSessions(cell->getLevelKey(), cell->getBackupTime());
            }

            if (generalRes.isErr() || sessionsRes.isErr()){
                if (generalRes.isErr())
                    log::error("{}", generalRes.unwrapErr());
                if (sessionsRes.isErr())
                    log::error("{}", sessionsRes.unwrapErr());

                geode::Notification::create("Failed to revert to backup!", NotificationIcon::Error)->show();
            }
            else geode::Notification::create("Successfully reverted to backup!", NotificationIcon::Success)->show();

            DTLayer::get()->onClose(nullptr);
            settingsLayer->keyBackClicked();
        }
    });
}
void SaveOptions::onBackupDelete(BackupCell* cell){
    if (DTLayer::get()->m_MyLevelStats.isErr()) return;

    createChoiceAlert("WARNING!", "Deleting this backup is irreversible.\nAre you sure you want to do this?", "No", "Yes", [&, cell](bool btn2){
        if (btn2){
            auto _ = StatsManager::deleteBackup(cell->getLevelKey(), cell->getBackupTime());
            if (_.isErr()){
                log::error("{}", _.unwrapErr());
                return;
            }

            updateBackupsList();
        }
    });
}

void SaveOptions::updateBackupsList(){
    backupsScrollLayer->m_contentLayer->removeAllChildren();

    if (DTLayer::get()->m_MyLevelStats.isOk()){
        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();

        auto backupsList = StatsManager::getBackupsCount(stats.levelKey);
        int z = 0;
        for (const auto& backupTime : backupsList){
            auto cell = BackupCell::create(backupsScrollLayer->getContentWidth() - 5, stats.levelKey, backupTime);
            cell->onRevertCallback = std::bind(&SaveOptions::onBackupRevert, this, std::placeholders::_1);
            cell->onDeleteCallback = std::bind(&SaveOptions::onBackupDelete, this, std::placeholders::_1);
            backupsScrollLayer->m_contentLayer->addChild(cell);

            if (!this->isEnabled()){
                cell->setOpacity(0);
                cell->setEnabled(false);
            }
            cell->setZOrder(z++);
        }
    }

    backupsScrollLayer->m_contentLayer->updateLayout();
    backupsScrollLayer->moveToTop();
}

void SaveOptions::onExport(CCObject*){

}

void SaveOptions::onImport(CCObject*){

}