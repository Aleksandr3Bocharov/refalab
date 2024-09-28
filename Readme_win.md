README для сборки РефалАБ в Windows 
--------------------------------------

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

Для запуска тестовых примеров войдите в X\RefalAB и используйте скрипт RefalABclg.bat,

	RefalABclg tests\{name}

где {name} := hello | fact | test0..test5 .
 
Для запуска примера "fact" (факториал n) вы должны ввести целое число n.

Введите 0 или пустую строку для выхода. 

Используйте RefalABdclg.bat для запуска отладчика.
