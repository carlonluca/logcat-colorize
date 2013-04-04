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
const string VERSION = "0.4";

const string HELP = 
    NAME + " v" + VERSION + "\n"
    "\n"
    "A simple script to colorize Android debugger's logcat output.\n"
    "To use this, you should pipe from adb output. See examples below.\n"
    "\n"
    "Note: Valid ONLY for Brief, Time and ThreadTime formats at this point.\n"
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
    boost::regex pattern;
    boost::smatch match(const string raw) {
    	string::const_iterator start;
		start = raw.begin();
		boost::smatch results;
		boost::match_flag_type flags = boost::match_default;
		boost::regex_search(start, raw.end(), results, this->pattern, flags);
		return results;
    }

public:
    const int type = -1;
    Format(const string pattern) {
        this->l = Logcat { /*date   */ "",
        				   /*level  */ "",
        				   /*tag    */ "",
        				   /*process*/ "",
        				   /*message*/ "",
        				   /*thread */ "" };
        this->pattern = pattern;
    }
    virtual ~Format() {};
    static const int BRIEF;
    static const int PROCESS;
    static const int TAG;
    static const int RAW;
    static const int TIME;
    static const int THREADTIME;
    static const int LONG;
    
    virtual void parse(const string raw) = 0;
    virtual bool valid() { return false; }
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
	const int type = Format::BRIEF;
	Brief() : Format("^([SVDIWEF])/(.*?)\\(([ 0-9]{1,})\\): (.*)$") {}
	~Brief() {}
    virtual void parse(const string raw) {
        boost::smatch matches = this->match(raw);
        if (matches.size() >= 5) {
        	this->l.date = "";
        	this->l.level = matches[1];
        	this->l.message = matches[4];
        	this->l.process = matches[3];
        	this->l.tag = matches[2];
        	this->l.thread = "";
        }
    }
    virtual bool valid() {
        return this->l.level != "" && this->l.process != "";
    }
};

class Time : public Format {

public:
	const int type = Format::TIME;
	Time() : Format("^([0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}) ([SVDIWEF])/(.*?)\\(([ 0-9]{1,})\\): (.*)$") {}
	~Time() {}
    virtual void parse(string raw) {
    	boost::smatch matches = this->match(raw);
		if (matches.size() >= 6) {
			this->l.date = matches[1];
			this->l.level = matches[2];
			this->l.message = matches[5];
			this->l.process = matches[4];
			this->l.tag = matches[3];
			this->l.thread = "";
		}
    }
    virtual bool valid() {
        return this->l.date != "" && this->l.level != "" && this->l.process != "";
    }
};

class ThreadTime : public Format {

public:
	const int type = Format::THREADTIME;
	ThreadTime() : Format("^([0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3})  ([0-9]{1,})  ([0-9]{1,}) ([SVDIWEF]) (.*?): (.*)$") {}
	~ThreadTime() {}
    virtual void parse(string raw) {
    	boost::smatch matches = this->match(raw);
		if (matches.size() >= 7) {
			this->l.date = matches[1];
			this->l.level = matches[4];
			this->l.message = matches[6];
			this->l.process = matches[2];
			this->l.tag = matches[5];
			this->l.thread = matches[3];
		}
    }
    virtual bool valid() {
        return this->l.date != "" && this->l.level != "" && this->l.process != "" && this->l.thread != "";
    }
};


Format* getFormat(const string raw) {

	//
	// At this point we don't know yet which format is being used, so guess it
	// (from the more complex first)
	//

	ThreadTime tt = ThreadTime();
	tt.parse(raw);
	if (tt.valid())
		return new ThreadTime();

	Time t = Time();
	t.parse(raw);
	if (t.valid())
		return new Time();

	Brief b = Brief();
	b.parse(raw);
	if (b.valid())
		return new Brief();

	// If nothing was found
	return NULL;
}


int main() {

    if (!isatty(fileno(stdin))) {
        /*
        Stdin is coming from a pipe or redirection
        That's how we want to use this program
        */
        
        string line;
        Format* f = NULL;

        while (getline(cin, line)) {
            
            // ignore non logging stuff
            if (line.substr(0, 9) == "---------") continue;

            if (f == NULL) {
            	// only need to do this once
            	f = getFormat(line);
            }
            if (f == NULL) {
                cout << "ERROR: format not supported. Pipe either brief, time or threadtime formats only." << endl;
                return 1;
            }

            // output in colors
            f->parse(line);
           	f->print();
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
