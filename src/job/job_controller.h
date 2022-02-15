#pragma once

#include "base/base_job_controller.h"

#include <atomic>

namespace boost
{
    namespace asio
    {
        class thread_pool;
    }
}

namespace job
{

    class JobController final : public BaseJobController
    {
    public:
        JobController();

        virtual ~JobController();

        virtual bool DoJob(std::unique_ptr<BaseJob> job) final override;

        virtual void Stop() final override;

    private:
        bool ShouldStop() const;
        void ForceStop();
        void WaitForFinish();
        void ExceptionCatcherWrapper(const std::function<void()> &job);

        std::unique_ptr<boost::asio::thread_pool> m_thread_pool;
        std::atomic_bool m_stop_flag{false};
    };

}