#include "sfen_stream.h"
#include "position.h"
#include "thread.h"

namespace Stockfish::Tools {

    EpdSfenOutputStream::EpdSfenOutputStream(std::string filename)
        : m_stream(filename_with_extension(filename, extension), openmode)
    {
    }

    void EpdSfenOutputStream::write(const PSVector& sfens)
    {
        // We need a Position and Thread to unpack the sfens
        // Get the main thread for unpacking positions
        auto th = Threads.main();
        Position pos;
        StateInfo si;

        for (const auto& psv : sfens)
        {
            // Unpack the sfen to get the position
            set_from_packed_sfen(pos, psv.sfen, &si, th);

            // Write the FEN string followed by a newline
            m_stream << pos.fen() << '\n';
        }
    }

}
