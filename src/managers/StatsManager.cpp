#include "StatsManager.hpp"
#include "../utils/Settings.hpp"
#include "../utils/Dev.hpp"
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace geode::prelude;

std::vector<std::string> StatsManager::splitStr(const std::string& str, const std::string& delim) {
    size_t posStart = 0;
    size_t posEnd;
    size_t delimLen = delim.length();

    std::vector<std::string> res;

    while ((posEnd = str.find(delim, posStart)) != std::string::npos) {
        std::string token = str.substr(posStart, posEnd - posStart);
        posStart = posEnd + delimLen;
        if (!token.empty())
            res.push_back(token);
    }

    res.push_back(str.substr(posStart));
    return res;
}

/* static member variables
=========================== */
std::set<std::string> StatsManager::m_playedLevels{};
bool StatsManager::m_scheduleCreateNewSession = false;

GJGameLevel* StatsManager::currentLoggingLevelRef = nullptr;
Result<LevelStats> StatsManager::currentLoggingLevelStats = Err("No level is currently being logged");

std::filesystem::path StatsManager::m_savesFolderPath = Settings::getSavePath();

int StatsManager::MainLevelIDs[26]{
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 5001, 5002, 5003, 5004, 3001
};

std::array<std::string, 62> StatsManager::m_AllFontsMap{
    "bigFont.fnt",
    "chatFont.fnt",
    "goldFont.fnt",
    "gjFont01.fnt",
    "gjFont02.fnt",
    "gjFont03.fnt",
    "gjFont04.fnt",
    "gjFont05.fnt",
    "gjFont06.fnt",
    "gjFont07.fnt",
    "gjFont08.fnt",
    "gjFont09.fnt",
    "gjFont10.fnt",
    "gjFont11.fnt",
    "gjFont12.fnt",
    "gjFont13.fnt",
    "gjFont14.fnt",
    "gjFont15.fnt",
    "gjFont16.fnt",
    "gjFont17.fnt",
    "gjFont18.fnt",
    "gjFont19.fnt",
    "gjFont20.fnt",
    "gjFont21.fnt",
    "gjFont22.fnt",
    "gjFont23.fnt",
    "gjFont24.fnt",
    "gjFont25.fnt",
    "gjFont26.fnt",
    "gjFont27.fnt",
    "gjFont28.fnt",
    "gjFont29.fnt",
    "gjFont30.fnt",
    "gjFont31.fnt",
    "gjFont32.fnt",
    "gjFont33.fnt",
    "gjFont34.fnt",
    "gjFont35.fnt",
    "gjFont36.fnt",
    "gjFont37.fnt",
    "gjFont38.fnt",
    "gjFont39.fnt",
    "gjFont40.fnt",
    "gjFont41.fnt",
    "gjFont42.fnt",
    "gjFont43.fnt",
    "gjFont44.fnt",
    "gjFont45.fnt",
    "gjFont46.fnt",
    "gjFont47.fnt",
    "gjFont48.fnt",
    "gjFont49.fnt",
    "gjFont50.fnt",
    "gjFont51.fnt",
    "gjFont52.fnt",
    "gjFont53.fnt",
    "gjFont54.fnt",
    "gjFont55.fnt",
    "gjFont56.fnt",
    "gjFont57.fnt",
    "gjFont58.fnt",
    "gjFont59.fnt",
};

/* main functions
================== */

Result<LevelStats> StatsManager::getLevelStats(GJGameLevel* const& level, bool isBackup){
    std::string levelKey = StatsManager::getLevelKey(level).unwrapOr("-1");
    return getLevelStats(levelKey, isBackup);
}

void StatsManager::setCurrentLogLevel(GJGameLevel* const& level){
    currentLoggingLevelRef = level;
    currentLoggingLevelStats = getLevelStats(level, false);
    if (currentLoggingLevelStats.isErr()) currentLoggingLevelStats = Err("No level is currently being logged");
}

void StatsManager::logDeath(const int& percent) {
    if (currentLoggingLevelStats.isErr()) {
        log::error("Failed to log death: {}", currentLoggingLevelStats.unwrapErr());
        return;
    }

    auto loggingLvl = currentLoggingLevelStats.unwrap();

    auto session = StatsManager::getSession();
    if (!session) return;

    auto percentKey = std::to_string(percent);

    loggingLvl.deaths[percentKey]++;
    session->deaths[percentKey]++;

    if (percent > loggingLvl.currentBest) {
        loggingLvl.currentBest = percent;
        loggingLvl.newBests.insert(percent);
    }

    if (percent > session->currentBest) {
        session->currentBest = percent;
        session->newBests.insert(percent);
    }

    StatsManager::updateSessionLastPlayed();
    StatsManager::setLevelStats(loggingLvl, currentLoggingLevelRef, false);
}

void StatsManager::logDeaths(const std::vector<int>& percents) {
    if (currentLoggingLevelStats.isErr()) {
        log::error("Failed to log death: {}", currentLoggingLevelStats.unwrapErr());
        return;
    }

    auto loggingLvl = currentLoggingLevelStats.unwrap();
    
    auto session = StatsManager::getSession();
    if (!session) return;

    for (int i = 0; i < percents.size(); i++)
    {
        auto percentKey = std::to_string(percents[i]);

        loggingLvl.deaths[percentKey]++;
        session->deaths[percentKey]++;

        if (percents[i] > loggingLvl.currentBest) {
            loggingLvl.currentBest = percents[i];
            loggingLvl.newBests.insert(percents[i]);
        }

        if (percents[i] > session->currentBest) {
            session->currentBest = percents[i];
            session->newBests.insert(percents[i]);
        }
    }

    StatsManager::updateSessionLastPlayed();
    StatsManager::setLevelStats(loggingLvl, currentLoggingLevelRef, false);
}

void StatsManager::logRun(const Run& run) {
    if (currentLoggingLevelStats.isErr()) {
        log::error("Failed to log run: {}", currentLoggingLevelStats.unwrapErr());
        return;
    }

    auto loggingLvl = currentLoggingLevelStats.unwrap();

    bool TrackRun = false;
    if (loggingLvl.RunsToSave.size()){
        if (loggingLvl.RunsToSave[0] == -1){
            TrackRun = true;
        }
        else{
            for (int i = 0; i < loggingLvl.RunsToSave.size(); i++)
            {
                if (loggingLvl.RunsToSave[i] == run.start){
                    TrackRun = true;
                    break;
                }
            }
        }
    }

    if (!TrackRun) return;

    auto session = StatsManager::getSession();
    if (!session) return;

    auto runKey = fmt::format("{}-{}",
        run.start,
        run.end
    );

    loggingLvl.runs[runKey]++;
    session->runs[runKey]++;

    StatsManager::updateSessionLastPlayed();
    StatsManager::setLevelStats(loggingLvl, currentLoggingLevelRef, false);
}

void StatsManager::logRuns(const std::vector<Run>& runs) {
    if (currentLoggingLevelStats.isErr()) {
        log::error("Failed to log runs: {}", currentLoggingLevelStats.unwrapErr());
        return;
    }

    auto loggingLvl = currentLoggingLevelStats.unwrap();

    bool TrackRun = false;
    for (int i = 0; i < runs.size(); i++)
    {
        if (loggingLvl.RunsToSave.size()){
            if (loggingLvl.RunsToSave[0] == -1){
                TrackRun = true;
            }
            else{
                for (int i = 0; i < loggingLvl.RunsToSave.size(); i++)
                {
                    if (loggingLvl.RunsToSave[i] == runs[i].start){
                        TrackRun = true;
                        break;
                    }
                }
            }
        }
            
        if (!TrackRun) return;

        auto session = StatsManager::getSession();
        if (!session) return;

        auto runKey = fmt::format("{}-{}",
            runs[i].start,
            runs[i].end
        );

        loggingLvl.runs[runKey]++;
        session->runs[runKey]++;
    }

    StatsManager::updateSessionLastPlayed();
    StatsManager::setLevelStats(loggingLvl, currentLoggingLevelRef, false);
}

/* utility functions
===================== */
long long StatsManager::getNowSeconds() {
    using namespace std::chrono;
    auto now = system_clock::now();
    return time_point_cast<seconds>(now).time_since_epoch().count();
}

Result<std::string> StatsManager::getLevelKey(GJGameLevel* const& level) {
	if (!level) return Err("invalid level!");

	std::string levelId;

    if (level->m_levelType == GJLevelType::Editor){
        levelId += std::to_string(EditorIDs::getID(level));
    }
    else{
        levelId += std::to_string(level->m_levelID.value());
    }

	// local level postfix
	if (level->m_levelType == GJLevelType::Local)
		levelId += "-local";

    if (level->m_levelType == GJLevelType::Editor)
		levelId += "-editor";

	// daily/weekly postfix
	if (level->m_dailyID > 0)
		levelId += "-daily";

	// gauntlet level postfix
	if (level->m_gauntletLevel)
		levelId += "-gauntlet";

	return Ok(levelId);
}

Run StatsManager::splitRunKey(const std::string& runKey) {
    auto runKeySplit = StatsManager::splitStr(runKey, "-");

    auto start = std::stof(runKeySplit[0]);
    auto end = std::stof(runKeySplit[1]);

    Run r;
    r.start = start;
    r.end = end;

    return r;
}

Session* StatsManager::getSession() {
    if (currentLoggingLevelStats.isErr()) {
        log::error("Failed to get session: {}", currentLoggingLevelStats.unwrapErr());
        return nullptr;
    }

    auto loggingLevel = currentLoggingLevelStats.unwrap();

    if (loggingLevel.currentBest == -1) return nullptr;

    auto currentSession = &loggingLevel.sessions[loggingLevel.sessions.size() - 1];

    // new sessions can be scheduled
    // and are created when the player dies
    if (!m_scheduleCreateNewSession) return currentSession;
    m_scheduleCreateNewSession = false;

    auto levelKeyRes = StatsManager::getLevelKey(currentLoggingLevelRef);

    if (levelKeyRes.isErr()) return currentSession;

    // the user has played the level
    // if a new session is created
    m_playedLevels.insert(levelKeyRes.unwrap());

    // create the new session
    auto session = Session {
        .lastPlayed = -1,
        .deaths = {},
        .runs = {},
        .newBests = {},
        .currentBest = -1,
        .sessionStartDate = StatsManager::getNowSeconds()
    };

    loggingLevel.sessions.push_back(session);
    return &loggingLevel.sessions[loggingLevel.sessions.size() - 1];
}

void StatsManager::updateSessionLastPlayed(bool save) {
    auto now = StatsManager::getNowSeconds();
    auto session = StatsManager::getSession();

    session->lastPlayed = now;

    if (save && currentLoggingLevelStats.isOk()) {
        StatsManager::setLevelStats(currentLoggingLevelStats.unwrap(), currentLoggingLevelRef, false);
    }
}

void StatsManager::scheduleCreateNewSession(const bool& scheduled) {
    if (currentLoggingLevelStats.isErr()) {
        log::error("Failed to get session: {}", currentLoggingLevelStats.unwrapErr());
        return;
    }

    auto loggingLevel = currentLoggingLevelStats.unwrap();

    if (loggingLevel.currentBest != -1)
        m_scheduleCreateNewSession = scheduled;
}

bool StatsManager::hasPlayedLevel() {
    if (currentLoggingLevelStats.isErr()) {
        log::error("Failed to check if level has been played: {}", currentLoggingLevelStats.unwrapErr());
        return false;
    }

    auto loggingLevel = currentLoggingLevelStats.unwrap();
    
    auto levelKeyRes = StatsManager::getLevelKey(currentLoggingLevelRef);

    if (levelKeyRes.isErr()) return false;

    return m_playedLevels.contains(levelKeyRes.unwrap());
}

/* internal functions
======================= */

void StatsManager::setLevelStats(const LevelStats& stats, GJGameLevel* const& level, bool isBackup) {
    auto levelKeyRes = StatsManager::getLevelKey(level);
    if (levelKeyRes.isOk())
        setLevelStats(stats, levelKeyRes.unwrap(), isBackup);
}

void StatsManager::setLevelStats(const LevelStats& stats, const std::string& levelKey, bool isBackup) {    
    auto levelSaveFilePath = m_savesFolderPath / (levelKey + (isBackup ? ".deathsBackup" : ".json"));

    // create the json file if it doesnt exist
    if (!std::filesystem::exists(levelSaveFilePath)) {
        std::ofstream levelSaveFile(levelSaveFilePath);
        levelSaveFile.close();
    }

    // save the data
    auto indentation = Dev::MINIFY_SAVE_FILE
        ? matjson::NO_INDENTATION
        : 4;

    auto jsonStr = matjson::Value(stats).dump(indentation);
    auto _ = file::writeString(levelSaveFilePath, jsonStr);
}

Result<LevelStats> StatsManager::getLevelStats(const std::string& levelKey, bool isBackup) {
    auto levelSaveFilePath = m_savesFolderPath / (levelKey + (isBackup ? ".deathsBackup" : ".json"));

    log::info("Getting level stats for: {}, path: {}", levelKey, levelSaveFilePath.string());
    if (std::filesystem::exists(levelSaveFilePath)){
        log::info("Found level stats file: {}", levelSaveFilePath.string());
        GEODE_UNWRAP_INTO(auto json, file::readJson(levelSaveFilePath));
        log::info("Successfully read level stats file: {}", levelSaveFilePath.string());
        
        return json.as<LevelStats>();
    }

    log::info("No level stats file found for: {}, path: {}", levelKey, levelSaveFilePath.string());

    return Err("0 No stats exist for level!");
}

Result<> StatsManager::deleteLevelStats(const std::string& levelKey){
    auto levelSaveFilePath = m_savesFolderPath / (levelKey + ".json");

    if (std::filesystem::exists(levelSaveFilePath)){
        std::filesystem::remove(levelSaveFilePath);
        return Ok();
    }

    return Err("Cant delete stats because level save file does not exist: " + levelSaveFilePath.string());
}

Result<std::tuple<NewBests, int>> StatsManager::calcNewBests(GJGameLevel* const& level) {
    NewBests newBests{};
    std::stringstream bestsStream(level->m_personalBests);
    std::string currentBest;
    int currentPercent = 0;

    while (std::getline(bestsStream, currentBest, ',')) {
        GEODE_UNWRAP_INTO(auto tempPercent, geode::utils::numFromString<int>(currentBest));
        
        currentPercent += tempPercent;
        newBests.insert(currentPercent);
    }

    return Ok(std::make_tuple(newBests, currentPercent));
}

std::string StatsManager::getFont(const int& fontID){
    for (int i = 0; i < m_AllFontsMap.size(); i++)
    {
        if (i == fontID)
            return m_AllFontsMap[i];
    }
    return m_AllFontsMap[1];
}

std::string StatsManager::getFontName(const int& fontID){
    for (int i = 0; i < m_AllFontsMap.size(); i++)
    {
        if (i == fontID){
            std::string fontName;

            if (i == 0)
                fontName = "Big Font";
            else if (i == 1)
                fontName = "Chat Font";
            else if (i == 2)
                fontName = "Gold Font";
            else{
                fontName = "Font " + std::to_string(i - 2);
            }

            return fontName;
        }
    }
    return "Chat Font";
}

std::array<std::string, 62> StatsManager::getAllFonts(){
    return m_AllFontsMap;
}

Result<std::vector<std::pair<std::string, LevelStats>>> StatsManager::getAllLevels(){
    GEODE_UNWRAP_INTO(auto allLevels, file::readDirectory(m_savesFolderPath));

    std::vector<std::pair<std::string, LevelStats>> toReturn{};

    std::vector<std::string> failedKeys{};

    for (int i = 0; i < allLevels.size(); i++)
    {
        if (allLevels[i].extension().string() == ".json"){
            auto currentLevel = StatsManager::getLevelStats(allLevels[i].stem().string(), false);
            if (currentLevel.isErr()){
                Notification::create(fmt::format("failed getting some levels, errors send in logs.", allLevels[i].stem().string(), currentLevel.unwrapErr()), nullptr)->show();
                failedKeys.push_back(allLevels[i].stem().string());
                continue;
            }

            auto stats = currentLevel.unwrap();

            toReturn.push_back(std::make_pair(allLevels[i].stem().string(), stats));
        }
    }

    if (failedKeys.size()){
        log::warn("Failed loading some level! failed level keys are:");
        log::warn("------------");

        for (const auto& failedKey : failedKeys)
            log::warn("- {}", failedKey);
        
        log::warn("------------");
        log::warn("Make sure to check these levels and revert to their backups if possible!");
    }
    
    return Ok(toReturn);
}

std::pair<std::string, std::string> StatsManager::splitLevelKey(const std::string& levelKey){
    auto levelKeySplit = splitStr(levelKey, "-");

    std::pair<std::string, std::string> toReturn;

    toReturn.first = levelKeySplit[0];

    if (levelKeySplit.size() == 2){
        toReturn.second = levelKeySplit[1];
    }
    else{
        toReturn.second = "online";
    }

    return toReturn;
}

int StatsManager::getDifficulty(GJGameLevel* const& level){
    if (level->m_autoLevel)
        return -1;

    if (level->m_ratingsSum != 0)
        if (level->m_demon == 1){
            int fixedNum = level->m_demonDifficulty;

            if (fixedNum != 0)
                fixedNum -= 2;

            return 6 + fixedNum;
        }
        else{
            return level->m_ratingsSum / level->m_ratings;
        }
    else 
        return 0;
}

void StatsManager::setPath(const std::filesystem::path& path){
    m_savesFolderPath = path;
}

void StatsManager::computeLPSArray(const std::string& pat, int M, std::vector<int>& lps) {
    int length = 0;
    int i = 1;
    lps[0] = 0;

    while (i < M) {
        if (pat[i] == pat[length]) {
            length++;
            lps[i] = length;
            i++;
        }
        else {
            if (length != 0) {
                length = lps[length - 1];
            }
            else {
                lps[i] = 0;
                i++;
            }
        }
    }
}

std::vector<int> StatsManager::KMPSearch(const std::string& pat, const std::string& txt) {
    std::vector<int> toReturn{};

    int M = pat.size();
    int N = txt.size();
    std::vector<int> lps(M);

    computeLPSArray(pat, M, lps);

    int i = 0;
    int j = 0;
    while (i < N) {
        if (pat[j] == txt[i]) {
            j++;
            i++;
        }

        if (j == M) {
            toReturn.push_back(i - j);
            j = lps[j - 1];
        }
        else if (i < N && pat[j] != txt[i]) {
            if (j != 0) {
                j = lps[j - 1];
            }
            else {
                i++;
            }
        }
    }

    return toReturn;
}

int StatsManager::getCursorPosition(CCLabelBMFont* const& text, CCLabelBMFont* const& cursor){
    if (text->getString() == nullptr) return -1;
    if (text->getString()[0] == '\0') return -1;

    std::string tempS = text->getString();
    if (tempS == " ") return 0;

    int index = -1;

    CCObject* child;

    CCARRAY_FOREACH(text->getChildren(), child){
        if (auto node = typeinfo_cast<CCNode*>(child)){
            if (node->isVisible()){
                index++;

                if (node->getParent()->convertToWorldSpace(node->getPosition()).x > cursor->getParent()->convertToWorldSpace(cursor->getPosition()).x)
                    return index;
            }
        }
    }
    
    return index + 1;
}

bool StatsManager::isKeyInIndex(const std::string& s, const int& index, const std::string& key) {
    if (index + key.length() > s.length()) return false;

    return s.substr(index, key.length()) == key;
}

std::string StatsManager::workingTime(long long value){
    if(value < 0) return fmt::format("NA ({})", value);
    if(value == 0) return "NA";

    int hours = value / 3600;
    int minutes = (value % 3600) / 60;
    int seconds = value % 60;

    std::ostringstream stream;
    if(hours > 0) stream << hours << "h ";
    if(minutes > 0) stream << minutes << "m ";
    stream << seconds << "s";

    return stream.str();
}
