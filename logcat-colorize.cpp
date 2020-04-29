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
#include <optional>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
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
    "                       (i.e, -s '\bWORD\b')\n"
    "       --list-ansi     list available ansi escape codes to format the output\n"
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
    static const string fdefault;
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
    static const string bdefault;
};

const string Color::fblack     = "30";
const string Color::fred       = "31";
const string Color::fgreen     = "32";
const string Color::fyellow    = "33";
const string Color::fblue      = "34";
const string Color::fpurple    = "35";
const string Color::fcyan      = "36";
const string Color::fwhite     = "97";
const string Color::fdefault   = "39";
const string Color::bblack     = "40";
const string Color::bred       = "41";
const string Color::bgreen     = "42";
const string Color::byellow    = "43";
const string Color::bblue      = "44";
const string Color::bpurple    = "45";
const string Color::bcyan      = "46";
const string Color::bwhite     = "47";
const string Color::bdefault   = "49";

struct Attribute
{
    static const string reset;
    static const string bold;
    static const string faint;
    static const string underline;
    static const string slowBlink;
    static const string fastBlink;
    static const string reverse;
};

const string Attribute::reset     = "0";
const string Attribute::bold      = "1";
const string Attribute::faint     = "2";
const string Attribute::underline = "4";
const string Attribute::slowBlink = "5";
const string Attribute::fastBlink = "6";
const string Attribute::reverse   = "7";

class AnsiSequence
{
public:
    AnsiSequence() {}
    AnsiSequence(const string& attr, const string& bg, const string& fg) :
        m_attr(attr),
        m_bg(bg),
        m_fg(fg) {
            stringstream stream;
            stream << "\033[" << attr << ";" << bg << ";" << fg << "m";
            m_str = stream.str();
        }

    const string& str() const { return m_str; }
    friend ostream& operator<<(ostream& stream, const AnsiSequence& seq);

private:
    string m_attr;
    string m_bg;
    string m_fg;
    string m_str;
};

class AnsiSequenceReset : public AnsiSequence
{
public:
    AnsiSequenceReset() : AnsiSequence(Attribute::reset, Color::bdefault, Color::fdefault) {}
};

ostream& operator<<(ostream& stream, const AnsiSequence& seq)
{
    stream << seq.m_str;
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
    boost::regex escapeSequencePattern;
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
        this->escapeSequencePattern = "\\^\\[(\\d+);(\\d+);(\\d+)m$";

        RESET = AnsiSequenceReset();

        stringstream ss;
        ss << AnsiSequence(Attribute::reset, Color::bred, Color::fwhite)
           << "$1"
           << RESET;
        this->spotlight_color = ss.str();

        // Parse configuration.
        parseConfiguration();
    }

    virtual ~Format() {};
    static const int BRIEF;
    static const int PROCESS;
    static const int TAG;
    static const int RAW;
    static const int TIME;
    static const int THREADTIME;
    static const int LONG;

    static AnsiSequence ID_VERBOSE;
    static AnsiSequence ID_DEBUG;
    static AnsiSequence ID_INFO;
    static AnsiSequence ID_WARNING;
    static AnsiSequence ID_ERROR;
    static AnsiSequence ID_FATAL;
    static AnsiSequence MSG_VERBOSE;
    static AnsiSequence MSG_DEBUG;
    static AnsiSequence MSG_INFO;
    static AnsiSequence MSG_WARNING;
    static AnsiSequence MSG_ERROR;
    static AnsiSequence MSG_FATAL;
    static AnsiSequence RESET;
    
    virtual void parse(const string raw) = 0;
    virtual bool valid() { return false; }

    void print() {
        stringstream out;

        // date
        if (this->l.date != "") {
            static AnsiSequence seq = AnsiSequence(Attribute::reset, Color::bdefault, Color::fpurple);
            out << seq
                << " " << spotIfNeeded(l.date, seq) << " " << RESET;
        }
        
        // level
        AnsiSequence* idSeq = nullptr;
        AnsiSequence* msgSeq = nullptr;
        if (l.level == "D") {
            idSeq = &ID_DEBUG;
            msgSeq = &MSG_DEBUG;
        }
        else if (l.level == "V") {
            idSeq = &ID_VERBOSE;
            msgSeq = &MSG_VERBOSE;
        }
        else if (l.level == "I") {
            idSeq = &ID_INFO;
            msgSeq = &MSG_INFO;
        }
        else if (l.level == "W") {
            idSeq = &ID_WARNING;
            msgSeq = &MSG_WARNING;
        }
        else if (l.level == "E") {
            idSeq = &ID_ERROR;
            msgSeq = &MSG_ERROR;
        }
        else if (l.level == "F") {
            idSeq = &ID_FATAL;
            msgSeq = &MSG_FATAL;
        }

        if (this->l.level != "") {
            if (idSeq)
                out << *idSeq << " " << this->l.level << " " << RESET;
            out << " ";
        }
        
        // process/thread
        if (this->l.process != "") {
            static AnsiSequence seq = AnsiSequence(Attribute::reset, Color::bblack, Color::fcyan);
            stringstream _out;
            _out << "[" << this->l.process
                 << (this->l.thread != "" ? "/" + this->l.thread : "")
                 << "] ";
            out << seq
                << spotIfNeeded(_out.str(), seq)
                << RESET;
        }
        
        // tag    
        if (this->l.tag != "") {
            static AnsiSequence seq = AnsiSequence(Attribute::reset, Color::bblack, Color::fwhite);
            out << seq
                << spotIfNeeded(l.tag, seq)
                << RESET;
        }

        // message
        if (this->l.message != "") {
            out << " ";
            if (idSeq)
                out << *msgSeq;
            out << spotIfNeeded(l.message, (msgSeq ? *msgSeq : RESET));
        }

        out << RESET;
        cout << out.str() << endl;
    }

private:
    void parseConfiguration() {

#define RESET_FORMAT(level) \
    reset_format("LOGCAT_COLORIZE_" #level, level)

        RESET_FORMAT(ID_DEBUG);
        RESET_FORMAT(ID_VERBOSE);
        RESET_FORMAT(ID_INFO);
        RESET_FORMAT(ID_WARNING);
        RESET_FORMAT(ID_ERROR);
        RESET_FORMAT(ID_FATAL);
        RESET_FORMAT(MSG_DEBUG);
        RESET_FORMAT(MSG_VERBOSE);
        RESET_FORMAT(MSG_INFO);
        RESET_FORMAT(MSG_WARNING);
        RESET_FORMAT(MSG_ERROR);
        RESET_FORMAT(MSG_FATAL);
    }

    void reset_format(const string& envVarName, AnsiSequence& ansiSequence)
    {
        boost::optional<AnsiSequence> custom = parseEscapeSequenceVariable(envVarName);
        if (custom)
            ansiSequence = custom.get();
    }

    boost::optional<AnsiSequence> parseEscapeSequenceVariable(const string& envVar) {
        char* envValue = getenv(envVar.c_str());
        if (!envValue)
            return boost::none;
        
        const string escapeSequenceString(envValue);
        string::const_iterator start;
        start = escapeSequenceString.begin();
        boost::smatch results;
        boost::match_flag_type flags = boost::match_default;
        boost::regex_search(start, escapeSequenceString.end(), results, this->escapeSequencePattern, flags);
        if (results.size() >= 4) {
            string attr = results[1];
            string bg = results[2];
            string fg = results[3];
            return AnsiSequence(attr, bg, fg);
        }
        
        return boost::none;
    }

    string spotIfNeeded(const string& log, const AnsiSequence& resume) {
        if (!spotlight_pattern.empty())
            return boost::regex_replace(log, spotlight_pattern, spotlight_color + resume.str());
        else
            return log;
    }
};

const int Format::BRIEF      = 0;
const int Format::PROCESS    = 1;
const int Format::TAG        = 2;
const int Format::RAW        = 3;
const int Format::TIME       = 4;
const int Format::THREADTIME = 5;
const int Format::LONG       = 6;

AnsiSequence Format::ID_VERBOSE = AnsiSequence(Attribute::bold, Color::bcyan, Color::fwhite);
AnsiSequence Format::ID_DEBUG   = AnsiSequence(Attribute::bold, Color::bblue, Color::fwhite);
AnsiSequence Format::ID_INFO    = AnsiSequence(Attribute::bold, Color::bgreen, Color::fwhite);
AnsiSequence Format::ID_WARNING = AnsiSequence(Attribute::bold, Color::byellow, Color::fwhite);
AnsiSequence Format::ID_ERROR   = AnsiSequence(Attribute::bold, Color::bred, Color::fwhite);
AnsiSequence Format::ID_FATAL   = ID_ERROR;

AnsiSequence Format::MSG_VERBOSE = AnsiSequence(Attribute::reset, Color::bdefault, Color::fcyan);
AnsiSequence Format::MSG_DEBUG   = AnsiSequence(Attribute::reset, Color::bdefault, Color::fblue);
AnsiSequence Format::MSG_INFO    = AnsiSequence(Attribute::reset, Color::bdefault, Color::fgreen);
AnsiSequence Format::MSG_WARNING = AnsiSequence(Attribute::reset, Color::bdefault, Color::fyellow);
AnsiSequence Format::MSG_ERROR   = AnsiSequence(Attribute::reset, Color::bdefault, Color::fred);
AnsiSequence Format::MSG_FATAL   = MSG_ERROR;

AnsiSequence Format::RESET       = AnsiSequenceReset();

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
    std::unique_ptr<Format> out(new ThreadTime());
    out->parse(raw);
    if (!out->valid()) {
        out.reset(new Time());
        out->parse(raw);
        if (!out->valid()) {
            out.reset(new Brief());
            out->parse(raw);
            if (!out->valid()) {
                out.reset(new Process());
                out->parse(raw);
                if (!out->valid()) {
                    out.reset(new Tag());
                    out->parse(raw);
                    if (!out->valid()) {
                        out = NULL;
                    }
                }
            }
        }
    }
    return out.release();
}

void list_ansi()
{
    vector<string> fgs {
        Color::fdefault,
        Color::fblack,
        Color::fred,
        Color::fgreen,
        Color::fyellow,
        Color::fblue,
        Color::fpurple,
        Color::fcyan,
        Color::fwhite
    };

    vector<string> bgs {
        Color::bdefault,
        Color::bblack,
        Color::bred,
        Color::bgreen,
        Color::byellow,
        Color::bblue,
        Color::bpurple,
        Color::bcyan,
        Color::bwhite
    };

    vector<string> attrs {
        Attribute::reset,
        Attribute::bold,
        Attribute::faint,
        Attribute::underline,
        Attribute::slowBlink,
        Attribute::fastBlink,
        Attribute::reverse
    };

    int bkgCount = 0;
    for (const string& bg: bgs) {
        cout << endl << "Background " << bkgCount++ << ":" << endl;
        for (const string& fg: fgs) {
            for (const string& attr: attrs) {
                cout << AnsiSequence(attr, bg, fg)
                     << "^["
                     << attr
                     << ";"
                     << bg
                     << ";"
                     << fg
                     << "m"
                     << AnsiSequenceReset() << " ";
            }

            cout << endl;
        }
    }

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
          ("ignore,i", "")
          ("list-ansi", "");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << HELP << std::endl;
            return SUCCESS;
        }

        if (vm.count("list-ansi")) {
            list_ansi();
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
