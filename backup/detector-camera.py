import time
import math
import random
import colorsys
import numpy as np
# assert cv2.__version__[0] == '3', 'The fisheye module requires opencv version >= 3.0.0'
from numpy import array,eye
from PIL import Image, ImageDraw, ImageFont
import cv2
import python.darknet as dn

# prepare YOLO

net = dn.load_net(str.encode("SpeedBumps/yolov3-voc.cfg"),
                  str.encode("yolov3-voc_final.weights"), 0)
meta = dn.load_meta(str.encode("SpeedBumps/voc.data"))

# box colors
box_colors = None

K=np.array([[276.4375427291772, 0.0, 313.6263575113481], [0.0, 275.41723500085385, 179.6311970659696], [0.0, 0.0, 1.0]])
D=np.array([[-0.02851433608691995], [0.014295456531162357], [-0.02846891799622809], [0.015314864345745591]])


new_K = cv2.fisheye.estimateNewCameraMatrixForUndistortRectify(K, D, (640, 360), eye(3), balance=1)
map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, eye(3), new_K, (640, 360), cv2.CV_16SC2)

def generate_colors(num_classes):
    global box_colors

    if box_colors != None and len(box_colors) > num_classes:
        return box_colors

    hsv_tuples = [(x / num_classes, 1., 1.) for x in range(num_classes)]
    box_colors = list(map(lambda x: colorsys.hsv_to_rgb(*x), hsv_tuples))
    box_colors = list(
        map(lambda x: (int(x[0] * 255), int(x[1] * 255), int(x[2] * 255)),
            box_colors))
    random.seed(123456)  # Fixed seed for consistent colors across runs.
    # Shuffle colors to decorrelate adjacent classes.
    random.shuffle(box_colors)
    random.seed(None)  # Reset seed to default.

outMatrix = np.mat([[ 1.99889264 , -1.37652546e-01 ,  -6.08656029e+02] , [ 5.26897553e-02 ,  -1.18095947e-01 ,   4.95524903e+02] , [-7.42909750e-02 ,   2.36055954e+00 ,  -4.17259832e+02]])

def draw_boxes(img, result):

    image = Image.fromarray(img)

    font = ImageFont.truetype(font='font/FiraMono-Medium.otf', size=20)
    thickness = (image.size[0] + image.size[1]) // 300

    num_classes = len(result)
    generate_colors(num_classes)

    index = 0
    for objection in result:
        index += 1
        class_name, class_score, (x, y, w, h) = objection
        # print(name, score, x, y, w, h)

        left = int(x - w / 2)
        right = int(x + w / 2)
        top = int(y - h / 2)
        bottom = int(y + h / 2)

        leftPos = x
        topPos = y
        posVec = outMatrix * np.mat([[leftPos] , [topPos] , [1]])
        posVec[0,0] = posVec[0,0] / posVec[2,0]
        posVec[1,0] = posVec[1,0] / posVec[2,0]
        print("left=" + str(leftPos) + "  top=" + str(topPos) + "  x=" + str(posVec[0,0]) + "  y=" + str(posVec[1,0]))


        label = '{} {:.2f}'.format(class_name.decode('utf-8'), class_score)
        #label = 'distance:' + str(math.sqrt(posVec[0,0]*posVec[0,0] + posVec[1,0] * posVec[1,0])) + "m"
        #label = 'x:' + str(posVec[0,0]) + '  y:' + str(posVec[1,0])
        label = 'distance:' + str(posVec[1,0])

        draw = ImageDraw.Draw(image)
        label_size = draw.textsize(label, font)

        top = max(0, np.floor(top + 0.5).astype('int32'))
        left = max(0, np.floor(left + 0.5).astype('int32'))
        bottom = min(image.size[1], np.floor(bottom + 0.5).astype('int32'))
        right = min(image.size[0], np.floor(right + 0.5).astype('int32'))
        print(label, (left, top), (right, bottom))

        if top - label_size[1] >= 0:
            text_origin = np.array([left, top - label_size[1]])
        else:
            text_origin = np.array([left, top + 1])

        for i in range(thickness):
            draw.rectangle([left + i, top + i, right - i,
                            bottom - i], outline=box_colors[index - 1])
        draw.rectangle(
            [tuple(text_origin), tuple(text_origin + label_size)],
            fill=box_colors[index - 1])
        draw.text(text_origin, label, fill=(255, 255, 255), font=font)
        del draw

    return np.array(image)


def array_to_image(arr):
    arr = arr.transpose(2, 0, 1)
    c = arr.shape[0]
    h = arr.shape[1]
    w = arr.shape[2]
    arr = (arr / 255.0).flatten()
    data = dn.c_array(dn.c_float, arr)
    im = dn.IMAGE(w, h, c, data)
    return im


def detect(net, meta, image, thresh=.5, hier_thresh=.5, nms=.45):
    #boxes = dn.make_boxes(net)
    #probs = dn.make_probs(net)
    #num = dn.num_boxes(net)
    dn.network_detect(net, image, thresh, hier_thresh, nms, boxes, probs)
    res = []
    for j in range(num):
        for i in range(meta.classes):
            if probs[j][i] > 0:
                res.append(
                    (meta.names[i], probs[j][i], (boxes[j].x, boxes[j].y, boxes[j].w, boxes[j].h)))
    res = sorted(res, key=lambda x: -x[1])
    dn.free_ptrs(dn.cast(probs, dn.POINTER(dn.c_void_p)), num)
    return res


def undistort(img):
    time1 = time.clock()
    img = cv2.resize(img , (640 , 360))
    undistorted_img = cv2.remap(img, map1, map2, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
    #undistorted_img = cv2.resize(undistorted_img , (640,360))
    time2 = time.clock()
    print("undistort " + str(time2-time1))
    return undistorted_img



def pipeline(img):
    # image data transform
    # img - cv image
    # im - yolo image
    img = undistort(img)
    
    im = array_to_image(img)
    
    dn.rgbgr_image(im)
    
  


    tic = time.time()
    result = dn.detect(net, meta, im)
    toc = time.time()
    #print("detect " + str(toc - tic), result)

    img_final = draw_boxes(img, result)
    return img_final


count_frame, process_every_n_frame = 0, 1
# get camera device
cap = cv2.VideoCapture("a.mp4")
cap.set(cv2.CAP_PROP_FRAME_WIDTH,640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT,360)

while(True):
    # get a frame
    ret, frame = cap.read()
    count_frame += 1

    # show a frame
    #img = cv2.resize(frame, (640, 360))  # resize image half
    #cv2.imshow("Video", img)
    img = frame

    # if running slow on your computer, try process_every_n_frame = 10
    if count_frame % process_every_n_frame == 0:
        cv2.imshow("YOLO", pipeline(img))

    # press keyboard 'q' to exit
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
