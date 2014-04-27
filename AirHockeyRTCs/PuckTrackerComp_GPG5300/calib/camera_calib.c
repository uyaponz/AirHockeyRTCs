#include <stdio.h>
// for opencv
#include <opencv/cv.h>
#include <opencv/highgui.h>
// #include <opencv/legacy/legacy.hpp>

#define IMAGE_NUM  (25)         /* 画像数 */
#define PAT_ROW    (7)          /* パターンの行数 */
#define PAT_COL    (10)         /* パターンの列数 */
#define PAT_SIZE   (PAT_ROW*PAT_COL)
#define ALL_POINTS (IMAGE_NUM*PAT_SIZE)
#define CHESS_SIZE (50.0)       /* パターン1マスの1辺サイズ[mm] */

int
main (int argc, char *argv[])
{

  char file_set_name[50];

  int i, j, k;
  int corner_count, found;
  int p_count[IMAGE_NUM];
  IplImage *src_img[IMAGE_NUM];
  CvSize pattern_size = cvSize (PAT_COL, PAT_ROW);
  CvPoint3D32f objects[ALL_POINTS];
  CvPoint2D32f *corners = (CvPoint2D32f *) cvAlloc (sizeof (CvPoint2D32f) * ALL_POINTS);
  CvMat object_points;
  CvMat image_points;
  CvMat point_counts;
  CvMat *intrinsic = cvCreateMat (3, 3, CV_32FC1);
  CvMat *rotation = cvCreateMat (1, 3, CV_32FC1);
  CvMat *translation = cvCreateMat (1, 3, CV_32FC1);
  CvMat *distortion = cvCreateMat (1, 4, CV_32FC1); // 4 or 5

  // (1)キャリブレーション画像の読み込み
  printf("file set name: ");
  scanf("%s",file_set_name);
  for (i = 0; i < IMAGE_NUM; i++) {
    char buf[52];
    sprintf (buf, "%s%02d.png",file_set_name,i);
    if ((src_img[i] = cvLoadImage (buf, CV_LOAD_IMAGE_COLOR)) == NULL) {
      fprintf (stderr, "cannot load image file : %s\n", buf);
    }
  }

  // (2)3次元空間座標の設定
  for (i = 0; i < IMAGE_NUM; i++) {
    for (j = 0; j < PAT_ROW; j++) {
      for (k = 0; k < PAT_COL; k++) {
        objects[i * PAT_SIZE + j * PAT_COL + k].x = j * CHESS_SIZE;
        objects[i * PAT_SIZE + j * PAT_COL + k].y = k * CHESS_SIZE;
        objects[i * PAT_SIZE + j * PAT_COL + k].z = 0.0;
      }
    }
  }
  //  cvInitMatHeader (&object_points, ALL_POINTS, 3, CV_32FC1, objects);
  cvInitMatHeader (&object_points, ALL_POINTS, 3, CV_32FC1, objects,CV_AUTOSTEP);

  // (3)チェスボード（キャリブレーションパターン）のコーナー検出
  int found_num = 0;
  cvNamedWindow ("Calibration", CV_WINDOW_AUTOSIZE);
  for (i = 0; i < IMAGE_NUM; i++) {
    //    found = cvFindChessboardCorners (src_img[i], pattern_size, &corners[i * PAT_SIZE], &corner_count);
    found = cvFindChessboardCorners (src_img[i], pattern_size, &corners[i * PAT_SIZE], &corner_count, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
    fprintf (stderr, "%02d...", i);
    if (found) {
      fprintf (stderr, "ok\n");
      found_num++;
    }
    else {
      fprintf (stderr, "fail\n");
    }
    // (4)コーナー位置をサブピクセル精度に修正，描画
    IplImage *src_gray = cvCreateImage (cvGetSize (src_img[i]), IPL_DEPTH_8U, 1);
    cvCvtColor (src_img[i], src_gray, CV_BGR2GRAY);
    cvFindCornerSubPix (src_gray, &corners[i * PAT_SIZE], corner_count,
                        cvSize (3, 3), cvSize (-1, -1), cvTermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
    cvDrawChessboardCorners (src_img[i], pattern_size, &corners[i * PAT_SIZE], corner_count, found);
    p_count[i] = corner_count;
    cvShowImage ("Calibration", src_img[i]);
    cvWaitKey (0);
  }
  cvDestroyWindow ("Calibration");

  if (found_num != IMAGE_NUM)
    return -1;
  //  cvInitMatHeader (&image_points, ALL_POINTS, 1, CV_32FC2, corners);
  //  cvInitMatHeader (&point_counts, IMAGE_NUM, 1, CV_32SC1, p_count);
  cvInitMatHeader (&image_points, ALL_POINTS, 1, CV_32FC2, corners, CV_AUTOSTEP);
  cvInitMatHeader (&point_counts, IMAGE_NUM, 1, CV_32SC1, p_count, CV_AUTOSTEP);

  // (5)内部パラメータ，歪み係数の推定
  //  cvCalibrateCamera2 (&object_points, &image_points, &point_counts, cvSize (640, 480), intrinsic, distortion);
  cvCalibrateCamera2 (&object_points, &image_points, &point_counts, cvSize (640, 480), intrinsic, distortion, NULL, NULL, 0 // CV_CALIB_FIX_ASPECT_RATIO
);

  // (6)外部パラメータの推定
  CvMat sub_image_points, sub_object_points;
  int base = 0;
  //  cvGetRows (&image_points, &sub_image_points, base * PAT_SIZE, (base + 1) * PAT_SIZE);
  //  cvGetRows (&object_points, &sub_object_points, base * PAT_SIZE, (base + 1) * PAT_SIZE);
  cvGetRows (&image_points, &sub_image_points, base * PAT_SIZE, (base + 1) * PAT_SIZE, 1);
  cvGetRows (&object_points, &sub_object_points, base * PAT_SIZE, (base + 1) * PAT_SIZE, 1);
  //  cvFindExtrinsicCameraParams2 (&sub_object_points, &sub_image_points, intrinsic, distortion, rotation, translation);
  cvFindExtrinsicCameraParams2 (&sub_object_points, &sub_image_points, intrinsic, distortion, rotation, translation,0); // opencv2/calib3d/calib3d.hpp

  // (7)XMLファイルへの書き出し
  CvFileStorage *fs;
  //  fs = cvOpenFileStorage ("camera.xml", 0, CV_STORAGE_WRITE);
  fs = cvOpenFileStorage ("camera.xml", 0, CV_STORAGE_WRITE, NULL);
  // opencv2/core/core_c.h
  cvWrite (fs, "intrinsic", intrinsic);
  cvWrite (fs, "rotation", rotation);
  cvWrite (fs, "translation", translation);
  cvWrite (fs, "distortion", distortion);
  cvReleaseFileStorage (&fs);

  for (i = 0; i < IMAGE_NUM; i++) {
    cvReleaseImage (&src_img[i]);
  }

  return 0;
}
