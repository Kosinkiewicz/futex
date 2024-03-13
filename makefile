#
# Makefile
#
#  Created on: Mar 12, 2024
#      Author: Leszek Kosinkiewicz<kosinkiewicz@gmail.com>
#
#  this is under the Apache License Version 2.0.
#

EXECUTABLE := futex

CXXFLAGS := -std=c++14 -pthread -O2 -g -pedantic -Wall -Wextra -Werror

LIBS := \
	-lgtest \
	-lstdc++ \
	-lpthread

TEST_OPT := --gtest_break_on_failure

repeat ?= 1
install_path ?= /usr/local/include

CXX := g++
LD := g++
RM := rm -rfv
CP := cp -v

all: help

help: 
	@cat help.txt

opt: | $(EXECUTABLE)
	@./$(EXECUTABLE) --help

test : $(EXECUTABLE)
	./$< $(TEST_OPT) --gtest_repeat=$(repeat) $(opt)

$(EXECUTABLE) : compile
	$(LD) -o $@ $(OBJS) $(LIBS)

HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,$(OBJECTS))
INSTALL_HEADERS := $(addprefix $(install_path)/,$(HEADERS))

$(OBJS): $(HEADERS) $(SOURCES) | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o : %.cpp
	@echo compile $< ...
	@$(CXX) $(CXXFLAGS) -c $< -o $@

compile : $(HEADERS) $(SOURCES) | $(OBJS)

clean :
	@$(RM) $(EXECUTABLE) $(OBJDIR)

install:
	@$(CP) *.h $(install_path) 
uninstall:
	@$(RM) $(INSTALL_HEADERS)

.PHONY : all
.PHONY : help
.PHONY : opt
.PHONY : install
.PHONY : uninstall
.PHONY : clean
.PHONY : test
.PHONY : compile
.SILENT : $(EXECUTABLE)
.SILENT : $(OBJDIR)
.SILENT : $(OBJS)
