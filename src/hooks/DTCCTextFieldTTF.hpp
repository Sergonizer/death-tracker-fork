#include <Geode/modify/CCTextFieldTTF.hpp>

using namespace geode::prelude;

class $modify(DTCCTextFieldTTF, CCTextFieldTTF) {
    public:
        void DTInsertText(const char* text, int len, enumKeyCodes key);
};