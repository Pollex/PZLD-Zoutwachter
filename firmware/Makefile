####
#### Sample Makefile for building applications with the RIOT OS
####
#### The example file system layout is:
#### ./application Makefile
#### ../../RIOT
####

# Set the name of your application:
APPLICATION = elektrode-switch
COMPILE_COMMANDS_PATH = $(CURDIR)/compile_commands.json

EXTERNAL_BOARD_DIRS = $(CURDIR)/boards
BOARD ?= zoutwachter-proto

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../RIOT

QUIET ?= 1

# Modules to include:
EXTERNAL_MODULE_DIRS += modules

USEMODULE += ztimer ztimer_msec
USEMODULE += pcal6524 stws281x
USEMODULE += shell

#INCLUDES += -Iapplication_include

ifeq ($(FAT_TEST),1)
CFLAGS += -DAPP_DO_FAT=1
endif

include $(RIOTBASE)/Makefile.include
