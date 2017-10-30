
OPEN_LIBS=-lopencv_imgproc -lopencv_core  -lopencv_highgui
OTHER_LIBS=-lpugixml

all: 
	mkdir ../BIN || echo "../BIN directory already exists"
	make -C clusterLinies;
	make -C imageLocalExtrema 
	make -C Extremely_Randomized_Trees
	make -C labelPointsFromBaselines
	g++ -o pointsToClassifierData pointsToClassifierData.cc  ${OPEN_LIBS} -O3
	g++ -o drawBaselines drawBaselines.cc  ${OPEN_LIBS} ${OTHER_LIBS} -O3
	cp clusterLinies/mdbscan imageLocalExtrema/imageLocalExtrema Extremely_Randomized_Trees/ext_rand_trees labelPointsFromBaselines/labelPointsFromBaselines pointsToClassifierData drawBaselines  TranskribusBaseLineEvaluationScheme-0.1.3-jar-with-dependencies.jar ../BIN

clean:
	rm pointsToClassifierData
	rm drawBaselines
	make -C clusterLinies clean;
	make -C imageLocalExtrema clean;
	make -C Extremely_Randomized_Trees clean
	make -C labelPointsFromBaselines clean
