#
# File:      Makefile
#
# Purpose:   Ease compiling pain!
#
# Author:    BRAGA, Bruno <bruno.braga@gmail.com>
#
# Copyright:
#            Licensed under the Apache License, Version 2.0 (the "License");
#            you may not use this file except in compliance with the License.
#            You may obtain a copy of the License at
#
#            http://www.apache.org/licenses/LICENSE-2.0
#
#            Unless required by applicable law or agreed to in writing, software
#            distributed under the License is distributed on an "AS IS" BASIS,
#            WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#            implied. See the License for the specific language governing
#            permissions and limitations under the License.
#
# Notes:     
#            Bugs, issues and requests are welcome at:
#            https://bitbucket.org/brunobraga/logcat-colorize/issues
#

#CC=g++
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
   CXXFLAGS += -lboost_regex -lboost_program_options -std=c++0x
else ifeq ($(UNAME_S),Darwin)
   CXXFLAGS += -L/opt/local/lib -lboost_regex-mt -lboost_program_options-mt -std=c++0x -I/opt/local/include -Wno-deprecated-register
endif
EXEC=logcat-colorize
DEPS=logcat-colorize.cpp

PREFIX ?= /usr

INSTALLDIR=$(DESTDIR)$(PREFIX)/bin

$(EXEC): $(DEPS)
	$(CXX) $(DEPS) -o $(EXEC) $(CXXFLAGS)

$(INSTALLDIR):
	mkdir -pv $(INSTALLDIR)

clean:
	rm -f $(EXEC)

install: $(EXEC) $(INSTALLDIR)
	install -m 0755 $(EXEC) $(INSTALLDIR)

uninstall:
	rm -f $(INSTALLDIR)/$(EXEC)

.PHONY: install clean
