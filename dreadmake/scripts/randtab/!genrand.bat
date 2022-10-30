@echo off
gawk -f _genrand.awk _genrand.awk >gen_rand.inc
pause
