README для сборки РефалАБ в Windows 
-------------------------------------

[Mingw-W64](https://winlibs.com/) (UCRT Runtime gcc 14.2.0 или позже (with POSIX threads) Win32) требуется.

Распакуйте дистрибутив в любую директорию X.

	cd X\RefalAB

Переименуйте сборочный файл для Windows Makefile_win в Makefile.

	mingw32-make

The result is the refal-compiler bin\refal2 
and the refal-interpreter from intermidiate language - 'yazyk sborki' 
allocated as set of objects modules in the lib\librefal2.a library.


*** Начало работы ***
------------------------

To start test examples pass to X\refal2 und use script r2clg 

	r2clg tests\<name>

where <name>:= hello | fact | test0..test5
 
For start of "fact" example (factorial n) you must enter the integer value n 
(for example in 1..10000 range). Enter 0 for exit. 

Use r2dclg instead r2clg for start of debugger.
