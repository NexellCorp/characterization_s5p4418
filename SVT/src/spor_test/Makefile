######################################################################
SUB_DIRS	:=
SUB_DIRS	+= host
SUB_DIRS	+= target

all: compile_subdirs

compile_subdirs:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir || exit $?;	\
	done

clean:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir clean;		\
	done
