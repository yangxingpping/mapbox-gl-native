#pragma once

#include <mbgl/util/range.hpp>

namespace mbgl {
namespace gl {

class Depth {
public:
    enum Function {
        Never        = 0x0200,
        Less         = 0x0201,
        Equal        = 0x0202,
        LessEqual    = 0x0203,
        Greater      = 0x0204,
        NotEqual     = 0x0205,
        GreaterEqual = 0x0206,
        Always       = 0x0207
    };

    Function func;
    bool mask;
    Range<float> range;

    static Depth disabled() {
       return Depth { Always, false, { 0.0, 1.0 } };
    }
};

} // namespace gl
} // namespace mbgl
