EXTRA_DIST = \
	ALL.inc \
	COLCODE.inc \
	IMGFUN.inc

noinst_LIBRARIES = libimgfun_x86.a
libimgfun_x86_a_SOURCES = \
	CRC.asm \
	IB.asm \
	IB2.asm \
	IB_32.asm \
	IB_A.asm \
	IB_AR.asm \
	IB_AT.asm \
	IB_ATD.asm \
	IB_ATDM.asm \
	IB_ATM.asm \
	IB_ATR.asm \
	IB_ATRD.asm \
	IB_ATRDM.asm \
	IB_DW.asm \
	IB_R.asm \
	IB_RD.asm \
	IB_T.asm \
	IB_TD.asm \
	IB_TDM.asm \
	IB_TM1.asm \
	IB_TM2.asm \
	IB_TM3.asm \
	IB_TR.asm \
	IB_TRD.asm \
	IB_TRDM.asm \
	IC.asm \
	IC_R.asm \
	IJ_T.asm \
	IR.asm \
	IR_A.asm \
	IR_AM.asm \
	IR_BAR.asm \
	IR_M.asm \
	I_BAR.asm \
	I_BLACK.asm \
	I_CTRL.asm \
	I_EMASK.asm \
	I_EREMAP.asm \
	I_FONT.asm \
	I_FREMAP.asm \
	I_LINE.asm \
	I_PIXEL.asm \
	I_READ.asm \
	I_SNOW.asm \
	I_XOR.asm

SUFFIXES = .asm
.asm.o:
	$(JWASM_CMD) $(JWASM_FMT) -q -Fo $@ $<
