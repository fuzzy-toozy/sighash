#include <atomic>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "base/base_job.h"


namespace job
{

    static inline constexpr size_t gkDefaultBlockSize = 1048576u;

    template <typename DigestType>
    class SignatureCalculatorJob final : public BaseJob
    {
    public:
        SignatureCalculatorJob(const std::filesystem::path &in_file_path,
                               const std::filesystem::path &out_file_path,
                               const size_t chunk_size_bytes = gkDefaultBlockSize)
            : m_in_file_path(in_file_path)
            , m_out_file_path(out_file_path)
            // Так максимально загрузим воркеры, хотя по памяти может быть неочень конечно
            , m_tasks_size_limit_bytes(2 * chunk_size_bytes * (std::thread::hardware_concurrency()))
            , m_chunk_size_bytes(chunk_size_bytes)
        {
            // За одно полетит исключение если файла нет
            m_file_size_bytes = std::filesystem::file_size(m_in_file_path);
            if (m_file_size_bytes < m_chunk_size_bytes)
            {
                throw std::runtime_error("File size is less than a block size");
            }

            m_in_file_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            m_in_file_stream.open(m_in_file_path, std::ios::binary);
            size_t total_chunks = m_file_size_bytes / m_chunk_size_bytes;
            // Если есть неполный чанк в конце - добавляем
            total_chunks = (m_file_size_bytes % m_chunk_size_bytes > 0) ? total_chunks + 1 : total_chunks;
            m_results.insert(std::begin(m_results), total_chunks, {});
        }

        virtual ~SignatureCalculatorJob() = default;

        virtual std::function<void()> MakeJob() final override
        {
            if (UnfinishedJobsLimitReached())
            {
                throw std::runtime_error("Can't produce more tasks. Unfinished tasks size limit reached!");
            }

            std::vector<char> file_chunk;
            ReadFileChunk(file_chunk);
            m_current_tasks_size_bytes += file_chunk.capacity();

            auto task = [this, file_chunk = std::move(file_chunk), result_idx = m_result_idx]()
            { ProcessFileChunk(file_chunk, result_idx); };
            ++m_result_idx;

            return task;
        }

        virtual bool UnfinishedJobsLimitReached() const final override
        {
            return m_current_tasks_size_bytes >= m_tasks_size_limit_bytes;
        }

        virtual bool HasMoreJob() const override
        {
            return m_read_bytes != m_file_size_bytes;
        }

        virtual void Finish() final override
        {
            try
            {
                std::ofstream ofs(m_out_file_path);
                for (const auto &job_result : m_results)
                {
                    ofs << job_result << std::endl;
                }
            }
            catch (const std::exception &ex)
            {
                std::cerr << "File operation failed: " << ex.what() << std::endl;
                return;
            }

            std::cout << "Signature saved to: " << m_out_file_path << std::endl;
        }

    private:
        void ProcessFileChunk(const std::vector<char> &file_chunk, const size_t result_idx)
        {
            DigestType digest;
            const auto chunk_size = file_chunk.capacity();
            digest.update(file_chunk.data(), chunk_size);
            const auto &res = digest.final();
            m_results[result_idx] = digest.BytesToHEX(res.data(), res.size(), true);
            m_current_tasks_size_bytes -= chunk_size;
        }

        void ReadFileChunk(std::vector<char> &read_buf)
        {
            size_t left_to_process = m_file_size_bytes - m_read_bytes;
            const size_t chunk_to_read = (left_to_process < m_chunk_size_bytes) ? left_to_process : m_chunk_size_bytes;
            // Если использовать конструктор с вставкой элементов или insert, то вызывается куча бесполезных memset'ов
            read_buf.reserve(chunk_to_read);
            m_in_file_stream.read(read_buf.data(), read_buf.capacity());
            m_read_bytes += read_buf.capacity();
        }

        // Сюда кладем результаты работы каждой таки
        std::vector<std::string> m_results;

        std::filesystem::path m_in_file_path;
        std::filesystem::path m_out_file_path;
        std::ifstream m_in_file_stream;

        // Ограничение сверху для размера данных незавершенных задач чтобы не кушать много памяти за зря
        const size_t m_tasks_size_limit_bytes{0};

        // 
        const size_t m_chunk_size_bytes{0};
        std::atomic<size_t> m_current_tasks_size_bytes{0};
        size_t m_read_bytes{0};
        size_t m_result_idx{0};
        size_t m_file_size_bytes{0};
    };

} // namespace job