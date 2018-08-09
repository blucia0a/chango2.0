#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
class CameraAndWindow{


public:
      // create all necessary instances
      CvCapture * camera;
      CvMemStorage* storage;
      CameraAndWindow();

};
