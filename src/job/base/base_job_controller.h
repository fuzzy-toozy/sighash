#pragma once

#include <memory>
#include "base_job.h"

namespace job
{

    class BaseJobController
    {
    public:
        virtual ~BaseJobController() = default;

        virtual void Stop() = 0;
        virtual bool DoJob(std::unique_ptr<BaseJob> job) = 0;
    };

} // namespace job
