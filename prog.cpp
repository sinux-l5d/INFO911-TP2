#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>
using namespace cv;

Mat convolution(Mat input, Mat F)
{
  Mat output = Mat::zeros(input.size(), input.type());

  // Convolution input * F
  for (int i = 1; i < input.rows - 1; i++)
  {
    for (int j = 1; j < input.cols - 1; j++)
    {

      // application du filtre F au pixel (i,j)
      float sum = 0.0;
      for (int k = -1; k <= 1; k++)
      {
        for (int l = -1; l <= 1; l++)
        {
          sum += input.at<uchar>(i + k, j + l) * F.at<float>(k + 1, l + 1);
        }
      }
      output.at<uchar>(i, j) = sum;
    }
  }

  return output;
}

Mat filtreM(Mat input)
{
  /*
  1/16 2/16 1/16
  2/16 4/16 2/16
  1/16 2/16 1/16
  */
  Mat filter = (Mat_<float>(3, 3) << 1, 2, 1,
                2, 4, 2,
                1, 2, 1);
  return convolution(input, filter / 16.0);
}

int main(int argc, char *argv[])
{
  namedWindow("Youpi");        // crée une fenêtre
  Mat input = imread(argv[1]); // lit l'image donnée en paramètre
  if (input.channels() == 3)
    cv::cvtColor(input, input, COLOR_BGR2GRAY);
  Mat inputOrigine = input.clone();
  while (true)
  {
    int keycode = waitKey(50);
    int asciicode = keycode & 0xff;
    if (asciicode == 'q')
      break;
    switch (asciicode)
    {
    case 'a':
      input = filtreM(input);
      break;
    case 'm':
      // filtre median
      medianBlur(input, input, 3);
      break;
    case 'r':
      input = inputOrigine.clone();
      break;
    default:
      break;
    }
    imshow("Youpi", input); // l'affiche dans la fenêtre
  }
}
