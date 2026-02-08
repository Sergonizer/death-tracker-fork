#pragma once

#include <Geode/utils/async.hpp>

template<class R, class P>
struct ProgressFuture : arc::Pollable<ProgressFuture<R, P>, R> {

    using Result = R;
    using Progress = P;
    using ProgressFunc = geode::FunctionRef<void(const P&)>;
    using InnerFuture = arc::Future<Result>;

    ProgressFuture(
        const geode::Function<InnerFuture(const ProgressFunc&)>& func, 
        const geode::Function<void(std::weak_ptr<Progress>)>& progress
    );
    ~ProgressFuture();

    ProgressFuture(ProgressFuture&&) noexcept = default;
    ProgressFuture& operator=(ProgressFuture&&) noexcept = delete;
    ProgressFuture(ProgressFuture const&) = delete;
    ProgressFuture& operator=(ProgressFuture const&) = delete;

    std::optional<ProgressFuture> poll(arc::Context& cx);

private:
    const geode::Function<InnerFuture(const ProgressFunc&)> m_func;
    geode::Function<void(std::weak_ptr<Progress>)> m_progress;

    std::optional<InnerFuture> m_future;
    bool m_started = false;
};