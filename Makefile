k-means:
	g++ -std=c++11 -o bin/k-means -Wall -ansi -O3 k-means.cpp

kohonen:
	g++ -std=c++11 -o bin/kohonen -Wall -ansi -O3 kohonen.cpp

movie_km:
	ffmpeg -i km_prototypes/kmeans-%06d.ppm -y kmeans.mpeg

movie_ko:
	ffmpeg -i ko_prototypes/kmeans-%06d.ppm -y kohonen.mpeg

clean:
	rm k-means kohonen