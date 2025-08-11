#include <Geode/modify/CCLabelBMFont.hpp>

using namespace geode::prelude;

#include <codecvt>
#include <locale>

class $modify(DTCCLabelBMFont, CCLabelBMFont) {
    struct Fields{
        std::optional<ccColor3B> lastColor = std::nullopt;
    };

    public:
        void updateLabel();

    private:

};