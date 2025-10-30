#ifndef _PUZZLE_GENERATOR_H_
#define _PUZZLE_GENERATOR_H_

#include "position.h"

#include <sstream>

namespace Stockfish::Tools {

    // Automatic generation of puzzles
    void generate_puzzles(std::istringstream& is);
}

#endif
