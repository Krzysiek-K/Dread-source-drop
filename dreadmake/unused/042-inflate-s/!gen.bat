@echo off

cpp inflate.s inflate_preproc.s

gawk -f _label_fix.awk inflate_preproc.s >inflate_gen.s

pause
