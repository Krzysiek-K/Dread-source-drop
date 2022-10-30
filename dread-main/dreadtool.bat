@echo off


set TOOL=..\dread-main\dreadtool.exe

if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%
if not exist %TOOL% set TOOL=..\%TOOL%

start %TOOL%
