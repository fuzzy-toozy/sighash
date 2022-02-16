#include <iostream>
#include <thread>

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

#include "job/job_controller.h"

namespace job
{

    JobController::JobController()
        : m_thread_pool(std::make_unique<boost::asio::thread_pool>(std::thread::hardware_concurrency() - 1))
    {
    }

    bool JobController::DoJob(std::unique_ptr<BaseJob> job)
    {
        bool result{false};
        try
        {
            while (!ShouldStop() && job->HasMoreJob())
            {
                if (job->UnfinishedJobsLimitReached())
                {
                    continue;
                }

                auto new_job = job->MakeJob();
                auto wrapped_job = [this, job = std::move(new_job)]()
                {
                    ExceptionCatcherWrapper(job);
                };

                boost::asio::post(*m_thread_pool, std::move(wrapped_job));
            }
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Exception during job creaton: " << ex.what() << std::endl;
            ForceStop();
        }

        WaitForFinish();

        if (!ShouldStop())
        {
            try
            {
                job->Finish();
                result = true;
            }
            catch (const std::exception &ex)
            {
                std::cerr << "Exception during processing job results." << std::endl;
            }
        }

        return result;
    }

     JobController::~JobController()
     {}

    void JobController::Stop()
    {
        m_stop_flag = true;
    }


    bool JobController::ShouldStop() const
    {
        return m_stop_flag;
    }


    void JobController::ForceStop()
    {
        Stop();
        m_thread_pool->stop();
    }


    void JobController::WaitForFinish()
    {
        m_thread_pool->wait();
        m_thread_pool->join();
    }


    void JobController::ExceptionCatcherWrapper(const std::function<void()> &job)
    {
        try
        {
            job();
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Exception while running job: " << ex.what() << std::endl;
            ForceStop();
        }
    }


} // namespace job