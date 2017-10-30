OPEN_LIBS=-lopencv_imgproc -lopencv_core -lopencv_highgui
PROGS=imageLocalExtrema
OPTIM=-O3
all:${PROGS}
imageLocalExtrema: imageLocalExtrema.cc 
	g++ -o imageLocalExtrema imageLocalExtrema.cc ${OPEN_LIBS}  ${OPTIM}
clean:
	rm  imageLocalExtrema
