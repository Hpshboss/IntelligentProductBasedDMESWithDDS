#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <typeinfo>
#include <vector>

using namespace cv;

int main(int argc, char* argv[]) {
    if ( argc != 2 ) {
        printf("usage: DisplayImage.out <Image_Path>n");
        return -1;
    }

    Mat image;
    image = imread( argv[1], 1 );

    if ( !image.data ) {
        printf("No image data n");
        return -1;
    }

    std::cout << "Rows: " << image.rows << std::endl;
    std::cout << "Cols: " << image.cols << std::endl;
    
    std::vector<uchar> buffer;
    imencode(".jpg", image, buffer);

    Mat imageDecoded = imdecode(buffer, 1);

    imwrite("droneNew.jpg", imageDecoded);

    return 0;
}