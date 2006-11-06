#
# Copyright (C) 2003-2006 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 
#

## the regular library
lib_LTLIBRARIES = libawb.la
libawb_la_SOURCES = util.h util.cpp uniondir.h uniondir.cpp \
  inifile.h inifile.cpp workspace.h workspace.cpp module.h module.cpp \
  modparam.h modparam.cpp model.h model.cpp model_builder.h model_builder.cpp \
  benchmark.h benchmark.cpp benchmark_runner.h benchmark_runner.cpp

EXTRA_DIST = doxygen.config
#-----------------------------------------------------------------------------
# Generate documentation with Doxygen
#-----------------------------------------------------------------------------
.PHONY: dox
dox: doxygen.config
	doxygen doxygen.config
	@echo "Doxygen output is at `pwd`/dox/html/index.html"

clean-local:
	@rm -rf dox

##
## component tests
##
check_PROGRAMS = test-util test-uniondir test-inifile test-workspace \
  test-modparam test-module test-model test-benchmark test-model_builder \
  test-benchmark_runner

## util
test_util_SOURCES = util.cpp
test_util_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_util_LDADD = libawb.la

## uniondir
test_uniondir_SOURCES = uniondir.cpp
test_uniondir_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_uniondir_LDADD = libawb.la

## inifile
test_inifile_SOURCES = inifile.cpp
test_inifile_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_inifile_LDADD = libawb.la

## workspace
test_workspace_SOURCES = workspace.cpp
test_workspace_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_workspace_LDADD = libawb.la

## modparam
test_modparam_SOURCES = modparam.cpp
test_modparam_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_modparam_LDADD = libawb.la

## module
test_module_SOURCES = module.cpp
test_module_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_module_LDADD = libawb.la

## model
test_model_SOURCES = model.cpp
test_model_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_model_LDADD = libawb.la

## benchmark
test_benchmark_SOURCES = benchmark.cpp
test_benchmark_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_benchmark_LDADD = libawb.la

## model_builder
test_model_builder_SOURCES = model_builder.cpp
test_model_builder_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_model_builder_LDADD = libawb.la

## benchmark_runner
test_benchmark_runner_SOURCES = benchmark_runner.cpp
test_benchmark_runner_CXXFLAGS = -DTESTS $(AM_CXXFLAGS)
test_benchmark_runner_LDADD = libawb.la

## tests that must succeed
TESTS = test-util test-uniondir test-inifile test-workspace \
  test-modparam test-module test-model test-benchmark test-model_builder \
  test-benchmark_runner

## tests that must fail
#XFAIL_TESTS =
