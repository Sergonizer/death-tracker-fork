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

typedef struct PlaytimePair {
    uint64_t playtimeF0;
    uint64_t playtimeRuns;

    PlaytimePair& operator+=(const PlaytimePair& other) {
        playtimeF0 += other.playtimeF0;
        playtimeRuns += other.playtimeRuns;
        return *this;
    }
    
} PlaytimePair;

template <>
struct matjson::Serialize<PlaytimePair> {
    static Result<PlaytimePair> fromJson(const matjson::Value& value) {
        PlaytimePair pair;
        GEODE_UNWRAP_INTO(pair.playtimeF0, value["playtimeF0"].as<uint64_t>());
        GEODE_UNWRAP_INTO(pair.playtimeRuns, value["playtimeRuns"].as<uint64_t>());

        return Ok(pair);
    }

    static matjson::Value toJson(const PlaytimePair& value) {
        matjson::Value obj = matjson::makeObject({
            { "playtimeF0", value.playtimeF0 },
            { "playtimeRuns", value.playtimeRuns }
        });
        return obj;
    }
};

typedef struct GeneralData {
    Deaths deaths;
    Deaths runs;
    NewBests newBests;
    int currentBest;
    PlaytimePair playtimeGeneral;
    PlaytimePair playtimePaused;
    PlaytimePair playtimeDead;


    GeneralData& operator+=(const GeneralData& other) {
        for (auto& [key, val] : other.deaths) {
            deaths[key] += val;
        }
        for (auto& [key, val] : other.runs) {
            runs[key] += val;
        }
        newBests.insert(other.newBests.begin(), other.newBests.end());
        currentBest = other.currentBest > currentBest ? other.currentBest : currentBest;
        playtimeGeneral += other.playtimeGeneral;
        playtimePaused += other.playtimePaused;
        playtimeDead += other.playtimeDead;
        return *this;
    }
} GeneralData;

template <>
struct matjson::Serialize<GeneralData> {
    static Result<GeneralData> fromJson(const matjson::Value& value) {

        GeneralData stats;
        GEODE_UNWRAP_INTO(stats.deaths, value["deaths"].as<Deaths>());
        GEODE_UNWRAP_INTO(stats.runs, value["runs"].as<Deaths>());
        GEODE_UNWRAP_INTO(stats.newBests, value["newBests"].as<NewBests>());
        GEODE_UNWRAP_INTO(stats.currentBest, value["currentBest"].asInt());
        if (value.contains("playtimeGeneral")) {
            GEODE_UNWRAP_INTO(stats.playtimeGeneral, value["playtimeGeneral"].as<PlaytimePair>());
        }
        if (value.contains("playtimePaused")) {
            GEODE_UNWRAP_INTO(stats.playtimePaused, value["playtimePaused"].as<PlaytimePair>());
        }
        if (value.contains("playtimeDead")) {
            GEODE_UNWRAP_INTO(stats.playtimeDead, value["playtimeDead"].as<PlaytimePair>());
        }

        return Ok(stats);
    }

    static matjson::Value toJson(const GeneralData& value) {
        matjson::Value obj = matjson::makeObject({
            { "deaths", value.deaths },
            { "runs", value.runs },
            { "newBests", value.newBests },
            { "currentBest", value.currentBest },
            { "playtimeGeneral", value.playtimeGeneral },
            { "playtimePaused", value.playtimePaused },
            { "playtimeDead", value.playtimeDead }
        });
        return obj;
    }
};

typedef struct {
    std::string ownerLevelKey;
    long long lastPlayed;
    long long sessionStartDate;
    long long groupID;
    GeneralData data;
} Session;

template <>
struct matjson::Serialize<Session> {
    static Result<Session> fromJson(const matjson::Value& value) {
        Session session;
        GEODE_UNWRAP_INTO(session.lastPlayed, value["lastPlayed"].as<long long>());
        GEODE_UNWRAP_INTO(session.sessionStartDate, value["sessionStartDate"].as<long long>());
        
        if (value.contains("currentBest")) {
            GEODE_UNWRAP_INTO(session.data.currentBest, value["currentBest"].asInt());
        }
        if (value.contains("newBests")) {
            GEODE_UNWRAP_INTO(session.data.newBests, value["newBests"].as<NewBests>());
        }
        if (value.contains("deaths")) {
            GEODE_UNWRAP_INTO(session.data.deaths, value["deaths"].as<Deaths>());
        }
        if (value.contains("runs")) {
            GEODE_UNWRAP_INTO(session.data.runs, value["runs"].as<Deaths>());
        }

        if (value.contains("playtimeGeneral")) {
            GEODE_UNWRAP_INTO(session.data.playtimeGeneral, value["playtimeGeneral"].as<PlaytimePair>());
        }
        if (value.contains("playtimePaused")) {
            GEODE_UNWRAP_INTO(session.data.playtimePaused, value["playtimePaused"].as<PlaytimePair>());
        }
        if (value.contains("playtimeDead")) {
            GEODE_UNWRAP_INTO(session.data.playtimeDead, value["playtimeDead"].as<PlaytimePair>());
        }

        if (value.contains("data")){
            GEODE_UNWRAP_INTO(session.data, value["data"].as<GeneralData>());
        }

        return Ok(session);
    }

    static matjson::Value toJson(const Session& value) {
        matjson::Value obj = matjson::makeObject({
            { "lastPlayed", value.lastPlayed },
            { "sessionStartDate", value.sessionStartDate },
            { "data", value.data }
        });
        return obj;
    }
};

typedef struct Section {
    std::string name;
    int startPercent;
    int endPercent;

    bool isPercentInSection(int percent) const {
        return percent >= startPercent && percent < endPercent ||
            percent >= startPercent && endPercent == 100;
    }

    bool isValid() const {
        return !(startPercent > endPercent);
    }

    bool operator==(const Section& other) const {
        return startPercent == other.startPercent && endPercent == other.endPercent;
    }

} Section;

template <>
struct matjson::Serialize<Section> {
    static Result<Section> fromJson(const matjson::Value& value) {
        Section section;

        GEODE_UNWRAP_INTO(section.name, value["name"].asString());
        GEODE_UNWRAP_INTO(section.startPercent, value["start"].asInt());
        GEODE_UNWRAP_INTO(section.endPercent, value["end"].asInt());

        return Ok(section);
    }

    static matjson::Value toJson(const Section& value) {
        matjson::Value obj = matjson::makeObject({
            { "name", value.name },
            { "start", value.startPercent },
            { "end", value.endPercent },
        });
        return obj;
    }
};

typedef struct SessionGrouping {
    std::map<long long, std::set<std::string>, std::greater<long long>> group;
} SessionGrouping;

template <>
struct matjson::Serialize<std::map<long long, std::set<std::string>, std::greater<long long>>> {
    static Result<std::map<long long, std::set<std::string>, std::greater<long long>>> fromJson(const matjson::Value& value) {

        std::map<long long, std::set<std::string>, std::greater<long long>> resMap;
        auto objRes = value.as<std::map<std::string, std::set<std::string>>>().unwrapOr(std::map<std::string, std::set<std::string>>{});
        for (auto const& [strKey, strSet] : objRes) {
            auto res = geode::utils::numFromString<long long>(strKey);
            if (res.isErr()) continue;
            resMap.insert({res.unwrap(), strSet});
        }

        return Ok(resMap);
    }

    static matjson::Value toJson(const std::map<long long, std::set<std::string>, std::greater<long long>>& value) {
        std::map<std::string, std::set<std::string>> obj;
        for (auto const& kv : value) {
            obj[std::to_string(kv.first)] = kv.second;
        }
        return matjson::Value(obj);
    }
};

template <>
struct matjson::Serialize<SessionGrouping> {
    static Result<SessionGrouping> fromJson(const matjson::Value& value) {
        SessionGrouping grouping;
        GEODE_UNWRAP_INTO(grouping.group, value["group"].as<std::map<long long, std::set<std::string>, std::greater<long long>>>());

        return Ok(grouping);
    }

    static matjson::Value toJson(const SessionGrouping& value) {
        matjson::Value obj = matjson::makeObject({
            { "group", value.group }
        });
        return obj;
    }
};

template <>
struct matjson::Serialize<std::map<long long, SessionGrouping, std::greater<long long>>> {
    static Result<std::map<long long, SessionGrouping, std::greater<long long>>> fromJson(const matjson::Value& value) {

        std::map<long long, SessionGrouping, std::greater<long long>> resMap;
        auto objRes = value.as<std::map<std::string, SessionGrouping>>().unwrapOr(std::map<std::string, SessionGrouping>{});
        for (auto const& [strKey, sessionGrouping] : objRes) {
            auto res = geode::utils::numFromString<long long>(strKey);
            if (res.isErr()) continue;
            resMap.insert({res.unwrap(), sessionGrouping});
        }

        return Ok(resMap);
    }

    static matjson::Value toJson(const std::map<long long, SessionGrouping, std::greater<long long>>& value) {
        std::map<std::string, SessionGrouping> obj;
        for (auto const& kv : value) {
            obj[std::to_string(kv.first)] = kv.second;
        }
        return matjson::Value(obj);
    }
};

typedef struct SessionCategory {
    std::string groupName;
    std::map<long long, SessionGrouping, std::greater<long long>> grouping;
} SessionCategory;

template <>
struct matjson::Serialize<SessionCategory> {
    static Result<SessionCategory> fromJson(const matjson::Value& value) {
        SessionCategory category;
        GEODE_UNWRAP_INTO(category.groupName, value["groupName"].asString());
        GEODE_UNWRAP_INTO(category.grouping, value["grouping"].as<std::map<long long, SessionGrouping, std::greater<long long>>>());

        return Ok(category);
    }

    static matjson::Value toJson(const SessionCategory& value) {
        matjson::Value obj = matjson::makeObject({
            { "groupName", value.groupName },
            { "grouping", value.grouping }
        });
        return obj;
    }
};


typedef struct LevelMetadeta {
    std::map<int, int> runsToShow{};
    bool showAnyRun = true;
    std::set<std::string> linkedLevels{};
    std::string levelName = "Unknown name";
    int attempts = 0;
    int difficulty = 0;
    int hideUpto = 0;
    int realEndPercent = 100;
    bool resetAsDeath = false;
    bool autoBackup = true;
    std::vector<Section> sections{};
    bool hasGottenDataFromPT = false;
    std::vector<SessionCategory> sessionGroups;
} LevelMetadeta;

template <>
struct matjson::Serialize<std::map<int, int>> {
    static Result<std::map<int, int>> fromJson(const matjson::Value& value) {

        std::map<int, int> resMap;
        auto objRes = value.as<std::map<std::string, int>>().unwrapOr(std::map<std::string, int>{});
        for (auto const& [strFirst, second] : objRes) {
            auto res = geode::utils::numFromString<int>(strFirst);
            if (res.isErr()) continue;
            resMap.insert({res.unwrap(), second});
        }

        return Ok(resMap);
    }

    static matjson::Value toJson(const std::map<int, int>& value) {
        std::map<std::string, int> obj;
        for (auto const& kv : value) {
            obj[std::to_string(kv.first)] = kv.second;
        }
        return matjson::Value(obj);
    }
};

template <>
struct matjson::Serialize<LevelMetadeta> {
    static Result<LevelMetadeta> fromJson(const matjson::Value& value) {

        LevelMetadeta stats;
        if (value.contains("RunsToSave")){
            auto oldRunsToSave = value["RunsToSave"].as<std::set<int>>().unwrapOr(std::set<int>{});

            for (const auto& runToSave : oldRunsToSave)
                stats.runsToShow.insert({runToSave, runToSave});
        }
        else {
            GEODE_UNWRAP_INTO(stats.runsToShow, value["RunsToShow"].as<std::map<int, int>>());
        }
        if (value.contains("trackAnyRun")){
            GEODE_UNWRAP_INTO(stats.showAnyRun, value["trackAnyRun"].asBool());
        }
        else{
            GEODE_UNWRAP_INTO(stats.showAnyRun, value["showAnyRun"].asBool());
        }

        GEODE_UNWRAP_INTO(stats.linkedLevels, value["LinkedLevels"].as<std::set<std::string>>());
        GEODE_UNWRAP_INTO(stats.levelName, value["levelName"].asString());
        GEODE_UNWRAP_INTO(stats.attempts, value["attempts"].asInt());
        GEODE_UNWRAP_INTO(stats.difficulty, value["difficulty"].asInt());
        GEODE_UNWRAP_INTO(stats.hideUpto, value["hideUpto"].asInt());
        GEODE_UNWRAP_INTO(stats.realEndPercent, value["realEndPercent"].asInt());
        GEODE_UNWRAP_INTO(stats.resetAsDeath, value["resetAsDeath"].asBool());
        if (value.contains("autoBackup")){
            GEODE_UNWRAP_INTO(stats.autoBackup, value["autoBackup"].asBool());
        }
        if (value.contains("sections")){
            GEODE_UNWRAP_INTO(stats.sections, value["sections"].as<std::vector<Section>>());
        }
        if (value.contains("hasGottenDataFromPT")){
            GEODE_UNWRAP_INTO(stats.hasGottenDataFromPT, value["hasGottenDataFromPT"].asBool());
        }
        if (value.contains("sessionGroups")){
            GEODE_UNWRAP_INTO(stats.sessionGroups, value["sessionGroups"].as<std::vector<SessionCategory>>());
        }

        return Ok(stats);
    }

    static matjson::Value toJson(const LevelMetadeta& value) {
        matjson::Value obj = matjson::makeObject({
            { "RunsToShow", value.runsToShow },
            { "showAnyRun", value.showAnyRun },
            { "LinkedLevels", value.linkedLevels },
            { "levelName", value.levelName },
            { "attempts", value.attempts },
            { "difficulty", value.difficulty },
            { "hideUpto", value.hideUpto },
            { "realEndPercent", value.realEndPercent },
            { "resetAsDeath", value.resetAsDeath },
            { "autoBackup", value.autoBackup },
            { "sections", value.sections },
            { "hasGottenDataFromPT", value.hasGottenDataFromPT },
            { "sessionGroups", value.sessionGroups }
        });
        return obj;
    }
};

struct LevelData {
    std::string levelKey;
    LevelMetadeta metadata;
    Result<GeneralData> from0 = Err("Not loaded");
    std::set<long long> sessionNames;

    LevelData() = default;
};

struct BackupLevelData {
    std::optional<GeneralData> from0 = std::nullopt;
    std::optional<std::set<long long>> sessionNames = std::nullopt;
    long long backupDate;
};

struct DTLabelInfo_s {
    int minPlacementRange;
    int maxPlacementRange;
    int layer;
    std::string labelName = "new label";
    std::string text = "new label";
    std::string font = "bigFont.fnt";
    CCTextAlignment horizontalAlignment = CCTextAlignment::kCCTextAlignmentCenter;
    ccColor4B labelColor = {170, 170, 170, 255};
    ccColor4B textColor = {255, 255, 255, 255};
    float scale = 1;
    bool isExpanded;
    WrappingMode wrapping = WrappingMode::CUTOFF_WRAP;

    static inline const CCPoint MIN_MAX_SCALE = ccp(0.01f, 2);
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
        GEODE_UNWRAP_INTO(info.labelColor, value["labelColor"].as<ccColor4B>());
        GEODE_UNWRAP_INTO(info.textColor, value["color"].as<ccColor4B>());
        
        GEODE_UNWRAP_INTO(info.scale, value["scale"].asDouble());
        GEODE_UNWRAP_INTO(info.isExpanded, value["isExpanded"].asBool());
        if (value.contains("wrapping")){
            GEODE_UNWRAP_INTO(auto wrapping, value["wrapping"].asInt());
            info.wrapping = static_cast<WrappingMode>(wrapping);
        }

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
            { "color", value.textColor },
            { "labelColor", value.labelColor },

            { "scale", value.scale },
            { "isExpanded", value.isExpanded },
            { "wrapping", static_cast<int>(value.wrapping) },
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

enum DTGraphCoverage{
    GeneralCover,
    GeneralRunsCover,
    SessionCover,
    SessionRunsCover,
    SectionCover
};

enum DTGraphType{
    Passrate,
    Reachrate
};

struct DTGraphInfo {
    bool isEnabled = true;

    DTGraphCoverage coverage = DTGraphCoverage::GeneralCover;
    DTGraphType type = DTGraphType::Passrate;

    int orderPos;

    float thickness = 1.0f;
    float outlineThickness = 0.75f;
    float pointScale = 0.1f;

    ccColor4B color = {255, 255, 255, 255};
    ccColor4B outlineColor = { 113, 113, 113, 255 };
    ccColor4B pointColor = { 113, 113, 113, 255 };

    std::string name;
};

template <>
struct matjson::Serialize<DTGraphInfo> {
    static Result<DTGraphInfo> fromJson(const matjson::Value& value) {

        DTGraphInfo info;

        GEODE_UNWRAP_INTO(auto coverageInt, value["coverage"].asInt());
        info.coverage = static_cast<DTGraphCoverage>(coverageInt);
        GEODE_UNWRAP_INTO(auto typeInt, value["type"].asInt());
        info.type = static_cast<DTGraphType>(typeInt);
        
        GEODE_UNWRAP_INTO(info.thickness, value["thickness"].asDouble());
        GEODE_UNWRAP_INTO(info.outlineThickness, value["outlineThickness"].asDouble());
        
        GEODE_UNWRAP_INTO(info.color, value["color"].as<ccColor4B>());
        GEODE_UNWRAP_INTO(info.outlineColor, value["outlineColor"].as<ccColor4B>());
        
        GEODE_UNWRAP_INTO(info.pointColor, value["pointColor"].as<ccColor4B>());
        GEODE_UNWRAP_INTO(info.pointScale, value["pointScale"].asDouble());

        GEODE_UNWRAP_INTO(info.name, value["name"].asString());

        GEODE_UNWRAP_INTO(info.orderPos, value["orderPos"].asInt());
        GEODE_UNWRAP_INTO(info.isEnabled, value["isEnabled"].asBool());

        return Ok(info);
    }

    static matjson::Value toJson(const DTGraphInfo& value) {
        matjson::Value obj = matjson::makeObject({
            { "coverage", static_cast<int>(value.coverage) },
            { "type", static_cast<int>(value.type) },
            { "thickness", value.thickness },
            { "outlineThickness", value.outlineThickness },
            { "outlineColor", value.outlineColor },
            { "color", value.color },
            { "pointScale", value.pointScale },
            { "pointColor", value.pointColor },
            { "name", value.name },
            { "orderPos", value.orderPos },
            { "isEnabled", value.isEnabled }
        });
        return obj;
    }
};
