#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include "ChangoInput.h"
extern void onMouse(int event, int x, int y, int flags, void *n);



ChangoInput::ChangoInput(){
  cvSetMouseCallback(WINDOW_NAME_STR,onMouse,this);
}

