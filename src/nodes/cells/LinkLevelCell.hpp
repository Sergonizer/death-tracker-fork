#pragma once

#include <Geode/Geode.hpp>
#include "../../managers/StatsManager.hpp"

using namespace geode::prelude;


class LinkLevelCell : public CCNode {
    protected:
        bool init(const float& cellW, const std::string& levelKey, const LevelMetadeta& data, const bool& linked, const std::function<void(const std::string, LevelMetadeta, const bool&)>& callback);
    public:
        static LinkLevelCell* create(const float& cellW, const std::string& levelKey, const LevelMetadeta& data, const bool& linked, const std::function<void(const std::string, LevelMetadeta, const bool&)>& callback = NULL);

    private:
        //runs the given callback to this LinkLevelCell
        void MoveMe(CCObject*);

        std::string m_LevelKey;
        LevelMetadeta m_data;
        bool m_Linked;
        std::function<void(const std::string, LevelMetadeta, const bool&)> m_Callback;
};