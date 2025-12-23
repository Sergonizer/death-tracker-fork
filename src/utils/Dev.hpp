#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class Dev {
public:
    static const bool ENABLED;
    static const bool MINIFY_SAVE_FILE;

    static void fadeTextInput(TextInput* const input, bool fadeIn, float time, bool changeEnabled = true);
};