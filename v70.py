import sys
import argparse
import subprocess
import cv2
import time
import matplotlib.pyplot as plt
import v4l2capture
import select
import numpy as np

def read_cam(width, height, fps):
    # 配置摄像头
    v4l2_cmd = f'media-ctl -d /dev/media0 --set-v4l2 \'"m00_b_mvcam 7-003b":0[fmt:Y8_1X8/{width}x{height}@1/{fps} field:none]\''
    subprocess.run(v4l2_cmd, shell=True)
    
    # 使用 V4L2 打开摄像头
    video = v4l2capture.Video_device("/dev/video0")
    video.set_format(width, height, fourcc='GREY')
    format_info = video.get_format()
    real_width, real_height = format_info[:2] 
    print(f"Real format: {real_width}x{real_height}")
    video.create_buffers(1)
    video.queue_all_buffers()
    video.start()
    
    cv2.namedWindow("demo", cv2.WINDOW_AUTOSIZE)
    
    try:
        while True:
            # 等待摄像头数据
            select.select((video,), (), ())
            image_data = video.read_and_queue()
            print(f"Received data size: {len(image_data)}")
            # 将原始数据转换为numpy数组
            frame = np.frombuffer(image_data, dtype=np.uint8)
            frame = frame.reshape((height, 1280))
            
            print(frame.shape)
            cv2.imshow('demo', frame)
            
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
                
    except Exception as e:
        print(f"Error: {e}")
    finally:
        video.close()
        cv2.destroyAllWindows()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Read camera video stream')
    parser.add_argument('--width', type=int, default=1280, help='width of the video stream')
    parser.add_argument('--height', type=int, default=1080, help='height of the video stream')
    parser.add_argument('--fps', type=int, default=30, help='fps of the video stream')
    args = parser.parse_args()

    read_cam(args.width, args.height, args.fps)
