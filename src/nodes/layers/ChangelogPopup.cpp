#include "ChangelogPopup.hpp"

ChangelogPopup* ChangelogPopup::create() {
    auto ret = new ChangelogPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

// a little help from better edit on this one :)
bool ChangelogPopup::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(200, 250, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Changelog");
    

    auto changelog = Mod::get()->getMetadata().getChangelog().value_or("No changelog found!");
    auto version = Mod::get()->getVersion();
    std::string changes;

    bool validHeader = false;
    for (auto line : string::split(changelog, "\n")) {
        string::trimIP(line);
        if (line.empty()) continue;
        
        auto it = line.begin();
        switch (*it) {
            case '#': {
                while (it != line.end() && (*it == '#' || std::isspace(*it))) {
                    it++;
                }
                auto parse = VersionInfo::parse(std::string(it, line.end()));

                if (parse.isErr()) continue;

                auto ver = parse.unwrap();

                validHeader = version == ver;
                if (validHeader)
                    changes += "\n# " + ver.toNonVString() + "\n\n";

            } break;

            default: {
                if (changes.empty() || !validHeader) continue;

                changes += line + "\n";
            } break;
        }
    }

    auto textArea = MDTextArea::create(changes, m_size - ccp(15, 15 + m_title->getScaledContentHeight() / 2 + (m_size.height - m_title->getPositionY())));
    m_mainLayer->addChildAtPosition(textArea, Anchor::Center);
    textArea->setPositionY(textArea->getPositionY() - m_title->getScaledContentHeight() / 2);


    return true;
}