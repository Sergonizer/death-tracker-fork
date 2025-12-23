#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

struct GetBackupTaskResult {
    Result<BackupLevelData> backup;
    int deathCount;
    double backupFileSizeInKB;
};

using GetBackupTask = Task<GetBackupTaskResult>;

class BackupCell : public CCMenu {
    public:
        static BackupCell* create(float width, const std::string& levelKey, const long long backupTime);

        std::function<void(BackupCell*)> onRevertCallback = NULL;
        std::function<void(BackupCell*)> onDeleteCallback = NULL;

        std::string getLevelKey() const {
            return levelKey;
        }
        long long getBackupTime() const {
            return backupTime;
        }
        BackupLevelData* getBackupData() {
            return backupData ? &backupData.value() : nullptr;
        }

    private:
        bool init(float width, const std::string& levelKey, const long long backupTime);

        std::string levelKey;
        long long backupTime;

        void LoadBackupData();
        EventListener<GetBackupTask> getBackupListener;
        void onBackupLoaded(GetBackupTask::Event* event);

        std::optional<BackupLevelData> backupData = std::nullopt;

        CCLabelBMFont* titleLabel;
        CCLabelBMFont* descriptionLabel;

        void onRevert(CCObject* sender);
        void onDelete(CCObject* sender);
};