#
# (C) Copyright 2006
# Seung-Chull, Suh, Samsung Electronics, <sc.suh@samsung.com>
#
# SAMSUNG SMDK2440 board with S3C2440A (ARM920T) cpu
# SMDK2442 with S3C2442 uses same code of SMDK2440
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

#
# SMDK2440 has 1 bank of 64 MB DRAM
# 3000'0000 to 3400'0000 : TEXT_BASE=0x33e00000
#
# SMDK2442 MSP1 has 2 bank of 64 MB DRAM
# 3000'0000 to 3400'0000 : TEXT_BASE=0x33e00000
#
# SMDK2442 MSP3 has 2 bank of 96 MB DRAM (64+32)
# 3000'0000 to 3600'0000 : TEXT_BASE=0x35e00000
#
# Linux-Kernel is expected to be at 3000'8000, entry 3000'8000
# optionally with a ramdisk at 3080'0000
#
# we load ourself to relevant TEXT_BASE
#
# download area is 3000'0000
#
# when we use MMU 0x3000'0000 -> 0xc000'0000

TEXT_BASE = 0xc3e00000
