#
#	E:\Home\UC3\Dateq\Makefile
#

PATH := $(PATH):C:\intelFPGA_pro\17.0\embedded\host_tools\mentor\gnu\arm\baremetal\bin

PRJ_PATH = E:\Home\cyclone

MAKE=C:\intelFPGA_pro\17.0\embedded\ds-5\bin\make.exe

all:
	$(MAKE) -f E:\Home\cyclone\Debug\Makefile all

clean:
	$(MAKE) -f E:\Home\cyclone\Debug\Makefile clean

verbose:
