#automatically generated makefile
packagename := daisy
version := 1.2
author := Engin Tola
description := daisy descriptor
licence := GPL v2.0 or higher distributed by FSF
#........................................
installdir := /usr
external_sources :=
external_libraries := opencv
libdir := lib
srcdir := src
includedir:= include
#........................................
optimize := true
f77 := false
sse := false
multi-threading := false
parallelize := false
profile := false
#........................................
specialize := false
platform := native
#........................................
sources := daisy.cpp main.cpp fileio.cpp general.cpp interaction.cpp math.cpp matrix_manipulation.cpp

################################################################################
################# - MAKEFILE STATIC VARIABLES - ################################
################################################################################

sources_list = $(addprefix $(srcdir)/, $(sources))

objects       := $(filter %.o,$(subst   .c,.o,$(sources_list)))
objects       += $(filter %.o,$(subst  .cc,.o,$(sources_list)))
objects       += $(filter %.o,$(subst .cpp,.o,$(sources_list)))
dependencies  := $(subst .o,.d,$(objects))

libtarget     := $(libdir)/lib$(packagename).a
exetarget     := $(packagename)
pkgconfigfile := $(packagename).pc

automakefile := make.auto
commentfile  := makefile.comment

compiler := g++
CXX := ${compiler}


################################################################################
########################### - MAKEFILE FLAGS - #################################
################################################################################

ARFLAGS = ruv
CTAGFLAGS := -e -R --languages=c++,c

CXXFLAGS += ${define_flags} -I$(includedir)

ifneq ($(external_sources),)
 CXXFLAGS += `pkg-config --cflags ${external_sources}`
endif

ifneq ($(external_libraries),)
 CXXFLAGS += `pkg-config --cflags ${external_libraries}`
 LDFLAGS  += `pkg-config --libs ${external_libraries}`
endif

ifeq ($(f77),true)
 LDFLAGS += -lg2c
endif

ifeq ($(sse),true)
    CXXFLAGS += -msse -msse2
    CPPFLAGS += -msse -msse2
endif

CXXFLAGS += -fno-strict-aliasing -Wall

ifeq ($(multi-threading),true)
    CXXFLAGS += -lpthread
endif

ifeq ($(profile),true)
  CXXFLAGS+= -pg
endif

dbg_flags = -g
opt_flags = -O3 -DHAVE_INLINE -DGSL_RANGE_CHECK_OFF
spc_flags = '-march=$(platform)' -mfpmath=sse

ifeq ($(optimize),true)
  CXXFLAGS += $(opt_flags)
  ifeq ($(specialize),true)
     CXXFLAGS += $(spc_flags)
  endif
  ifeq ($(parallelize),true)
     CXXFLAGS += -fopenmp
  endif
else
  CXXFLAGS += ${dbg_flags}
  parallelize = false
endif

is_debug := $(wildcard .debug)

ifeq ($(strip $(is_debug)),)
  is_debug = true
else
  is_debug = false
endif

ifeq ($(optimize),true)
  state_file = .release
else
  state_file = .debug
endif
################################################################################
################################ - MAKEFILE RULES - ############################
################################################################################

.PHONY       : $(exetarget)
$(exetarget) : ${objects}
	@echo compiler path = ${compiler}
	@echo
	@echo ------------------ making executable
	@echo
	$(compiler) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

.PHONY : compilation
compilation:
ifeq ($(is_debug),true)
	@echo debug
else
	@echo release
endif

.PHONY  : library
library : $(libtarget)
	@echo
	@echo ------------------ making library $(libtarget)
	@echo

$(libtarget): $(objects)
	@echo
	@echo ------------------ creating library
	@echo
	$(AR) $(ARFLAGS) $@ $^

.PHONY : tags
tags   :
	@echo
	@echo ------------------ creating tag entries
	@echo
	@etags $(includedir)/*.h $(includedir)/$(packagename)/*.h $(srcdir)/*.cpp $(srcdir)/*.cc $(srcdir)/*.c

.PHONY : dox
dox    : Doxyfile
	@echo
	@echo ------------------ creating documentation
	@echo
	@doxygen Doxyfile

.PHONY : clean
clean  :
	@echo
	@echo ------------------ cleaning *.o exe lib
	@echo
	@echo rm -f $(objects) ${libtarget} ${exetarget} TAGS gmon.out
	@rm -f $(objects) ${libtarget} ${exetarget} TAGS gmon.out

.PHONY : cleanaux
cleanaux  :
	@echo
	@echo ------------------ cleaning *.o *.d
	@echo
	@echo rm -f $(objects) ${dependencies} TAGS gmon.out
	@rm -f $(objects) ${dependencies} TAGS gmon.out

.PHONY   : cleandox
cleandox :
	@echo
	@echo ------------------ removing documentation
	@echo
	@rm -rf doc

.PHONY : cleandist
cleandist  :
	@echo
	@echo ------------------ cleaning everything
	@echo
	@rm -f $(pkgconfigfile) $(libtarget) $(packagename) $(objects) ${exetarget} $(dependencies) TAGS gmon.out

.PHONY : clear
clear :
	@rm -rf \#* ${dependencies}

.PHONY: install-lib
install-lib: $(libtarget) pkgfile uninstall
	@echo
	@echo ------------------ installing library and header files
	@echo
	@echo ------------------ installing at $(installdir)
	@echo
	@mkdir -p $(installdir)/include/$(packagename)
	@cp -vfR $(includedir)/* $(installdir)/include/
	@mkdir -p $(installdir)/lib/pkgconfig
	@cp -vfR $(libtarget)  $(installdir)/lib
	@echo
	@echo ------------------ installing the pkg-config file to $(installdir)/lib/pkgconfig. \
		Remember to add this path to your PKG_CONFIG_PATH variable
	@echo
	@cp $(pkgconfigfile) $(installdir)/lib/pkgconfig/

.PHONY: install
install: $(exetarget)
	@cp -f $(exetarget) $(installdir)/bin

.PHONY: install-dev
install-dev : $(libtarget) pkgfile uninstall
	@echo
	@echo ------------------ installing library and development files
	@echo
	@echo ------------------ installing at $(installdir)
	@echo
	@mkdir -p $(installdir)/include/$(packagename)
	@cp -vfR $(includedir)/$(packagename)/[!.]* $(installdir)/include/$(packagename)
	@mkdir -p $(installdir)/lib/pkgconfig
	@cp -vfR $(libtarget)  $(installdir)/lib                 # copy the static library
	@mkdir -p $(installdir)/src/$(packagename)                 # create the source directory
	@cp -vfR $(srcdir)/*.c* $(installdir)/src/$(packagename) # copy development files
	@cp -vf makefile $(installdir)/src/$(packagename)
	@cp $(pkgconfigfile) $(installdir)/lib/pkgconfig/

.PHONY: uninstall
uninstall:
	@echo
	@echo ------------------ uninstalling if-installed
	@echo
	@rm -rf $(installdir)/include/$(packagename)
	@rm -f   $(installdir)/$(libtarget)
	@rm -rf $(installdir)/src/$(packagename)
	@rm -f   $(installdir)/lib/pkgconfig/$(pkgconfigfile)
	@rm -f   $(installdir)/bin/$(exetarget)

ifneq "$(MAKECMDGOALS)" "clean"
  include $(dependencies)
endif

%.d : %.cc
	@echo
	@echo ------------------ creating dependencies for $@
	@echo
	$(compiler) $(CXXFLAGS) $(TARGET_ARCH) -MM $< | \
	sed 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: ,' > $@.tmp
	mv -f $@.tmp $@
	@echo

%.d : %.cpp
	@echo
	@echo ------------------ creating dependencies for $@
	@echo
	$(compiler) $(CXXFLAGS) $(TARGET_ARCH) -MM $< | \
	sed 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: ,' > $@.tmp
	mv -f $@.tmp $@
	@echo

.PHONY : pkgfile
pkgfile:
	@echo
	@echo ------------------ creating pkg-config file
	@echo
	@echo "# Package Information for pkg-config"    >  $(pkgconfigfile)
	@echo "# Author: $(author)" 			>> $(pkgconfigfile)
	@echo "# Created: `date`"			>> $(pkgconfigfile)
	@echo "# Licence: $(licence)"			>> $(pkgconfigfile)
	@echo 						>> $(pkgconfigfile)
	@echo prefix=$(installdir)       		>> $(pkgconfigfile)
	@echo exec_prefix=$$\{prefix\}     		>> $(pkgconfigfile)
	@echo libdir=$$\{exec_prefix\}/lib 		>> $(pkgconfigfile)
	@echo includedir=$$\{prefix\}/include   	>> $(pkgconfigfile)
	@echo 						>> $(pkgconfigfile)
	@echo Name: "$(packagename)" 			>> $(pkgconfigfile)
	@echo Description: "$(description)" 		>> $(pkgconfigfile)
	@echo Version: "$(version)" 			>> $(pkgconfigfile)
	@echo Libs: -L$$\{libdir} -l$(packagename) 	>> $(pkgconfigfile)
	@echo Cflags: -I$$\{includedir\} ${define_flags}>> $(pkgconfigfile)
	@echo Requires: ${external_libraries}           >> $(pkgconfigfile)
	@echo 						>> $(pkgconfigfile)

.PHONY : revert
revert :
	@mv -f makefile.in makefile

.PHONY : export
export :
	@echo "#automatically generated makefile"         >  $(automakefile)
	@echo packagename := ${packagename}               >> ${automakefile}
	@echo version := ${version}                       >> ${automakefile}
	@echo author := ${author}                         >> ${automakefile}
	@echo description := "${description}"             >> ${automakefile}
	@echo licence := ${licence}                       >> ${automakefile}
	@echo "#........................................" >> ${automakefile}
	@echo installdir := ${installdir}                 >> $(automakefile)
	@echo external_sources := ${external_sources}     >> ${automakefile}
	@echo external_libraries := ${external_libraries} >> ${automakefile}
	@echo libdir := ${libdir}                         >> ${automakefile}
	@echo srcdir := ${srcdir}                         >> ${automakefile}
	@echo includedir:= ${includedir}                  >> ${automakefile}
	@echo "#........................................" >> ${automakefile}
	@echo optimize := ${optimize}                     >> ${automakefile}
	@echo f77 := ${f77}                               >> ${automakefile}
	@echo sse := ${sse}                               >> ${automakefile}
	@echo multi-threading := ${multi-threading}       >> ${automakefile}
	@echo parallelize := ${parallelize}               >> ${automakefile}
	@echo profile := ${profile}                       >> ${automakefile}
	@echo "#........................................" >> ${automakefile}
	@echo specialize := ${specialize}                 >> ${automakefile}
	@echo platform := ${platform}                     >> ${automakefile}
	@echo "#........................................" >> ${automakefile}
	@echo sources := ${sources_list}                  >> ${automakefile}
	@echo                                             >> ${automakefile}
	@cat ${MAKEFILE_HEAVEN}/static-variables.makefile >> ${automakefile}
	@cat ${MAKEFILE_HEAVEN}/flags.makefile            >> ${automakefile}
	@cat ${MAKEFILE_HEAVEN}/rules.makefile            >> ${automakefile}
	@echo >> ${automakefile}
	@mv makefile makefile.in
	@mv ${automakefile} makefile

.PHONY : flags
flags :
	@echo
	@echo ------------------ build flags
	@echo
	@echo ldflags  = $(LDFLAGS)
	@echo cxxflags = $(CXXFLAGS)
	@echo source_list = ${sources_list}

.PHONY : gflat
gflat :
	@gprof $(packagename) gmon.out -p | more

.PHONY : gcall
gcall :
	@gprof $(packagename) gmon.out -q | more

.PHONY : state
state  :
	@echo
	@echo "package name      : ${packagename} v${version} by ${author}"
	@echo "                   (${description}) "
	@echo "------------------------------------------------------------------------"
	@echo "install directory : ${installdir}"
	@echo "external sources  : ${external_sources}"
	@echo "external libs     : ${external_libraries}"
	@echo "fortran support   : ${f77}"
	@echo "------------------------------------------------------------------------"
	@echo "optimize          : ${optimize}"
	@echo "parallelize       : ${parallelize}"
	@echo "profile           : ${profile}"
	@echo "sse               : ${sse}"
	@echo "multi-threading   : ${multi-threading}"
	@echo "------------------------------------------------------------------------"
	@echo "specialize        : ${specialize} for ${platform}"
	@echo "------------------------------------------------------------------------"
	@echo "sources           : ${sources_list}"
	@echo "------------------------------------------------------------------------"
	@echo ldflags  = $(LDFLAGS)
	@echo cxxflags = $(CXXFLAGS)
	@echo sources = ${sources_list}
	@echo objects = ${objects}

.PHONY : rules
rules :
	@echo
	@echo ------------------ legitimate rules
	@echo
	@echo "(nothing)   : makes the executable : "
	@echo "library     : generates the library"
	@echo "tags        : generates etags files"
	@echo "dox         : generates the doxygen documentation if Doxyfile exists"
	@echo "clean       : cleans up .o lib and exe files"
	@echo "cleanaux    : cleans auxilary files"
	@echo "cleandox    : cleans up the documentation"
	@echo "cleandist   : cleans everything except source+headers"
	@echo "install     : installs the executable"
	@echo "install-lib : installs the library"
	@echo "install-dev : installs the library along with documentation files"
	@echo "uninstall   : uninstalls the library"
	@echo "pkgfile     : generates the pkg-config file"
	@echo "flags       : shows the flags that will be used"
	@echo "gflat       : shows gprof profiler flat view result"
	@echo "gcall       : shows gprof profiler call graph view result"
	@echo "rules       : shows this text"
	@echo "state       : show the configuration state of the package"
	@echo "export      : export the makefile"
	@echo "revert      : moves makefile.in to makefile"
	@echo "clear       : clears #* & dependency files"
	@echo

.PHONY : comment
comment :
	@echo "#=============================================================================#" > ${commentfile}
	@echo "# This program is free software; you can redistribute it and/or modify it     #" >> ${commentfile}
	@echo "# under the terms of the GNU General Public License version 2 (or higher) as  #" >> ${commentfile}
	@echo "# published by the Free Software Foundation.                                  #" >> ${commentfile}
	@echo "#                                                                             #" >> ${commentfile}
	@echo "# This program is distributed in the hope that it will be useful, but WITHOUT #" >> ${commentfile}
	@echo "# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       #" >> ${commentfile}
	@echo "# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public Licence for   #" >> ${commentfile}
	@echo "# more details.                                                               #" >> ${commentfile}
	@echo "#                                                                             #" >> ${commentfile}
	@echo "# Written and (C) by                                                          #" >> ${commentfile}
	@echo "# Engin Tola                                                                  #" >> ${commentfile}
	@echo "#                                                                             #" >> ${commentfile}
	@echo "# web   : http://cvlab.epfl.ch/~tola                                          #" >> ${commentfile}
	@echo "# email : engin.tola@epfl.ch                                                  #" >> ${commentfile}
	@echo "#                                                                             #" >> ${commentfile}
	@echo "#=============================================================================#" >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# for a more detailed explanation visit "                                        >> ${commentfile}
	@echo "# http://cvlab.epfl.ch/~tola/makefile_heaven.html"                               >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# this is the directory the library will be installed if you issue a"            >> ${commentfile}
	@echo "# 'make install-lib' command:"                                                   >> ${commentfile}
	@echo '# headers to $${installdir}/$${packagename}/include'                             >> ${commentfile}
	@echo '# library to $$(installdir)/lib'                                                 >> ${commentfile}
	@echo '# pkg-file to $$(installdir)/lib/pkgconfig/'                                     >> ${commentfile}
	@echo "# 'make install' command:"                                                       >> ${commentfile}
	@echo '# executable to $${installdir}/bin/'                                             >> ${commentfile}
	@echo 'installdir  := $(installdir)'                                                    >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# this is the name of the package. i.e if it is 'cvlab' the executable"          >> ${commentfile}
	@echo "# will be named as 'cvlab' and if this is a library its name will be"            >> ${commentfile}
	@echo "# 'libcvlab.a'"                                                                  >> ${commentfile}
	@echo "packagename := ${packagename}"                                                   >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "version     := ${version}"                                                       >> ${commentfile}
	@echo "author      := ${author}"                                                        >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# you can write a short description here about the package"                      >> ${commentfile}
	@echo "description := ${description}"                                                   >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# i'm for gpl but you can edit it yourself"                                      >> ${commentfile}
	@echo "licence     := ${licence}"                                                       >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# the external libraries and sources are managed (included, linked)"             >> ${commentfile}
	@echo "# using the pkg-config program. if you don't have it, you cannot use"            >> ${commentfile}
	@echo "# this template to include/link libraries. get it from"                          >> ${commentfile}
	@echo "# http://pkg-config.freedesktop.org/wiki/"                                       >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo '# external sources: uses pkg-config as pkg-config --cflags $${external_sources}' >> ${commentfile}
	@echo "# if you don't need any source, set it to 'none'"                                >> ${commentfile}
	@echo "external_sources := ${external_sources}"                                         >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo '# external sources: uses pkg-config as "pkg-config --cflags $${external_libraries}"' >> ${commentfile}
	@echo '# for CXXFLAGS and pkg-config --libs $${external_libraries} for library inclusions' >> ${commentfile}
	@echo '# if you do not need any external library, set it to "none".'                    >> ${commentfile}
	@echo "# the order is important for linking. write the name of the package that depends" >> ${commentfile}
	@echo "# on another package first."                                                     >> ${commentfile}
	@echo "# external_libraries := none"                                                    >> ${commentfile}
	@echo "external_libraries := ${external_libraries}"                                     >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# fortran to c conversion ? I need this for Lapack - ATLAS library"              >> ${commentfile}
	@echo "# stuff (also lpp above)"                                                        >> ${commentfile}
	@echo "f77 := ${f77}"                                                                   >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# if optimized -> no debug info is produced --> applies -O3 flag if"             >> ${commentfile}
	@echo "# set to true"                                                                   >> ${commentfile}
	@echo "# optimize := true/false"                                                        >> ${commentfile}
	@echo "optimize    := ${optimize}"                                                      >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# this is for laptops and stuff with intel pentium M processors, if"             >> ${commentfile}
	@echo "# you are not sure of your system, just set 'specialize' to false. if"           >> ${commentfile}
	@echo "# it is a different one look for the -march option param of gcc and"             >> ${commentfile}
	@echo "# write your platforms name optimize for pentium4 ? / disabled if"               >> ${commentfile}
	@echo "# optimize is false"                                                             >> ${commentfile}
	@echo "specialize  := ${specialize}"                                                    >> ${commentfile}
	@echo "platform    := ${platform}"                                                      >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# do you want openmp support ? if you've never heard of it say 'false'"          >> ${commentfile}
	@echo "# parallelize := true/false"                                                     >> ${commentfile}
	@echo "parallelize := ${parallelize}"                                                   >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# pthread support"                                                               >> ${commentfile}
	@echo "multi-threading := ${multi-threading}"                                           >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# enable sse instruction sets ( sse sse2 )"                                      >> ${commentfile}
	@echo "sse := ${sse}"                                                                   >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# generate profiler data if true.  "                                             >> ${commentfile}
	@echo "#   ! set the optimize = false if you want annotation support.  "                >> ${commentfile}
	@echo "#  !! if you don't compile libraries with this flag, profiler won't be "         >> ${commentfile}
	@echo "#      able to make measurements for those libraries.  "                         >> ${commentfile}
	@echo "# !!! after running your program, you can see the results with"                  >> ${commentfile}
	@echo "#      'make gflat' and 'make gcall'"                                            >> ${commentfile}
	@echo "profile := ${profile}"                                                           >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# do not change for linux /usr type directory structures. this structure means"  >> ${commentfile}
	@echo '# .cpp/cc files reside in ./srcdir and .h files reside in ./include/$$(packagename)/' >> ${commentfile}
	@echo "# if you are building a library, the lib$(packagename).a will be in ./lib file." >> ${commentfile}
	@echo "# libdir      := lib"                                                            >> ${commentfile}
	@echo "# srcdir      := src"                                                            >> ${commentfile}
	@echo "# includedir  := include"                                                        >> ${commentfile}
	@echo "# If you'd like to have everything in the main directory"                        >> ${commentfile}
	@echo "libdir      := ${libdir}"                                                        >> ${commentfile}
	@echo "srcdir      := ${srcdir}"                                                        >> ${commentfile}
	@echo "includedir  := ${includedir}"                                                    >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# what to compile ? include .cpp and .c files here in your project"              >> ${commentfile}
	@echo "# if you don't have a main() function in one of the sources, you'll get an error" >> ${commentfile}
	@echo "# if you're building an executable. for a library, it won't complain for anything." >> ${commentfile}
	@echo "sources     := ${sources}"                                                       >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "################################################################################">> ${commentfile}
	@echo "####################### LOAD PRESET SETTINGS ###################################">> ${commentfile}
	@echo "################################################################################">> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# these are the magic files that this interface depends."                        >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# some temp operations."                                                         >> ${commentfile}
	@echo "include ${MAKEFILE_HEAVEN}/static-variables.makefile"                            >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# flag settings for gcc like CXXFLAGS, LDFLAGS...  to see the active"            >> ${commentfile}
	@echo "# flag definitions, issue 'make flags' command"                                  >> ${commentfile}
	@echo "include ${MAKEFILE_HEAVEN}/flags.makefile"                                       >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "# rules are defined here. to see a list of the available rules, issue 'make rules'" >> ${commentfile}
	@echo                                                                                   >> ${commentfile}
	@echo "include $(MAKEFILE_HEAVEN)/rules.makefile"                                       >> ${commentfile}
	@echo                                                                                   >> ${commentfile}

