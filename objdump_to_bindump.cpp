/**
 * @file objdump_to_bindump.cpp
 * 
 * Reads a file as produced by objdump and converts the hex
 * representations of machine-language instructions into 
 * pure binary representations.
 *
 * Usage: 
 *      objdump_to_hexdump [-b] objdump_output_file
 *
 *      Options: 
 *          -b      Produce only the binary output (remove other 
 *                  information from the objdump output)
 *              
 * Use with -b flag to produce binary-only output -- otherwise the
 * output will be the same as input but with binary inline.
 *
 * @copyright (c) 2014 Jason L Causey, Arkansas State University
 *                Released under the MIT license:
 *                http://opensource.org/licenses/MIT
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <algorithm>

void usage(std::string msg="");
int  process_objdump_file(std::string filename, bool full_output);
void process_line(std::string line);

int main(int argc, const char* argv[])
{
    // Sanity checks:
    if(argc < 2){
        usage();
    }
    int  i = 1;
    bool full_output = true;
    std::string token = std::string(argv[i]);
    std::string filename;
    if (token == "-b"){
        full_output = false;
        i++;
    }
    else if (argc > 2){ 
        usage(std::string("Unknown option: ") + token);
    }
    if(i < argc){
        filename = std::string(argv[i]);
    }
    else{
        usage("Missing objdump_output_file.");
    }

    return process_objdump_file(filename, full_output);
}

/**
 * Print the program's usage message and exit.
 * @param msg   An optional message to print.  If the message is 
 *              non-empty, the return code will be 1.
 */
void usage(std::string msg){
    std::cout << msg << msg != "" ? "\n\n" : "";
    std::cout << 
        "Usage: \n"
        "     objdump_to_hexdump [-b] objdump_output_file\n"
        "      \n"
        "     Options: \n"
        "         -b      Produce only the binary output (remove other \n"
        "                 information from the objdump output)\n"
        "         \n"
        "     Note:\n"
        "         Use with the `-b` flag to strip everything except the binary\n"
        "         output.  All other data from the original objdump_output_file\n"
        "         is ignored.\n\n\n";
        exit(msg != "" ? 1 : 0);
}

/**
 * Processes the file `filename`, which should be the output of an 
 * "objdump -d" (or similar) command.  The file's contents are echoed
 * to stdout, but hex values in the commands are changed to binary.
 * @param  filename    name of input file
 * @param  full_output `true` to produce full output, `false` to remove
 *                     everything except the binary command codes
 * @return             0 if everything is processed OK, 1 otherwise.
 */
int process_objdump_file(std::string filename, bool full_output){
    int ret_code = 0;
    std::ifstream fin(filename.c_str());
    if(!fin.is_open()){
        std::cerr << "Failed to open " << filename << " for input.\n";
        ret_code = 1;
    }
    else{
        // Read the file line by line:
        std::string line;
        while(getline(fin, line)){
            // Lines are a header (usually an address) followed by a colon,
            // followed by a tab, followed by the hex instruction:
            //    Hex instruction is up to 7 two-digit tokens separated 
            //    by a single space, in a column of width 21.
            // followed by a tab, followed by the assembly instruction.
            size_t pos = line.find("\t");
            if(pos == std::string::npos || line[pos-1] != ':'){
                if(full_output) std::cout << line << "\n";
            }
            else{
                // If the line has a tab, start converting the hex:
                pos++;
                if(full_output) std::cout << line.substr(0, pos);       // original header
                size_t end_pos = line.find("\t", pos);                  // find the instruction part
                if(end_pos == std::string::npos){                       // if no tab is found, then 
                    end_pos = line.size() - 1;                          // all that is left is hex 
                }                                                       // (happens on continuation lines)
                std::string hex_part = line.substr(pos, end_pos-pos+1); // extract the hex instruction
                std::string bin_part;                                   // make room for binary string 
                size_t cur_pos = 0;
                // For each part of the "hex part", produce a binary replacement:
                for(int token = 0; token < 7; token++){
                    for(int tlen = 0; tlen < 2; tlen++, cur_pos++){
                        if( cur_pos < hex_part.size() && !isspace(hex_part[cur_pos]) ){
                            std::stringstream ss, os;                   // streams will help with conversion:
                            ss << std::hex << hex_part[cur_pos];        // get hex string into a stream
                            int value;                                  // so that it can easily be 
                            ss >> value;                                // extracted into an integer
                            for(int i = 0; i < 4; i++){                 // then compute the four 
                                os << value % 2;                        // corresponding bits
                                value /= 2;                             // by mod-and-divide 
                            }
                            std::string token = os.str();               // get the bits into a string, which is 
                            bin_part += std::string(token.rbegin(), token.rend()); // reversed, so reverse and concatenate.
                        }
                        else{
                            bin_part += "    ";                         // every "space" needs to pad to four.
                        }
                    }
                    bin_part += " ";                                    // separate with a space
                    cur_pos++;
                }
                std::cout << bin_part;                                  // output the binary and the rest if needed
                if(full_output) std::cout << line.substr(end_pos, std::string::npos);
                std::cout << "\n";
            }
        }
        fin.close();
    }
    return ret_code;
}
