echo clean
pause
del *.dll *.exe *.obj *.lib *.exp
echo make dll
pause
cl -LD libeuseq.cpp
echo make exe
pause
cl /EHsc main.cpp /link libeuseq.lib
echo run exe
pause
main
echo done
pause
