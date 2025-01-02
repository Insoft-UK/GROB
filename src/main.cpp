// The MIT License (MIT)
// 
// Copyright (c) 2024 Insoft. All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <iomanip>
#include <cstdint>
#include "version_code.h"
#include "List.hpp"
#include "bmp.hpp"
#include "ColorTable.hpp"

#define NAME "BLOB"

static std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type pos = in.tellg();
    in.close();
    return pos;
}

static size_t loadBinaryFile(const char* filename, TBitmap& bitmap) {
    size_t fsize;
    std::ifstream infile;
    
    if ((fsize = filesize(filename)) == 0) return 0;
    
    infile.open(filename, std::ios::in | std::ios::binary);
    
    if (!infile.is_open()) return 0;

    if (!(bitmap.data = malloc(fsize))) return 0;
    infile.read((char *)bitmap.data, fsize);
    bitmap.bitWidth = 0;
    
    infile.close();
    return fsize;
}


// MARK: - Command Line


void help(void)
{
    std::cout << "Copyright (C) 2024-" << YEAR << " Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "\n";
    std::cout << "Usage: grob <input-file> [-o <output-file>] [-n <name>] [-g <1…9>] [-ppl] \n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -o <output-file>           Specify the filename for generated PPL code.\n";
    std::cout << "  -n <name>                  Custom name\n";
    std::cout << "  -g <1…9>                   Graphic object 1-9 to use if file is an image\n";
    std::cout << "  -ppl                       Wrap PPL code between #PPL...#END\n";
    std::cout << "\n";
    std::cout << "Additional Commands:\n";
    std::cout << "  grob {--version | --help}\n";
    std::cout << "    --version                Display the version information.\n";
    std::cout << "    --help                   Show this help message.\n";
}

void version(void) {
    std::cout << "Copyright (C) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n";
    std::cout << "Built on: " << DATE << "\n";
    std::cout << "Licence: MIT License\n\n";
    std::cout << "For more information, visit: http://www.insoft.uk\n";
}

void error(void)
{
    std::cout << "grob: try 'grob -help' for more information\n";
    exit(0);
}

void info(void) {
    std::cout << "Copyright (c) 2024 Insoft. All rights reserved.\n";
    std::cout << "Insoft "<< NAME << " version, " << VERSION_NUMBER << " (BUILD " << VERSION_CODE << ")\n\n";
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
        
        if ( strcmp( argv[n], "--help" ) == 0 ) {
            help();
            exit(0);
        }
        
        if ( strcmp( argv[n], "--version" ) == 0 ) {
            version();
            exit(0);
            return 0;
        }
        
        if ( strcmp( argv[n], "-ppl" ) == 0 ) {
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
    
    info();
    
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
    
    
    if (!filesize(ifilename.c_str())) {
        std::cout << "file '" << ifilename << "' not found.\n";
        exit(0x01);
    }
    
    if (name.empty()) {
        std::smatch match;
        regex_search(ofilename, match, std::regex(R"(^.*[\/\\](.*)\.(.*)$)"));
        name = match[1].str();
    }
    
    size_t lengthInBytes = 0;
    TBitmap bitmap{};
    bitmap = loadBitmapImage(ifilename);
    if (!bitmap.data) {
        lengthInBytes = loadBinaryFile(ifilename.c_str(), bitmap);
    } else {
        lengthInBytes = bitmap.width / (8 / bitmap.bitWidth) * bitmap.height;
        if (bitmap.bitWidth == 32) lengthInBytes = bitmap.width * bitmap.height * sizeof(uint32_t);
        if (bitmap.bitWidth == 16) lengthInBytes = bitmap.width * bitmap.height * sizeof(uint16_t);
    }

    
    
    std::string utf8;
    std::ostringstream os;
    if (pplus) utf8.append("#PPL\n");
    
    
    switch (bitmap.bitWidth) {
        case 0:
            utf8 += "LOCAL " + name + ":={" + List::ppl(bitmap.data, lengthInBytes, List::Format::Binary, 8) + "};\n";
            break;
            
        case 4:
            
            utf8 += "LOCAL " + name + ":={\n";
            utf8 += "4," + std::to_string(bitmap.width) + "," + std::to_string(bitmap.height);
            utf8 += "," + std::to_string(bitmap.palette.size()) + ",\n";
            for (int i = 0; i < bitmap.palette.size(); i += 1) {
                uint32_t color = bitmap.palette.at(i);
                color = color >> 8 | (255 - (color & 255)) << 24;
                os << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << color << ":32h,";
            }
            utf8 += os.str();
            utf8 += "\n" + List::ppl(bitmap.data, lengthInBytes, List::Format::Binary, 8) + "};\n";
            break;
            
            
        case 16:
            utf8 += "DIMGROB_P(G" + std::to_string(grob) + "," + std::to_string(bitmap.width) + "," + std::to_string(bitmap.height) + ",{\n" + List::ppl(bitmap.data, lengthInBytes, List::Format::HighColor, 8) + "});\n";
            break;
            
        default:
            utf8 += "DIMGROB_P(G" + std::to_string(grob) + "," + std::to_string(bitmap.width) + "," + std::to_string(bitmap.height) + ",{\n" + List::ppl(bitmap.data, lengthInBytes, List::Format::TrueColor, 8) + "});\n";
            break;
    }
    if (pplus) utf8.append("#END\n");
    
    
    releaseBitmap(bitmap);
    
   
    saveAs(ofilename, utf8);
    
    
    return 0;
}
