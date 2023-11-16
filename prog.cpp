#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>
#include <iostream>
using namespace cv;

Mat convolution(Mat input, Mat F, float delta = 0.0)
{
  Mat output = Mat::zeros(input.size(), CV_32F);

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

Mat const Laplacien = (Mat_<float>(3, 3) << 0, 1, 0,
                       1, -4, 1,
                       0, 1, 0);
Mat filtreRehausseur(Mat input, float coef)
{
  return input - convolution(input, coef * Laplacien);
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

  Mat output = Mat::zeros(input.size(), CV_32F);
  for (int i = 0; i < input.rows; i++)
  {
    for (int j = 0; j < input.cols; j++)
    {
      output.at<float>(i, j) = sqrt(pow(sobelX.at<float>(i, j), 2) + pow(sobelY.at<float>(i, j), 2));
    }
  }

  return output;
}

Mat contours(Mat input, double seuil)
{
  //  contours à la Marr-Hildreth
  Mat output = Mat(input.size(), CV_32F, Scalar(255.0));

  Mat grad = gradiant(input);
  Mat laplacien = convolution(input, Laplacien);

  for (int i = 0; i < input.rows; i++)
  {
    for (int j = 0; j < input.cols; j++)
    {
      if (grad.at<float>(i, j) < seuil)
        continue;

      // si parmis les 8 voisin de [i,j] on a des positifs et des négatifs, on met le pixel en noir
      bool asNeg = false;
      bool asPos = false;
      for (int k = -1; k <= 1; k++)
      {
        for (int l = -1; l <= 1; l++)
        {
          if (k == 0 && l == 0)
            continue;
          if (laplacien.at<float>(i + k, j + l) < 0)
            asNeg = true;
          if (laplacien.at<float>(i + k, j + l) > 0)
            asPos = true;
          if (asNeg && asPos)
            goto outer;
        }
      }
    outer:
      if (asNeg && asPos)
        output.at<float>(i, j) = 0.0;
    }
  }
  return output;
}

int img(int argc, char *argv[])
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

  float seuil = 20.0;
  createTrackbar("seuil (en %)", "Filter", nullptr, 100, NULL);
  setTrackbarPos("seuil (en %)", "Filter", seuil);

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
    case 'c':
      seuil = getTrackbarPos("seuil (en %)", "Filter");
      inputR = contours(inputR, seuil);
      break;
    default:
      break;
    }
    inputR.convertTo(input, CV_8U);
    imshow("Filter", input); // l'affiche dans la fenêtre
  }
  return 0;
}

int cam(int argc, char **argv)
{
  VideoCapture cap(0);
  if (!cap.isOpened())
    return 1;
  Mat frame, edges;
  namedWindow("Cam", WINDOW_AUTOSIZE);
  bool stop = false;
  int key_code = -1;
  int ascii_code = -1;

  float alpha = 60.0;
  createTrackbar("alpha (en %)", "Cam", nullptr, 100, NULL);
  setTrackbarPos("alpha (en %)", "Cam", alpha);

  float seuil = 20.0;
  createTrackbar("seuil (en %)", "Cam", nullptr, 100, NULL);
  setTrackbarPos("seuil (en %)", "Cam", seuil);
  uint combo = 1;
  while (!stop)
  {
    cap >> frame;
    cvtColor(frame, edges, COLOR_BGR2GRAY);
    edges.convertTo(edges, CV_32F);
    key_code = waitKey(10);
    if (key_code != -1)
      combo = (key_code & 0xff) == ascii_code ? combo + 1 : 1;
    ascii_code = key_code > 0 ? key_code & 0xff : ascii_code; // change only if key_code > 0
    for (int i = 0; i < combo; i++)
    {
      switch (ascii_code)
      {
      case 'a':
        edges = filtreM(edges);
        break;
      case 'm':
        medianBlur(edges, edges, 3);
        break;
      case 'r':
        ascii_code = -1;
        break;
      case 's':
        alpha = getTrackbarPos("alpha (en %)", "Cam");
        edges = filtreRehausseur(edges, alpha / 100.0);
        break;
      case 'x':
        edges = sobel(edges, true);
        break;
      case 'y':
        edges = sobel(edges, false);
        break;
      case 'g':
        edges = gradiant(edges);
        break;
      case 'c':
        seuil = getTrackbarPos("seuil (en %)", "Cam");
        edges = contours(edges, seuil);
        break;
      case 'q':
        stop = true;
      default:
        goto outer;
      }
    }
  outer:
    edges.convertTo(edges, CV_8U);
    imshow("Cam", edges);
  }
  return 0;
}

int main(int argc, char **argv)
{
  // two subcommands: img and cam
  if (argc > 1 && strcmp(argv[1], "img") == 0)
    return img(argc - 1, argv + 1);
  else if (argc > 1 && strcmp(argv[1], "cam") == 0)
    return cam(argc - 1, argv + 1);
  else
    std::cout << "usage: " << argv[0] << " img|cam [args]" << std::endl;
  return 1;
}