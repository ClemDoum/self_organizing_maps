#include "Base.h"
#include <math.h>
// g++ -std=c++11 -o k-means -Wall -ansi -O3 bin/kohonen.cpp
// bin/kohonen train 0.08 1.15 1 10000 10
// bin/kohonen train 0.075 1.17 1 10000 20
// ffmpeg -i ko_prototypes/kohonen-%06d.ppm kohonen.mpeg -y 

// We store prototype images in a grid
#define WIDTH  12
#define HEIGHT 12
#define LIMIT 50
#define SIZE 255.0
#define IMG_FOLDER "ko_prototypes/kohonen"
#define PROTO_FILE_NAME "prototype.txt"
typedef uci::Map<WIDTH,HEIGHT,
uci::Database::imagette::width,
uci::Database::imagette::height> Prototypes;

// In a image, indexes are defined as follow
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


// Set a prototype to an image from the DB
void setProto(Prototypes::imagette& w,const uci::Database::imagette& xi) {
	for(int i = 0 ; i < uci::Database::imagette::height ; ++i) {
		for(int j = 0 ; j < uci::Database::imagette::width ; ++j) {
			w(i,j) = (double)(xi(i,j));
		}
	}
}


// Randomly initializes a prototype
void initProto(Prototypes::imagette& w) {
	for(int i = 0 ; i < uci::Database::imagette::height ; ++i) {
		for(int j = 0 ; j < uci::Database::imagette::width ; ++j) {
			w(i,j) = (double)(rand()/(double)RAND_MAX)*SIZE;
		}
	}
}

// Compute the 
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
				r < uci::Database::imagette::height &&
				0 < c &&
				c < uci::Database::imagette::width)
				m += (double)im(r,c);
		}
	}
	return (m/area);
}


// Gaussian mask for prototypes
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
	double d_max = uci::Database::imagette::width*uci::Database::imagette::height*SIZE*SIZE;

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

// Compute the Gaussian function
double gaussian(const double& gamma, const double& d){
	return exp(-(d*d)/gamma);
}

// Compute the distance on the graph
double winningRate(const int& i_winner, const int& j_winner, const int& i, const int& j) {
	double di = ((i_winner-i));
	double dj = ((j_winner-j));
	return sqrt(di*di + dj*dj);
}

// Move the prototype
void learnProto(const double& gamma, Prototypes::imagette& im_proto,
	const uci::Database::imagette& im_db,
	const double &alpha,
	const int& i_winner,
	const int& j_winner,
	const int& i_proto,
	const int& j_proto){
	double h = gaussian(gamma, winningRate(i_winner, j_winner, i_proto, j_proto));
	for(int row = 0 ; row < uci::Database::imagette::height ; ++row) {
		for(int col = 0 ; col < uci::Database::imagette::width ; ++col) {
			im_proto(row, col) += alpha*
			h*
			((double)im_db(row, col)-im_proto(row, col));
			im_proto(row, col) = std::min(im_proto(row, col), SIZE);
			im_proto(row, col) = std::max(im_proto(row, col), 0.0);
		}
	}
}

// Save the image if it corresponds to a sample image
void saveImage(Prototypes& protos, const int& rate, const int& i) {
	if (i%rate == 0){
		protos.PPM(IMG_FOLDER, floor(i/rate));
	}
}


// Computes the alpha parameter depending on the iteration
double computeAlpha(const int& max_iter, const double& alpha_min, const int& i){
	int a = floor(1*max_iter/12);
	int b = floor(3*max_iter/12);
	int c = floor(6*max_iter/12);
	int d = floor(10*max_iter/12);
	if (i < a)
		return 0.9;
	else if (i < b)
		return 0.4;
	else if (i < c)
		return 0.3;
	else if (i < d)
		return 0.18;
	else
		return alpha_min;
}


// Check the user inputs
bool getUserInput(const int& argc, char* argv[], std::string& mode, double& alpha_min, double& gamma, int& radius, int& max_iter, int& rate, int& test_db_size){
	try {
		if (argc != 3 && argc != 7)
			throw std::runtime_error("Wrong number of arguments");
		// Program mode
		mode = argv[1]; 

		if (mode.compare("train") == 0){
			// Algo parameters
			alpha_min = atof(argv[2]);
			gamma = atof(argv[3]);
			radius = atoi(argv[4]);
			// Maximum iteration
			max_iter = atoi(argv[5]);
			// Image save every rate
			rate = atoi(argv[6]);
		}
		else if (mode.compare("predict") == 0){
			test_db_size = atoi(argv[2]);
		}
		else
			std::runtime_error("You should either predict or train.");
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl << std::endl;
		std::cout << "Please enter the parameter as describe below:" << std::endl<< std::endl;
		std::cout << "To train: bin/kohonen train gamma radius iteration image_rate" << std::endl;						
		std::cout << "For instance: bin/kohonen train 0.075 1.17 1 10000 20" << std::endl << std::endl;						
		std::cout << "To predict: bin/kohonen predict" << std::endl;						
		std::cout << "For instance: bin/kohonen predict 100" << std::endl;										

		return false;
	}
	return true;
}


// Save the prototypes in image files
void saveProtos(const Prototypes& protos){
	std::ofstream file;

	file.open(PROTO_FILE_NAME);
	if(!file) {
		std::cerr << "Error : saveProtos : "
		<< "Je ne peux pas ouvrir \"" 
		<< PROTO_FILE_NAME
		<< "\". Je quitte."
		<< std::endl;
		::exit(1);
	}

	// On each row of the file we put a proto data
	for (int row = 0; row < HEIGHT; ++row){
		for (int col = 0; col < WIDTH; ++col){
			file << 99 << ' '
			<< row << ' '
			<< col;
			for(int i = 0 ; i < uci::Database::imagette::height ; ++i) {
				for(int j = 0 ; j < uci::Database::imagette::width ; ++j) {
					file << ' ' << protos(row, col)(i, j);
				}
			}
			file << std::endl;
		}
	}

	file.close();
}

// Reads the prototypes from the protofile
Prototypes readProto(){
	std::ifstream file;
	Prototypes p;

	file.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
	try {
		int r, c, l;
		file.open(PROTO_FILE_NAME);
		while (true){
			file >> l >> r >> c;
			for(int i = 0 ; i < uci::Database::imagette::height ; ++i) {
				for(int j = 0 ; j < uci::Database::imagette::width ; ++j) {
					file >> p(r, c)(i, j);
				}
			}
		}
	}
	catch(const std::exception& e) {
		std::cout << "Erreur: " << e.what() << std::endl;
	}
	return p;
}

// Finds the lable of a deternined prototype
int findProtoLabel(const int& row, const int& col){
	std::ifstream file;
	int res;

	file.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
	try {
		int r, c, l;
		double d;
		file.open(PROTO_FILE_NAME);
		while (true){
			file >> l >> r >> c;
			if ( r == row && c == col)
				res = l;
			for(int i = 0 ; i < uci::Database::imagette::height ; ++i) {
				for(int j = 0 ; j < uci::Database::imagette::width ; ++j) {
					file >> d;
				}
			}
		}
	}
	catch(const std::exception& e) {
		std::cout << "Erreur: " << e.what() << std::endl;
	}
	return res;
}


int main(int argc, char* argv[]) {
	// Program mode
	std::string mode;
	// Algo parameters
	double alpha_min;
	double gamma;
	int radius;
	// Maximal number of iterations
	int max_iter;
	// Image rate
	int rate;
	// Test set size
	int test_db_size;

	// Cheking the user input
	if (getUserInput(argc, argv, mode, alpha_min, gamma, radius, max_iter, rate, test_db_size)){
		uci::Database database;
		srand(time(NULL));

		if (mode.compare("train") == 0){
			// Initialize prototypes
			Prototypes prototypes;
			for (int row = 0; row < HEIGHT; ++row){
				for (int col = 0; col < WIDTH; ++col){
					initProto(prototypes(row, col));
				}
			}

			// Iterate on the examples
			int r, c;
			int i = 0;
			while (i < max_iter){
				// Random DB image
				int a = floor((rand()/(double)RAND_MAX)*LIMIT);
				for (int j=0; j<a; ++j)
					database.Next();
				// Compute the winner proto
				winnerProto(radius, prototypes, database.input, r, c);
				// Move the protos
				for (int row = 0; row < HEIGHT; ++row){
					for (int col = 0; col < WIDTH; ++col){
						learnProto(gamma, prototypes(row, col), database.input,
							computeAlpha(max_iter, alpha_min, i), r, c, row, col);
					}
				}
				// Print the new map
				saveImage(prototypes, rate, i);
				++ i;
			}
			// Save the protos to a determined format
			saveProtos(prototypes);
		}

		// Predict the labels
		if (mode.compare("predict") == 0) {
		// double risk = 0;
		// int i = 0;
		// while (i < test_db_size){
		// 		// Random DB image
		// 	int a = floor((rand()/(double)RAND_MAX)*LIMIT);
		// 	for (int j=0; j<a; ++j)
		// 		database.Next();
		// 		// Choose the winner
		// 		// Compute the error
		// 	uci::Database::imagette& image = database.input;
		// 		// Read the labelized protos
		// 	Prototypes saved_protos = readProto();
		// 		// Find the winner
		// 	int row, col;
		// 	winnerProto(radius, saved_protos, image, row, col);
		// 		// Check if the label was right
		// 	if (findProtoLabel(row, col) != database.what){
		// 		++risk;
		// 	}
		// }
		// risk = risk/(double)test_db_size;
		// std::cout << "Empirical Risk: " << risk << std::endl;
			Prototypes saved_protos = readProto();
			// saved_protos.PPM("test_result", 0);
		}
	}

	return 0;
}