#!/bin/make
OF_ROOT=${CURDIR}/third_party/openFrameworks
OF_INCLUDE =$(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk
CXXFLAGS += -g -rdynamic
include $(OF_INCLUDE)
