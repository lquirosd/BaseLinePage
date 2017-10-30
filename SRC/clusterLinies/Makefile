all: mdbscan
OPEN_LIBS=-lopencv_imgproc -lopencv_core -lopencv_highgui -lpugixml
mdbscan: mdbscan.cc  slopeClass.cc 
	g++ -o mdbscan mdbscan.cc  slopeClass.cc -I. -O3 ${OPEN_LIBS} 

clean:
	rm mdbscan


