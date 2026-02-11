#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>
#include <nodes/SimpleToggler.hpp>
#include <nodes/BackupCell.hpp>

using namespace geode::prelude;

class SaveOptions : public OptionsNode, public FLAlertLayerProtocol {
    public:
        static SaveOptions* create(const CCSize& size);
    private:
        virtual bool setup() override;

        void onDelete(CCObject*);
        void onBackup(CCObject*);
        void onAutoBackupsSettings(CCObject*);
        void onExport(CCObject*);
        void onImport(CCObject*);

        void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

        void createChoiceAlert(const std::string& title, const std::string& desc, const std::string& btn1, const std::string& btn2, geode::Function<void(bool)> callback);
        std::map<FLAlertLayer*, geode::Function<void(bool)>> choiceAlertsMap{};

        SimpleToggler* ultimitedBackupsToggler;
        TextInput* maxBackupsInput;
        ScrollLayer* backupsScrollLayer;

        void onBackupRevert(BackupCell* cell);
        void onBackupDelete(BackupCell* cell);

        void updateBackupsList();

    public:
        virtual void onOpened() override;
        virtual void onClosed() override;
};