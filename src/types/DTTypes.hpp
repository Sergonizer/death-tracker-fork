#pragma once

#include <Geode/Geode.hpp>
#include <matjson/stl_serialize.hpp>

using namespace geode::prelude;

typedef std::map<std::string, int> Deaths;
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
    std::string ownerLevelKey;
    long long lastPlayed;
    Deaths deaths;
    Deaths runs;
    NewBests newBests;
    int currentBest;
    long long sessionStartDate;
} Session;

template <>
struct matjson::Serialize<Session> {
    static Result<Session> fromJson(const matjson::Value& value) {
        Session session;
        GEODE_UNWRAP_INTO(session.lastPlayed, value["lastPlayed"].as<long long>());
        GEODE_UNWRAP_INTO(session.deaths, value["deaths"].as<Deaths>());
        GEODE_UNWRAP_INTO(session.runs, value["runs"].as<Deaths>());
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

typedef struct {
    Deaths deaths;
    Deaths runs;
    NewBests newBests;
    int currentBest;
} GeneralData;

template <>
struct matjson::Serialize<GeneralData> {
    static Result<GeneralData> fromJson(const matjson::Value& value) {

        GeneralData stats;
        GEODE_UNWRAP_INTO(stats.deaths, value["deaths"].as<Deaths>());
        GEODE_UNWRAP_INTO(stats.runs, value["runs"].as<Deaths>());
        GEODE_UNWRAP_INTO(stats.newBests, value["newBests"].as<NewBests>());
        GEODE_UNWRAP_INTO(stats.currentBest, value["currentBest"].asInt());

        return Ok(stats);
    }

    static matjson::Value toJson(const GeneralData& value) {
        matjson::Value obj = matjson::makeObject({
            { "deaths", value.deaths },
            { "runs", value.runs },
            { "newBests", value.newBests },
            { "currentBest", value.currentBest }
        });
        return obj;
    }
};

typedef struct LevelMetadeta {
    std::set<int> RunsToSave{};
    bool trackAnyRun = true;
    std::set<std::string> LinkedLevels{};
    std::string levelName = "Unknown name";
    int attempts = 0;
    int difficulty = 0;
    int hideUpto = 0;
    int hideRunLength = 0;
    int realEndPercent = 100;
    bool resetAsDeath = false;
} LevelMetadeta;

template <>
struct matjson::Serialize<LevelMetadeta> {
    static Result<LevelMetadeta> fromJson(const matjson::Value& value) {

        LevelMetadeta stats;
        GEODE_UNWRAP_INTO(stats.RunsToSave, value["RunsToSave"].as<std::set<int>>());
        GEODE_UNWRAP_INTO(stats.LinkedLevels, value["LinkedLevels"].as<std::set<std::string>>());
        GEODE_UNWRAP_INTO(stats.levelName, value["levelName"].asString());
        GEODE_UNWRAP_INTO(stats.attempts, value["attempts"].asInt());
        GEODE_UNWRAP_INTO(stats.difficulty, value["difficulty"].asInt());
        GEODE_UNWRAP_INTO(stats.hideRunLength, value["hideRunLength"].asInt());
        GEODE_UNWRAP_INTO(stats.hideUpto, value["hideUpto"].asInt());
        GEODE_UNWRAP_INTO(stats.trackAnyRun, value["trackAnyRun"].asBool());
        GEODE_UNWRAP_INTO(stats.realEndPercent, value["realEndPercent"].asInt());
        GEODE_UNWRAP_INTO(stats.resetAsDeath, value["resetAsDeath"].asBool());

        return Ok(stats);
    }

    static matjson::Value toJson(const LevelMetadeta& value) {
        matjson::Value obj = matjson::makeObject({
            { "RunsToSave", value.RunsToSave },
            { "trackAnyRun", value.trackAnyRun },
            { "LinkedLevels", value.LinkedLevels },
            { "levelName", value.levelName },
            { "attempts", value.attempts },
            { "difficulty", value.difficulty },
            { "hideRunLength", value.hideRunLength },
            { "hideUpto", value.hideUpto },
            { "realEndPercent", value.realEndPercent },
            { "resetAsDeath", value.resetAsDeath },
        });
        return obj;
    }
};

typedef struct {
    std::string levelKey;
    LevelMetadeta metadata;
    GeneralData from0;
    std::set<long long> sessionNames;
} LevelData;

struct DTLabelInfo_s {
    int minPlacementRange;
    int maxPlacementRange;
    int layer;
    std::string labelName = "new label";
    std::string text = "new label";
    std::string font = "bigFont.fnt";
    CCTextAlignment horizontalAlignment = CCTextAlignment::kCCTextAlignmentCenter;
    ccColor4B color = {255, 255, 255, 255};
    float scale = 1;
    bool isExpanded;
};
typedef struct DTLabelInfo_s DTLabelInfo;

template <>
struct matjson::Serialize<DTLabelInfo> {
    static Result<DTLabelInfo> fromJson(const matjson::Value& value) {
        DTLabelInfo info;
        GEODE_UNWRAP_INTO(info.minPlacementRange, value["minPlacementRange"].asInt());
        GEODE_UNWRAP_INTO(info.maxPlacementRange, value["maxPlacementRange"].asInt());
        GEODE_UNWRAP_INTO(info.layer, value["layer"].asInt());
        
        GEODE_UNWRAP_INTO(info.labelName, value["labelName"].asString());
        GEODE_UNWRAP_INTO(info.text, value["text"].asString());
        GEODE_UNWRAP_INTO(info.font, value["font"].asString());
        GEODE_UNWRAP_INTO(auto horizontalAlignment, value["horizontalAlignment"].asInt());
        info.horizontalAlignment = static_cast<CCTextAlignment>(horizontalAlignment);
        GEODE_UNWRAP_INTO(info.color, value["color"].as<ccColor4B>());
        
        GEODE_UNWRAP_INTO(info.scale, value["scale"].asDouble());
        GEODE_UNWRAP_INTO(info.isExpanded, value["isExpanded"].asBool());

        return Ok(info);
    }

    static matjson::Value toJson(const DTLabelInfo& value) {
        matjson::Value obj = matjson::makeObject({
            { "minPlacementRange", value.minPlacementRange },
            { "maxPlacementRange", value.maxPlacementRange },
            { "layer", value.layer },

            { "labelName", value.labelName },
            { "text", value.text },
            { "font", value.font },
            { "horizontalAlignment", static_cast<int>(value.horizontalAlignment) },
            { "color", value.color },

            { "scale", value.scale },
            { "isExpanded", value.isExpanded },
        });
        return obj;
    }
};

struct DTColumnInfo_s {
    static float minWidth;

    int orderPos = 0;
    float currentWidth = minWidth;
    ccColor3B color = {255, 255, 255};
};
typedef struct DTColumnInfo_s DTColumnInfo;

template <>
struct matjson::Serialize<DTColumnInfo> {
    static Result<DTColumnInfo> fromJson(const matjson::Value& value) {
        DTColumnInfo info;
        GEODE_UNWRAP_INTO(info.orderPos, value["orderPos"].asInt());
        GEODE_UNWRAP_INTO(info.currentWidth, value["currentWidth"].asDouble());
        GEODE_UNWRAP_INTO(info.color, value["color"].as<ccColor3B>());

        return Ok(info);
    }

    static matjson::Value toJson(const DTColumnInfo& value) {
        matjson::Value obj = matjson::makeObject({
            { "orderPos", value.orderPos },
            { "currentWidth", value.currentWidth },
            { "color", value.color }
        });
        return obj;
    }
};

struct DTLayoutV3_s {
    std::vector<DTColumnInfo> columns;
    std::vector<DTLabelInfo> labels;

    bool isEmpty() const{
        return !columns.size() && !labels.size();
    }
};
typedef struct DTLayoutV3_s DTLayoutV3;

template <>
struct matjson::Serialize<DTLayoutV3> {
    static Result<DTLayoutV3> fromJson(const matjson::Value& value) {
        DTLayoutV3 info;
        GEODE_UNWRAP_INTO(info.columns, value["columns"].as<std::vector<DTColumnInfo>>());
        GEODE_UNWRAP_INTO(info.labels, value["labels"].as<std::vector<DTLabelInfo>>());

        return Ok(info);
    }

    static matjson::Value toJson(const DTLayoutV3& value) {
        matjson::Value obj = matjson::makeObject({
            { "columns", value.columns },
            { "labels", value.labels }
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

struct stringCustomazations{
    std::string seperator = "{nl}";
    std::string format = "{per}% X{d}";
};

template <>
struct matjson::Serialize<stringCustomazations> {
    static Result<stringCustomazations> fromJson(const matjson::Value& value) {
        stringCustomazations sc;
        GEODE_UNWRAP_INTO(sc.seperator, value["seperator"].asString());
        GEODE_UNWRAP_INTO(sc.format, value["format"].asString());

        return Ok(sc);
    }

    static matjson::Value toJson(const stringCustomazations& value) {
        matjson::Value obj = matjson::makeObject({
            { "seperator", value.seperator },
            { "format", value.format }
        });
        return obj;
    }
};

typedef struct {
    long long lastPlayed;
    Deaths deaths;
    Deaths runs;
    NewBests newBests;
    int currentBest;
    long long sessionStartDate;
} V2Session;

template <>
struct matjson::Serialize<V2Session> {
    static Result<V2Session> fromJson(const matjson::Value& value) {
        V2Session session;
        GEODE_UNWRAP_INTO(session.lastPlayed, value["lastPlayed"].as<long long>());
        GEODE_UNWRAP_INTO(session.deaths, value["deaths"].as<Deaths>());
        GEODE_UNWRAP_INTO(session.runs, value["runs"].as<Deaths>());
        GEODE_UNWRAP_INTO(session.newBests, value["newBests"].as<NewBests>());
        GEODE_UNWRAP_INTO(session.currentBest, value["currentBest"].asInt());

        if (value.contains("sessionStartDate")){
            GEODE_UNWRAP_INTO(session.sessionStartDate, value["sessionStartDate"].as<long long>());
        }
        else
            session.sessionStartDate = -1;

        return Ok(session);
    }

    static matjson::Value toJson(const V2Session& value) {
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

struct V2LevelStats_s {
    Deaths deaths;
    Deaths runs;
    NewBests newBests;
    int currentBest;
    std::vector<V2Session> sessions;
    std::vector<int> RunsToSave{-1};
    std::vector<std::string> LinkedLevels;
    std::string levelName = "Unknown name";
    int attempts;
    int difficulty;
    int hideUpto;
    int hideRunLength;
};
typedef struct V2LevelStats_s V2LevelStats;

template <>
struct matjson::Serialize<V2LevelStats> {
    static Result<V2LevelStats> fromJson(const matjson::Value& value) {

        V2LevelStats stats;
        GEODE_UNWRAP_INTO(stats.deaths, value["deaths"].as<Deaths>());
        GEODE_UNWRAP_INTO(stats.runs, value["runs"].as<Deaths>());
        GEODE_UNWRAP_INTO(stats.newBests, value["newBests"].as<NewBests>());
        GEODE_UNWRAP_INTO(stats.currentBest, value["currentBest"].asInt());
        GEODE_UNWRAP_INTO(stats.sessions, value["sessions"].as<std::vector<V2Session>>());

        if (value.contains("RunsToSave")){
            GEODE_UNWRAP_INTO(stats.RunsToSave, value["RunsToSave"].as<std::vector<int>>());
        }

        if (value.contains("LinkedLevels")){
            GEODE_UNWRAP_INTO(stats.LinkedLevels, value["LinkedLevels"].as<std::vector<std::string>>());
        }
        else
            stats.LinkedLevels = value["LinkedLevels"].as<std::vector<std::string>>().unwrapOr(std::vector<std::string>{});
        
        if (value.contains("levelName")){
            GEODE_UNWRAP_INTO(stats.levelName, value["levelName"].asString());
        }
        else
            stats.levelName = "-1";

        if (value.contains("attempts")){
            GEODE_UNWRAP_INTO(stats.attempts, value["attempts"].asInt());
        }
        else
            stats.attempts = -1;

        if (value.contains("difficulty")){
            GEODE_UNWRAP_INTO(stats.difficulty, value["difficulty"].asInt());
        }
        else
            stats.difficulty = 0;

        if (value.contains("hideRunLength")){
            GEODE_UNWRAP_INTO(stats.hideRunLength, value["hideRunLength"].asInt());
        }
        else
            stats.hideRunLength = 0;

        if (value.contains("hideUpto")){
            GEODE_UNWRAP_INTO(stats.hideUpto, value["hideUpto"].asInt());
        }
        else
            stats.hideUpto = 0;

        return Ok(stats);
    }

    static matjson::Value toJson(const V2LevelStats& value) {
        matjson::Value obj = matjson::makeObject({
            { "deaths", value.deaths },
            { "runs", value.runs },
            { "newBests", value.newBests },
            { "currentBest", value.currentBest },
            { "sessions", value.sessions },
            { "RunsToSave", value.RunsToSave },
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
