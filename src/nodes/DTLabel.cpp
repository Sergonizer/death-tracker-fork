#include <nodes/DTLabel.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <regex>

DTLabel* DTLabel::create(const DTLabelInfo& info, float gridSize) {
    auto ret = new DTLabel();
    if (ret && ret->init(info, gridSize)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DTLabel::init(const DTLabelInfo& info, float gridSize){
    if (!CCNode::init()) return false;

    this->labelInfo = info;
    this->gridSize = gridSize;

    if (labelInfo.X == -1) labelInfo.X = 5000 / gridSize / 2;
    if (labelInfo.Y == -1) labelInfo.Y = 5000 / gridSize / 2;

    textArea = SimpleTextArea::create(labelInfo.text, labelInfo.font, labelInfo.scale);
    textArea->setZOrder(3);
    this->addChild(textArea);

    editVisualsContainer = CCNode::create();
    this->addChild(editVisualsContainer);

    contentOutline = CCScale9Sprite::createWithSpriteFrameName("square_01_small_001.png");
    contentOutline->setAnchorPoint({0, 0});
    contentOutline->setZOrder(2);
    contentOutline->setVisible(false);
    editVisualsContainer->addChild(contentOutline);

    selectedShadow = CCScale9Sprite::createWithSpriteFrameName("playerSquare_001.png");
    selectedShadow->setAnchorPoint({0, 0});
    selectedShadow->setZOrder(1);
    selectedShadow->setColor({0, 0, 0});
    selectedShadow->setOpacity(100);
    selectedShadow->setVisible(false);
    editVisualsContainer->addChild(selectedShadow);

    menu = CCMenu::create();
    menu->setPosition({0, 0});
    this->addChild(menu);

    clickHitbox = CCMenuItem::create(this, menu_selector(DTLabel::onSelected));
    clickHitbox->setEnabled(false);
    clickHitbox->setAnchorPoint({0, 0});
    menu->addChild(clickHitbox);

    auto circleParent = CCLayer::create();
    this->addChild(circleParent);

    loadingCircle = LoadingCircle::create();
    loadingCircle->setParentLayer(circleParent);
    loadingCircle->show();
    loadingCircle->setVisible(false);

    updateState();

    return true;
}

void DTLabel::updateState(){
    updateTransform();
    
    updateText();
}

void DTLabel::updateTransform(){
    auto localPos = gridToLocalPosition(labelInfo.X, labelInfo.Y);

    this->setPosition({localPos.x + gridSize / 2, localPos.y + gridSize / 2});

    this->setScale(labelInfo.scale);
}

void DTLabel::updateText(){
    if (!runningTask.isNull())
        runningTask.cancel();

    runningTask = textUpdateTask::run([&](auto progress, auto hasBeenCancelled) -> textUpdateTask::Result{
        loadingCircle->setVisible(true);

        usedKeys.clear();

        Loader::get()->queueInMainThread([&] {
            this->setContentSize(labelInfo.contentSize);

            textArea->setWidth(this->getContentWidth());

            this->setAnchorPoint({
                static_cast<int>(labelInfo.horizontalAlignment) / 2.f,
                static_cast<int>(labelInfo.verticalAlignment) / 2.f
            });

            textArea->setAlignment(labelInfo.horizontalAlignment);

            auto modifiedStr = modifyText(labelInfo.text);
            textArea->setText(modifiedStr);

            textArea->setFont(labelInfo.font);

            textArea->setColor(labelInfo.color);

            if (!labelInfo.infinityResize){
                float overallHeight = 0;
                int lineAmount = 0;
                for (const auto& line : textArea->getLines())
                {
                    overallHeight += line->getContentHeight() + textArea->getLinePadding();
                    if (overallHeight > this->getContentHeight()) break;
                    lineAmount++;
                }
                
                textArea->setMaxLines(lineAmount);
            }
            else{
                textArea->setMaxLines(0);
                this->setContentHeight(textArea->getContentHeight());
            }

            textArea->setPositionX(textArea->getWidth() / 2);
            textArea->setPositionY(this->getContentHeight() / 2);

            if (labelInfo.verticalAlignment != CCTextAlignment::kCCTextAlignmentCenter){
                if (labelInfo.verticalAlignment == CCTextAlignment::kCCTextAlignmentLeft)
                    textArea->setPositionY(textArea->getPositionY() - (this->getContentHeight() - textArea->getContentHeight()) / 2);
                else
                    textArea->setPositionY(textArea->getPositionY() + (this->getContentHeight() - textArea->getContentHeight()) / 2);
            }
            
            if (colorData.size()){
                auto lines = textArea->getLines();

                int overallIndex = 0;

                for (const auto& label : lines)
                {
                    int localIndex = 0;
                    std::string text = label->getString();

                    for (const auto& character : CCArrayExt<CCFontSprite*>(label->getChildren()))
                    {
                        if (localIndex == text.size()){
                            continue;
                        }

                        if (colorData.contains(overallIndex))
                            currentColor = colorData[overallIndex];

                        if (currentColor != std::nullopt)
                            character->setColor(currentColor.value());
                        
                        overallIndex++;
                        localIndex++;
                    }

                    if (label->getChildrenCount() == 0)
                        overallIndex++;

                    if (modifiedStr.size() && modifiedStr.size() > overallIndex && modifiedStr[overallIndex] == '\n')
                        overallIndex++;

                    if (colorData.contains(overallIndex))
                        currentColor = colorData[overallIndex];
                }
            }

            contentOutline->setContentSize(this->getContentSize());
            selectedShadow->setContentSize(this->getContentSize());
            clickHitbox->setContentSize(this->getContentSize());
            
            loadingCircle->setPosition(-loadingCircle->getContentSize() / 2 + this->getContentSize() / 2);
            loadingCircle->setVisible(false);
        });

        return true;
    });

    textUpdateListener.bind([&](textUpdateTask::Event* e){
        if (auto value = e->getValue()){
            loadingCircle->setVisible(false);
        }
        else if (e->isCancelled()){ //on cancled
            loadingCircle->setVisible(false);
        }
    });
    textUpdateListener.setFilter(runningTask);
}

CCPoint DTLabel::localToGridPosition(CCPoint localPosition){
    CCPoint gridPos = localPosition / gridSize;
    gridPos.x = static_cast<int>(gridPos.x);
    gridPos.y = static_cast<int>(gridPos.y);

    return gridPos;
}

CCPoint DTLabel::gridToLocalPosition(int x, int y){
    CCPoint localPos;
    localPos.x = x * gridSize;
    localPos.y = y * gridSize;

    return localPos;
}

std::string DTLabel::modifyText(const std::string& str){

    struct KeyParams{
        public:
            DTLayer* const dtLayer;
            const std::string& value;
            int insertIndex;
            bool isCancellation;

            KeyParams(DTLayer* dt, const std::string& val, int idx, bool cancel)
            : dtLayer(dt), value(val), insertIndex(idx), isCancellation(cancel) {}
    };

    using KeyResult = std::optional<std::string>;
    using KeyFunc = std::function<KeyResult(const KeyParams&)>;

    #define LAMBDA [](KeyParams) -> KeyResult

    std::map<std::string, KeyFunc> keysPossible{
        {"nl", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "\n";
        }},
        {"f0", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "deaths string aaa";
        }},
        {"runs", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "runs string";
        }},
        {"lvln", [&](const KeyParams& params) -> KeyResult {
            if (!params.dtLayer || params.isCancellation) return std::nullopt;
            return params.dtLayer->m_Level->m_levelName;
        }},
        {"att", [&](const KeyParams& params) -> KeyResult {
            if (!params.dtLayer || params.isCancellation) return std::nullopt;
            return std::to_string(params.dtLayer->m_Level->m_attempts.value());
        }},
        {"s0", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "selected session f0 string";
        }},
        {"sruns", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "selected session run string";
        }},
        {"ptf0", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "playtime from 0";
        }},
        {"ptrun", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "playtime in runs";
        }},
        {"ptall", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "playtime overall";
        }},
        {"ssd", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "selected session date";
        }},
        {"sst", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            return "selected session time";
        }},
        {"color", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation){
                colorData.insert({params.insertIndex, std::nullopt});
                return "";
            }

            auto hexRes = cocos::cc3bFromHexString(params.value);
            if (hexRes.isErr()) return std::nullopt;
            colorData.insert({params.insertIndex, hexRes.unwrap()});
            return "";
        }},
        {"nbc", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            colorData.insert({params.insertIndex, Save::getNewBestColor()});
            return "";
        }},
        {"sbc", [&](const KeyParams& params) -> KeyResult {
            if (params.isCancellation) return std::nullopt;
            colorData.insert({params.insertIndex, Save::getSessionBestColor()});
            return "";
        }},
    };

    colorData.clear();
    currentColor = std::nullopt;

    std::regex specialKeyRegex(R"(\{([A-Za-z0-9_\\]+)(?:-([^{}]*))?\})");

    std::string result;
    std::sregex_iterator begin(str.begin(), str.end(), specialKeyRegex);
    std::sregex_iterator end;

    int lastPos = 0;
    for (auto it = begin; it != end; ++it) {
        const auto& match = *it;

        int matchStart = match.position();
        int matchEnd = matchStart + match.length();

        result += str.substr(lastPos, matchStart - lastPos);

        std::string key = match.str(1);
        
        bool isCancellation = false;
        if (key.size() && key[0] == '\\'){
            isCancellation = true;
            key = key.erase(0, 1);
        }

        std::string value = match.size() > 2 ? match.str(2) : "";

        auto dtLayer = DTLayer::get();

        if (keysPossible.contains(key)) {
            int insertIndex = result.size();

            auto currentRes = keysPossible[key](KeyParams(dtLayer, value, insertIndex, isCancellation));
            
            result += !currentRes.has_value() ? match.str() : currentRes.value();
        }
        else {
            result += match.str();
        }

        lastPos = matchEnd;
    }

    result += str.substr(lastPos);

    return result;
}

void DTLabel::enterEditMode(){
    contentOutline->setVisible(true);
    clickHitbox->setEnabled(true);
    labelInfoBackup = labelInfo;
}

void DTLabel::exitEditMode(){
    contentOutline->setVisible(false);
    clickHitbox->setEnabled(false);
    wasCreatedThisEdit = false;

    if (markedForDeletion){
        auto dtLayer = DTLayer::get();
        if (dtLayer != nullptr)
            dtLayer->removeLabel(this, true);
        else
            this->removeMeAndCleanup();
    }
}

void DTLabel::onSelected(CCObject*){
    clickHitbox->setEnabled(false);
    isSelected = true;
    selectedShadow->setVisible(true);
    if (onClicked != NULL)
        onClicked(this);
}

void DTLabel::onDeselected(){
    clickHitbox->setEnabled(true);
    selectedShadow->setVisible(false);
    isSelected = false;
}

bool DTLabel::touchMoved(CCTouch* touch){
    if (!isSelected || isTouchlocked) return false;
    auto touchLocalPos = this->getParent()->convertTouchToNodeSpace(touch);
    touchLocalPos.x = std::clamp(touchLocalPos.x, 0.0f, this->getParent()->getContentWidth() - gridSize);
    touchLocalPos.y = std::clamp(touchLocalPos.y, 0.0f, this->getParent()->getContentHeight() - gridSize);
    if (!isTouching && !this->boundingBox().containsPoint(touchLocalPos)){
        isTouchlocked = true;
        return false;
    }

    auto gridPos = localToGridPosition(touchLocalPos);

    if (!isTouching)
        touchStartGridPoint = gridPos;

    isTouching = true;

    if (touchStartGridPoint == gridPos) return true;

    labelInfo.X = gridPos.x;
    labelInfo.Y = gridPos.y;

    updateState();

    touchStartGridPoint = ccp(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

    return true;
}

void DTLabel::touchEnded(CCTouch* touch){
    isTouching = false;
    isTouchlocked = false;
}

void DTLabel::revert(){
    if (wasCreatedThisEdit){
        auto dtlayer = DTLayer::get();
        if (dtlayer != nullptr)
            dtlayer->removeLabel(this, true);
        else
            this->removeMeAndCleanup();

        return;
    }

    if (markedForDeletion){
        menu->setEnabled(true);
        textArea->setVisible(true);
        editVisualsContainer->setVisible(true);
        markedForDeletion = false;
    }

    labelInfo = labelInfoBackup;

    updateState();
}

void DTLabel::softDelete(){
    markedForDeletion = true;
    menu->setEnabled(false);
    textArea->setVisible(false);
    editVisualsContainer->setVisible(false);
}

// , {
//         "X": 50,
//         "Y": 50,
//         "text": "Session:{nl}{ssd}{nl}{s0}{nl} ",
//         "font": "bigFont.fnt",
//         "horizontalAlignment": 1,
//         "verticalAlignment": 1,
//         "color": {
//             "r": 255,
//             "g": 217,
//             "b": 166,
//             "a": 255
//         },
//         "scale": 0.5,
//         "contentWidth": 100,
//         "contentHeight": 100,
//         "infinityResize": false
//     }, {
//         "X": 50,
//         "Y": 50,
//         "text": "Runs:{nl}{runs}{nl} ",
//         "font": "bigFont.fnt",
//         "horizontalAlignment": 1,
//         "verticalAlignment": 1,
//         "color": {
//             "r": 85,
//             "g": 168,
//             "b": 112,
//             "a": 255
//         },
//         "scale": 0.5,
//         "contentWidth": 100,
//         "contentHeight": 100,
//         "infinityResize": false
//     }, {
//         "X": 50,
//         "Y": 50,
//         "text": "Session Runs:{nl}{sruns}{nl} ",
//         "font": "bigFont.fnt",
//         "horizontalAlignment": 1,
//         "verticalAlignment": 1,
//         "color": {
//             "r": 251,
//             "g": 176,
//             "b": 94,
//             "a": 255
//         },
//         "scale": 0.5,
//         "contentWidth": 100,
//         "contentHeight": 100,
//         "infinityResize": false
//     }, {
//         "X": 50,
//         "Y": 50,
//         "text": "{lvln}:",
//         "font": "goldFont.fnt",
//         "horizontalAlignment": 1,
//         "verticalAlignment": 1,
//         "color": {
//             "r": 255,
//             "g": 255,
//             "b": 255,
//             "a": 255
//         },
//         "scale": 1,
//         "contentWidth": 100,
//         "contentHeight": 100,
//         "infinityResize": false
//     }, {
//         "X": 50,
//         "Y": 50,
//         "text": "{att} attempts",
//         "font": "chatFont.fnt",
//         "horizontalAlignment": 1,
//         "verticalAlignment": 1,
//         "color": {
//             "r": 255,
//             "g": 255,
//             "b": 255,
//             "a": 255
//         },
//         "scale": 0.75,
//         "contentWidth": 100,
//         "contentHeight": 100,
//         "infinityResize": false
//     }, {
//         "X": 50,
//         "Y": 50,
//         "text": "playtime - {ptall}",
//         "font": "gjFont17.fnt",
//         "horizontalAlignment": 1,
//         "verticalAlignment": 1,
//         "color": {
//             "r": 255,
//             "g": 255,
//             "b": 255,
//             "a": 255
//         },
//         "scale": 0.44999998807907104,
//         "contentWidth": 100,
//         "contentHeight": 100,
//         "infinityResize": false
//     }