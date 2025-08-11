#include "../hooks/DTCCLabelBMFont.hpp"

#include <string>
#include <regex>
#include <map>
#include <sstream>
#include <iomanip>

void removeColorTags(
    const std::string& input, 
    std::string& output, 
    std::map<int, std::optional<ccColor3B>>& colorMap)
{
    std::regex colorTagRegex(R"(<c-([0-9A-Fa-f]{6})>)");
    std::regex cancelTagRegex(R"(</c>)");

    output.clear();
    colorMap.clear();

    size_t inputIndex = 0;
    size_t outputIndex = 0;

    while (inputIndex < input.size()) {
        std::string remaining = input.substr(inputIndex);

        // Check cancellation tag first (because it's shorter)
        std::smatch cancelMatch;
        if (std::regex_search(remaining, cancelMatch, cancelTagRegex) && cancelMatch.position() == 0) {
            // Insert null color at current outputIndex (cancellation)
            colorMap[static_cast<int>(outputIndex)] = std::nullopt;
            inputIndex += cancelMatch.length();  // skip cancellation tag
            continue;
        }

        // Check color tag
        std::smatch colorMatch;
        if (std::regex_search(remaining, colorMatch, colorTagRegex) && colorMatch.position() == 0) {
            std::string hex = colorMatch[1].str();

            auto colorRes = cocos::cc3bFromHexString(hex);
            if (colorRes.isErr()){
                output += input[inputIndex];
                ++inputIndex;
                ++outputIndex;
            }
            else{
                ccColor3B color = colorRes.unwrap();
                colorMap[static_cast<int>(outputIndex)] = color;
                inputIndex += colorMatch.length();  // skip color tag
            }
            continue;
        }

        // No tags found at current pos, copy character
        output += input[inputIndex];
        ++inputIndex;
        ++outputIndex;
    }
}

void DTCCLabelBMFont::updateLabel()
{
    std::string cleaned;
    std::map<int, std::optional<ccColor3B>> colors;

    removeColorTags(m_sInitialStringUTF8, cleaned, colors);
    setString(cleaned.c_str());

    
    
    CCLabelBMFont::updateLabel();

    if (colors.size()){
        int index = 0;
        for (const auto& child : CCArrayExt<CCFontSprite*>(this->getChildren()))
        {
            if (colors.contains(index))
                m_fields->lastColor = colors[index];

            if (m_fields->lastColor != std::nullopt)
                child->setColor(m_fields->lastColor.value());

            index++;
        }
        
    }
}

