/*
 File:      logcat-colorize.cpp

 Purpose:   Colorize Android's logcat output in command-line windows
            Works on linux/mac terminals only.      

 Author:    BRAGA, Bruno <bruno.braga@gmail.com>

 Copyright:
            Licensed under the Apache License, Version 2.0 (the "License");
            you may not use this file except in compliance with the License.
            You may obtain a copy of the License at

            http://www.apache.org/licenses/LICENSE-2.0

            Unless required by applicable law or agreed to in writing, software
            distributed under the License is distributed on an "AS IS" BASIS,
            WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
            implied. See the License for the specific language governing
            permissions and limitations under the License.

 Notes:     
            Bugs, issues and requests are welcome at:
            https://bitbucket.org/brunobraga/logcat-colorize/issues


 Dependencies:
 
            libboost-regex-dev 

 Compiling:
            g++ logcat-colorize.cpp -o logcat-colorize -lboost_regex -std=c++0x
*/

#include <unistd.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <boost/regex.hpp>
using namespace std;

const string NAME = "logcat-colorize";
const string VERSION = "0.3";

const string HELP = 
    NAME + " v" + VERSION + "\n"
    "\n"
    "A simple script to colorize Android debugger's logcat output.\n"
    "To use this, you should pipe from adb output. See examples below.\n"
    "\n"
    "Examples:\n"
    "    Simplest usage:\n"
    "    adb logcat | " + NAME + "\n"
    "\n"
    "    Using specific device, with time details, and filtering:\n"
    "    adb -s emulator-5556 logcat -v time System.err:V *:S | " + NAME + "\n" 
    "\n"
    "    Piping to grep for regex filtering (much better than adb filter):\n"
    "    adb logcat -v time | egrep -i '(sensor|wifi)' | " + NAME + "\n"
    "\n"
    "Author: BRAGA, Bruno <bruno.braga@gmail.com>\n"
    "\n"
    "    Comments or bugs are welcome at:\n"
    "    https://bitbucket.org/brunobraga/logcat-colorize/issues\n";

class Color {

public:
    static const string fblack;
    static const string fred;
    static const string fgreen;
    static const string fyellow;
    static const string fblue;
    static const string fpurple;
    static const string fcyan;
    static const string fwhite;
    static const string fgrey;
    static const string bblack;
    static const string bred;
    static const string bgreen;
    static const string byellow;
    static const string bblue;
    static const string bpurple;
    static const string bcyan;
    static const string bwhite;
    static const string bold;
    static const string underline;
    static const string reset;
};

const string Color::fblack     = "\033[0;30m";
const string Color::fred       = "\033[0;31m";
const string Color::fgreen     = "\033[0;32m";
const string Color::fyellow    = "\033[0;33m";
const string Color::fblue      = "\033[0;34m";
const string Color::fpurple    = "\033[0;35m";
const string Color::fcyan      = "\033[0;36m";
const string Color::fwhite     = "\033[0;37m";
const string Color::fgrey      = "\033[1;30m";
const string Color::bblack     = "\033[40m";
const string Color::bred       = "\033[41m";
const string Color::bgreen     = "\033[42m";
const string Color::byellow    = "\033[43m";
const string Color::bblue      = "\033[44m";
const string Color::bpurple    = "\033[45m";
const string Color::bcyan      = "\033[46m";
const string Color::bwhite     = "\033[47m";
const string Color::bold       = "\033[1m";
const string Color::underline  = "\033[4m";
const string Color::reset      = "\033[0m";

struct Logcat {
    string date;
    string level;
    string tag;
    string process;
    string message; 
    string thread;
};


class Format {

protected:
    Logcat l;

public:
    static const int BRIEF;
    static const int PROCESS;
    static const int TAG;
    static const int RAW;
    static const int TIME;
    static const int THREADTIME;
    static const int LONG;
    
    virtual void parse(string raw) {}
    virtual bool ok() { return false; }
    void print() {
        
        string out = "";
        
        // date    
        if (this->l.date != "") 
            out += Color::fpurple + " " + this->l.date + " " + Color::reset;
        
        // level
        if (this->l.level != "") {
            if (this->l.level == "V") out += Color::fblack + Color::bcyan   + Color::bold + " " + this->l.level + " " + Color::reset;
            if (this->l.level == "D") out += Color::fwhite + Color::bblue   + Color::bold + " " + this->l.level + " " + Color::reset;
            if (this->l.level == "I") out += Color::fwhite + Color::bgreen  + Color::bold + " " + this->l.level + " " + Color::reset;
            if (this->l.level == "W") out += Color::fblack + Color::byellow + Color::bold + " " + this->l.level + " " + Color::reset;
            if (this->l.level == "E") out += Color::fblack + Color::bred    + Color::bold + " " + this->l.level + " " + Color::reset;
            if (this->l.level == "F") out += Color::fblack + Color::bred    + Color::bold + " " + this->l.level + " " + Color::reset;
        }
        
        // process/thread
        if (this->l.process != "") {
            out += " " + Color::fcyan + Color::bblack + "[" + this->l.process + (this->l.thread != "" ? "/" + this->l.thread : "") + "]" + Color::reset;
        }
        
        // tag    
        if (this->l.tag != "")
            out += " " + Color::fwhite + this->l.tag + Color::reset;

        // log message
        if (this->l.message != "") {
            if (this->l.level == "V") out += Color::fblack + " " + this->l.message;
            if (this->l.level == "D") out += Color::fblue + " " + this->l.message;
            if (this->l.level == "I") out += Color::fgreen + " " + this->l.message;
            if (this->l.level == "W") out += Color::fyellow + " " + this->l.message;
            if (this->l.level == "E") out += Color::fred + " " + this->l.message;
            if (this->l.level == "F") out += Color::fred + " " + this->l.message;
            if (this->l.level == "S") out += Color::fblack + " " + this->l.message;
        }
        out +=  Color::reset;
        cout << out << endl;
    }
};

const int Format::BRIEF      = 0;
const int Format::PROCESS    = 1;
const int Format::TAG        = 2;
const int Format::RAW        = 3;
const int Format::TIME       = 4;
const int Format::THREADTIME = 5;
const int Format::LONG       = 6;

class Brief : public Format {

public:
    virtual void parse(string raw) {
        this->l = Logcat { /*date*/ "", /*level*/ "", /*tag*/ "", /*process*/ "", /*message*/ "", /*thread*/ "" };
        const boost::regex pattern("^([SVDIWEF])/(.*)\\(([ 0-9]{1,})\\): (.*)$");
        string::const_iterator start, end;
        start = raw.begin();
        end = raw.end();
        boost::smatch what;
        boost::match_flag_type flags = boost::match_default;
        while(boost::regex_search(start, end, what, pattern, flags)) {
            if (what.size() >= 5)
                this->l = Logcat { /*date*/ "", /*level*/ what[1], /*tag*/ what[2], /*process*/ what[3], /*message*/ what[4], /*thread*/ "" };
            start = what[0].second;
        }
    }
    virtual bool ok() {
        return this->l.level != "" && this->l.tag != "" && this->l.process != "";
    }
};

class Time : public Format {

public:
    virtual void parse(string raw) {
        this->l = Logcat { /*date*/ "", /*level*/ "", /*tag*/ "", /*process*/ "", /*message*/ "", /*thread*/ "" };
        const boost::regex pattern("^([0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}) ([SVDIWEF])/(.*)\\(([ 0-9]{1,})\\): (.*)$");
        string::const_iterator start, end;
        start = raw.begin();
        end = raw.end();
        boost::smatch what;
        boost::match_flag_type flags = boost::match_default;
        while(boost::regex_search(start, end, what, pattern, flags)) {
            if (what.size() >= 6)
                this->l = Logcat { /*date*/ what[1], /*level*/ what[2], /*tag*/ what[3], /*process*/ what[4], /*message*/ what[5], /*thread*/ "" };

            start = what[0].second;
        }
    }
    virtual bool ok() {
        return this->l.date != "" && this->l.level != "" && this->l.tag != "" && this->l.process != "";
    }
};

class ThreadTime : public Format {

public:
    virtual void parse(string raw) {
        this->l = Logcat { /*date*/ "", /*level*/ "", /*tag*/ "", /*process*/ "", /*message*/ "", /*thread*/ "" };
        const boost::regex pattern("^([0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3})  ([0-9]{1,})  ([0-9]{1,}) ([SVDIWEF]) (.*): (.*)$");
        string::const_iterator start, end;
        start = raw.begin();
        end = raw.end();
        boost::smatch what;
        boost::match_flag_type flags = boost::match_default;
        while(boost::regex_search(start, end, what, pattern, flags)) {
            if (what.size() >= 6)
                this->l = Logcat { /*date*/ what[1], /*level*/ what[4], /*tag*/ what[5], /*process*/ what[2], /*message*/ what[6], /*thread*/ what[3] };
            start = what[0].second;
        }
    }
    virtual bool ok() {
        return this->l.date != "" && this->l.level != "" && this->l.tag != "" && this->l.process != "" && this->l.thread != "";
    }
};

int main() {

    if (!isatty(fileno(stdin))) {
        /*
        Stdin is coming from a pipe or redirection
        That's how we want to use this program
        */
        
        string line;
        int format = -1; 
        bool firstTime = true;
        ThreadTime tt = ThreadTime();
        Time t = Time();
        Brief b = Brief();

        while (getline(cin, line)) {
            
            // ignore non logging stuff
            if (line.substr(0, 9) == "---------") continue;
            
            if (firstTime || format == Format::THREADTIME) {
                tt.parse(line);
                if (tt.ok()) {
                    format = Format::THREADTIME;
                    tt.print();
                }
            }
            if (firstTime || format == Format::TIME) {
                t.parse(line);
                if (t.ok()) {
                    format = Format::TIME;
                    t.print();
                }
            }
            if (firstTime || format == Format::BRIEF) {
                b.parse(line);
                if (b.ok()) {
                    format = Format::BRIEF;
                    b.print();
                }
            }
            if (format < 0) {
                cout << "ERROR: format not supported. Pipe either brief, time or threadtime formats only." << endl;
                return 1;
            }
            firstTime = false;
        }
    }
    else {
        /*
        Stdin is the terminal, so there is nothing to do
        just display help information and exit
        */
        cout << HELP << endl;
        return 0;
    }

    return 0;
}
