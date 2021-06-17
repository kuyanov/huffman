#pragma once

#include <istream>
#include <ostream>

namespace Arch
{

[[nodiscard]] int compress(std::istream *in, std::ostream *out);

[[nodiscard]] int decompress(std::istream *in, std::ostream *out);

}
