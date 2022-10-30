@echo off
gawk -f _gen_blit.awk _gen_blit.awk >statusbar_blit_code.i
pause
