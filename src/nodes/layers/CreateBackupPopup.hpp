#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CreateBackupPopup : public Popup<> {
    protected:
        bool setup() override;

    public:
        static CreateBackupPopup* create();

        void setCallback(const std::function<void(bool, std::optional<int>)>& callback) {
            this->callback = callback;
        }

    private:
        std::function<void(bool, std::optional<int>)> callback = NULL;

        std::optional<int> sessions = -1;
        bool general = true;

        void createBackupClicked(cocos2d::CCObject* sender);
};