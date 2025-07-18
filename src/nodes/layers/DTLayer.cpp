#include "../layers/DTLayer.hpp"
#include "../../managers/DTPopupManager.hpp"
#include "../../utils/Settings.hpp"
#include "../LabelLayoutWindow.hpp"
#include "../layers/DTGraphLayer.hpp"
#include "../layers/DTLinkLayer.hpp"
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include "../../hooks/DTColorSelectPopup.hpp"
#include "../layers/DTLevelSpecificSettingsLayer.hpp"
#include <Geode/ui/GeodeUI.hpp>

DTLayer* DTLayer::create(GJGameLevel* const& Level) {
    auto ret = new DTLayer();
    if (ret && ret->initAnchored(368, 280, Level, "square01_001.png", {0.f, 0.f, 94.f, 94.f})) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTLayer::setup(GJGameLevel* const& level) {
    //create trackerLayer
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_Level = level;

    // ================================== //
    // loading data

    auto lvlStatsRes = StatsManager::getLevelStats(m_Level, false);
    m_MyLevelStats = lvlStatsRes.unwrapOrDefault();
    if (lvlStatsRes.isErr()) {
        StatsManager::setLevelStats(m_MyLevelStats, m_Level, false);
    }

    StatsManager::setCurrentLogLevel(level);

    DTLayer::UpdateSharedStats();
    // ================================== //

    /*
     * main page
    */

    this->setID("dt-layer");
    this->setZOrder(100);

    if (Save::getLastOpenedVersion() != Mod::get()->getVersion().toNonVString()){
        Save::setLastOpenedVersion(Mod::get()->getVersion().toNonVString());
        FLAlertLayer::create(nullptr, fmt::format("Death Tracker {} Changelog", Mod::get()->getVersion().toVString()).c_str(), fmt::format(
            "{}",
            "- <cg>iOS support</c>"
        ), "OK", nullptr, 415, false, 200, 0.75f)->show();
    }

    return true;
}

void DTLayer::update(float delta){

}

void DTLayer::onEditLayout(CCObject* sender){
    DTLayer::EditLayoutEnabled(true);
}

void DTLayer::textChanged(CCTextInputNode* input){

}

void DTLayer::textInputOpened(CCTextInputNode* input){

}

void DTLayer::textInputClosed(CCTextInputNode* input){

}

ResultTask DTLayer::updateSessionString(const int& session){

}

void DTLayer::onEditLayoutApply(CCObject*){

}

bool DTLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = true;
    if (pTouch->getLocation() != ccp(0, CCDirector::sharedDirector()->getWinSize().height))
        ClickPos = pTouch;
    return true;
}

void DTLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent){
    if (pTouch->getLocation() != ccp(0, CCDirector::sharedDirector()->getWinSize().height))
        ClickPos = pTouch;
}

void DTLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = false;
}

void DTLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = false;
}

void DTLayer::EditLayoutEnabled(const bool& b){

}

void DTLayer::changeScrollSizeByBoxes(const bool& moveToTop){

}

void DTLayer::RefreshText(bool moveToTop){

}

std::string DTLayer::modifyString(std::string ToModify){
    /*
    keys to check for

    {f0} - runs from 0

    {runs} - runs

    {lvln} - levels name
    
    {att} - level attempts (with linked levels attempts)

    {s0} - selected session runs from 0
    
    {sruns} - selected session runs

    {nl} - new line

    {ssd}

    {sst}
    
    */
}

ResultTask DTLayer::refreshStrings(){

}

DeathStringTask DTLayer::CreateDeathsInfo(const Deaths& deaths, const NewBests& newBests){

}

DeathStringTask DTLayer::CreateRunsInfo(const Runs runs){
    
}

//better info time calc

uint64_t DTLayer::timeInMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string DTLayer::decodeBase64Gzip(const std::string& input) {
    return ZipUtils::decompressString(input, false, 0);
}

inline bool objectIDIsSpeedPortal(int id) {
    return (id == 200 || id == 201 || id == 202 || id == 203 || id == 1334);
}

inline int speedToPortalId(int speed) {
    switch(speed) {
    default:
        return 201;
        break;
    case 1:
        return 200;
        break;
    case 2:
        return 202;
        break;
    case 3:
        return 203;
        break;
    case 4:
        return 1334;
        break;
    }
}

inline float travelForPortalId(int speed) {
    switch (speed)
    {
    case 200:
        return 251.16008f;
        break;
    default:
        return 311.58011f;
        break;
    case 202:
        return 387.42014f;
        break;
    case 203:
        return 468.00015f;
        break;
    case 1334:
        return 576.00018f;
        break;
    }
}

float DTLayer::timeForLevelString(const std::string& levelString) {
    try {
        auto a = timeInMs();

        auto decompressString = decodeBase64Gzip(levelString);
        auto c = timeInMs();
        std::stringstream responseStream(decompressString);
        std::string currentObject;
        std::string currentKey;
        std::string keyID;

        float prevPortalX = 0;
        int prevPortalId = 0;

        float timeFull = 0;

        float maxPos = 0;
        while(getline(responseStream, currentObject, ';')){
            size_t i = 0;
            int objID = 0;
            float xPos = 0;
            bool checked = false;

            std::stringstream objectStream(currentObject);
            while(getline(objectStream, currentKey, ',')) {
                if(i % 2 == 0) keyID = currentKey;
                else {
                    if(keyID == "1") objID = geode::utils::numFromString<int>(currentKey).unwrapOr(0);
                    else if(keyID == "2") xPos = geode::utils::numFromString<float>(currentKey).unwrapOr(0);
                    else if(keyID == "13") checked = geode::utils::numFromString<int>(currentKey).unwrapOr(0);
                    else if(keyID == "kA4") prevPortalId = speedToPortalId(geode::utils::numFromString<int>(currentKey).unwrapOr(0));
                }
                i++;

                if(xPos != 0 && objID != 0 && checked == true) break;
            }

            if(maxPos < xPos) maxPos = xPos;
            if(!checked || !objectIDIsSpeedPortal(objID)) continue;

            timeFull += (xPos - prevPortalX) / travelForPortalId(prevPortalId);
            prevPortalId = objID;
            prevPortalX = xPos;
        }

        timeFull += (maxPos - prevPortalX) / travelForPortalId(prevPortalId);
        auto b = timeInMs() - a;
        return timeFull;
    } catch(std::exception e) {
        log::error("An exception has occured while calculating time for levelString: {}", e.what());
        return 0;
    }
}

void DTLayer::updatePlaytime(std::vector<DeathInfo> deaths, bool runs){

}

void DTLayer::addBox(CCObject*){

}

void DTLayer::updateRunsAllowed(){
    if (m_MyLevelStats.currentBest != -1)
        StatsManager::setLevelStats(m_MyLevelStats, m_Level, false);
    DTLayer::refreshAll();
}

void DTLayer::FLAlert_Clicked(FLAlertLayer* layer, bool selected){

}

void DTLayer::onClose(cocos2d::CCObject*) {

}

void DTLayer::openGraphMenu(CCObject*){
    auto graph = DTGraphLayer::create(this);
    graph->setZOrder(100);
    this->addChild(graph);
}

void DTLayer::OnLinkButtonClicked(CCObject*){
    auto lLayer = DTLinkLayer::create(this);
    lLayer->setZOrder(100);
    this->addChild(lLayer);
}

void DTLayer::UpdateSharedStats(){
    m_SharedLevelStats = m_MyLevelStats;

    for (int i = 0; i < m_MyLevelStats.LinkedLevels.size(); i++)
    {
        auto currStats = StatsManager::getLevelStats(m_MyLevelStats.LinkedLevels[i], false).unwrapOrDefault();
        if (currStats.levelName == "Unknown name"){
            Notification::create("failed to get data for linked level - " + m_MyLevelStats.LinkedLevels[i])->show();
            continue;
        }

        m_SharedLevelStats.attempts += currStats.attempts;

        m_SharedLevelStats.sessions.reserve(m_SharedLevelStats.sessions.size() + std::distance(currStats.sessions.begin(),currStats.sessions.end()));
        m_SharedLevelStats.sessions.insert(m_SharedLevelStats.sessions.end(),currStats.sessions.begin(),currStats.sessions.end());

        for (int r = 0; r < m_MyLevelStats.RunsToSave.size(); r++)
        {
            bool addMeRun = true;
            for (int r2 = 0; r2 < currStats.RunsToSave.size(); r2++)
            {
                if (currStats.RunsToSave[r2] == m_MyLevelStats.RunsToSave[r])
                    addMeRun = false;
            }
            
            if (addMeRun)
                currStats.RunsToSave.push_back(m_MyLevelStats.RunsToSave[r]);
        }
        for (int r = 0; r < currStats.RunsToSave.size(); r++)
        {
            bool addMeRun = true;
            for (int r2 = 0; r2 < m_MyLevelStats.RunsToSave.size(); r2++)
            {
                if (m_MyLevelStats.RunsToSave[r2] == currStats.RunsToSave[r])
                    addMeRun = false;
            }
            
            if (addMeRun)
                m_MyLevelStats.RunsToSave.push_back(currStats.RunsToSave[r]);
        }

        std::ranges::sort(m_MyLevelStats.RunsToSave, [](const int a, const int b) {
            return a < b;
        });

        std::ranges::sort(currStats.RunsToSave, [](const int a, const int b) {
            return a < b;
        });

        if (m_MyLevelStats.currentBest != -1)
            StatsManager::setLevelStats(m_MyLevelStats, m_Level, false);
        if (currStats.currentBest != -1)
            StatsManager::setLevelStats(currStats, m_MyLevelStats.LinkedLevels[i], false);
        
        for (const auto& [death, count] : currStats.deaths)
        {
            m_SharedLevelStats.deaths[death] += count;
        }
        
        for (const auto& [run, count] : currStats.runs)
        {
            m_SharedLevelStats.runs[run] += count;
        }

        if (m_SharedLevelStats.currentBest < currStats.currentBest)
            m_SharedLevelStats.currentBest = currStats.currentBest;

        m_SharedLevelStats.newBests.insert(currStats.newBests.begin(), currStats.newBests.end());
    }

    std::ranges::sort(m_SharedLevelStats.sessions, [](const Session a, const Session b) {
        return a.lastPlayed > b.lastPlayed;
    });
}

void DTLayer::onSettings(CCObject*){
    geode::openSettingsPopup(Mod::get());
}

void DTLayer::onLayoutInfo(CCObject*){
    auto alert = FLAlertLayer::create("Help", "The boxes (labels) here represent the text displayed.\n \nYou drag them around to change their order and <cy>double click</c> any of them for more options.\nClick the <cg>plus</c> button to add a new label.\nThe button on the bottom left will <cy>reset everything back to default.</c>", "Ok");
    alert->show();
}

void DTLayer::onCopyInfo(CCObject*){
    auto alert = FLAlertLayer::create("Help", "Click a label to copy its text.", "Ok");
    alert->show();
}

void DTLayer::copyText(CCObject*)
{

}

void DTLayer::clickedWindow(CCNode* nwindow){

}

void DTLayer::onResetLayout(CCObject*){

}

void DTLayer::editnbcColor(CCObject*){

}

void DTLayer::editsbcColor(CCObject*){

}

void DTLayer::onSpecificSettings(CCObject*){

}

void DTLayer::onMoveTransitionEnded(CCObject* LSSL){

}

void DTLayer::updateColor(cocos2d::ccColor4B const& color){

}

void DTLayer::refreshAll(bool moveToTop){

}

void DTLayer::onRefreshFinished(ResultTask::Event* event){
}