#ifndef _SFEN_STREAM_H_
#define _SFEN_STREAM_H_

#include "packed_sfen.h"
#include "sfen_packer.h"

#include <cassert>
#include <optional>
#include <fstream>
#include <string>
#include <memory>

namespace Stockfish::Tools {

    enum struct SfenOutputType
    {
        Bin,
        Epd
    };

    static bool ends_with(const std::string& lhs, const std::string& end)
    {
        if (end.size() > lhs.size()) return false;

        return std::equal(end.rbegin(), end.rend(), lhs.rbegin());
    }

    static bool has_extension(const std::string& filename, const std::string& extension)
    {
        return ends_with(filename, "." + extension);
    }

    static std::string filename_with_extension(const std::string& filename, const std::string& ext)
    {
        if (ends_with(filename, ext))
        {
            return filename;
        }
        else
        {
            return filename + "." + ext;
        }
    }

    struct BasicSfenInputStream
    {
        virtual std::optional<PackedSfenValue> next() = 0;
        virtual bool eof() const = 0;
        virtual ~BasicSfenInputStream() {}
    };

    struct BinSfenInputStream : BasicSfenInputStream
    {
        static constexpr auto openmode = std::ios::in | std::ios::binary;
        static inline const std::string extension = "bin";

        BinSfenInputStream(std::string filename) :
            m_stream(filename, openmode),
            m_eof(!m_stream)
        {
        }

        std::optional<PackedSfenValue> next() override
        {
            PackedSfenValue e;
            if(m_stream.read(reinterpret_cast<char*>(&e), sizeof(PackedSfenValue)))
            {
                return e;
            }
            else
            {
                m_eof = true;
                return std::nullopt;
            }
        }

        bool eof() const override
        {
            return m_eof;
        }

        ~BinSfenInputStream() override {}

    private:
        std::fstream m_stream;
        bool m_eof;
    };

    struct BasicSfenOutputStream
    {
        virtual void write(const PSVector& sfens) = 0;
        virtual ~BasicSfenOutputStream() {}
    };

    struct BinSfenOutputStream : BasicSfenOutputStream
    {
        static constexpr auto openmode = std::ios::out | std::ios::binary | std::ios::app;
        static inline const std::string extension = "bin";

        BinSfenOutputStream(std::string filename) :
            m_stream(filename_with_extension(filename, extension), openmode)
        {
        }

        void write(const PSVector& sfens) override
        {
            m_stream.write(reinterpret_cast<const char*>(sfens.data()), sizeof(PackedSfenValue) * sfens.size());
        }

        ~BinSfenOutputStream() override {}

    private:
        std::fstream m_stream;
    };

    struct EpdSfenOutputStream : BasicSfenOutputStream
    {
        static constexpr auto openmode = std::ios::out | std::ios::app;
        static inline const std::string extension = "epd";

        EpdSfenOutputStream(std::string filename);

        void write(const PSVector& sfens) override;

        ~EpdSfenOutputStream() override {}

    private:
        std::fstream m_stream;
    };

    inline std::unique_ptr<BasicSfenInputStream> open_sfen_input_file(const std::string& filename)
    {
        if (has_extension(filename, BinSfenInputStream::extension))
            return std::make_unique<BinSfenInputStream>(filename);

        return nullptr;
    }

    inline std::unique_ptr<BasicSfenOutputStream> create_new_sfen_output(const std::string& filename, SfenOutputType sfen_output_type)
    {
        switch(sfen_output_type)
        {
            case SfenOutputType::Bin:
                return std::make_unique<BinSfenOutputStream>(filename);
            case SfenOutputType::Epd:
                return std::make_unique<EpdSfenOutputStream>(filename);
        }

        assert(false);
        return nullptr;
    }

    inline std::unique_ptr<BasicSfenOutputStream> create_new_sfen_output(const std::string& filename)
    {
        if (has_extension(filename, BinSfenOutputStream::extension))
            return std::make_unique<BinSfenOutputStream>(filename);

        return nullptr;
    }
}

#endif