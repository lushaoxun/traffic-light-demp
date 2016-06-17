import cv2
import numpy as np


class ProcessImg():
    def __init__(self):
        self.circles = None
        self.jishu = 0
        pass

    def process(self, imgPath, h4, h5, h6, h7, h8, h9):
        print h4
        img = cv2.imread(imgPath)
  
        img = cv2.medianBlur(img, 5)
        gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

        circles = cv2.HoughCircles(gray,cv2.HOUGH_GRADIENT,int(h4),int(h5),
                                    param1=int(h6),param2=int(h7),minRadius=int(h8),maxRadius=int(h9))
        if circles == None:
            return 0
        circles = np.uint16(np.around(circles))
        
        self.circles = circles[0, : ]
        for i in circles[0,:]:
            # draw the outer circle
            cv2.circle(img,(i[0],i[1]),i[2],(0,255,0),2)
            # draw the center of the circle
            cv2.circle(img,(i[0],i[1]),2,(0,0,255),3)

        #cv2.imshow('detected circles',img)
        #cv2.waitKey(0)
        #cv2.destroyAllWindows()

        return len(self.circles) * 3

    def getOne(self, index):
        ret = None
        temp = self.jishu % 3
        index = self.jishu / 3
        if temp == 0:
            ret = self.circles[index][0]
        elif temp == 1:
            ret = self.circles[index][1]
        elif temp == 2:
            ret = self.circles[index][2]
        self.jishu = self.jishu + 1
        return int(ret)
