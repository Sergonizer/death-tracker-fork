#pragma once

#include <Geode/Geode.hpp>
#include <matjson/stl_serialize.hpp>

using namespace geode::prelude;

typedef std::map<std::string, int> Deaths;
typedef std::map<std::string, int> Runs;
typedef std::set<int> NewBests;

struct Run_s{
    int start;
    int end;

    Run_s(int _start, int _end) : start(_start), end(_end){}

    Run_s() = default;
};
typedef struct Run_s Run; 

struct DeathInfo_s{
    Run run;
    bool isNewBest;
    int deaths;

    DeathInfo_s(Run _run, bool _isNewBest, int _deaths, float _passrate) : run(_run), isNewBest(_isNewBest), deaths(_deaths){}
    DeathInfo_s(Run _run, int _deaths, float _passrate) : run(_run), deaths(_deaths){}
};
typedef struct DeathInfo_s DeathInfo;

typedef struct {
    long long lastPlayed;
    Deaths deaths;
    Runs runs;
    NewBests newBests;
    int currentBest;
    long long sessionStartDate;
} Session;

struct LevelStats_s {
    Deaths deaths;
    Runs runs;
    NewBests newBests;
    int currentBest;
    std::vector<Session> sessions;
    std::vector<int> RunsToSave;
    bool trackAnyRun;
    std::vector<std::string> LinkedLevels;
    std::string levelName = "Unknown name";
    int attempts;
    int difficulty;
    int hideUpto;
    int hideRunLength;
};
typedef struct LevelStats_s LevelStats;

template <>
struct matjson::Serialize<Session> {
    static Result<Session> fromJson(const matjson::Value& value) {
        Session session;
        GEODE_UNWRAP_INTO(session.lastPlayed, value["lastPlayed"].as<long long>());
        GEODE_UNWRAP_INTO(session.deaths, value["deaths"].as<Deaths>());
        GEODE_UNWRAP_INTO(session.runs, value["runs"].as<Runs>());
        GEODE_UNWRAP_INTO(session.newBests, value["newBests"].as<NewBests>());
        GEODE_UNWRAP_INTO(session.currentBest, value["currentBest"].asInt());
        GEODE_UNWRAP_INTO(session.sessionStartDate, value["sessionStartDate"].as<long long>());

        return Ok(session);
    }

    static matjson::Value toJson(const Session& value) {
        matjson::Value obj = matjson::makeObject({
            { "lastPlayed", value.lastPlayed },
            { "deaths", value.deaths },
            { "runs", value.runs },
            { "newBests", value.newBests },
            { "currentBest", value.currentBest },
            { "sessionStartDate", value.sessionStartDate },
        });
        return obj;
    }
};

template <>
struct matjson::Serialize<LevelStats> {
    static Result<LevelStats> fromJson(const matjson::Value& value) {

        LevelStats stats;
        GEODE_UNWRAP_INTO(stats.deaths, value["deaths"].as<Deaths>());
        GEODE_UNWRAP_INTO(stats.runs, value["runs"].as<Runs>());
        GEODE_UNWRAP_INTO(stats.newBests, value["newBests"].as<NewBests>());
        GEODE_UNWRAP_INTO(stats.currentBest, value["currentBest"].asInt());
        GEODE_UNWRAP_INTO(stats.sessions, value["sessions"].as<std::vector<Session>>());
        GEODE_UNWRAP_INTO(stats.RunsToSave, value["RunsToSave"].as<std::vector<int>>());
        GEODE_UNWRAP_INTO(stats.LinkedLevels, value["LinkedLevels"].as<std::vector<std::string>>());
        GEODE_UNWRAP_INTO(stats.levelName, value["levelName"].asString());
        GEODE_UNWRAP_INTO(stats.attempts, value["attempts"].asInt());
        GEODE_UNWRAP_INTO(stats.difficulty, value["difficulty"].asInt());
        GEODE_UNWRAP_INTO(stats.hideRunLength, value["hideRunLength"].asInt());
        GEODE_UNWRAP_INTO(stats.hideUpto, value["hideUpto"].asInt());

        if (value.contains("trackAnyRun")){
            GEODE_UNWRAP_INTO(stats.trackAnyRun, value["trackAnyRun"].asBool());
        }

        return Ok(stats);
    }

    static matjson::Value toJson(const LevelStats& value) {
        matjson::Value obj = matjson::makeObject({
            { "deaths", value.deaths },
            { "runs", value.runs },
            { "newBests", value.newBests },
            { "currentBest", value.currentBest },
            { "sessions", value.sessions },
            { "RunsToSave", value.RunsToSave },
            { "trackAnyRun", value.trackAnyRun },
            { "LinkedLevels", value.LinkedLevels },
            { "levelName", value.levelName },
            { "attempts", value.attempts },
            { "difficulty", value.difficulty },
            { "hideRunLength", value.hideRunLength },
            { "hideUpto", value.hideUpto },
        });
        return obj;
    }
};

typedef struct {
    std::string name;
    float value;
    std::string type;
} prismSetting;

template <>
struct matjson::Serialize<prismSetting> {
    static Result<prismSetting> fromJson(const matjson::Value& value) {
        prismSetting setting;
        GEODE_UNWRAP_INTO(setting.name, value["name"].asString());
        setting.value = 0;
        GEODE_UNWRAP_INTO(setting.type, value["type"].asString());

        if (value["value"].isBool()){
            GEODE_UNWRAP_INTO(auto valRes, value["value"].asBool());
            setting.value = valRes ? 1 : 0;
        }
        else{
            GEODE_UNWRAP_INTO(setting.value, value["value"].asDouble());
        }

        return Ok(setting);
    }
};

struct DTLabelInfo_s {
    int X = -1;
    int Y = -1;

    std::string text = "new label";
    std::string font = "bigFont.fnt";
    CCTextAlignment horizontalAlignment = CCTextAlignment::kCCTextAlignmentCenter;
    CCTextAlignment verticalAlignment = CCTextAlignment::kCCTextAlignmentCenter;
    ccColor4B color = {255, 255, 255, 255};

    float scale = 1;
    CCSize contentSize = {100, 100};
    bool infinityResize;
};
typedef struct DTLabelInfo_s DTLabelInfo;

template <>
struct matjson::Serialize<DTLabelInfo> {
    static Result<DTLabelInfo> fromJson(const matjson::Value& value) {
        DTLabelInfo info;
        GEODE_UNWRAP_INTO(info.X, value["X"].asInt());
        GEODE_UNWRAP_INTO(info.Y, value["Y"].asInt());

        GEODE_UNWRAP_INTO(info.text, value["text"].asString());
        GEODE_UNWRAP_INTO(info.font, value["font"].asString());
        GEODE_UNWRAP_INTO(auto horizontalAlignment, value["horizontalAlignment"].asInt());
        info.horizontalAlignment = static_cast<CCTextAlignment>(horizontalAlignment);
        GEODE_UNWRAP_INTO(auto verticalAlignment, value["verticalAlignment"].asInt());
        info.verticalAlignment = static_cast<CCTextAlignment>(verticalAlignment);
        GEODE_UNWRAP_INTO(info.color, value["color"].as<ccColor4B>());

        GEODE_UNWRAP_INTO(info.scale, value["scale"].asDouble());
        GEODE_UNWRAP_INTO(auto contentWidth, value["contentWidth"].asDouble());
        GEODE_UNWRAP_INTO(auto contentHeight, value["contentHeight"].asDouble());
        info.contentSize = CCSize(contentWidth, contentHeight);
        GEODE_UNWRAP_INTO(info.infinityResize, value["infinityResize"].asBool());

        return Ok(info);
    }

    static matjson::Value toJson(const DTLabelInfo& value) {
        matjson::Value obj = matjson::makeObject({
            { "X", value.X },
            { "Y", value.Y },

            { "text", value.text },
            { "font", value.font },
            { "horizontalAlignment", static_cast<int>(value.horizontalAlignment) },
            { "verticalAlignment", static_cast<int>(value.verticalAlignment) },
            { "color", value.color },

            { "scale", value.scale },
            { "contentWidth", value.contentSize.width },
            { "contentHeight", value.contentSize.height },
            { "infinityResize", value.infinityResize },
        });
        return obj;
    }
};

typedef struct {
    std::string labelName;
    std::string text;
    int line;
    int position;
    ccColor4B color;
    CCTextAlignment alignment;
    int font;
    float fontSize;
} V2LabelLayout;

template <>
struct matjson::Serialize<V2LabelLayout> {
    static Result<V2LabelLayout> fromJson(const matjson::Value& value) {
        V2LabelLayout layout;
        GEODE_UNWRAP_INTO(layout.labelName, value["labelName"].asString());
        GEODE_UNWRAP_INTO(layout.text, value["text"].asString());
        GEODE_UNWRAP_INTO(layout.line, value["line"].asInt());
        GEODE_UNWRAP_INTO(layout.position, value["position"].asInt());
        GEODE_UNWRAP_INTO(layout.color, value["color"].as<ccColor4B>());
        GEODE_UNWRAP_INTO(auto alignment, value["alignment"].asInt());
        layout.alignment = static_cast<CCTextAlignment>(alignment);
        GEODE_UNWRAP_INTO(layout.font, value["font"].asInt());
        GEODE_UNWRAP_INTO(layout.fontSize, value["fontSize"].asDouble());

        return Ok(layout);
    }

    static matjson::Value toJson(const V2LabelLayout& value) {
        matjson::Value obj = matjson::makeObject({
            { "labelName", value.labelName },
            { "text", value.text },
            { "line", value.line },
            { "position", value.position },
            { "color", value.color },
            { "alignment", static_cast<int>(value.alignment) },
            { "font", value.font },
            { "fontSize", value.fontSize }
        });
        return obj;
    }
};

struct ViewState{
    CCPoint pos = ccp(-100000, -100000);
    float zoom = -100000;
};

template <>
struct matjson::Serialize<ViewState> {
    static Result<ViewState> fromJson(const matjson::Value& value) {
        ViewState layout;
        GEODE_UNWRAP_INTO(layout.pos.x, value["posX"].asDouble());
        GEODE_UNWRAP_INTO(layout.pos.y, value["posY"].asDouble());
        GEODE_UNWRAP_INTO(layout.zoom, value["zoom"].asDouble());

        return Ok(layout);
    }

    static matjson::Value toJson(const ViewState& value) {
        matjson::Value obj = matjson::makeObject({
            { "posX", value.pos.x },
            { "posY", value.pos.y },
            { "zoom", value.zoom },
        });
        return obj;
    }
};

