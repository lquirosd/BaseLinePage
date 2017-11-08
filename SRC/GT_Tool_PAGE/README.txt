Project works with QT4.
Some problems have been detected if you have installed both qt3 and qt4

To compile the project must do:

qmake -project
Add "QT+=opengl" to the .pro
qmake
make
