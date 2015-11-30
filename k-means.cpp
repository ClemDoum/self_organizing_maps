#include "Base.h"
#include <math.h>
// g++ -std=c++11 -o k-means -Wall -ansi -O3 bin/k-means.cpp
// ffmpeg -i km_prototypes/kmeans-%06d.ppm kmeans.mpeg -y 
// On va stocker les imagettes-prototypes au sein d'une grille.
#define WIDTH  8
#define HEIGHT 8
#define LIMIT 50
#define IMG_FOLDER "km_prototypes/kmeans"
typedef uci::Map<WIDTH,HEIGHT,
uci::Database::imagette::width,
uci::Database::imagette::height> Prototypes;

// Pour une imagette, les indices sont definis comme suit
//
//         j
//         |
//   ......|.....
//   ......|.....
//   ......|.....
//   ......|.....
//   ......|.....
//   ......#------ i   img(i,j)
//   ............
//   ............
//   ............
//   ............
//   ............  


// Cette fonction permet d'affecter un prototype (dont les pixels sont
// des double dans [0,255]) a une imagette tiree de la base (dont les
// pixels sont des unsigned char). Le & evite les copies inutiles.
void initProto(Prototypes::imagette& w,const uci::Database::imagette& xi) {
	for(int i = 0 ; i < uci::Database::imagette::height ; ++i) {
		for(int j = 0 ; j < uci::Database::imagette::width ; ++j) {
			w(i,j) = (double)(xi(i,j));
		}
	}
}

// Compute the mean
template<typename ImageType>
double computePixelMean(const int& radius,
	const ImageType& im,
	const int& i,
	const int& j){
	double m = 0.0;
	int rmin = i-radius;
	int rmax = i+radius;
	int cmin = j-radius;
	int cmax = j+radius;
	int area = (2*radius+1)*(2*radius+1);
	for(int r = rmin ; r <= rmax ; ++r) {
		for(int c = cmin ; c <= cmax ; ++c) {
			if ( 0 < r &&
				r < ImageType::height &&
				0 < c &&
				c < ImageType::width)
				m += (double)im(r,c);
		}
	}
	return (m/area);
}



// Gaussian mask
template<typename ImageType>
ImageType gaussianize(const int& radius, const ImageType& im){
	ImageType res;
	for(int i = 0 ; i < ImageType::height ; ++i) {
		for(int j = 0 ; j < ImageType::width ; ++j) {
			res(i,j) = computePixelMean(radius, im, i, j);
		}
	}
	return res;
}


// Compute the distance between an imagette to a proto
double distanceProto(const int& radius, const Prototypes::imagette& im_proto,
	const uci::Database::imagette& im_db) {
	double d = 0.0;
	Prototypes::imagette im_f_proto = gaussianize(radius, im_proto);
	uci::Database::imagette im_f_db = gaussianize(radius, im_db);

	for (int r = 0; r < uci::Database::imagette::height; ++r) {
		for (int c = 0; c <  uci::Database::imagette::width; ++c) {
			d += (im_f_db(r, c) - im_f_proto(r, c))*(im_f_db(r, c) - im_f_proto(r, c));
		}
	}
	return d;
}


// Find the closest prototype to an imagette
void winnerProto(const int& radius, const Prototypes& prototypes,
	const uci::Database::imagette& im, int &r, int &c) {
	// Find the closest proto
	double d_max = uci::Database::imagette::width*uci::Database::imagette::height*255*255;

	for (int row = 0; row < HEIGHT; ++row){
		for (int col = 0; col < WIDTH; ++col){
			int d = distanceProto(radius, prototypes(row, col), im);
			if (d < d_max){
				d_max = d;
				r = row;
				c = col;
			}
		}
	}
}

// Move the prototype
void learnProto(Prototypes::imagette& im_proto, const uci::Database::imagette& im_db, const double &alpha){
	for(int row = 0 ; row < uci::Database::imagette::height ; ++row) {
		for(int col = 0 ; col < uci::Database::imagette::width ; ++col) {
			im_proto(row, col) += alpha*((double)im_db(row, col)-im_proto(row, col));
			im_proto(row, col) = std::min(im_proto(row, col), 255.0);
			im_proto(row, col) = std::max(im_proto(row, col), 0.0);
		}
	}
}

// Save the image if it correspond
// to a sample image
void saveImage(Prototypes& protos, const int& rate, const int& i) {
	if (i%rate == 0){
		protos.PPM(IMG_FOLDER, floor(i/rate));
	}
}

int main(int argc, char* argv[]) {
	uci::Database database;

	double alpha = atof(argv[1]);
	int radius = atoi(argv[2]);
	int max_iter = atoi(argv[3]);
	// Image save every rate
	int rate = atoi(argv[4]);

	srand(time(NULL));
	// Initialize prototypes
	Prototypes prototypes;
	for (int row = 0; row < HEIGHT; ++row){
		for (int col = 0; col < WIDTH; ++col){
			// Random DB image
			int i = floor((rand()/(double)RAND_MAX)*LIMIT);
			for (int j=0; j<i; ++j)
				database.Next();
			// Initialize with this image
			initProto(prototypes(row, col), database.input);
		}
	}

	int r, c;
	int i = 0;
	while (i < max_iter) {
		// Random DB image
		int a = floor((rand()/(double)RAND_MAX)*LIMIT);
		for (int j=0; j<a; ++j)
			database.Next();
		winnerProto(radius, prototypes, database.input, r, c);
		learnProto(prototypes(r, c), database.input, alpha);
		saveImage(prototypes, rate, i);
		++ i;
	}

	return 0;
}