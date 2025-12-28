#include <nodes/DTGraphNode.hpp>

DTGraphNode* DTGraphNode::create() {
    auto ret = new DTGraphNode();
    if (ret && ret->init()) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool DTGraphNode::init(){


    return true;
}



void DTGraphNode::addGraphForDeaths(const std::string& graphName, const std::vector<DeathInfo>& deaths, GraphType type, float thickness, const ccColor4B& color, bool clearOther){
    // int bestRun = 0;

    // for (auto best : deaths)
    // {
    //     if (best.run.end > bestRun) bestRun = best.run.end;
    // }
    
    // if (!deaths.size()) return;

    // std::vector<DeathInfo> fixedDS = deaths;

    // float RunStartPercent = fixedDS[0].run.start;

    // GraphLine myGraph;

    // if (allGraphs.contains(graphName))
    //     myGraph = allGraphs[graphName];

    // if (myGraph.GraphPointsContainer) myGraph.GraphPointsContainer->removeAllChildrenWithCleanup(true);
    // else {
    //     auto MenuForGP = CCMenu::create();
    //     MenuForGP->setID(graphName + "-point-menu");
    //     MenuForGP->setPosition({0,0});
    //     MenuForGP->setZOrder(1);
    //     myGraph.GraphPointsContainer = MenuForGP;
    //     graphsContainer->addChild(MenuForGP);
    // }

    // myGraph.thickness = thickness;

    // fixedDS[fixedDS.size() - 1].passrate = 0;

    // if (type == GraphType::PassRate){
    //     add the min and max points if needed
    //     if (fixedDS[0].run.end > RunStartPercent){
    //         auto info = DeathInfo(Run(RunStartPercent, RunStartPercent), 0, 100);
    //         fixedDS.insert(fixedDS.begin(), info);
    //     }
        
    //     if (fixedDS[fixedDS.size() - 1].run.end < 100){
    //         auto info = DeathInfo(Run(RunStartPercent, 100), 100, 0);
    //         fixedDS.push_back(info);
    //     }
    //     else{
    //         //fixedDS[fixedDS.size() - 1].passrate = 100;
    //     }

    //     //log::info("added extras");

    //     CCPoint previousPoint = ccp(-1, -1);

    //     for (const auto& deathI : fixedDS)
    //     {
    //         //save point
    //         //CCPoint myPoint = ccp(deathI.run.end, deathI.passrate);

            
    //         //add extra points
    //         if (previousPoint.x != -1){

    //             //add a before point if needed
    //             if (previousPoint.x != myPoint.x - 1){

    //                 if (previousPoint.x != myPoint.x - 2 && previousPoint.y != 100 && previousPoint.x + 1 <= bestRun){
    //                     myGraph.points.push_back(ccp(previousPoint.x + 1, 100) * scaling);
    //                 }

    //                 if (myPoint.x - 1 <= bestRun && myPoint.y != 100)
    //                     myGraph.points.push_back(ccp(myPoint.x - 1, 100) * scaling);
    //             }
    //         }

    //         myGraph.points.emplace_back(myPoint * scaling);
    //         previousPoint = myPoint;
    //     }

    //     //add wrapping
    //     myGraph.points.emplace_back(myGraph.points[myGraph.points.size() - 1].x + 100, myGraph.points[myGraph.points.size() - 1].y);
    //     myGraph.points.emplace_back(myGraph.points[myGraph.points.size() - 1].x + 100, -100);
    //     myGraph.points.emplace_back(myGraph.points[0].x, -100);
    // }
    // else if (type == GraphType::ReachRate){

    //     int overallCount = 0;
    //     std::vector<std::pair<int, int>> percentageDeaths{};
    //     for (int i = fixedDS.size() - 1; i >= 0; i--)
    //     {
    //         overallCount += fixedDS[i].deaths;
    //         percentageDeaths.emplace(percentageDeaths.begin(), fixedDS[i].run.end, overallCount);
    //     }

    //     if (percentageDeaths[0].first > RunStartPercent)
    //         percentageDeaths.emplace(percentageDeaths.begin(), RunStartPercent, overallCount);
        
    //     if (percentageDeaths[percentageDeaths.size() - 1].first < 100){
    //         percentageDeaths.emplace_back(percentageDeaths[percentageDeaths.size() - 1].first + 1, 0);

    //         if (percentageDeaths[percentageDeaths.size() - 1].first != 100)
    //             percentageDeaths.emplace_back(100, 0);
    //     }
            

    //     for (int i = 0; i < percentageDeaths.size(); i++)
    //     {
    //         float reachRate = static_cast<float>(percentageDeaths[i].second) / overallCount;
    //         myGraph.points.emplace_back(percentageDeaths[i].first * scaling.width, reachRate * scaling.height * 100);
    //     }
        
    // }

    // //log::info("added lines");

    // ccColor3B colorOfPoints;

    // if ((color.r + color.g + color.b) / 3 > 200)
    //     colorOfPoints = {255, 255, 255};
    // else
    //     colorOfPoints = { 136, 136, 136};


    // if (myGraph.lineNode) myGraph.lineNode->clear();
    // else myGraph.lineNode = CCDrawNode::create();
    // myGraph.lineNode->setID(graphName + "-graph");

    // log::info("{} | {}", myGraph.points.size(), deaths.size());

    // bool isFirst = false;
    // CCPoint prevPoint;
    // for (const CCPoint& linePoint : myGraph.points)
    // {
    //     if (linePoint.x >= 0 && linePoint.x <= 100 * scaling.width && linePoint.y >= 0 && linePoint.y <= 100 * scaling.height)
    //     {
    //         auto pointText = fmt::format("{}%", linePoint.x / scaling.width);
    //         if (RunStartPercent != 0)
    //             pointText.insert(0, fmt::format("{}% - ", RunStartPercent));

    //         auto GP = GraphPoint::create(pointText, linePoint.y / scaling.height, colorOfPoints);
    //         //GP->setDelegate(this);
    //         GP->setPosition(linePoint);
    //         //GP->setScale(Settings::getGraphPointSize() / 20 + 0.01f);
    //         GP->setScale(0.1f);
    //         myGraph.GraphPointsContainer->addChild(GP);
    //     }

    //     if (!isFirst){
    //         prevPoint = linePoint;
    //         isFirst = true;
    //         continue;
    //     }

    //     for (int attempts = 0; attempts < 5; attempts++) {
    //         if (myGraph.lineNode->drawSegment(prevPoint, linePoint, myGraph.thickness, ccc4FFromccc4B(color)))
    //             break;
    //     }
    //     prevPoint = linePoint;
    // }

    // graphsContainer->addChild(myGraph.lineNode);

    // if (!allGraphs.contains(graphName))
    //     allGraphs.emplace(graphName, myGraph);
}

void DTGraphNode::setGraphColorByName(const std::string& graphName, const ccColor4B& newColor){
    // if (!allGraphs.contains(graphName)) return;

    // allGraphs[graphName].lineNode->clear();

    // CCPoint prevPoint;
    // bool isFirst = false;

    // for (const CCPoint& linePoint : allGraphs[graphName].points)
    // {
    //     if (isFirst){
    //         isFirst = true;
    //         prevPoint = linePoint;
    //         continue;
    //     }

    //     for (int attempts = 0; attempts < 5; attempts++) {
    //         if (allGraphs[graphName].lineNode->drawSegment(prevPoint, linePoint, allGraphs[graphName].thickness, ccc4FFromccc4B(newColor)))
    //             break;
    //     }

    //     prevPoint = linePoint;
    // }
}

void DTGraphNode::eraseGraphByName(const std::string& graphName){
    // if (!allGraphs.contains(graphName)) return;

    // allGraphs[graphName].lineNode->removeMeAndCleanup();
    // allGraphs[graphName].GraphPointsContainer->removeMeAndCleanup();
    // allGraphs[graphName].points.clear();

    // allGraphs.erase(graphName);
}

