#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

using namespace cv;
using namespace std;

struct MinArgmin
{
  float min;  //min value
  int argmin; //position of min value
};

struct twoMat
{
  Mat gray;
  Mat color;
};


float sobel(int i, int j, const Mat &im);
int m; int n;
void find_seam(int arr[], const Mat &im);
MinArgmin getMinArgmin(const float* arr, const int length);
Mat seam_carving(const Mat &imcol, Mat &imgray, int m_new, int n_new);
twoMat delete_seam(const int *arr, const Mat &im_gray, const Mat &im_col);


int main()
{

  Mat original_image;
  Mat im_gray;
  original_image = imread("landscape.jpg", 1 );
  im_gray = imread("landscape.jpg", 0 );

  if( argc == 2 || !original_image.data )
    {
      printf( "No image data \n" );
      return -1;
    }

  m=im_gray.rows;
  n=im_gray.cols;

  Mat final_result=seam_carving(original_image, im_gray, m-100, n-100);
  namedWindow( "Result", WINDOW_AUTOSIZE );
  imshow( "Result", final_result);
  namedWindow( "Original", WINDOW_AUTOSIZE );
  imshow( "Original", original_image);

  waitKey(0);
  return 0;
}

float sobel(int i, int j, const Mat &im)
{
  int pos_j=std::max(j-1,0);
  int pos_i=std::max(i-1,0);
  int m_i=std::min(i+1,im.rows);
  int n_j=std::min(j+1,im.cols);
  float ix=-2*im.at<uchar>(i,pos_j) + 2* im.at<uchar>(i,n_j) - im.at<uchar>(m_i,pos_j) + im.at<uchar>(m_i,n_j) - im.at<uchar>(pos_i,pos_j) + im.at<uchar>(pos_i,n_j);
  float iy=im.at<uchar>(m_i,pos_j) + 2* im.at<uchar>(m_i,j) + im.at<uchar>(m_i,n_j) - ( im.at<uchar>(pos_i,pos_j) + 2* im.at<uchar>(pos_i,j) + im.at<uchar>(pos_i,n_j));
  return abs(ix) + abs(iy);
}



void find_seam(int arr[], const Mat &im,char *q)
{
  if(q=='v')
{
  m=im.rows;
  n=im.cols;
}
else
{
  m=im.cols;
  n=im.rows;
}
  float C[m][n];
  int trace[m][n];
  float min_args[3];
  int j_min;
  MinArgmin cj;
  for(int j=0; j<n; j++)
  {
    C[0][j]=sobel(0,j, im);
  }
  for(int i=1; i<m; i++)
  {
    for(int j=0; j<n; j++)
    {
      float left;
      float right;
      float center;
      left=C[i-1][j-1];
      center=C[i-1][j];
      right=C[i-1][j+1];
      if(j==0){
        right=C[i-1][j+1];
        center=C[i-1][j];
        left=right+1; }
        else if(j==n-1)
        {
          center=C[i-1][j];
          left=C[i-1][j-1];
          right=left+1;
        }
        else
        {
          left=C[i-1][j-1];
          center=C[i-1][j];
          right=C[i-1][j+1];
        }
      min_args[0]=left;
      min_args[1]=center;
      min_args[2]=right;
      cj=getMinArgmin(min_args, 3);
      C[i][j]=cj.min+sobel(i,j, im);

      trace[i][j]=cj.argmin;
    }
  }

  int optcol=getMinArgmin(C[m-1],n).argmin;

  arr[m-1]=optcol;
  for(int k=m-2; k>=0; k--)
  {
    arr[k]=trace[k+1][arr[k+1]];
  }
}

MinArgmin getMinArgmin(const float* arr, const int length)
{
  int argmin=0;
  float min=arr[0];
  for(int i=1; i<length; i++)
  {
    if(arr[i]<min)
    {
      min=arr[i]; argmin=i;
    }
  }
  return {min, argmin};
}

Mat seam_carving(const Mat &imcol, Mat &imgray, int m_new, int n_new)
{
  Mat result=imcol.clone();
  int horz_diff=imcol.rows-m_new;
  int vert_diff=imcol.cols-n_new;
  int m_n=max(imcol.rows, imcol.cols);
  int seam[m_n];
  for(int i=0; i<vert_diff; i++)
  {
      find_seam(seam,imgray,'v');
      twoMat both=delete_seam(seam,imgray,result);
      imgray=both.gray;
      result=both.color;
  }
  imgray=imgray.t();
  result=result.t();
  for(int i=0; i<horz_diff; i++)
  {
      find_seam(seam,imgray,'h');
      twoMat both=delete_seam(seam,imgray,result);
      imgray=both.gray;
      result=both.color;
  }
  imgray=imgray.t();
  result=result.t();
  return result;
}

twoMat delete_seam(const int *arr, const Mat &imgray, const Mat &im_col)
{
  if(imgray.cols!=im_col.cols || imgray.rows!=im_col.rows)
  {
    printf("Matrices don't have the same dimension \n");
  }
  int m=imgray.rows;
  int n=imgray.cols;
  Mat temp_gray(m,n-1,CV_8UC1);
  Mat temp_col(m,n-1,CV_8UC3);
  for(int i=0; i<m; i++)
  {
    for(int j=0; j<n-1; j++)
    {
      if(j<arr[i])
      {
        temp_gray.at<uchar>(i,j)=imgray.at<uchar>(i,j);
        temp_col.at<Vec3b>(i,j)=im_col.at<Vec3b>(i,j);
      }
      else
      {
        temp_gray.at<uchar>(i,j)=imgray.at<uchar>(i,j+1);
        temp_col.at<Vec3b>(i,j)=im_col.at<Vec3b>(i,j+1);
      }
    }
  }
  return {temp_gray, temp_col};
}
