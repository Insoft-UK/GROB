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

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <iomanip>
#include <cstdint>
#include "build.h"
#include "List.hpp"

static const char* BMP_MAGIC = "BM";

//#define __attribute__(x)

//#pragma pack(push, 1)     /* set alignment to 1 byte boundary */
/* Windows 3.x bitmap file header */
typedef struct __attribute__((__packed__)){
    char      fileType[2];   /* magic - always 'B' 'M' */
    uint32_t    fileSize;
    int32_t    tableOffsett;  /* offset in bytes to actual bitmap table */
    uint32_t    dataOffset;    /* offset in bytes to actual bitmap data */
} BMPHeader;

/* Windows 3.x bitmap full header, including file header */

typedef struct __attribute__((__packed__)){
    BMPHeader fileHeader;
    uint32_t    biSize;
    int32_t    biWidth;
    int32_t    biHeight;
    int16_t    biPlanes;           // Number of colour planes, set to 1
    int16_t    biBitCount;         // Colour bits per pixel. 1 4 8 16 24 or 32
    uint32_t    biCompression;      // *Code for the compression scheme
    uint32_t    biSizeImage;        // *Size of the bitmap bits in bytes
    int32_t    biXPelsPerMeter;    // *Horizontal resolution in pixels per meter
    int32_t    biYPelsPerMeter;    // *Vertical resolution in pixels per meter
    uint32_t    biClrUsed;          // *Number of colours defined in the palette
    uint32_t    biClImportant;      // *Number of important colours in the image
} BIPHeader;
//#pragma pack(pop)

struct Data {
    List::Format fmt;
    unsigned width, height;
    
    void *bytes;
    unsigned long length;
};


void loadBMP(const char* filename, Data& data) {
    BIPHeader bip_header;
    std::ifstream infile;
    infile.open(filename, std::ios::in | std::ios::binary);
    
    infile.read((char *)&bip_header, sizeof(BIPHeader));

    // Check the file type is a BMP
    if (strncmp(bip_header.fileHeader.fileType, BMP_MAGIC, 2) != 0) {
        infile.close();
        return;
    }

    data.width = bip_header.biWidth;
    data.height = abs(bip_header.biHeight);

    if (16 == bip_header.biBitCount) data.fmt = List::Format::HighColor;
    if (32 == bip_header.biBitCount) {
        data.fmt = List::Format::TrueColor;
    }
    if (24 == bip_header.biBitCount) {
        data.fmt = List::Format::TrueColor;
    }


    infile.seekg(bip_header.fileHeader.dataOffset, std::ios::beg);
    data.bytes = malloc(bip_header.biSizeImage);
    if (!data.bytes) return;
    data.length = bip_header.biSizeImage;
    infile.read((char *)data.bytes, data.length);

    if (bip_header.biHeight > 0) {
        size_t bytesPerLine = bip_header.biSizeImage / abs(bip_header.biHeight);
        void *buf = malloc(bytesPerLine);
        if (!buf) return;
        for (int n=abs(bip_header.biHeight); n; --n) {

        }
    }
    
    infile.close();
}

static std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type pos = in.tellg();
    in.close();
    return pos;
}

static void loadBinaryFile(const char* filename, Data &data) {
    size_t fsize;
    std::ifstream infile;
    
    if ((fsize = filesize(filename)) == 0) return;
    
    infile.open(filename, std::ios::in | std::ios::binary);
    
    if (!infile.is_open()) return;

    if (!(data.bytes = malloc(fsize))) return;
    data.length = fsize;
    infile.read((char *)data.bytes, fsize);
    data.fmt = List::Format::Binary;
    
    infile.close();
}


// MARK: - Command Line

/*
 The decimalToBase24 function converts a given
 base 10 integer into its base 24 representation using a
 specific set of characters. The character set is
 comprised of the following 24 symbols:

     •    Numbers: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
     •    Letters: C, D, F, H, J, K, M, N, R, U, V, W, X, Y
     
 Character Selection:
 The choice of characters was made to avoid confusion
 with common alphanumeric representations, ensuring
 that each character is visually distinct and easily
 recognizable. This set excludes characters that closely
 resemble each other or numerical digits, promoting
 clarity in representation.
 */
static std::string decimalToBase24(int num) {
    if (num == 0) {
        return "C";
    }

    const std::string base24Chars = "0123456789CDFHJKMNRUVWXY";
    std::string base24;

    while (num > 0) {
        int remainder = num % 24;
        base24 = base24Chars[remainder] + base24; // Prepend character
        num /= 24; // Integer division
    }

    return base24;
}

static std::string getBuildCode(void) {
    std::string str;
    int majorVersionNumber = BUILD_NUMBER / 100000;
    str = std::to_string(majorVersionNumber) + decimalToBase24(BUILD_NUMBER - majorVersionNumber * 100000);
    return str;
}

void help(void)
{
    int rev = BUILD_NUMBER / 1000 % 10;
    
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft GROB version, " << BUILD_NUMBER / 100000 << "." << BUILD_NUMBER / 10000 % 10 << (rev ? "." + std::to_string(rev) : "")
    << " (BUILD " << getBuildCode() << "-" << decimalToBase24(BUILD_DATE) << ")\n";
    std::cout << "\n";
    std::cout << "Usage: grob <input-file> [-o <output-file>] [-n] [-g] \n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output-file>           Specify the filename for generated PPL code.\n";
    std::cout << "  -n                         Custom name\n";
    std::cout << "  -g                         Graphic object 1-9 to use if file is an image\n";
    std::cout << "  -p+                        Wrap PPL code between #PPL...#END for P+\n";
    std::cout << "\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  grob {-version | -help}\n";
    std::cout << "    -version                 Display the version information.\n";
    std::cout << "    -help                    Show this help message.\n";
}

void version(void) {
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft GROB version, " << BUILD_NUMBER / 100000 << "." << BUILD_NUMBER / 10000 % 10 << "." << BUILD_NUMBER / 1000 % 10
    << " (BUILD " << getBuildCode() << ")\n";
    std::cout << "Built on: " << CURRENT_DATE << "\n";
    std::cout << "Licence: MIT License\n\n";
    std::cout << "For more information, visit: http://www.insoft.uk\n";
}

void error(void)
{
    std::cout << "grob: try 'pplref -help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout << "Copyright (c) 2024 Insoft. All rights reserved.\n";
    int rev = BUILD_NUMBER / 1000 % 10;
    std::cout << "Insoft PPL Reformat version, " << BUILD_NUMBER / 100000 << "." << BUILD_NUMBER / 10000 % 10 << (rev ? "." + std::to_string(rev) : "")
    << " (BUILD " << getBuildCode() << "-" << decimalToBase24(BUILD_DATE) << ")\n\n";
}


void saveAs(std::string& filename, const std::string& str) {
    std::ofstream outfile;
    outfile.open(filename, std::ios::out | std::ios::binary);

    if(!outfile.is_open()) {
        error();
        exit(0x02);
    }
    
    bool utf16le = false;
    
    if (std::string::npos != filename.rfind(".hpprgm")) utf16le = true;
    
    if (utf16le) {
        outfile.put(0xFF);
        outfile.put(0xFE);
    }
    
    uint8_t* ptr = (uint8_t*)str.c_str();
    for ( int n = 0; n < str.length(); n++) {
        if (0xc2 == ptr[0]) {
            ptr++;
            continue;
        }
        
        if (utf16le) {
            if (0xE0 <= ptr[0]) {
                // 3 Bytes
                uint16_t utf16 = ptr[0];
                utf16 <<= 6;
                utf16 |= ptr[1] & 0b111111;
                utf16 <<= 6;
                utf16 |= ptr[1] & 0b111111;
                
#ifndef __LITTLE_ENDIAN__
                utf16 = utf16 >> 8 | utf16 << 8;
#endif
                outfile.write((const char *)&utf16, 2);
                
                ptr+=3;
                continue;
            }
        }
        
        if ('\r' == ptr[0]) {
            ptr++;
            continue;
        }

        // Output as UTF-16LE
        outfile.put(*ptr++);
        if (utf16le) outfile.put('\0');
    }
    
    outfile.close();
}


int main(int argc, const char * argv[]) {
    std::string ifilename, ofilename, prefix, sufix, name;
    int grob = 1;
    bool pplus = false;

    if ( argc == 1 )
    {
        error();
        exit( 0 );
    }
   
    for( int n = 1; n < argc; n++ ) {
        if ( strcmp( argv[n], "-o" ) == 0 || strcmp( argv[n], "--out" ) == 0 ) {
            if ( n + 1 >= argc ) {
                error();
                exit(100);
            }
            ofilename = argv[n + 1];
            if (std::string::npos == ofilename.rfind('.')) {
                ofilename += ".hpprgm";
            }

            n++;
            continue;
        }
        
        if ( strcmp( argv[n], "-help" ) == 0 ) {
            help();
            exit(0);
        }
        
        if ( strcmp( argv[n], "-version" ) == 0 ) {
            version();
            exit(0);
            return 0;
        }
        
        if ( strcmp( argv[n], "-p+" ) == 0 ) {
            pplus = true;
            continue;
        }
        
        if ( strcmp( argv[n], "-g" ) == 0 ) {
            if ( n + 1 >= argc ) {
                info();
                exit(0);
            }
            
            n++;
            grob = atoi(argv[n]);
        
            continue;
        }
        
        if ( strcmp( argv[n], "-n" ) == 0 )
        {
            if ( n + 1 >= argc ) {
                error();
                exit(-1);
            }
            
            n++;
            name = argv[n];
        
            continue;
        }
        
        
        if (ifilename.empty()) ifilename = argv[n];
    }
    
    if (ofilename.empty()) {
        std::smatch m;
        if (regex_search(ifilename, m, std::regex(R"(\w*(\.\w+)?$)"))) {
            ofilename = m.str();
        } else {
            ofilename = ifilename;
        }
        ofilename = regex_replace(ofilename, std::regex(R"((\.\w+)?$)"), "");
        ofilename += ".hpprgm";
    }
    
    Data data = {
        .fmt = List::Format::Binary,
        .width = 0,
        .height = 0,
        .bytes = 0,
        .length = 0
    };
    
    
    if (!filesize(ifilename.c_str())) {
        std::cout << "file '" << ifilename << "' not found.\n";
        exit(0x01);
    }

    loadBMP(ifilename.c_str(), data);
    if (!data.length) {
        loadBinaryFile(ifilename.c_str(), data);
    }
    
    
    std::string utf8;
    

    
    if (data.fmt == List::Format::TrueColor || data.fmt == List::Format::HighColor) {
        std::ostringstream os;
        if (pplus) os << "#PPL\n";
        os << "DIMGROB_P(G1," << data.width << "," << data.height << "," << List::ppl(data.bytes, data.length, data.fmt, data.width / (List::Format::HighColor == data.fmt ? 4 : 2)) << ");\n";
        if (pplus) os << "#END\n";
        utf8 = os.str();
    } else {
        if (name.empty()) {
            std::string s = ifilename;
            if (std::string::npos != s.rfind('/')) s = s.substr(s.rfind('/') + 1, s.length() - s.rfind('/') - 1);
            name = regex_replace(s, std::regex(R"(\.\w+)"), "");
        }
        if (pplus)
            utf8 = "#PPL\nLOCAL " + name + ":=" + List::ppl(data.bytes, data.length, data.fmt, 8) + ";\n#END\n";
        else
            utf8 = "LOCAL " + name + ":=" + List::ppl(data.bytes, data.length, data.fmt, 8) + ";\n";
        
    }
    
    free(data.bytes);
    
   
    saveAs(ofilename, utf8);
    return 0;
}
