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

    textArea = SimpleTextArea::create(labelInfo.text, labelInfo.font, labelInfo.scale);
    this->addChild(textArea);

    updateState();

    return true;
}

void DTLabel::updateState(){
    auto localPos = gridToLocalPosition(labelInfo.X, labelInfo.Y);

    this->setPosition({localPos.x + gridSize / 2, localPos.y + gridSize / 2});

    this->setContentSize(labelInfo.contentSize);
    this->setAnchorPoint({
        static_cast<int>(labelInfo.horizontalAlignment) / 2.f,
        static_cast<int>(labelInfo.verticalAlignment) / 2.f
    });

    textArea->setText(modifyText(labelInfo.text));
    textArea->setFont(labelInfo.font);

    textArea->setWidth(this->getContentWidth());
    if (!labelInfo.infinityResize){
        textArea->setContentHeight(this->getContentHeight());
    }

    textArea->setAlignment(labelInfo.horizontalAlignment);
    textArea->setColor(labelInfo.color);
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
    std::map<std::string, std::string> keysPossible{
        {"nl", "\n"},
        {"f0", DTLayer::get() == nullptr ? "" : "death string"},
        {"runs", DTLayer::get() == nullptr ? "" : "runs string"},
        {"lvln", DTLayer::get() == nullptr ? "" : DTLayer::get()->m_Level->m_levelName},
        {"att", DTLayer::get() == nullptr ? "" : std::to_string(DTLayer::get()->m_Level->m_attempts.value())},
        {"s0", DTLayer::get() == nullptr ? "" : "selected session f0 string"},
        {"sruns", DTLayer::get() == nullptr ? "" : "selected session run string"},
        {"ptf0", DTLayer::get() == nullptr ? "" : /*StatsManager::workingTime(playtime from 0)*/ "playtime from 0"},
        {"ptrun", DTLayer::get() == nullptr ? "" : /*StatsManager::workingTime(playtime in runs)*/ "playtime in runs"},
        {"ptall", DTLayer::get() == nullptr ? "" : /*StatsManager::workingTime(playtime overall)*/ "playtime overall"},
        {"ssd", DTLayer::get() == nullptr ? "" : "selected session date"},
        {"sst", DTLayer::get() == nullptr ? "" : "selected session time"},
    };

    std::regex specialKeyRegex(R"(\{([^{}]*(?:\{[^{}]*\}[^{}]*)*)\})");

    std::string result;
    std::sregex_iterator begin(str.begin(), str.end(), specialKeyRegex);
    std::sregex_iterator end;

    size_t lastPos = 0;
    for (auto it = begin; it != end; ++it) {
        const auto& match = *it;

        size_t matchStart = match.position();
        size_t matchEnd = matchStart + match.length();

        // Add text before match
        result += str.substr(lastPos, matchStart - lastPos);

        // Extract the key without the braces
        std::string key = match.str(1); // group 1: the part inside {}
        
        if (keysPossible.contains(key)){
            if (keysPossible[key] == "")
                result += match.str();
            else
                result += keysPossible[key];
        }
        else
            result += match.str();

        lastPos = matchEnd;
    }

    // Add any remaining text after last match
    result += str.substr(lastPos);
    
    return result;
}
