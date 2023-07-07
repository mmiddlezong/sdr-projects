#include <fstream>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main()
{
    const string fileName = "image.jpg";

    Mat image = imread(fileName, IMREAD_COLOR);
    if (image.empty())
    {
        cerr << "Error loading image.\n";
        return -1;
    }

    vector<pair<Vec3b, int>> rleData;
    Vec3b curPixel = image.at<Vec3b>(0, 0);
    short runLength = 1;

    for (int i = 0; i < image.rows; ++i)
    {
        for (int j = 0; j < image.cols; ++j)
        {
            if (i == 0 && j == 0)
                continue; // Skip the first pixel
            Vec3b pixel = image.at<Vec3b>(i, j);
            if (pixel == curPixel)
            {
                runLength++;
            }
            else
            {
                rleData.push_back(make_pair(curPixel, runLength));
                curPixel = pixel;
                runLength = 1;
            }
        }
    }

    rleData.push_back(make_pair(curPixel, runLength));

    ofstream outfile("encoded-data.rle", ios::binary);
    for (const auto &pair : rleData)
    {
        Vec3b pixel = pair.first;
        short runLength = pair.second;
        cout << pixel << " " << runLength << endl;
        outfile.write(reinterpret_cast<const char *>(&pixel), sizeof(Vec3b));
        outfile.write(reinterpret_cast<const char *>(&runLength), sizeof(short));
    }
    outfile.close();

    // Decoding
    vector<pair<Vec3b, int>> decoded_rle_data;
    ifstream infile("encoded-data.rle", ios::binary);

    while (infile.peek() != EOF)
    {
        Vec3b pixel;
        short run_length;
        infile.read(reinterpret_cast<char *>(&pixel), sizeof(Vec3b));
        infile.read(reinterpret_cast<char *>(&run_length), sizeof(short));
        decoded_rle_data.push_back(make_pair(pixel, run_length));
    }
    infile.close();

    Mat decoded_image(image.rows, image.cols, CV_8UC3); // Assuming it is a 3-channel image
    int position = 0;
    for (const auto &pair : decoded_rle_data)
    {
        Vec3b pixel = pair.first;
        int run_length = pair.second;
        for (int i = 0; i < run_length; ++i)
        {
            int row = position / decoded_image.cols;
            int col = position % decoded_image.cols;
            decoded_image.at<Vec3b>(row, col) = pixel;
            position++;
        }
    }
    imwrite("decoded_image.jpg", decoded_image);


    return 0;
}
