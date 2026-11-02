#include "packed_sfen.h"
#include "sfen_stream.h"

#include "misc.h"

#include "syzygy/tbprobe.h"

#include <chrono>
#include <cstring>
#include <fstream>
#include <limits>
#include <list>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <atomic>

namespace Stockfish::Tools {

    // Helper class for exporting Sfen
    struct SfenWriter
    {
        // File name to write and number of threads to create
        SfenWriter(std::string filename_, int thread_num, uint64_t save_count, SfenOutputType sfen_output_type, size_t buffer_size = 5000)
            : sfen_write_size(buffer_size)
        {
            sfen_buffers_pool.reserve((size_t)thread_num * 10);
            sfen_buffers.resize(thread_num);
            sfen_buffer_mutexes = std::make_unique<std::mutex[]>(thread_num);
            periodic_flush_interval = std::chrono::minutes(1);
            last_periodic_flush = std::chrono::steady_clock::now();

            auto out = sync_region_cout.new_region();
            out << "INFO (sfen_writer): Creating new data file at " << filename_ << std::endl;

            sfen_format = sfen_output_type;
            output_file_stream = create_new_sfen_output(filename_, sfen_format);
            filename = filename_;
            save_every = save_count;

            finished = false;

            file_worker_thread = std::thread([&] { this->file_write_worker(); });
        }

        ~SfenWriter()
        {
            flush();

            finished = true;
            file_worker_thread.join();
            output_file_stream.reset();

#if !defined(NDEBUG)
            {
                // All buffers should be empty since file_worker_thread
                // should have written everything before exiting.
                for (const auto& p : sfen_buffers) { assert(p == nullptr); (void)p ; }
                assert(sfen_buffers_pool.empty());
            }
#endif
        }

        void write(size_t thread_id, const PackedSfenValue& psv)
        {
            // We have a buffer for each thread and add it there.
            // If the buffer overflows, write it to a file.

            std::unique_ptr<PSVector> full_buf;

            {
                // The buffer is owned by this thread, but the file worker thread
                // may take it away for a periodic flush, so it has to be locked.
                // This lock is practically uncontended.
                std::unique_lock<std::mutex> lk(sfen_buffer_mutexes[thread_id]);

                // This buffer is prepared for each thread.
                auto& buf = sfen_buffers[thread_id];

                // Secure since there is no buf at the first time
                // and immediately after writing the thread buffer.
                if (!buf)
                {
                    buf = std::make_unique<PSVector>();
                    buf->reserve(sfen_write_size);
                }

                buf->push_back(psv);

                if (buf->size() >= sfen_write_size)
                    full_buf = std::move(buf);
            }

            if (full_buf)
            {
                // If you load it in sfen_buffers_pool, the worker will do the rest.

                // Critical section since sfen_buffers_pool is shared among threads.
                std::unique_lock<std::mutex> lk(mutex);
                sfen_buffers_pool.emplace_back(std::move(full_buf));
            }
        }

        void flush()
        {
            for (size_t i = 0; i < sfen_buffers.size(); ++i)
            {
                flush(i);
            }
        }

        // Move what remains in the buffer for your thread to a buffer for writing to a file.
        void flush(size_t thread_id)
        {
            std::unique_ptr<PSVector> buf;
            {
                std::unique_lock<std::mutex> lk(sfen_buffer_mutexes[thread_id]);

                // There is a case that buf==nullptr, so that check is necessary.
                if (sfen_buffers[thread_id] && sfen_buffers[thread_id]->size() != 0)
                    buf = std::move(sfen_buffers[thread_id]);
            }

            if (buf)
            {
                std::unique_lock<std::mutex> lk(mutex);
                sfen_buffers_pool.emplace_back(std::move(buf));
            }
        }

        // Dedicated thread to write to file
        void file_write_worker()
        {
            while (!finished || sfen_buffers_pool.size())
            {
                // Periodically move the partially filled per-thread buffers
                // to the pool so that rarely produced data gets written too.
                // This must not be done while holding the pool mutex,
                // see the lock order in write().
                const auto now = std::chrono::steady_clock::now();
                if (now - last_periodic_flush >= periodic_flush_interval)
                {
                    flush();
                    last_periodic_flush = now;
                }

                std::vector<std::unique_ptr<PSVector>> buffers;
                {
                    std::unique_lock<std::mutex> lk(mutex);

                    // Atomically swap take the filled buffers and
                    // create a new buffer pool for threads to fill.
                    buffers = std::move(sfen_buffers_pool);
                    sfen_buffers_pool = std::vector<std::unique_ptr<PSVector>>();
                }

                if (!buffers.size())
                {
                    // Poor man's condition variable.
                    sleep(100);
                }
                else
                {
                    for (auto& buf : buffers)
                    {
                        output_file_stream->write(*buf);

                        sfen_write_count += buf->size();

                        // Add the processed number here, and if it exceeds save_every,
                        // change the file name and reset this counter.
                        sfen_write_count_current_file += buf->size();
                        if (sfen_write_count_current_file >= save_every)
                        {
                            sfen_write_count_current_file = 0;

                            // Sequential number attached to the file
                            int n = (int)(sfen_write_count / save_every);

                            // Rename the file and open it again.
                            // Add ios::app in consideration of overwriting.
                            // (Depending on the operation, it may not be necessary.)
                            std::string new_filename = filename + "_" + std::to_string(n);
                            output_file_stream = create_new_sfen_output(new_filename, sfen_format);

                            auto out = sync_region_cout.new_region();
                            out << "INFO (sfen_writer): Creating new data file at " << new_filename << std::endl;
                        }
                    }
                }
            }
        }

    private:

        std::unique_ptr<BasicSfenOutputStream> output_file_stream;

        // A new net is saved after every save_every sfens are processed.
        uint64_t save_every = std::numeric_limits<uint64_t>::max();

        // File name passed in the constructor
        std::string filename;

        // Thread to write to the file
        std::thread file_worker_thread;

        // Flag that all threads have finished
        std::atomic<bool> finished;

        SfenOutputType sfen_format;

        // buffer before writing to file
        // sfen_buffers is the buffer for each thread
        // sfen_buffers_pool is a buffer for writing.
        // After loading the phase in the former buffer by SFEN_WRITE_SIZE,
        // transfer it to the latter.
        std::vector<std::unique_ptr<PSVector>> sfen_buffers;
        // Mutexes guarding the per-thread buffers in sfen_buffers
        std::unique_ptr<std::mutex[]> sfen_buffer_mutexes;
        std::vector<std::unique_ptr<PSVector>> sfen_buffers_pool;

        std::chrono::steady_clock::duration periodic_flush_interval;
        std::chrono::steady_clock::time_point last_periodic_flush;

        // Mutex required to access sfen_buffers_pool.
        // Never acquire a mutex in sfen_buffer_mutexes while holding this one.
        std::mutex mutex;

        // Number of sfens written in total, and the
        // number of sfens written in the current file.
        uint64_t sfen_write_count = 0;
        uint64_t sfen_write_count_current_file = 0;

        // Amount of sfens required to flush the buffer.
        size_t sfen_write_size;
    };
}
