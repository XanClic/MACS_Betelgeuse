#include <macs/macs.hpp>

#include "betelgeuse.hpp"


namespace betelgeuse
{
    bool _dbl_buf;
}


bool betelgeuse::init(int width, int height, bool double_buffering)
{
    _dbl_buf = double_buffering;

    return macs::init(width, height);
}
