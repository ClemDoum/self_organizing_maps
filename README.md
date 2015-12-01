# C++ implementation of handwritten digits vector quantization

The Base.h was not implemented by myself, it was written by my teacher 
[Hervé Frezza](http://www.metz.supelec.fr/metz/personnel/frezza/).

I only focused on the algorithm part.

## Dependencies

[ffmpeg](https://www.ffmpeg.org/) needs to be installed

### On MacOS X

    brew install ffmpeg

### On Ubuntu
    
    apt-get install ffmpeg


## A. Online K-means

Classical implementation of the k-means clustering algorithm.

It was mainly implemented so that it can be used as a base for the Kohonen 
Self Organizing Maps algorithm.

### Train

Compile it like this:

    make k-means
    
Run it like this:

    bin/k-means 0.2 1 2000 2

The user inputs are not handled nicely so don't forget any parameter:

* 1st argument: learning rate for the winner prototype

* 2nd argument: radius used for the gaussian mask.
    
    A gaussian mask is used to smooth the cartesian distance used to compute 
    the distance between images

* 3rd argument: number of learning iterations

* 4th argument: sample rate for the map image saving. In the above example we 
record an image every 2 iterations.


### Visualize

Once the algorithm has run you can create a video of the training like this:
    
    ffmpeg -i km_prototypes/kmeans-%06d.ppm kmeans.mpeg -y

or:

    movie_km



## B. Kohonen Self Organizing Maps

More advanced vector quantization algorithm: http://en.wikipedia.org/wiki/Self-organizing_map

Comparing the videos, we can see that it outperforms k-means.
Competitive learning leads to a better convergence.
Prototypes are organizing themselves on the grid.

### Train

Compile the code:

    make kohonen
    
Train the algorithm:

    bin/kohonen train gamma radius iterations image_rate
    
For instance run:
    
    bin/kohonen train 0.075 1.17 1 10000 20


Parameters description:

* 1st param: algorithm functioning mode
    You have to use "train".
    I wanted to use the map to achieve classification with it so maybe in the 
    future I will add a "predict" mode

* 2nd param: gamma

    Gamma parameter used to compute the distance between 2 prototypes in the grid.

    It must be understood that in the algorithm we use 2 different distances: 
    the distance between 2 images in terms of pixels 
    (cartisian distance + gaussian mask) and the between two prototypes on the 
    organizing map.
    
    Gamma is used to compute this second distance.

* 3rd param: radius
It's the radius used in the gaussian mask before computing the cartesian distance

* 4th param: number of iteration
I recommend using 10000 iterations

* 5th param: sample saving rate for images
For instance if you use 2 we record an image every 2 iterations.


### Visualize

Once the algorithm has run you can create a video of the training like this:

    ffmpeg -i ko_prototypes/kohonen-%06d.ppm kohonen.mpeg -y

or

    make movie_ko

