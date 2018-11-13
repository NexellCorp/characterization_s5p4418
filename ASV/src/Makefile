######################################################################
SUB_DIRS	:=
SUB_DIRS	+= cache_test_app
SUB_DIRS	+= cache_drv
SUB_DIRS	+= main

all: compile_subdirs 

compile_subdirs:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir || exit $?;	\
	done

clean:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir clean;		\
	done
