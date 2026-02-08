#include "ProgressFuture.hpp"

template<class R, class P>
ProgressFuture<R, P>::ProgressFuture(
    const geode::Function<InnerFuture(const ProgressFunc&)>& func, 
    const geode::Function<void(std::weak_ptr<Progress>)>& progress)
    : m_func(std::move(func)), m_progress(std::move(progress)) {}

template<class R, class P>
ProgressFuture<R, P>::~ProgressFuture() = default;

template<class R, class P>
std::optional<ProgressFuture<R, P>> ProgressFuture<R, P>::poll(arc::Context& cx) {
    if (!m_started) {
        m_started = true;

        auto progress = std::make_shared<Progress>();
        std::weak_ptr<Progress> weakProgress = progress;

        ProgressFunc progressFunc = [this, progress, weakProgress](const P& value) {
            *progress = value;

            if (m_progress) {
                m_progress(weakProgress);
            }
        };

        m_future.emplace(m_func(progressFunc));
    }

    auto result = m_future->poll(cx);

    if (!result.has_value()) {
        return std::nullopt;
    }

    return std::move(*result);
}