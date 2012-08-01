#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# SAMSUNG SMDK2460 board with S3C2460A (ARM926EJ-S) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

# If you don't use MMU, TEXT_BASE should be modified
# 496PKG on SMDK2460 		: TEXT_BASE = 0x21e00000
# 416PKG and mSDRM on SMDK2460	: TEXT_BASE = 0x31e00000
# 416PKG and mDDR on SMDK2460	: TEXT_BASE = 0x23e00000
#


ifndef TEXT_BASE
TEXT_BASE = 0xc3e00000
endif

