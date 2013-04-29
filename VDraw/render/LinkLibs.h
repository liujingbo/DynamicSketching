#pragma once

#ifdef _DEBUG
#pragma comment(lib, "opencv_core243d.lib")
#pragma comment(lib, "opencv_highgui243d.lib")
#pragma comment(lib, "opencv_imgproc243d.lib")
#pragma comment(lib, "opencv_ml243d.lib")
#pragma comment(lib, "opencv_legacy243d.lib")

#else
#pragma comment(lib, "opencv_core243.lib")
#pragma comment(lib, "opencv_highgui243.lib")
#pragma comment(lib, "opencv_imgproc243.lib")
#pragma comment(lib, "opencv_ml243.lib")
#pragma comment(lib, "opencv_legacy243.lib")
#endif