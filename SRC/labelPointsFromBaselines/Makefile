all: labelPointsFromBaselines
OPEN_LIBS=-lopencv_imgproc -lopencv_core -lopencv_highgui -lpugixml
labelPointsFromBaselines: labelPointsFromBaselines.cc
	g++ -o  labelPointsFromBaselines labelPointsFromBaselines.cc ${OPEN_LIBS} -O3

clean:
	rm labelPointsFromBaselines
