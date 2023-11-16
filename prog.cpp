#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>
using namespace cv;

Mat convolution(Mat input, Mat F, float delta = 0.0)
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
          sum += input.at<float>(i + k, j + l) * F.at<float>(k + 1, l + 1);
        }
      }
      output.at<float>(i, j) = sum + delta;
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

Mat filtreRehausseur(Mat input, float coef)
{
  /*
  Laplacien
  0 1 0
  1 -4 1
  0 1 0
  */
  Mat filter = coef *
               (Mat_<float>(3, 3) << 0, 1, 0,
                1, -4, 1,
                0, 1, 0);
  return input - convolution(input, filter);
}

Mat const SobelX = (Mat_<float>(3, 3) << -1, 0, 1,
                    -2, 0, 2,
                    -1, 0, 1) /
                   4.0;
Mat const SobelY = (Mat_<float>(3, 3) << -1, -2, -1,
                    0, 0, 0,
                    1, 2, 1) /
                   4.0;
Mat sobel(Mat input, bool isX)
{
  // filtre matrice Sobel dérivée en x
  Mat filter = SobelX;
  if (!isX)
    filter = SobelY;

  return convolution(input, filter, 128.0);
}

Mat gradiant(Mat input)
{
  // using SobelX and SobelY
  Mat sobelX = convolution(input, SobelX);
  Mat sobelY = convolution(input, SobelY);

  Mat output = Mat::zeros(input.size(), input.type());
  for (int i = 0; i < input.rows; i++)
  {
    for (int j = 0; j < input.cols; j++)
    {
      output.at<float>(i, j) = sqrt(pow(sobelX.at<float>(i, j), 2) + pow(sobelY.at<float>(i, j), 2));
    }
  }

  return output;
}

int main(int argc, char *argv[])
{
  namedWindow("Filter");       // crée une fenêtre
  Mat input = imread(argv[1]); // lit l'image donnée en paramètre
  if (input.channels() == 3)
    cv::cvtColor(input, input, COLOR_BGR2GRAY);
  Mat inputOrigine = input.clone();
  Mat inputR;
  input.convertTo(inputR, CV_32F);

  float alpha = 60.0;
  createTrackbar("alpha (en %)", "Filter", nullptr, 100, NULL);
  setTrackbarPos("alpha (en %)", "Filter", alpha);

  while (true)
  {
    int keycode = waitKey(50);
    int asciicode = keycode & 0xff;
    if (asciicode == 'q')
      break;
    input.convertTo(inputR, CV_32F);
    switch (asciicode)
    {
    case 'a': // filtre moyenneur (average)
      inputR = filtreM(inputR);
      break;
    case 'm': // filtre median
      medianBlur(inputR, inputR, 3);
      break;
    case 'r': // reset
      inputR = inputOrigine.clone();
      break;
    case 's': // filtre rehausseur
      alpha = getTrackbarPos("alpha (en %)", "Filter");
      inputR = filtreRehausseur(inputR, alpha / 100.0);
      break;
    case 'x':
      inputR = sobel(inputR, true);
      break;
    case 'y':
      inputR = sobel(inputR, false);
      break;
    case 'g':
      inputR = gradiant(inputR);
      break;
    default:
      break;
    }
    inputR.convertTo(input, CV_8U);
    imshow("Filter", input); // l'affiche dans la fenêtre
  }
}
