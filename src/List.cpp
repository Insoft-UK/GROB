/*
 The MIT License (MIT)
 
 Copyright (c) 2024 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "List.hpp"

#include <cstring>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <regex>

static int _col = 8;

template <typename T>
T swap_endian(T u)
{
    static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

static std::string listHighColor(const void *data, const size_t lengthInBytes)
{
    std::ostringstream os;
    uint64_t n;
    size_t count = 0;
    size_t length = lengthInBytes;
    uint64_t *bytes = (uint64_t *)data;
    
    os << "{\n";
    while (length >= 8) {
        n = *bytes++;
        
#ifndef __LITTLE_ENDIAN__
        /*
         This platform utilizes big-endian, not little-endian. To ensure
         that data is processed correctly when generating the list, we
         must convert between big-endian and little-endian.
         */
        n = swap_endian<uint64_t>(n);
#endif
        os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << n << ":64h";
    
        if (length - 8 >= 8) os << ",";
        if (++count % _col == 0) os << '\n';
        length-=8;
    }
    if (count % _col != 0) os << '\n';
    os << "}";
    return os.str();
}

static std::string listTrueColor(const void *data, const size_t lengthInBytes)
{
    std::ostringstream os;
    uint64_t n;
    size_t count = 0;
    size_t length = lengthInBytes / 8;
    uint64_t *bytes = (uint64_t *)data;
    
    os << "{\n";
    while (length >= 8) {
        n = *bytes++;
        
#ifndef __LITTLE_ENDIAN__
        /*
         This platform utilizes big-endian, not little-endian. To ensure
         that data is processed correctly when generating the list, we
         must convert between big-endian and little-endian.
         */
        n = swap_endian<uint64_t>(n);
#endif
        os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << n << ":32h";
    
        if (length - 8 > 8) os << ",";
        if (++count % _col == 0) os << '\n';
        length-=8;
    }
    if (count % _col != 0) os << '\n';
    os << "}";
    return os.str();
}

std::string list64(const void *data, const size_t lengthInBytes)
{
    std::ostringstream os;
    uint64_t n;
    size_t count = 0;
    size_t length = lengthInBytes;
    uint64_t *bytes = (uint64_t *)data;
    
    os << "{\n";
    
    while (length >= 8) {
        n = *bytes++;
        
#ifndef __LITTLE_ENDIAN__
        /*
         This platform utilizes big-endian, not little-endian. To ensure
         that data is processed correctly when generating the list, we
         must convert between big-endian and little-endian.
         */
        n = swap_endian<uint64_t>(n);
#endif
        os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << n << ":64h";
        
        if (length - 8 > 8) os << ",";
        if (++count % _col == 0) os << '\n';
        length-=8;
    }
    if (count % _col != 0) os << '\n';
    os << "}";
    return os.str();
}

// A list is limited to 10,000 elements. Attempting to create a longer list will result in error 38 (Insufficient memory) being thrown.
std::string List::ppl(const void *data, const size_t lengthInBytes, Format fmt, int col) {
    _col = col;
    switch (fmt) {
        case Format::HighColor:
            return listHighColor(data, lengthInBytes);
            
        case Format::TrueColor:
            return listTrueColor(data, lengthInBytes);
            
        default:
            return list64(data, lengthInBytes);
    }
    
    return std::string("");
}



