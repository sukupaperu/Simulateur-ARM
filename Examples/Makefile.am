CC=$(GCC_CROSS)
CCAS=$(GCC_CROSS)
AM_CFLAGS=-mbig-endian
AM_CCASFLAGS=-mbig-endian
AM_LDFLAGS=-nostdlib -T $(srcdir)/linker_script -n --entry main
AM_LDFLAGS+=-Wl,-EB

if HAVE_ARM_COMPILER
noinst_PROGRAMS=example1 example2 example3 example4 insertion_sort ex_ADC ex_AND ex_EOR ex_MRS ex_RSB ex_RSC ex_SBC ex_SUB ex_MVN ex_ORR ex_BIC ex_TST ex_TEQ ex_CMP ex_CMN

all-am: $(PROGS)
endif

example1_SOURCES=example1.s
example2_SOURCES=example2.s
example3_SOURCES=example3.s
example4_SOURCES=example4.s
insertion_sort_SOURCES=insertion_sort.c
ex_ADC_SOURCES=ex_ADC.s
ex_AND_SOURCES=ex_AND.s
ex_EOR_SOURCES=ex_EOR.s
ex_MRS_SOURCES=ex_MRS.s
ex_RSB_SOURCES=ex_RSB.s
ex_RSC_SOURCES=ex_RSC.s
ex_SBC_SOURCES=ex_SBC.s
ex_SUB_SOURCES=ex_SUB.s
ex_MVN_SOURCES=ex_MVN.s
ex_ORR_SOURCES=ex_ORR.s
ex_BIC_SOURCES=ex_BIC.s
ex_TST_SOURCES=ex_TST.s
ex_TEQ_SOURCES=ex_TEQ.s
ex_CMP_SOURCES=ex_CMP.s
ex_CMN_SOURCES=ex_CMN.s

EXTRA_DIST=linker_script

clean-local:
	$(RM) $(PROGS)
