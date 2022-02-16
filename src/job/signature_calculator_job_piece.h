#pragma once

#include <vector>
#include <atomic>

#include "base/base_job.h"

namespace job
{

    class SignatureCalculatorJobContext
    {
        using result_type = std::vector<std::string>;

    public:
        void InitResults(const size_t total)
        {
            m_results.insert(std::begin(m_results), total, {});
        }

        void AddResult(std::string &&result, const size_t idx)
        {
            m_results[idx] = std::move(result);
        }

        void AddTasksSize(const size_t size_bytes)
        {
            m_current_tasks_size_bytes += size_bytes;
        }

        void SubtractTasksSize(const size_t size_bytes)
        {
            m_current_tasks_size_bytes -= size_bytes;
        }

        size_t GetTasksSize() const
        {
            return m_current_tasks_size_bytes;
        }

        const result_type &GetResults() const
        {
            return m_results;
        }

    private:
        std::atomic<size_t> m_current_tasks_size_bytes{0};
        result_type m_results;
    };

    template <typename DigestType>
    class SignatureCalculatorJobPiece : public BaseJobPiece
    {
    public:
        SignatureCalculatorJobPiece(SignatureCalculatorJobContext &context, std::vector<char> &&file_chunk, const size_t result_idx)
            : m_context(context)
            , m_file_chunk(std::move(file_chunk))
            , m_result_idx(result_idx)
        {
        }

        virtual void Process() final override
        {
            DigestType digest;
            const auto chunk_size = m_file_chunk.capacity();
            digest.update(m_file_chunk.data(), chunk_size);
            const auto &res = digest.final();

            m_context.AddResult(digest.BytesToHEX(res.data(), res.size(), true), m_result_idx);
            m_context.SubtractTasksSize(chunk_size);
        }

    private:
        std::vector<char> m_file_chunk;
        SignatureCalculatorJobContext &m_context;
        const size_t m_result_idx;
    };

} // namespace job