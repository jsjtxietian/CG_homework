import cv2
# assert cv2.__version__[0] == '3', 'The fisheye module requires opencv version >= 3.0.0'
import numpy as np
from numpy import array,eye

K=array([[827.6234652673947, 0.0, 939.1581869953802], [0.0, 825.3135033186057, 539.8705444406974], [0.0, 0.0, 1.0]])
D=array([[-0.018539535960456127], [-0.009118082753460773], [-0.004507294555062113], [0.00766180111362084]])
img=cv2.imread('/home/lab318/xt/darknet/input.jpg')
new_K = cv2.fisheye.estimateNewCameraMatrixForUndistortRectify(K, D, (1920, 1080), eye(3), balance=1)
map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, eye(3), new_K, (1920, 1080), cv2.CV_16SC2)
undistorted_img = cv2.remap(img, map1, map2, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
cv2.imwrite('output.jpg',undistorted_img)
