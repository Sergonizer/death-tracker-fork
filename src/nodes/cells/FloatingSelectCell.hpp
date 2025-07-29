#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FloatingSelectCell : public CCNode {
    public:
        static FloatingSelectCell* create(float width, const std::string& optionText, const std::function<void(FloatingSelectCell*)>& callback);
      
        SimpleTextArea* optionLabel;
    private:
        bool init(float width, const std::string& optionText, const std::function<void(FloatingSelectCell*)>& callback);

        std::function<void(FloatingSelectCell*)> callback = NULL;
};