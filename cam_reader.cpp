#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>

struct Buffer {
    void* start;
    size_t length;
};

int read_cam(int width, int height, int fps) {
    // Step 1: Configure camera with media-ctl
    std::string cmd = "media-ctl -d /dev/media0 --set-v4l2 '\"m00_b_mvcam 7-003b\":0[fmt:Y8_1X8/"
                      + std::to_string(width) + "x" + std::to_string(height) +
                      "@1/" + std::to_string(fps) + " field:none]'";
    system(cmd.c_str());

    // Step 2: Open V4L2 device
    int fd = open("/dev/video0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/video0");
        return -1;
    }

    // Step 3: Set video format
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;  // 修改为 multiplanar 类型
    // 获取当前格式
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
        perror("VIDIOC_G_FMT");
        close(fd);
        return -1;
    }

     // 设置新的格式
    fmt.fmt.pix_mp.width = width;
    fmt.fmt.pix_mp.height = height;
    fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_GREY;
    fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    fmt.fmt.pix_mp.num_planes = 1;
    
    printf("Setting format: %dx%d\n", width, height);
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        close(fd);
        return -1;
    }
    printf("func is %s,%d\n",__func__,__LINE__);
    std::cout << "Real format: " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << std::endl;

    // Step 4: Request buffers
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("VIDIOC_REQBUFS");
        close(fd);
        return -1;
    }

    std::vector<Buffer> buffers(req.count);
    for (size_t i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[1];
        memset(&buf, 0, sizeof(buf));
        memset(&planes, 0, sizeof(planes));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("VIDIOC_QUERYBUF");
            close(fd);
            return -1;
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

        if (buffers[i].start == MAP_FAILED) {
            perror("mmap");
            close(fd);
            return -1;
        }

        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            close(fd);
            return -1;
        }
    }

    // Step 5: Start streaming
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("VIDIOC_STREAMON");
        close(fd);
        return -1;
    }

    // Step 6: Capture loop
    cv::namedWindow("demo", cv::WINDOW_AUTOSIZE);

    while (true) {
        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            perror("VIDIOC_DQBUF");
            break;
        }

        // Convert raw GREY image to OpenCV Mat
        cv::Mat frame(height, width, CV_8UC1, buffers[buf.index].start);
        std::cout << "Received frame: " << frame.cols << "x" << frame.rows << std::endl;

        cv::imshow("demo", frame);

        if (cv::waitKey(1) == 'q') {
            break;
        }

        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            break;
        }
    }

    // Step 7: Clean up
    ioctl(fd, VIDIOC_STREAMOFF, &type);
    for (size_t i = 0; i < buffers.size(); i++) {
        munmap(buffers[i].start, buffers[i].length);
    }
    close(fd);
    cv::destroyAllWindows();
    return 0;
}

int main(int argc, char** argv) {
    int width = 1280;
    int height = 1080;
    int fps = 30;

    if (argc >= 2) width = std::stoi(argv[1]);
    if (argc >= 3) height = std::stoi(argv[2]);
    if (argc >= 4) fps = std::stoi(argv[3]);

    return read_cam(width, height, fps);
}

