#pragma once

#include <functional>

namespace job
{

    class BaseJob
    {
    public:
        BaseJob() {}
        virtual ~BaseJob() = default;
        virtual std::function<void()> MakeJob() = 0;
        virtual bool HasMoreJob() const = 0;
        virtual bool UnfinishedJobsLimitReached() const = 0;
        virtual void Finish() = 0;
    };

} // namespace job