#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class StatsManager {
private:
    static std::set<std::string> m_playedLevels;
    
    static bool m_scheduleCreateNewSession;

    // internal functions

    //gets new bests from level
    static Result<std::tuple<NewBests, int>> calcNewBests(GJGameLevel* const& level);
    static std::array<std::string, 62> m_AllFontsMap;

    static LevelStats* currentLoggingLevelStatsRef;
    static GJGameLevel* currentLoggingLevelRef;

public:
    static LevelStats currentLoggingLevelStats;
    StatsManager() = delete;

    static int MainLevelIDs[26];

    static std::filesystem::path m_savesFolderPath;

#pragma region level setters/getters

    //load and retrieve the stats for a level
    static Result<LevelStats> getLevelStats(GJGameLevel* const& level, bool isBackup);
    //get level stats by a levels key
    static Result<LevelStats> getLevelStats(const std::string& levelKey, bool isBackup);

    //save data to a specific level
    //@param stats data to save
    //@param level level to save data for
    static void setLevelStats(const LevelStats& stats, GJGameLevel* const& level, bool isbackup);
    //save data to a specific level
    //@param stats data to save
    //@param levelKey a level key of the level to save data for
    static void setLevelStats(const LevelStats& stats, const std::string& levelKey, bool isbackup);

    static Result<> deleteLevelStats(const std::string& levelKey);
    
#pragma endregion

#pragma region logging

    static void setCurrentLogLevel(GJGameLevel* const& level);

    //save a normal mode death to the loaded levels save file
    static void logDeath(const int& percent);
    //save an array of normal mode deaths to the loaded levels save file
    static void logDeaths(const std::vector<int>& percents);
    //save run to the loaded levels save file
    static void logRun(const Run& run);
    //save an array of run to the loaded levels save file
    static void logRuns(const std::vector<Run>& runs);

#pragma endregion

    // utility functions

    //gets the epoch time
    static long long getNowSeconds();
    //get the a levels level key
    static Result<std::string> getLevelKey(GJGameLevel* const& level);
    //convert a run string into a run struct
    static Run splitRunKey(const std::string& runKey);
    //get the current ongoing session
    static Session* getSession();
    //update this sessions last played time
    static void updateSessionLastPlayed(bool save = false);
    //schedule the creation of a new session
    static void scheduleCreateNewSession(const bool& scheduled);
    //if youve played the loaded level before, returns true
    static bool hasPlayedLevel();
    //get a font by a fontID
    static std::string getFont(const int& fontID);
    //get a fonts name by a fontID
    static std::string getFontName(const int& fontID);
    //get an array of all fonts
    static std::array<std::string, 62> getAllFonts();
    //gets an array of all levels you have progress on
    static Result<std::vector<std::pair<std::string, LevelStats>>> getAllLevels();
    //seperate a level key to [levelID, levelType]
    static std::pair<std::string, std::string> splitLevelKey(const std::string& levelKey);
    //get the difficulty of a level
    //
    // -1 = auto, 0 = NA, 1 = Easy, 2 = Normal
    // 3 = Hard, 4 = Harder, 5 = Insane, 6 = Hard Demon
    // 7 = Easy Demon, 8 = Medium Demon, 9 = Insane Demon, 10 = Extreme Demon
    static int getDifficulty(GJGameLevel* const& level);

    //sets the current deaths save folder path
    static void setPath(const std::filesystem::path& path);

    // return a splitted version of the string provided, devided by the delim
    static std::vector<std::string> splitStr(const std::string& str, const std::string& delim);

    //KMP search

    static void computeLPSArray(const std::string& pat, int M, std::vector<int>& lps);
    static std::vector<int> KMPSearch(const std::string& pat, const std::string& txt);
    
    //gets the index in which the CCLabelBMFonts cursor is located on the string
    static int getCursorPosition(CCLabelBMFont* const& text, CCLabelBMFont* const& cursor);

    //get if a key is in an index of a string
    //@param s the string to look for the key in
    //@param index the index where to look for the key in
    //@param key the key to look for in the string
    static bool isKeyInIndex(const std::string& s, const int& Index, const std::string& key);

    //better info calc :)
    //converts time to a working time string
    static std::string workingTime(long long value);
};