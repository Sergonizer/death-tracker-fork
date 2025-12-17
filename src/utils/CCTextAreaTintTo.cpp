#include <utils/CCTextAreaTintTo.hpp>

CCTextAreaTintTo* CCTextAreaTintTo::create(float duration, const ccColor3B& toColor)
{
    auto ret = new CCTextAreaTintTo();
    if (ret && ret->initWithDuration(duration, toColor)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCTextAreaTintTo::initWithDuration(float duration, const ccColor3B& toColor)
{
    if (!CCActionInterval::initWithDuration(duration)) return false;

    this->endColor = toColor;
    return true;
}

void CCTextAreaTintTo::startWithTarget(CCNode* target)
{
    CCActionInterval::startWithTarget(target);

    auto textArea = dynamic_cast<SimpleTextArea*>(target);
    if (!textArea) return;

    auto c = textArea->getColor();
    this->startColor = { c.r, c.g, c.b };
    this->deltaR = (int)endColor.r - (int)startColor.r;
    this->deltaG = (int)endColor.g - (int)startColor.g;
    this->deltaB = (int)endColor.b - (int)startColor.b;
}

void CCTextAreaTintTo::update(float time)
{
    if (!m_pTarget) return;

    auto textArea = dynamic_cast<SimpleTextArea*>(m_pTarget);
    if (!textArea) return;

    const int r = (int)startColor.r + (int)(deltaR * time);
    const int g = (int)startColor.g + (int)(deltaG * time);
    const int b = (int)startColor.b + (int)(deltaB * time);

    auto old = textArea->getColor();
    ccColor4B newColor = { (GLubyte)r, (GLubyte)g, (GLubyte)b, old.a };
    textArea->setColor(newColor);
}
