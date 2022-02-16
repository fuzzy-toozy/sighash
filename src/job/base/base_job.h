#pragma once

#include <functional>
#include <stdexcept>
#include <memory>
#include <utility>

namespace job
{
    class BaseJobPiece
    {
    public:
        virtual ~BaseJobPiece() = default;
        virtual void Process() = 0;
    };

    class BaseJob
    {
    public:
        virtual ~BaseJob() = default;

        virtual std::function<void()> MakeJob()
        {
            if (UnfinishedJobsLimitReached())
            {
                throw std::runtime_error("Can't produce more tasks. Unfinished tasks size limit reached!");
            }
            auto job_piece = MakeJobPiece();
            return [this, job_piece = std::move(job_piece)]()
            { job_piece->Process(); };
        }

        virtual bool HasMoreJob() const = 0;
        virtual bool UnfinishedJobsLimitReached() const = 0;
        virtual void Finish() = 0;
        virtual std::shared_ptr<BaseJobPiece> MakeJobPiece() = 0;
    };

} // namespace job