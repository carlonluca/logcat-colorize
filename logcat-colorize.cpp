/*
 File:      logcat-colorize.cpp

 Purpose:   Colorizes Android's logcat output in command-line windows
            Works on linux/mac terminals only. 

 Author:    BRAGA, Bruno <bruno.braga@gmail.com>
 Author:    CARLON, Luca <carlon.luca@gmail.com>

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
            https://github.com/carlonluca/logcat-colorize/issues


 Dependencies:
 
            libboost-regex-dev 
            libboost-program-options-dev 

 Compiling:
            See Makefile.
*/

#include <unistd.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
using namespace std;

const string NAME = "logcat-colorize";
const string VERSION = "0.9.0";

const int SUCCESS = 0;
const int ERROR_UNKNOWN = 1;

const string HELP = 
    NAME + " v" + VERSION + "\n"
    "\n"
    "A simple script to colorize Android debugger's logcat output.\n"
    "To use this, you MUST pipe from adb output. See examples below.\n"
    "Valid ONLY for Tag, Process, Brief, Time and ThreadTime formats.\n"
    "Other formats are simply not parsed here.\n"
    "\n"
    "Usage: adb logcat [options] | " + NAME + " [options] \n"
    "\n"
    "Options:\n"
    "   -i, --ignore        does not output non-matching data\n"
    "                       (by default, those are printed out without colorizing)\n"
    "   -h, --help          prints this help information\n"
    "   -s, --spotlight     highlight pattern in the output, value as REGEXP\n"
    "                       (i.e, -s '\bWORD\b'\n"
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
    "Author: CARLON, Luca <carlon.luca@gmail.com>\n"
    "\n"
    "    Comments or bugs are welcome at:\n"
    "    https://github.com/carlonluca/logcat-colorize/issues\n";

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

const string Color::fblack     = "30";
const string Color::fred       = "31";
const string Color::fgreen     = "32";
const string Color::fyellow    = "33";
const string Color::fblue      = "34";
const string Color::fpurple    = "35";
const string Color::fcyan      = "36";
const string Color::fwhite     = "37";
const string Color::fgrey      = "30";
const string Color::bblack     = "40";
const string Color::bred       = "41";
const string Color::bgreen     = "42";
const string Color::byellow    = "43";
const string Color::bblue      = "44";
const string Color::bpurple    = "45";
const string Color::bcyan      = "46";
const string Color::bwhite     = "47";
const string Color::reset      = "0";

struct Attribute
{
    static const string reset;
    static const string bold;
    static const string faint;
    static const string underline;
    static const string slowBlink;
    static const string fastBlink;
};

const string Attribute::reset     = "0";
const string Attribute::bold      = "1";
const string Attribute::faint     = "2";
const string Attribute::underline = "4";
const string Attribute::slowBlink = "5";
const string Attribute::fastBlink = "6";

class AnsiSequence
{
public:
    AnsiSequence(const string& attr, const string& bg, const string& fg) :
        m_attr(attr), m_bg(bg), m_fg(fg) {}
    string str() {
        stringstream ss;
        ss << *this;
        return ss.str();
    }
    friend std::ostream& operator<<(std::ostream& stream, const AnsiSequence& seq);

private:
    const string& m_attr;
    const string& m_bg;
    const string& m_fg;
};

class AnsiSequenceReset : public AnsiSequence
{
public:
    AnsiSequenceReset() : AnsiSequence(Attribute::reset, Color::reset, Color::reset) {}
};

ostream& operator<<(ostream& stream, const AnsiSequence& seq)
{
    stream << "\033[" << seq.m_attr << ";" << seq.m_bg << ";" << seq.m_fg << "m";
    return stream;
}

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
    boost::regex spotlight_pattern;
    string spotlight_color;
    boost::smatch match(const string& raw) {
        string::const_iterator start;
        start = raw.begin();
        boost::smatch results;
        boost::match_flag_type flags = boost::match_default;
        boost::regex_search(start, raw.end(), results, this->pattern, flags);
        return results;
    }

public:
    void setSpotlight(const string& spotlight) {
        this->spotlight_pattern = (boost::format("(%1%)") % spotlight).str();
    }

    const int type = -1;
    Format(const string& pattern) {
        this->l = Logcat { /*date   */ "",
                           /*level  */ "",
                           /*tag    */ "",
                           /*process*/ "",
                           /*message*/ "",
                           /*thread */ "" };
        this->pattern = pattern;

        stringstream ss;
        ss << AnsiSequence(Attribute::reset, Color::bred, Color::fwhite)
           << "$1"
           << AnsiSequenceReset();
        this->spotlight_color = ss.str();
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
        stringstream out;

        // date    
        if (this->l.date != "") 
            out << AnsiSequence(Attribute::reset, Color::reset, Color::fpurple) << " " << this->l.date << " " << AnsiSequenceReset();
        
        // level
        if (this->l.level != "") {
            if (this->l.level == "V")
                out << AnsiSequence(Attribute::bold, Color::bcyan, Color::fwhite) << " "
                    << this->l.level
                    << " " << AnsiSequenceReset();
            else if (this->l.level == "D")
                out << AnsiSequence(Attribute::bold, Color::bblue, Color::fwhite) << " "
                    << this->l.level
                    << " " << AnsiSequenceReset();
            else if (this->l.level == "I")
                out << AnsiSequence(Attribute::bold, Color::bgreen, Color::fwhite) << " "
                    << this->l.level
                    << " " << AnsiSequenceReset();
            else if (this->l.level == "W")
                out << AnsiSequence(Attribute::bold, Color::byellow, Color::fwhite) << " "
                    << this->l.level
                    << " " << AnsiSequenceReset();
            else if (this->l.level == "E")
                out << AnsiSequence(Attribute::bold, Color::bred, Color::fwhite) << " "
                    << this->l.level
                    << " " << AnsiSequenceReset();
            else if (this->l.level == "F")
                out << AnsiSequence(Attribute::bold, Color::bred, Color::fwhite) << " "
                    << this->l.level
                    << " " << AnsiSequenceReset();
            out << " ";
        }
        
        // process/thread
        if (this->l.process != "") {
            out << AnsiSequence(Attribute::reset, Color::bblack, Color::fcyan)
                << "[" << this->l.process
                << (this->l.thread != "" ? "/" + this->l.thread : "")
                << "] "
                << AnsiSequenceReset();
        }
        
        // tag    
        if (this->l.tag != "")
            out << AnsiSequence(Attribute::reset, Color::bblack, Color::fwhite)
                << this->l.tag
                << AnsiSequenceReset();

        // message
        if (this->l.message != "") {
            const string* messageColor;
            if (this->l.level == "V") messageColor = &Color::fblack;
            else if (this->l.level == "D") messageColor = &Color::fblue;
            else if (this->l.level == "I") messageColor = &Color::fgreen;
            else if (this->l.level == "W") messageColor = &Color::fyellow;
            else if (this->l.level == "E") messageColor = &Color::fred;
            else if (this->l.level == "F") messageColor = &Color::fred;
            else messageColor = &Color::fwhite;

            out << " ";
            out << AnsiSequence(Attribute::reset, Color::reset, *messageColor);
            if (!spotlight_pattern.empty())
                out << boost::regex_replace(this->l.message,
                                            spotlight_pattern,
                                            spotlight_color + AnsiSequence(Attribute::reset, Color::reset, *messageColor).str());
            else
                out << this->l.message;
        }

        out << AnsiSequenceReset();
        cout << out.str() << endl;
    }
};

const int Format::BRIEF      = 0;
const int Format::PROCESS    = 1;
const int Format::TAG        = 2;
const int Format::RAW        = 3;
const int Format::TIME       = 4;
const int Format::THREADTIME = 5;
const int Format::LONG       = 6;


class Tag : public Format {

public:
    const int type = Format::TAG;
    Tag() : Format("^([VDIWEF])/(.*?): (.*)$") {}
    ~Tag() {}
    virtual void parse(const string raw) {
        boost::smatch matches = this->match(raw);
        if (matches.size() >= 3) {
            this->l.date = "";
            this->l.level = matches[1];
            this->l.message = matches[3];
            this->l.process = "";
            this->l.tag = matches[2];
            this->l.thread = "";
        }
    }
    virtual bool valid() {
        return this->l.level != "";
    }
};

class Process : public Format {

public:
    const int type = Format::PROCESS;
    Process() : Format("^([VDIWEF])\\(([ 0-9]{1,})\\) (.*) \\(((.*?))\\)$") {}
    ~Process() {}
    virtual void parse(const string raw) {
        boost::smatch matches = this->match(raw);
        if (matches.size() >= 4) {
            this->l.date = "";
            this->l.level = matches[1];
            this->l.message = matches[3];
            this->l.process = matches[2];
            this->l.tag = matches[4];
            this->l.thread = "";
        }
    }
    virtual bool valid() {
        return this->l.level != "" && this->l.process != "";
    }
};


class Brief : public Format {

public:
    const int type = Format::BRIEF;
    Brief() : Format("^([VDIWEF])/(.*?)\\(([ 0-9]{1,})\\): (.*)$") {}
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
    Time() : Format("^([0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3}):? ([VDIWEF])/(.*?)\\(([ 0-9]{1,})\\): (.*)$") {}
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
    ThreadTime() : Format("^([0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]{3})[[:space:]]*([0-9]{1,})[[:space:]]*([0-9]{1,}) ([VDIWEF]) (.*?): (.*)$") {}
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
    Format * out = NULL;
    out = new ThreadTime();
    out->parse(raw);
    if (!out->valid()) {
        out = NULL;
        out = new Time();
        out->parse(raw);
        if (!out->valid()) {
            out = NULL;
            out = new Brief();
            out->parse(raw);
            if (!out->valid()) {
                out = NULL;
                out = new Process();
                out->parse(raw);
                if (!out->valid()) {
                    out = NULL;
                    out = new Tag();
                    out->parse(raw);
                    if (!out->valid()) {
                        out = NULL;
                    }
                }
            }
        }
    }
    return out;
}


int main(int argc, char** argv) {
    try {
        // parse command line arguments, if available
        bool ignore = false;
        namespace po = boost::program_options;
        po::options_description desc("Options");
        desc.add_options()
          ("help,h", "")
          ("spotlight,s",po::value<string>(), "")
          ("ignore,i", "");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << HELP << std::endl;
            return SUCCESS;
        }
        if (vm.count("ignore")) ignore = true;
        po::notify(vm);

        if (!isatty(fileno(stdin))) {
            /*
            Stdin is coming from a pipe or redirection
            That's how we want to use this program
            */

            string line;
            Format *f = NULL;

            while (getline(cin, line)) {
                if (f == NULL) {
                    // only need to do this once
                    f = getFormat(line);
                    if (f != NULL) {
                        if (vm.count("spotlight")) {
                            std::string line = vm["spotlight"].as<string>();
                            f->setSpotlight(line);
                        }
                    }
                }
                if (f == NULL) {
                    if (!ignore)
                        cout << line << endl;
                    continue;
                }

                // execute parsing
                f->parse(line);
                if (f->valid()) {
                    f->print();
                }
                else {
                    // hum... it matched before, but not in this line
                    // maybe something went wrong or not properly parseable
                    // according to the expected REGEX
                    if (!ignore)
                        cout << line << endl;
                }
            }
            delete f;
        }
        else {
            /*
            Stdin is the terminal, so there is nothing to do
            just display help information and exit
            */
            std::cout << HELP << std::endl;
        }

    }
    catch(std::exception& e) {
        std::cerr << "Oops! Something went wrong. Error: " << e.what() << std::endl;
        return ERROR_UNKNOWN;
    }
    return SUCCESS;
}
