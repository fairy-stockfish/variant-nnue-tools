#include "validate_training_data.h"

#include "uci.h"
#include "misc.h"
#include "thread.h"
#include "position.h"
#include "tt.h"

#include "nnue/evaluate_nnue.h"

#include "syzygy/tbprobe.h"

#include <sstream>
#include <fstream>
#include <unordered_set>
#include <iomanip>
#include <list>
#include <cmath>    // std::exp(),std::pow(),std::log()
#include <cstring>  // memcpy()
#include <memory>
#include <limits>
#include <optional>
#include <chrono>
#include <random>
#include <regex>
#include <filesystem>

using namespace std;
namespace sys = std::filesystem;

namespace Stockfish::Tools
{
    void validate_training_data(istringstream&)
    {
        std::cerr << "ERROR: The 'validate_training_data' command has been removed.\n";
        std::cerr << "Training data validation is no longer supported after removing binpack format dependencies.\n";
    }
}
