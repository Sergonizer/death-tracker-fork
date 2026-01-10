#include <managers/StatsManager.hpp>
#include <utils/Settings.hpp>
#include <utils/Dev.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <regex>

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

GJGameLevel* StatsManager::currentLevel = nullptr;
GeneralData StatsManager::currentFrom0{};
LevelMetadeta StatsManager::currentMetadata{};
Session StatsManager::currentSession{};

const std::string StatsManager::METADATA_FILE_NAME = "metadata";
const std::string StatsManager::FROM0_FILE_NAME = "general.dt";
const std::string StatsManager::SESSIONS_DIR_NAME = "sessions";
const std::string StatsManager::BACKUPS_DIR_NAME = "backups";

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

Result<LevelMetadeta> StatsManager::getMetadata(GJGameLevel* const level){
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return getMetadata(levelKey);
}
Result<LevelMetadeta> StatsManager::getMetadata(const std::string& levelKey){
    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::METADATA_FILE_NAME;

    if (!std::filesystem::exists(levelSaveFilePath))
        return Err("1 (no stats exist for level!)");

    GEODE_UNWRAP_INTO(auto json, file::readJson(levelSaveFilePath));
        
    return json.as<LevelMetadeta>();
}

Result<Session> StatsManager::getSession(GJGameLevel* const level, long long sessionTime){
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return getSession(levelKey, sessionTime);
}
Result<Session> StatsManager::getSession(const std::string& levelKey, long long sessionTime){
    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::SESSIONS_DIR_NAME / (std::to_string(sessionTime) + ".dt");

    if (!std::filesystem::exists(levelSaveFilePath))
        return Err("1 (no stats exist for level!)");

    GEODE_UNWRAP_INTO(auto json, file::readJson(levelSaveFilePath));
        
    return json.as<Session>();
}

Result<GeneralData> StatsManager::getGeneral(GJGameLevel* const level){
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return getGeneral(levelKey);
}
Result<GeneralData> StatsManager::getGeneral(const std::string& levelKey){
    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::FROM0_FILE_NAME;

    if (!std::filesystem::exists(levelSaveFilePath))
        return Err("1 (no stats exist for level!)");

    GEODE_UNWRAP_INTO(auto json, file::readJson(levelSaveFilePath));
        
    return json.as<GeneralData>();
}

Result<LevelData> StatsManager::getLevelData(GJGameLevel* const level){
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return getLevelData(levelKey);
}
Result<LevelData> StatsManager::getLevelData(const std::string& levelKey){
    LevelData data;
    GEODE_UNWRAP_INTO(data.metadata, StatsManager::getMetadata(levelKey));
    data.from0 = StatsManager::getGeneral(levelKey);
    data.sessionNames = StatsManager::getAllSessionTimesForLevel(levelKey);
    data.levelKey = levelKey;

    return Ok(data);
}

Result<BackupLevelData> StatsManager::getBackupData(const std::string& levelKey, long long backupName){
    BackupLevelData data;

    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::BACKUPS_DIR_NAME / std::to_string(backupName);
    data.backupDate = backupName;

    if (!std::filesystem::exists(levelSaveFilePath))
        return Err("1 (no stats exist for backup!)");

    if (std::filesystem::exists(levelSaveFilePath / StatsManager::FROM0_FILE_NAME)){
        auto levelStatsJsonRes = file::readJson(levelSaveFilePath / StatsManager::FROM0_FILE_NAME);
        if (levelStatsJsonRes.isErr()) return Err("Failed to get backup level stats!");
        auto levelStatsJson = levelStatsJsonRes.unwrap();
        
        auto metaJsonObjRes = levelStatsJson.as<GeneralData>();
        if (metaJsonObjRes.isErr()) return Err("Failed to get backup metadata!");
        data.from0 = metaJsonObjRes.unwrap();
    }

    if (std::filesystem::exists(levelSaveFilePath / StatsManager::SESSIONS_DIR_NAME)){
        std::set<long long> sessionDates{};

        for (const auto& entry : std::filesystem::directory_iterator(levelSaveFilePath / StatsManager::SESSIONS_DIR_NAME)) {
            if (!entry.is_directory()) continue;

            auto numRes = geode::utils::numFromString<long long>(entry.path().filename().string());
            if (numRes.isErr()) continue;

            sessionDates.insert(numRes.unwrap());
        }

        data.sessionNames = sessionDates;
    }

    return Ok(data);
}

Result<> StatsManager::setMetadata(const LevelMetadeta& stats, GJGameLevel* const level){
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    GEODE_UNWRAP(setMetadata(stats, levelKey));

    return Ok();
}
Result<> StatsManager::setMetadata(const LevelMetadeta& stats, const std::string& levelKey){
    createFilesIfNeeded(levelKey);

    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::METADATA_FILE_NAME;

    auto indentation = Dev::MINIFY_SAVE_FILE
        ? matjson::NO_INDENTATION
        : 4;

    // log::info("attempted path: {}", levelSaveFilePath.string());

    auto jsonStr = matjson::Value(stats).dump(indentation);
    GEODE_UNWRAP(file::writeString(levelSaveFilePath, jsonStr));

    return Ok();
}

Result<> StatsManager::setSession(Session& stats, GJGameLevel* const level, long long sessionTime, bool updateLastPlayed){
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    GEODE_UNWRAP(setSession(stats, levelKey, sessionTime, updateLastPlayed));

    return Ok();
}
Result<> StatsManager::setSession(Session& stats, const std::string& levelKey, long long sessionTime, bool updateLastPlayed){
    createFilesIfNeeded(levelKey);

    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::SESSIONS_DIR_NAME / (std::to_string(sessionTime) + ".dt");

    createFile(levelSaveFilePath);

    auto indentation = Dev::MINIFY_SAVE_FILE
        ? matjson::NO_INDENTATION
        : 4;

    if (updateLastPlayed){
        auto now = StatsManager::getNowSeconds();
        stats.lastPlayed = now;
    }

    stats.ownerLevelKey = levelKey;

    auto jsonStr = matjson::Value(stats).dump(indentation);
    GEODE_UNWRAP(file::writeString(levelSaveFilePath, jsonStr));

    return Ok();
}

Result<> StatsManager::setGeneral(const GeneralData& stats, GJGameLevel* const level){
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    GEODE_UNWRAP(setGeneral(stats, levelKey));

    return Ok();
}
Result<> StatsManager::setGeneral(const GeneralData& stats, const std::string& levelKey){
    createFilesIfNeeded(levelKey);

    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::FROM0_FILE_NAME;

    auto indentation = Dev::MINIFY_SAVE_FILE
        ? matjson::NO_INDENTATION
        : 4;

    auto jsonStr = matjson::Value(stats).dump(indentation);
    GEODE_UNWRAP(file::writeString(levelSaveFilePath, jsonStr));

    return Ok();
}

Result<> StatsManager::addBackup(const std::string& levelKey, bool saveLevelStats, std::optional<int> sessionsToSave){
    auto metaRes = getMetadata(levelKey);
    if (metaRes.isErr()) return Err("No level to back up! {}", metaRes.unwrapErr());
    auto metadata = metaRes.unwrap();
    
    createFilesIfNeeded(levelKey);

    auto levelBackupsFilePath = m_savesFolderPath / levelKey / StatsManager::BACKUPS_DIR_NAME;

    auto _ = geode::utils::file::createDirectory(levelBackupsFilePath);
    if (_.isErr()) return Err("failed to create backups folder! {}", _.unwrapErr());

    auto currBackupName = StatsManager::getNowSeconds();

    levelBackupsFilePath /= std::to_string(currBackupName);

    _ = geode::utils::file::createDirectory(levelBackupsFilePath);
    if (_.isErr()) return Err("failed to create backup folder! {}", _.unwrapErr());

    if (saveLevelStats){

        if (getGeneral(levelKey).isErr()){
            _ = deleteBackup(levelKey, currBackupName);

            return Err("backup failed! failed to read general stats");
        }

        if (!std::filesystem::copy_file(m_savesFolderPath / levelKey / StatsManager::FROM0_FILE_NAME, levelBackupsFilePath / StatsManager::FROM0_FILE_NAME, std::filesystem::copy_options::overwrite_existing))
            return Err("Failed to backup level stats!");
    }

    if (metadata.maxBackupsAmount != std::nullopt){
        auto count = getBackupsCount(levelKey);

        if (count.size() > metadata.maxBackupsAmount.value()){
            int index = 0;
            for (const auto& backupName : count)
            {
                if (index == count.size() - metadata.maxBackupsAmount.value()) break;

                _ = deleteBackup(levelKey, backupName);

                index++;
            }
        }
    }

    if (sessionsToSave == std::nullopt) return Ok();

    if (sessionsToSave.value() >= -1){
        levelBackupsFilePath /= StatsManager::SESSIONS_DIR_NAME;

        auto _ = geode::utils::file::createDirectory(levelBackupsFilePath);
        if (_.isErr()) return Err("failed to create backup sessions folder! {}", _.unwrapErr());

        std::set<long long, std::greater<long long>> sessionTimes{};
        auto nonSorted = getAllSessionTimesForLevel(levelKey);
        sessionTimes.insert(nonSorted.begin(), nonSorted.end());

        int numToSave = sessionsToSave.value();
        if (numToSave == -1)
            numToSave = sessionTimes.size();

        int index = 0;
        for (const auto& sessionTime : sessionTimes)
        {
            if (index == numToSave) break;

            if (getSession(levelKey, sessionTime).isErr()){
                log::error("Failed to backup session {}", sessionTime);

                continue;
            }

            if (!std::filesystem::copy_file(
                m_savesFolderPath / levelKey / StatsManager::SESSIONS_DIR_NAME / (std::to_string(sessionTime) + ".dt"), 
                levelBackupsFilePath / (std::to_string(sessionTime) + ".dt"), 
                std::filesystem::copy_options::overwrite_existing
            ))
                return Err("Failed to backup session {}", sessionTime);

            index++;
        }
    }

    return Ok();
}

void StatsManager::createFilesIfNeeded(const std::string& levelKey){
    auto _ = geode::utils::file::createDirectory(m_savesFolderPath);

    auto levelSaveFilePath = m_savesFolderPath / levelKey;

    // log::info("attempting to create level folder at {}", levelSaveFilePath.string());

    _ = geode::utils::file::createDirectory(levelSaveFilePath);
    if (_.isErr())
        log::error("failed to create level folder: {}", _.unwrapErr());

    createFile((levelSaveFilePath / METADATA_FILE_NAME));
    createFile((levelSaveFilePath / FROM0_FILE_NAME));
    _ = geode::utils::file::createDirectory((levelSaveFilePath / SESSIONS_DIR_NAME));
}

void StatsManager::createFile(const std::filesystem::path& path){
    if (std::filesystem::exists(path)) return;

    std::ofstream file(path);
    file.close();
}

void StatsManager::setCurrentLevel(GJGameLevel* const& level){
    currentLevel = level;

    if (level == nullptr) {
        currentFrom0 = GeneralData{};
        currentSession = Session{};
        currentMetadata = LevelMetadeta{};
        return;
    }

    auto from0Res = getGeneral(level);
    auto sessionCount = getAllSessionTimesForLevel(level);
    //auto sessionRes = sessionCount.isErr() ? Err(sessionCount.unwrapErr()) : getSession(level, sessionCount.unwrap());
    auto metadataRes = getMetadata(level);

    if (metadataRes.isErr()){
        currentLevel = nullptr;
        currentFrom0 = GeneralData{};
        currentSession = Session{};
        currentMetadata = LevelMetadeta{};
        log::error("Failed to apply current level stats as main stats ({})", metadataRes.unwrapErr());
        return;
    }

    if (from0Res.isErr()){
        auto newF0 = GeneralData{.currentBest = -1};

        from0Res = Ok(newF0);
    }

    currentFrom0 = from0Res.unwrap();
    //currentSession = sessionRes.unwrap();
    currentMetadata = metadataRes.unwrap();
}

void StatsManager::logDeath(const int& percent, bool instantSave) {
    if (currentLevel == nullptr) {
        log::error("Failed to log death");
        return;
    }

    auto percentKey = std::to_string(percent);

    currentFrom0.deaths[percentKey]++;

    if (percent > currentFrom0.currentBest) {
        currentFrom0.currentBest = percent;
        currentFrom0.newBests.insert(percent);
    }

    auto session = StatsManager::getCurrentSession();

    if (session) session->deaths[percentKey]++;

    if (session && percent > session->currentBest) {
        session->currentBest = percent;
        session->newBests.insert(percent);
    }
    

    if (instantSave){
        auto _ = StatsManager::setGeneral(currentFrom0, currentLevel);
        if (session) _ = StatsManager::setSession(*session, currentLevel, session->sessionStartDate, true);
    }
}

void StatsManager::logDeaths(const std::vector<int>& percents) {
    if (currentLevel == nullptr) {
        log::error("Failed to log deaths");
        return;
    }

    for (int i = 0; i < percents.size(); i++)
    {
        logDeath(percents[i], false);
    }

    auto _ = StatsManager::setGeneral(currentFrom0, currentLevel);
    _ = StatsManager::setSession(currentSession, currentLevel, currentSession.sessionStartDate, true);
}

void StatsManager::logRun(const Run& run, bool instantSave) {
    if (currentLevel == nullptr) {
        log::error("Failed to log deaths");
        return;
    }

    bool TrackRun = false;
    if (currentMetadata.trackAnyRun){
        TrackRun = true;
    }
    else if (currentMetadata.RunsToSave.contains(run.start)){
        TrackRun = true;
    }

    if (!TrackRun) return;

    auto session = StatsManager::getCurrentSession();
    if (!session) return;

    auto runKey = fmt::format("{}-{}",
        run.start,
        run.end
    );

    currentFrom0.runs[runKey]++;
    session->runs[runKey]++;
    
    if (instantSave){
        auto _ = StatsManager::setGeneral(currentFrom0, currentLevel);
        _ = StatsManager::setSession(currentSession, currentLevel, currentSession.sessionStartDate, true);
    }
}

void StatsManager::logRuns(const std::vector<Run>& runs) {
    if (currentLevel == nullptr) {
        log::error("Failed to log runs");
        return;
    }

    bool TrackRun = false;
    for (int i = 0; i < runs.size(); i++)
    {
        logRun(runs[i], false);
    }

    auto _ = StatsManager::setGeneral(currentFrom0, currentLevel);
    _ = StatsManager::setSession(currentSession, currentLevel, currentSession.sessionStartDate, true);
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
	if (level->m_levelType == GJLevelType::Main)
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

Result<Run> StatsManager::splitRunKey(const std::string& runKey) {
    auto runKeySplit = StatsManager::splitStr(runKey, "-");

    if (runKeySplit.size() == 0 || runKeySplit.size() > 2)
        return Err("Invalid run key!");

    int start = -1;
    int end = -1;

    if (runKeySplit.size() == 1){
        GEODE_UNWRAP_INTO(end, geode::utils::numFromString<int>(runKeySplit[0]));
    }
    else{
        GEODE_UNWRAP_INTO(start, geode::utils::numFromString<int>(runKeySplit[0]));
        GEODE_UNWRAP_INTO(end, geode::utils::numFromString<int>(runKeySplit[1]));
    }

    Run r;
    r.start = start;
    r.end = end;

    return Ok(r);
}

Result<std::string> StatsManager::createRunKey(const Run& runKey){
    if (runKey.start == -1){
        return Ok(std::to_string(runKey.end));
    }
    else if (runKey.start >= 0){
        return Ok(fmt::format("{}-{}", runKey.start, runKey.end));
    }

    return Err("Bad percentages");
}

Session* StatsManager::getCurrentSession() {
    if (currentLevel == nullptr) {
        log::error("Failed to get session");
        return nullptr;
    }

    auto currentSession = &StatsManager::currentSession;

    auto levelKeyRes = StatsManager::getLevelKey(currentLevel);

    if (levelKeyRes.isErr()) return currentSession;
    
    auto sessionsCount = getAllSessionTimesForLevel(levelKeyRes.unwrap());

    // new sessions can be scheduled
    // and are created when the player dies
    if (!sessionsCount.size() && !m_scheduleCreateNewSession) return nullptr;

    if (!m_scheduleCreateNewSession) return currentSession;
    m_scheduleCreateNewSession = false;

    log::info("creating a new session!");

    

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

    if (setSession(session, currentLevel, session.sessionStartDate, true).isErr()){
        log::error("Failed to create new session");
        return currentSession;
    }

    StatsManager::currentSession = session;
    return currentSession;
}

void StatsManager::scheduleCreateNewSession(const bool& scheduled) {
    if (currentLevel == nullptr) {
        log::error("Failed to get session");
        return;
    }

    m_scheduleCreateNewSession = scheduled;
}

bool StatsManager::hasPlayedLevel() {
    if (currentLevel == nullptr) {
        log::error("Failed to check if level has been played");
        return false;
    }
    
    auto levelKeyRes = StatsManager::getLevelKey(currentLevel);

    if (levelKeyRes.isErr()) return false;

    return m_playedLevels.contains(levelKeyRes.unwrap());
}

/* internal functions
======================= */

Result<> StatsManager::deleteLevelStats(const std::string& levelKey){
    auto levelSaveFilePath = m_savesFolderPath / levelKey;

    if (std::filesystem::is_directory(levelSaveFilePath)){
        if (std::filesystem::remove_all(levelSaveFilePath) == static_cast<std::uintmax_t>(-1)) return Err("Failed to delete stats folder!");
        return Ok();
    }

    return Err("Cant delete stats because level save file does not exist: " + levelSaveFilePath.string());
}

Result<> StatsManager::deleteBackup(const std::string& levelKey, long long backupName){
    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::BACKUPS_DIR_NAME / std::to_string(backupName);

    if (std::filesystem::is_directory(levelSaveFilePath)){
        if (std::filesystem::remove_all(levelSaveFilePath) == static_cast<std::uintmax_t>(-1)) return Err("Failed to delete backup {}!", backupName);
        return Ok();
    }

    return Err("Cant delete backup because level save file does not exist: " + levelSaveFilePath.string());
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

Result<std::vector<std::pair<std::string, LevelMetadeta>>> StatsManager::getAllLevels(){
    GEODE_UNWRAP_INTO(auto allLevels, file::readDirectory(m_savesFolderPath));

    std::vector<std::pair<std::string, LevelMetadeta>> toReturn{};

    std::vector<std::string> failedKeys{};

    for (int i = 0; i < allLevels.size(); i++)
    {
        if (std::filesystem::is_directory(allLevels[i])){
            auto currentDirName = allLevels[i].parent_path().string();
            log::info("Getting level stats for level: {}", currentDirName);

            auto currentLevel = StatsManager::getMetadata(currentDirName);
            if (currentLevel.isErr()){
                Notification::create(fmt::format("failed getting some levels, errors send in logs.", currentDirName, currentLevel.unwrapErr()), nullptr)->show();
                failedKeys.push_back(currentDirName);
                continue;
            }

            auto stats = currentLevel.unwrap();

            toReturn.push_back(std::make_pair(currentDirName, stats));
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

void StatsManager::updateCurrentSessionLastPlayed(){
    if (currentLevel == nullptr) {
        log::error("Failed to update current session last played");
        return;
    }

    auto session = StatsManager::getCurrentSession();
    if (!session) return;

    auto now = StatsManager::getNowSeconds();
    session->lastPlayed = now;

    auto _ = StatsManager::setSession(*session, currentLevel, session->sessionStartDate, true);
}

GJGameLevel* StatsManager::getCurrentLevel(){
    return StatsManager::currentLevel;
}

Result<std::set<long long>> StatsManager::getAllSessionTimesForLevel(GJGameLevel* const level){
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return Ok(getAllSessionTimesForLevel(levelKey));
}

std::set<long long> StatsManager::getAllSessionTimesForLevel(const std::string& levelKey){
    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::SESSIONS_DIR_NAME;

    if (!std::filesystem::exists(levelSaveFilePath)) return {};

    std::set<long long> toReturn{};

    for (const auto& entry : std::filesystem::directory_iterator(levelSaveFilePath)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".dt") continue;

        auto numRes = geode::utils::numFromString<long long>(entry.path().filename().string());
        if (numRes.isErr()) continue;

        toReturn.insert(numRes.unwrap());
    }

    return toReturn;
}

std::set<long long> StatsManager::getBackupsCount(const std::string& levelKey){
    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::BACKUPS_DIR_NAME;

    if (!std::filesystem::exists(levelSaveFilePath)) return {};

    std::set<long long> toReturn{};

    for (const auto& entry : std::filesystem::directory_iterator(levelSaveFilePath)) {
        if (!entry.is_directory()) continue;

        auto numRes = geode::utils::numFromString<long long>(entry.path().filename().string());
        if (numRes.isErr()) continue;

        toReturn.insert(numRes.unwrap());
    }

    return toReturn;
}

uintmax_t StatsManager::getBackupFileSize(const std::string& levelKey, long long backupName){
    auto levelSaveFilePath = m_savesFolderPath / levelKey / StatsManager::BACKUPS_DIR_NAME / std::to_string(backupName);

    if (!std::filesystem::exists(levelSaveFilePath)) return 0;

    uintmax_t totalSize = 0;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(levelSaveFilePath)) {
        if (!entry.is_regular_file()) continue;

        totalSize += std::filesystem::file_size(entry.path());
    }

    return totalSize;
}

Result<> StatsManager::reveretBackupSessions(const std::string& levelKey, long long backupName){
    auto bakupSessionDirPath = m_savesFolderPath / levelKey / StatsManager::BACKUPS_DIR_NAME / std::to_string(backupName) / StatsManager::SESSIONS_DIR_NAME;
    auto sessionDirPath = m_savesFolderPath / levelKey / StatsManager::SESSIONS_DIR_NAME;

    if (!std::filesystem::exists(bakupSessionDirPath)) return Err("No sessions backup found!");

    if (std::filesystem::exists(sessionDirPath)){
        if (std::filesystem::remove_all(sessionDirPath) == static_cast<std::uintmax_t>(-1)) return Err("Failed to delete current sessions folder!");
    }

    std::filesystem::copy(bakupSessionDirPath, sessionDirPath, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    return Ok();
}

Result<> StatsManager::convertV2SaveToV3(const std::string& levelKey){
    log::info("converting V2 save to V3...");

    if (!std::filesystem::exists(m_savesFolderPath)) return Err("No levels directory found");

    auto v2Path = m_savesFolderPath / (levelKey + ".json");

    if (!std::filesystem::exists(v2Path)) return Err("V2 level file doesnt exist!");

    auto res = file::readJson(v2Path);
    GEODE_UNWRAP_INTO(auto json, res);
    
    auto statsRes = json.as<V2LevelStats>();

    if (statsRes.isErr()) return Err("Error parsing V2 json!");

    auto stats = statsRes.unwrap();

    auto v3Meta = LevelMetadeta{};
    std::set<int> runsSet(stats.RunsToSave.begin(), stats.RunsToSave.end());
    bool trackAnyRun = false;
    if (runsSet.contains(-1)){
        trackAnyRun = true;
        runsSet.erase(-1);
    }
    v3Meta.RunsToSave = runsSet;
    v3Meta.trackAnyRun = trackAnyRun;
    std::set<std::string> linkedLevelsSet(stats.LinkedLevels.begin(), stats.LinkedLevels.end());
    v3Meta.LinkedLevels = linkedLevelsSet;
    v3Meta.levelName = stats.levelName;
    v3Meta.attempts = stats.attempts;
    v3Meta.difficulty = stats.difficulty;
    v3Meta.hideUpto = stats.hideUpto;
    v3Meta.hideRunLength = stats.hideRunLength;

    if (setMetadata(v3Meta, levelKey).isErr()) return Err("Failed to write V3 meta!");

    auto v3General = GeneralData{};
    v3General.deaths = stats.deaths;
    v3General.runs = stats.runs;
    v3General.currentBest = stats.currentBest;
    v3General.newBests = stats.newBests;

    if (setGeneral(v3General, levelKey).isErr()) return Err("Failed to write V3 general!");

    for (const auto& v2session : stats.sessions){
        auto v3Session = Session{};

        v3Session.ownerLevelKey = levelKey;
        v3Session.lastPlayed = v2session.lastPlayed;
        v3Session.deaths = v2session.deaths;
        v3Session.runs = v2session.runs;
        v3Session.newBests = v2session.newBests;
        v3Session.currentBest = v2session.currentBest;
        v3Session.sessionStartDate = v2session.sessionStartDate;

        if (setSession(v3Session, levelKey, v3Session.sessionStartDate, false).isErr()) return Err("Failed to write V3 session! session sd: {}", v3Session.sessionStartDate);
    }

    if (!std::filesystem::remove(v2Path)) return Err("Failed to erase old data!");
    auto backupPath = m_savesFolderPath / (levelKey + ".deathsBackup");

    if (std::filesystem::exists(backupPath)){
        if (!std::filesystem::remove(backupPath)) return Err("Failed to delete backup old data!");
    }

    return Ok();
}

std::vector<std::string> StatsManager::allV2FileLevelKeys(){
    std::vector<std::string> toReturn{};

    for (const auto& entry : std::filesystem::directory_iterator(m_savesFolderPath)){
        if (!entry.is_regular_file() || entry.path().extension() != ".json") continue;

        toReturn.push_back(entry.path().stem().string());
    }

    return toReturn;
}

std::vector<std::string> StatsManager::allV3FileLevelKeys(){
    std::vector<std::string> toReturn{};

    for (const auto& entry : std::filesystem::directory_iterator(m_savesFolderPath)){
        if (!entry.is_directory()) continue;

        toReturn.push_back(entry.path().stem().string());
    }

    return toReturn;
}