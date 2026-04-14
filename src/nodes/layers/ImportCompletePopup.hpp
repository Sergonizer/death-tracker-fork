#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class ImportCompletePopup : public Popup {
    protected:
        bool init(LevelData&& data, std::vector<Session> const& sessions);

    public:
        static ImportCompletePopup* create(LevelData&& data, std::vector<Session> const& sessions);

    private:
        LevelData data;
        std::vector<Session> sessions;

        void update(float dt);

        ScrollLayer* generalView;
};