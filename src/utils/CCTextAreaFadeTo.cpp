#include <utils/CCTextAreaFadeTo.hpp>

CCTextAreaFadeTo* CCTextAreaFadeTo::create(float duration, GLubyte toOpacity)
{
    auto ret = new CCTextAreaFadeTo();
    if (ret && ret->initWithDuration(duration, toOpacity)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCTextAreaFadeTo::initWithDuration(float duration, GLubyte toOpacity)
{
    if (!CCActionInterval::initWithDuration(duration)) return false;

    this->endOpacity = toOpacity;
    return true;
}

void CCTextAreaFadeTo::startWithTarget(CCNode* target)
{
    CCActionInterval::startWithTarget(target);

    auto textArea = dynamic_cast<SimpleTextArea*>(target);
    if (!textArea) return;

    auto c = textArea->getColor();
    this->startOpacity = c.a;
    this->deltaOpacity = (int)endOpacity - (int)startOpacity;
}

void CCTextAreaFadeTo::update(float time)
{
    if (!m_pTarget) return;

    auto textArea = dynamic_cast<SimpleTextArea*>(m_pTarget);
    if (!textArea) return;

    const int a = (int)startOpacity + (int)(deltaOpacity * time);

    auto old = textArea->getColor();
    ccColor4B newColor = { old.r, old.g, old.b, (GLubyte)a };
    textArea->setColor(newColor);
}
