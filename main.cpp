//
//  main.cpp
//  cpuOrg
//
//  Created by Steven Bumbera on 3/9/19.
//  Copyright © 2019 Steven Bumbera. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <regex>
#include <ctype.h>
#include <sstream>
#include <iomanip>
#include <bitset>


using namespace std;
using namespace boost::assign;
using namespace boost::algorithm;
using namespace regex_constants;

void encodeI(int op, int rs1, int rs2, int imm);
void encodeR(int op, int rs1, int rs2, int rd, int func);
void encodeJ(int op, int imm);
map<string, int>::iterator searchByValue(map<string, int> & maps, string val);
vector<string> getNumbers(string line);
void printFormatted(stringstream& ss);

int main () {
    
    map<string, int> mJump = map_list_of
    ("J", 2) ("JAL", 3)("JR", 18) ("JALR", 19);
    
    map<string, int> mBranch = map_list_of
    ("BEQZ", 4) ("BNEZ", 5) ("SEQUI", 48) ("SNEUI", 49) ("SLTUI", 50) ("SGTUI", 51) ("SLEUI", 52)
    ("SGEUI", 53) ("SEQI", 24) ("SNEI", 25) ("SLTI", 26) ("SGTI", 27) ("SLEI", 28) ("SGEI", 29);
    
    map<string, int> mALU = map_list_of
    ("ADDI", 8) ("ADDUI", 9) ("SUBI", 10) ("SUBUI", 11) ("ANDI", 12) ("ORI", 13) ("XORI", 14)
    ("SLLI", 54) ("SRLI", 55) ("SRAI", 56);
    
    map<string, int> mLoad = map_list_of
    ("LB", 32) ("LH", 33) ("LW", 35) ("LBU", 36) ("LHU", 37) ("LHI", 15);
    
    map<string, int> mStore = map_list_of
    ("SB", 40) ("SH", 41) ("SW", 43);
    
    map<string, int> mR = map_list_of
    ("SLL", 4) ("SRL", 6)
    ("SRA", 7) ("SLTU", 18) ("SGTU", 19) ("SLEU", 20) ("SGEU", 21) ("ADD", 32)
    ("ADDU", 33) ("SUB", 34) ("SUBU", 35) ("AND", 36) ("OR", 37) ("XOR", 38)
    ("SEQ", 40) ("SNE", 41) ("SLT", 42) ("SGT", 43) ("SLE", 44) ("SGE", 45);
    
    map<string, int> label;
    map<string, int> jLabel;
    vector<string> result;
    
    ifstream myfile ("example2.txt");
    string rs1tmp, rs2tmp, immtmp, rdtmp, firstWord, type, line, temp;
    int rs1, rs2, rd, op, func, labelLine, jLabelLine =0;
    signed int imm;
    int count = 0;
   // bool secondPass = false;
//

    
    if (myfile.is_open())
    {
        while ( getline (myfile,line, '\n') )
        {
            //FIRST PASS: GETTING LABELS
            to_upper(line);
            firstWord = line.substr(0, line.find(' '));
            count++;
            
            //Adding to label map
            
            if(line.find(":") != string::npos){
                temp = line.substr(0, line.find(':'));
                label.insert(make_pair(temp, count));
            }
            
            //Adding to jLabel map
            if(mJump.find(firstWord) != mJump.end())
                jLabel.insert(make_pair(firstWord, count));
            if(mBranch.find(firstWord) != mBranch.end())
                jLabel.insert(make_pair(firstWord, count));
        }
        myfile.clear();
        myfile.seekg(0, myfile.beg);
        while ( getline (myfile,line, '\n') )
        {
            to_upper(line);
            firstWord = line.substr(0, line.find(' '));
            if(line.find(":") != string::npos)
                cout << "LABEL LINE -> GARBAGE ->";
            
            
            //Determining what Type it belongs to
            if(mR.find(firstWord) != mR.end())
                type = "R";
            if(mJump.find(firstWord) != mJump.end())
                type = "jump";
            if(mLoad.find(firstWord) != mLoad.end())
                type = "load";
            if(mStore.find(firstWord) != mStore.end())
                type = "store";
            if(mALU.find(firstWord) != mALU.end())
                type = "alu";
            if(mBranch.find(firstWord) != mBranch.end())
                type = "branch";
            
            //Do things based on type
            if(type == "R")
            {
                map<string, int>::iterator it = searchByValue(mR, firstWord);
                func = it->second;
                
                result.clear();
                result = getNumbers(line);
                op = 0;
                stringstream(result[1]) >> rs1;
                stringstream(result[2]) >> rs2;
                stringstream(result[0]) >> rd;
                cout << firstWord << ": ";
                encodeR(op, rs1, rs2, rd, func);
            }
            if(type == "branch")
            {
                map<string, int>::iterator it = searchByValue(mBranch, firstWord);
                op = it->second;
                temp = line.substr(line.find_last_of(" ") + 1);
                map<string, int>::iterator it2 = searchByValue(label, temp);
                labelLine = it2->second;
                map<string, int>::iterator it3 = searchByValue(jLabel, firstWord);
                jLabelLine = it3->second;
                imm = (labelLine * 4) - ((jLabelLine*4) + 4);
                
                result.clear();
                result = getNumbers(line);
                stringstream(result[0]) >> rs1;
                
                cout << firstWord << ": ";
                encodeI(op, rs1, 0, imm);
            }
            
            if(type == "jump")
            {
                map<string, int>::iterator it = searchByValue(mJump, firstWord);
                op = it->second;
                if(firstWord == "JR" || firstWord == "JALR"){
                    result.clear();
                    result = getNumbers(line);
                    stringstream(result[0]) >> rs1;
                    rs2 = 0;
                    imm = 0;
                    cout << firstWord << ": ";
                    encodeI(op, rs1, rs2, imm);
                }
                
                if(firstWord == "JAL" || firstWord == "J"){
                    temp = line.substr(line.find(" ") + 1);
                    map<string, int>::iterator it2 = searchByValue(label, temp);
                    labelLine = it2->second;
                    map<string, int>::iterator it3 = searchByValue(jLabel, firstWord);
                    jLabelLine = it3->second;
                    imm = (labelLine * 4) - ((jLabelLine*4) + 4);
                    
                    cout << firstWord << ": ";
                    encodeJ(op, imm);
                }
                
            }
            
            if(type == "load")
            {
                map<string, int>::iterator it = searchByValue(mLoad, firstWord);
                op = it->second;
                
                result.clear();
                result = getNumbers(line);
                stringstream(result[0]) >> rs2;
                stringstream(result[1]) >> imm;
                stringstream(result[2]) >> rs1;
                if(result.size() < 3)
                    rs1 = 0;
                
                cout << firstWord << ": ";
                encodeI(op, rs1, rs2, imm);
            }
            
            if(type == "store")
            {
                map<string, int>::iterator it = searchByValue(mStore, firstWord);
                op = it->second;
                
                result.clear();
                result = getNumbers(line);
                stringstream(result[0]) >> imm;
                stringstream(result[1]) >> rs1;
                stringstream(result[2]) >> rs2;
                
                cout << firstWord << ": ";
                encodeI(op, rs1, rs2, imm);
            }
            
            if(type == "alu")
            {
                map<string, int>::iterator it = searchByValue(mALU, firstWord);
                op = it->second;
                
                result.clear();
                result = getNumbers(line);
                stringstream(result[0]) >> rs2;
                stringstream(result[1]) >> rs1;
                stringstream(result[2]) >> imm;
                
                cout << firstWord << ": ";
                encodeI(op, rs1, rs2, imm);
            }
            
        }
    }
    else cout << "Unable to open file";\
    return 0;
}

void encodeI(int op, int rs1, int rs2, int imm){
    int i = 0b00000000000000000000000000000000;
    op = op << 26;
    i = i | op;
    rs1 = rs1 << 21;
    i = i | rs1;
    rs2 = rs2 << 16;
    i = i | rs2;
    
    if(imm < 0){
        int y = 0;
        to_string(imm);
        bitset<32> foo(imm);
        
        for(int i = 16; i < 32; i++)
            foo.flip(i);
        string x = foo.to_string();
        y = stoi(x, nullptr, 2);
        i = i | y;
    }else
        i = i | imm;
    stringstream stream;
    stream << hex << i;
    printFormatted(stream);
}

void encodeR(int op, int rs1, int rs2, int rd, int func){
    int i = 0b00000000000000000000000000000000;
    
    op = op << 26;
    i = i | op;
    rs1 = rs1 << 21;
    i = i | rs1;
    
    rs2 = rs2 << 16;
    i = i | rs2;
    
    rd = rd << 11;
    i = i | rd;
    
    i = i | func;
    stringstream stream;
    stream << hex << i;
    printFormatted(stream);
    
}

void encodeJ(int op, int imm){
    int i = 0b00000000000000000000000000000000;
    op = op << 26;
    i = i | op;
    if(imm < 0){
        int y = 0;
        to_string(imm);
        bitset<32> foo(imm);
        
        for(int i = 26; i < 32; i++)
            foo.flip(i);
        string x = foo.to_string();
        y = stoi(x, nullptr, 2);
        i = i | y;
    }else
        i = i | imm;
    stringstream stream;
    stream << hex << i;
    printFormatted(stream);
    
}

map<string, int>::iterator searchByValue(map<string, int> & maps, string val)
{
    
    map<string, int>::iterator it = maps.begin();
    while(it != maps.end())
    {
        if(it->first == val)
            return it;
        it++;
    }
    return maps.end();
}

vector<string> getNumbers(string s){
    vector<string> result;
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    s=s.substr(s.find_first_of(" \t")+1);
    boost::char_separator<char> sep{" ,rR()"};
    tokenizer tok{s, sep};
    for (auto it = tok.begin(); it != tok.end(); ++it)
        result.push_back(*it);
    
    return result;
}

void printFormatted(stringstream& ss){
    string s(ss.str());
    s.insert(s.begin(), 8 - s.length(), '0');
    to_upper(s);
    cout << "0x" << s << endl;
}




