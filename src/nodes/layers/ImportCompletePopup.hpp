#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class ImportCompletePopup : public Popup {
    protected:
        bool init(LevelData data, std::vector<Session> const& sessions);

    public:
        static ImportCompletePopup* create(LevelData const& data, std::vector<Session> const& sessions);

    private:
        LevelData data;
        std::vector<Session> sessions;
};