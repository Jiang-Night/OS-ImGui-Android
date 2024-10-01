// Self lib
// System libs
#include <asm-generic/fcntl.h>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <unistd.h>
#include <vector>
// User libs
#include "touch.h"
#include <android/log.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <linux/uinput.h>

extern std::vector<ImVec2> touchPositions;
extern std::vector<ImVec2> boxPositions;

#define TAG "mainLog" // 这个是自定义的LOG的标识
#define LOGD(...) \
    __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__) // 定义LOGD类型
#define LOGI(...) \
    __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__) // 定义LOGI类型
#define LOGW(...) \
    __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__) // 定义LOGW类型
#define LOGE(...) \
    __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__) // 定义LOGE类型
#define LOGF(...) \
    __android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__) // 定义LOGF类型
#define maxE 5                                               // 最大允许获得的event数量 设置为5 过多会导致多线程资源浪费
#define maxF 10                                              // 最大手指 10个
#define UNGRAB 0
#define GRAB 1

struct touchObj {
    bool isTmpDown = false;
    bool isDown = false;
    int x = 0;
    int y = 0;
    int id = 0;
};

struct targ {
    int fdNum;
    float S2TX;
    float S2TY;
};

// Var
int fdNum = 0, origfd[maxE], nowfd;
static struct input_event event[128];
static struct input_event upEvent[3];
int inputFd;
bool bPad = false; // 判断是否为特殊平板
float touchScreenx, touchScreeny;
float offsetx = 0.0f, offsety = 0.0f;
float halfOffsetx = 0.0f, halfOffsety = 0.0f;
float t2sx, t2sy;
static pthread_t touch_loop; // 触摸线程
struct touchObj Finger[maxE][maxF];
int screenX, screenY; // uinput注册设备 也是minCnt设备的触摸屏的xy
float imgui_x, imgui_y = 0;
bool isUpdate = false;
bool isOnIMGUI = false;
bool touchInit = false;
extern bool showMenu;
extern int Orientation;
extern int touchScreenX, touchScreenY;

std::string exec(std::string command) {
    char buffer[128];
    std::string result = "";
    // Open pipe to file
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "popen failed!";
    }
    // read till end of process:
    while (!feof(pipe)) {
        // use buffer to read and add to result
        if (fgets(buffer, 128, pipe) != nullptr) {
            result += buffer;
        }
    }
    pclose(pipe);
    return result;
}

int get_touch_event_num() {
    DIR *dir = opendir("/dev/input/");
    dirent *ptr = NULL;
    int eventCount = 0;
    while ((ptr = readdir(dir))) {
        if (strstr(ptr->d_name, "event")) {
            eventCount++;
        }
    }
    closedir(dir);
    int *fdArray = (int *)malloc(eventCount * sizeof(int));
    for (int i = 0; i < eventCount; i++) {
        char temp[128];
        sprintf(temp, "/dev/input/event%d", i);
        fdArray[i] = open(temp, O_RDWR | O_NONBLOCK);
    }
    int ret = -1;
    input_event ie;
    for (;;) {
        for (int i = 0; i < eventCount; i++) {
            memset(&ie, 0, sizeof(ie));
            read(fdArray[i], &ie, sizeof(ie));
            if (ie.type == EV_ABS && ie.code == ABS_MT_TRACKING_ID && ie.value == -1) { // 屏幕触摸
                ret = i;
                break;
            }
        }
        if (ret >= 0) {
            break;
        }
        usleep(10000);
    }
    for (int i = 0; i < eventCount; i++) {
        close(fdArray[i]);
    }
    free(fdArray);
    return ret;
}
int get_volume_event_num(int code) {
    DIR *dir = opendir("/dev/input/");
    dirent *ptr = NULL;
    int eventCount = 0;
    while ((ptr = readdir(dir))) {
        if (strstr(ptr->d_name, "event")) {
            eventCount++;
        }
    }
    closedir(dir);
    int *fdArray = (int *)malloc(eventCount * sizeof(int));
    for (int i = 0; i < eventCount; i++) {
        char temp[128];
        sprintf(temp, "/dev/input/event%d", i);
        fdArray[i] = open(temp, O_RDWR | O_NONBLOCK);
    }
    int ret = -1;
    input_event ie;
    for (;;) {
        for (int i = 0; i < eventCount; i++) {
            memset(&ie, 0, sizeof(ie));
            read(fdArray[i], &ie, sizeof(ie));
            if (ie.type == EV_KEY && ie.code == code) { // 音量按下
                ret = i;
                break;
            }
        }
        if (ret >= 0) {
            break;
        }
        usleep(10000);
    }
    for (int i = 0; i < eventCount; i++) {
        close(fdArray[i]);
    }
    free(fdArray);
    return ret;
}

void dispatchIMGUITouchEvent(input_event ie) {
    int latest = 0;
    ImGuiIO &io = ImGui::GetIO();
    if (ie.type != EV_ABS) {
        return;
    }
    if (ie.code == ABS_MT_SLOT) {
        latest = ie.value;
        return;
    }
    if (latest != 0) {
        return;
    }
    if (ie.code == ABS_MT_TRACKING_ID) {
        if (ie.value == -1) {
            io.MouseDown[0] = false;
        } else {
            io.MouseDown[0] = true;
        }
        return;
    }
    isUpdate = false;
    if (ie.code == ABS_MT_POSITION_X) {
        imgui_x = (ie.value - halfOffsetx) * t2sx;
        isUpdate = true;
    }
    if (ie.code == ABS_MT_POSITION_Y) {
        imgui_y = (ie.value - halfOffsety) * t2sy;
        isUpdate = true;
    }
    if (!isUpdate) {
        return;
    }
    if (bPad) {
        switch (Orientation) {
        case 0:
            io.MousePos = ImVec2(touchScreeny - imgui_y, imgui_x);
            break;
        case 1:
            io.MousePos = ImVec2(imgui_x, imgui_y);
            break;
        case 2:
            io.MousePos = ImVec2(imgui_y, touchScreenx - imgui_x);
            break;
        case 3:
            io.MousePos = ImVec2(touchScreenx - imgui_x, touchScreeny - imgui_y);
            break;
        }
    } else {
        switch (Orientation) {
        case 0:
            io.MousePos = ImVec2(imgui_x, imgui_y);
            break;
        case 1:
            io.MousePos = ImVec2(imgui_y, touchScreenx - imgui_x);
            break;
        case 2:
            io.MousePos = ImVec2(touchScreenx - imgui_x, touchScreeny - imgui_y);
            break;
        case 3:
            io.MousePos = ImVec2(touchScreeny - imgui_y, imgui_x);
            break;
        }
    }
}

// 上传报文
void Upload() {
    int tmpCnt = 0, tmpCnt2 = 0, i, j;
    int size = rand() % 30;
    for (i = 0; i < fdNum; i++) {
        for (j = 0; j < maxF; j++) {
            if (Finger[i][j].isDown) {
                tmpCnt2++;          // 有手指按下了 计数
                if (tmpCnt2 > 10) { // 如果手指大于10了 那不正常 break
                    break;
                }
                event[tmpCnt].type = EV_ABS;
                event[tmpCnt].code = ABS_X;
                event[tmpCnt].value = Finger[i][j].x;
                tmpCnt++;

                event[tmpCnt].type = EV_ABS;
                event[tmpCnt].code = ABS_Y;
                event[tmpCnt].value = Finger[i][j].y;
                tmpCnt++;

                event[tmpCnt].type = EV_ABS;
                event[tmpCnt].code = ABS_MT_POSITION_X;
                event[tmpCnt].value = Finger[i][j].x;
                tmpCnt++;

                event[tmpCnt].type = EV_ABS;
                event[tmpCnt].code = ABS_MT_POSITION_Y;
                event[tmpCnt].value = Finger[i][j].y;
                tmpCnt++;

                event[tmpCnt].type = EV_ABS;
                event[tmpCnt].code = ABS_MT_TOUCH_MAJOR;
                event[tmpCnt].value = size;
                tmpCnt++;

                event[tmpCnt].type = EV_ABS;
                event[tmpCnt].code = ABS_MT_WIDTH_MAJOR;
                event[tmpCnt].value = size;
                tmpCnt++;

                event[tmpCnt].type = EV_ABS;
                event[tmpCnt].code = ABS_MT_TRACKING_ID;
                event[tmpCnt].value = Finger[i][j].id;
                tmpCnt++;

                event[tmpCnt].type = EV_SYN;
                event[tmpCnt].code = SYN_MT_REPORT;
                event[tmpCnt].value = 0;
                tmpCnt++;
            }
        }
    }
    if (tmpCnt == 0) { //
        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_MT_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;
    }
    event[tmpCnt].type = EV_SYN;
    event[tmpCnt].code = SYN_REPORT;
    event[tmpCnt].value = 0;
    tmpCnt++;
    // 写报文
    write(nowfd, event, sizeof(struct input_event) * tmpCnt);
}

void Down(int id, int x, int y) {
    int num = fdNum - 1;
    Finger[num][id].id = (num * 2 + 1) * maxF + id;
    if (bPad) {
        switch (Orientation) {
        case 0:
            Finger[num][id].x = (y / t2sx + halfOffsetx);
            Finger[num][id].y = (touchScreeny - x) / t2sy + halfOffsety;
            break;
        case 1:
            Finger[num][id].x = ((x / t2sx) + halfOffsetx);
            Finger[num][id].y = ((y / t2sy) + halfOffsety);
            break;
        case 2:
            // y = touchScreenx - (Finger[num][id].x - halfOffsetx) * t2sx;
            Finger[num][id].x = (touchScreenx - y) / t2sx + halfOffsetx;
            // x = (Finger[num][id].y - halfOffsety) * t2sy;
            Finger[num][id].y = (x / t2sy + halfOffsety);
            break;
        case 3:
            // x = touchScreenx - (Finger[num][id].x - halfOffsetx) * t2sx;
            Finger[num][id].x = ((touchScreenx - x) / t2sx) + halfOffsetx;
            // y = touchScreeny - (Finger[num][id].y - halfOffsety) * t2sy;
            Finger[num][id].y = ((touchScreeny - y) / t2sy) + halfOffsety;
            break;
        }
    } else {
        switch (Orientation) {
        case 0:
            Finger[num][id].x = ((x / t2sx) + halfOffsetx);
            Finger[num][id].y = ((y / t2sy) + halfOffsety);
            break;
        case 1:
            Finger[num][id].x = (touchScreenx - y) / t2sx + halfOffsetx;
            Finger[num][id].y = (x / t2sy + halfOffsety);
            break;
        case 2:
            Finger[num][id].x = ((touchScreenx - x) / t2sx) + halfOffsetx;
            Finger[num][id].y = ((touchScreeny - y) / t2sy) + halfOffsety;
            break;
        case 3:
            Finger[num][id].x = (y / t2sx + halfOffsetx);
            Finger[num][id].y = (touchScreeny - x) / t2sy + halfOffsety;
            break;
        }
    }
    Finger[num][id].isDown = true;
    Upload();
}

void Move(int id, int x, int y) {
    int num = fdNum - 1;
    Finger[num][id].id = (num * 2 + 1) * maxF + id;
    if (bPad) {
        switch (Orientation) {
        case 0:
            Finger[num][id].x = (y / t2sx + halfOffsetx);
            Finger[num][id].y = (touchScreeny - x) / t2sy + halfOffsety;
            break;
        case 1:
            Finger[num][id].x = ((x / t2sx) + halfOffsetx);
            Finger[num][id].y = ((y / t2sy) + halfOffsety);
            break;
        case 2:
            Finger[num][id].x = (touchScreenx - y) / t2sx + halfOffsetx;
            Finger[num][id].y = (x / t2sy + halfOffsety);
            break;
        case 3:
            Finger[num][id].x = ((touchScreenx - x) / t2sx) + halfOffsetx;
            Finger[num][id].y = ((touchScreeny - y) / t2sy) + halfOffsety;
            break;
        }
    } else {
        switch (Orientation) {
        case 0:
            Finger[num][id].x = ((x / t2sx) + halfOffsetx);
            Finger[num][id].y = ((y / t2sy) + halfOffsety);
            break;
        case 1:
            Finger[num][id].x = (touchScreenx - y) / t2sx + halfOffsetx;
            Finger[num][id].y = (x / t2sy + halfOffsety);
            break;
        case 2:
            Finger[num][id].x = ((touchScreenx - x) / t2sx) + halfOffsetx;
            Finger[num][id].y = ((touchScreeny - y) / t2sy) + halfOffsety;
            break;
        case 3:
            Finger[num][id].x = (y / t2sx + halfOffsetx);
            Finger[num][id].y = (touchScreeny - x) / t2sy + halfOffsety;
            break;
        }
    }
    Finger[num][id].isDown = true;
    Upload();
}

void Up(int id) {
    int num = fdNum - 1;
    Finger[num][id].isDown = false;
    Upload();
}

// 生成随机字符串
char *_genRandomString(int length) {
    int flag, i;
    srand((unsigned)time(NULL));
    char *tmpString = (char *)malloc(length * sizeof(char));
    for (i = 0; i < length - 1; i++) {
        flag = rand() % 3;
        switch (flag) {
        case 0:
            tmpString[i] = 'A' + rand() % 26;
            break;
        case 1:
            tmpString[i] = 'a' + rand() % 26;
            break;
        case 2:
            tmpString[i] = '0' + rand() % 10;
            break;
        default:
            tmpString[i] = 'x';
            break;
        }
    }
    tmpString[length - 1] = '\0';
    return tmpString;
}

// 对设备随机注册一些bit 使其难以被识别
void randomRegisterDevices(int fd) {
    int nums[41] = {ABS_X,
                    ABS_Y,
                    ABS_Z,
                    ABS_RX,
                    ABS_RY,
                    ABS_RZ,
                    ABS_THROTTLE,
                    ABS_RUDDER,
                    ABS_WHEEL,
                    ABS_GAS,
                    ABS_BRAKE,
                    ABS_HAT0Y,
                    ABS_HAT1X,
                    ABS_HAT1Y,
                    ABS_HAT2X,
                    ABS_HAT2Y,
                    ABS_HAT3X,
                    ABS_HAT3Y,
                    ABS_PRESSURE,
                    ABS_DISTANCE,
                    ABS_TILT_X,
                    ABS_TILT_Y,
                    ABS_TOOL_WIDTH,
                    ABS_VOLUME,
                    ABS_MISC,
                    ABS_MT_TOUCH_MAJOR,
                    ABS_MT_TOUCH_MINOR,
                    ABS_MT_WIDTH_MAJOR,
                    ABS_MT_WIDTH_MINOR,
                    ABS_MT_ORIENTATION,
                    ABS_MT_POSITION_X,
                    ABS_MT_POSITION_Y,
                    ABS_MT_TOOL_TYPE,
                    ABS_MT_BLOB_ID,
                    ABS_MT_TRACKING_ID,
                    ABS_MT_PRESSURE,
                    ABS_MT_DISTANCE,
                    ABS_MT_TOOL_X,
                    ABS_MT_TOOL_Y,
                    ABS_MAX,
                    ABS_CNT};
    for (int i = 0; i < 41; i++) {
        int num = rand() % 41;
        ioctl(fd, UI_SET_ABSBIT, nums[num]);
    }
}
targ tmp;
void *TypeA(void *arg) {
    int i = tmp.fdNum;
    float S2TX = tmp.S2TX;
    float S2TY = tmp.S2TY;
    struct input_event ie;
    int latest = 0;
    float Xcache, Ycache;
    while (true) {
        struct input_event iel[32];
        int32_t readSize = read(origfd[i], &iel, sizeof(iel));
        if (readSize <= 0 || (readSize % sizeof(struct input_event)) != 0) {
            continue;
        }
        size_t count = size_t(readSize) / sizeof(struct input_event);
        for (size_t j = 0; j < count; j++) {
            struct input_event ie = iel[j];
            dispatchIMGUITouchEvent(ie);
            if (isOnIMGUI) {
                continue;
            }
            if (ie.code == ABS_MT_SLOT) {
                latest = ie.value;
                continue;
            }
            if (ie.code == ABS_MT_TRACKING_ID) {
                if (ie.value == -1) { // 手指放开了
                    Finger[i][latest].isDown = false;
                } else {
                    Finger[i][latest].id = (i * 2 + 1) * maxF + latest;
                    Finger[i][latest].isDown = true;
                }
                continue;
            }
            if (ie.code == ABS_MT_POSITION_X) {
                Finger[i][latest].id = (i * 2 + 1) * maxF + latest;
                Finger[i][latest].x = (int)(ie.value * S2TX);
                Xcache = (int)(ie.value * S2TX);
                Finger[i][latest].isTmpDown = true;
                continue;
            }
            if (ie.code == ABS_MT_POSITION_Y) {
                Finger[i][latest].id = (i * 2 + 1) * maxF + latest;
                Finger[i][latest].y = (int)(ie.value * S2TY);
                Ycache = (int)(ie.value * S2TY);
                Finger[i][latest].isTmpDown = true;
                continue;
            }
            if (ie.code == SYN_REPORT) {
                if (Finger[i][latest].isTmpDown) {
                    Upload();
                }
                continue;
            }
        }
        usleep(100);
    }
    return 0;
}

// 初始化触摸和imgui触摸处理
void handleAndroidTouchAndIMGUI() { // 初始化触摸设置
    char temp[128];
    DIR *dir = opendir("/dev/input/");
    dirent *ptr = NULL;
    int eventCount = 0;
    int touch_num = get_touch_event_num(); // 获取触摸屏的event
    // 遍历/dev/input/event 获取全部的event事件数量
    while ((ptr = readdir(dir)) != NULL) {
        if (strstr(ptr->d_name, "event")) {
            eventCount++;
        }
    }
    struct input_absinfo abs, absX[maxE], absY[maxE];
    int fd, i, tmp1, tmp2;
    int minCnt =
        eventCount + 1; // 因为要判断出minCnt 所以minCnt必须至少比eventCount大1
    fdNum = 0;
    int touch_fd, index;
    // 遍历全部的event数量 获取到全部的可用触摸设备
    for (i = 0; i <= eventCount; i++) {
        sprintf(temp, "/dev/input/event%d", i);
        fd = open(temp, O_RDWR);
        if (fd) { // 如果fd被打开成功
            uint8_t *bits = NULL;
            ssize_t bits_size = 0;
            int res, j, k;
            bool itmp1 = false, itmp2 = false, itmp3 = false;
            // 获取每个event事件的配置 匹配bit是否有我们的目标 如果都存在
            // 则说明为触摸设备
            while (1) {
                res = ioctl(fd, EVIOCGBIT(EV_ABS, bits_size), bits);
                if (res < bits_size) {
                    break;
                }
                bits_size = res + 16;
                bits = (uint8_t *)realloc(bits, bits_size * 2);
                if (bits == NULL) {
                    printf("获取事件失败\n");
                    exit(0);
                }
            }
            // 获取每个event事件的配置 匹配bit是否有我们的目标 如果都存在
            // 则说明为触摸设备
            for (j = 0; j < res; j++) {
                for (k = 0; k < 8; k++) {
                    if (bits[j] & 1 << k && ioctl(fd, EVIOCGABS(j * 8 + k), &abs) == 0) {
                        if (j * 8 + k == ABS_MT_SLOT) {
                            itmp1 = true;
                            continue;
                        }
                        if (j * 8 + k == ABS_MT_POSITION_X) {
                            itmp2 = true;
                            continue;
                        }
                        if (j * 8 + k == ABS_MT_POSITION_Y) {
                            itmp3 = true;
                            continue;
                        }
                    }
                }
            }
            if (itmp1 && itmp2 && itmp3) {
                tmp1 = ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &absX[fdNum]);
                tmp2 = ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &absY[fdNum]);
                if (tmp1 == 0 && tmp2 == 0) {
                    // 如果tmp1和tmp2 都是0 则说明本次遍历的event是触摸设备
                    // if (i < minCnt) { // 后续会选取最小的minCnt作为伪造设备的根据
                    if (i == touch_num) {
                        origfd[0] = fd; // 做一个存储
                        ioctl(fd, EVIOCGRAB,
                              GRAB); // 对原event做屏蔽 以防止和我们创造的event抢接管权
                        // 原触摸程序会选取最小的触摸设备文件作为模拟创建的对象
                        // 但是这里为了方便部分多设备文件的分辨率和触摸屏分辨率不一样的情况
                        // 统一模拟创建为触摸屏设备文件
                        // 也就是固定minCnt为touch event
                        screenX =
                            absX[fdNum].maximum; // 获取最小event数的设备文件的screenX和Y
                                                 // 也就是触摸屏大小
                        screenY = absY[fdNum].maximum;
                        minCnt = i;
                        touch_fd = fd;
                        index = fdNum;
                    }
                    fdNum++;
                    if (fdNum >= maxE) { // 如果fd数量大于我们规定的最大数 则不再遍历
                                         // 太多线程浪费资源
                        break;
                    }
                }
            }
        }
    }
    // close(fd); // 确保fd在这里是close的
    // 判断minCnt是否有获取成功 后续会选取最小的minCnt作为伪造设备的根据
    if (minCnt > eventCount) {
        printf("获取屏幕驱动失败\n");
        exit(0);
    }
    // 创建uinput设备
    struct uinput_user_dev ui_dev;
    nowfd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (nowfd <= 0) {
        printf("打开驱动失败\n");
        exit(0);
    }
    srand((unsigned)time(NULL));
    memset(&ui_dev, 0, sizeof(ui_dev));
    char name[128], n2[128];
    for (int i = 0; i < rand() % 7 + 6; i++) {
        sprintf(n2, "%c", 'a' + rand() % 26);
        strcat(name, n2);
    }

    strncpy(ui_dev.name, name, UINPUT_MAX_NAME_SIZE);
    sprintf(name, "");
    for (int i = 0; i < rand() % 7 + 6; i++) {
        sprintf(n2, "%c", 'a' + rand() % 26);
        strcat(name, n2);
    }

    ioctl(nowfd, UI_SET_PHYS, name);
    // ui_dev.id.bustype = rand()%2==0?0:rand()%2+0x18;BUS_USB;
    ui_dev.id.vendor = rand() % 3;
    ui_dev.id.product = rand() % 3;
    ui_dev.id.version = (rand() % 2 == 0 ? 0 : 0x100);

    struct input_id id;
    if (!ioctl(touch_fd, EVIOCGID, &id)) {
        ui_dev.id.bustype = id.bustype;
        ui_dev.id.vendor = id.vendor;
        ui_dev.id.product = id.product;
        ui_dev.id.version = id.version;
    }

    ui_dev.absmin[ABS_MT_POSITION_X] = 0;
    ui_dev.absmax[ABS_MT_POSITION_X] = screenX;
    ui_dev.absmin[ABS_MT_POSITION_Y] = 0;
    ui_dev.absmax[ABS_MT_POSITION_Y] = screenY;
    ui_dev.absmin[ABS_MT_TOUCH_MAJOR] = 0;
    ui_dev.absmax[ABS_MT_TOUCH_MAJOR] = 255;
    ui_dev.absmin[ABS_MT_WIDTH_MAJOR] = 0;
    ui_dev.absmax[ABS_MT_WIDTH_MAJOR] = 255;
    ui_dev.absmin[ABS_X] = 0;
    ui_dev.absmax[ABS_X] = screenX;
    ui_dev.absmin[ABS_Y] = 0;
    ui_dev.absmax[ABS_Y] = screenY;
    ioctl(nowfd, UI_SET_PROPBIT, INPUT_PROP_DIRECT);
    ioctl(nowfd, UI_SET_EVBIT, EV_ABS);
    ioctl(nowfd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    ioctl(nowfd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
    ioctl(nowfd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
    ioctl(nowfd, UI_SET_EVBIT, EV_SYN);
    ioctl(nowfd, UI_SET_ABSBIT, ABS_X);
    ioctl(nowfd, UI_SET_ABSBIT, ABS_Y);
    ioctl(nowfd, UI_SET_ABSBIT, ABS_MT_TOUCH_MAJOR);
    ioctl(nowfd, UI_SET_ABSBIT, ABS_MT_WIDTH_MAJOR);
    randomRegisterDevices(nowfd); // 随机注册一些bit 使其难以被识别
    // 获取原event的bit 并且模仿注册
    uint8_t *bits = NULL;
    ssize_t bits_size = 0;
    int res, j, k;
    while (1) {
        res = ioctl(touch_fd, EVIOCGBIT(EV_KEY, bits_size), bits);
        if (res < bits_size) {
            break;
        }
        bits_size = res + 16;
        bits = (uint8_t *)realloc(bits, bits_size * 2);
        if (bits == NULL) {
            printf("获取事件失败\n");
            exit(0);
        }
    }
    for (j = 0; j < res; j++) {
        for (k = 0; k < 8; k++) {
            if (bits[j] & 1 << k) {
                if (j * 8 + k == BTN_TOUCH || j * 8 + k == BTN_TOOL_FINGER) {
                    continue;
                }
                ioctl(nowfd, UI_SET_KEYBIT, j * 8 + k);
            }
        }
    }
    // 上步骤疑似存在问题

    write(nowfd, &ui_dev, sizeof(ui_dev));
    if (ioctl(nowfd, UI_DEV_CREATE)) {
        printf("Heaven : Unable to create UINPUT device.\n");
        return;
    }
    bPad = false;
    // 手机有修改分辨率情况的适配
    if (screenX > screenY) {
        bPad = true; // 特殊平板手机判断
    }
    auto marketname = exec("getprop ro.product.vendor.marketname");
    if (marketname.find("Xiaomi Pad 6") != -1) {
        bPad = false; // 小米6
    }
    if (bPad) { // 对特殊平板的转化判断
        touchScreenx = touchScreenY;
        touchScreeny = touchScreenX;
    } else {
        touchScreenx = touchScreenX;
        touchScreeny = touchScreenY;
    }
    // 计算比率
    t2sx = float(touchScreenx) / (screenX - offsetx);
    t2sy = float(touchScreeny) / (screenY - offsety);
    // 为每个触摸设备创建触摸代理线程
    tmp.fdNum = 0;
    // 其余触摸设备的触摸屏大小不一定和minCnt的触摸屏大小完全一致 做比例转化
    tmp.S2TX =
        (float)(screenX - offsetx) / (float)(absX[index].maximum - offsetx);
    tmp.S2TY =
        (float)(screenY - offsety) / (float)(absY[index].maximum - offsety);
    pthread_create(&touch_loop, NULL, TypeA, nullptr);
    touchInit = true;
}
// 纯只读处理UI
void handleIMGUITouchEvent() {
    char temp[19];
    sprintf(temp, "/dev/input/event%d", get_touch_event_num());
    inputFd = open(temp, O_RDWR);
    struct input_absinfo absX, absY;
    ioctl(inputFd, EVIOCGABS(ABS_MT_POSITION_X), &absX);
    ioctl(inputFd, EVIOCGABS(ABS_MT_POSITION_Y), &absY);
    bPad = false;
    if (absX.maximum > absY.maximum) {
        bPad = true; // 特殊平板手机判断
    }
    auto marketname = exec("getprop ro.product.vendor.marketname");
    if (marketname.find("Xiaomi Pad 6") != -1) {
        bPad = false; // 小米6
    }
    if (bPad) { // 对特殊平板的转化判断
        touchScreenx = touchScreenY;
        touchScreeny = touchScreenX;
    } else {
        touchScreenx = touchScreenX;
        touchScreeny = touchScreenY;
    }
    // printf("bPad:%d %f %f\n", bPad, touchScreenx, touchScreeny);
    // 计算比率
    t2sx = float(touchScreenx) / (absX.maximum - offsetx);
    t2sy = float(touchScreeny) / (absY.maximum - offsety);
    int latest = 0;
    bool isUpdate = false;
    ImGuiIO &io = ImGui::GetIO();
    for (;;) {
        struct input_event iel[32];
        int32_t readSize = read(inputFd, &iel, sizeof(struct input_event));
        if (readSize <= 0 || (readSize % sizeof(struct input_event)) != 0) {
            continue;
        }
        size_t count = size_t(readSize) / sizeof(struct input_event);
        for (size_t j = 0; j < count; j++) {
            struct input_event ie = iel[j];
            if (ie.type != EV_ABS) {
                continue;
            }
            if (ie.code == ABS_MT_SLOT) {
                latest = ie.value;
                continue;
            }
            if (latest != 0) {
                continue;
            }
            if (ie.code == ABS_MT_TRACKING_ID) {
                if (ie.value == -1) {
                    io.MouseDown[0] = false;
                } else {
                    io.MouseDown[0] = true;
                }
                continue;
            }
            isUpdate = false;
            float x, y;
            if (ie.code == ABS_MT_POSITION_X) {
                x = (ie.value - halfOffsetx) * t2sx;
                isUpdate = true;
            }
            if (ie.code == ABS_MT_POSITION_Y) {
                y = (ie.value - halfOffsety) * t2sy;
                isUpdate = true;
            }
            if (!isUpdate) {
                continue;
            }
            if (bPad) {
                switch (Orientation) {
                case 0:
                    io.MousePos = ImVec2(touchScreeny - y, x);
                    break;
                case 1:
                    io.MousePos = ImVec2(x, y);
                    break;
                case 2:
                    io.MousePos = ImVec2(y, touchScreenx - x);
                    break;
                case 3:
                    io.MousePos = ImVec2(touchScreenx - x, touchScreeny - y);
                    break;
                }
            } else {
                switch (Orientation) {
                case 0:
                    io.MousePos = ImVec2(x, y);
                    break;
                case 1:
                    io.MousePos = ImVec2(y, touchScreenx - x);
                    break;
                case 2:
                    io.MousePos = ImVec2(touchScreenx - x, touchScreeny - y);
                    break;
                case 3:
                    io.MousePos = ImVec2(touchScreeny - y, x);
                    break;
                }
            }
        }
        usleep(100);
    }
}
void InitTouch(bool isReadOnly) {
    if (isReadOnly) {
        std::thread *touch_thread = new std::thread(handleIMGUITouchEvent);
        touch_thread->detach();
    } else {
        std::thread *touch_thread = new std::thread(handleAndroidTouchAndIMGUI);
        touch_thread->detach();
    }
}
void handleVolumeupEvent() {
    // 上音量键
    std::thread([]() {
        int fd;
        char temp[19];
        sprintf(temp, "/dev/input/event%d", get_volume_event_num(KEY_VOLUMEUP));
        // printf("%s\n", temp);
        fd = open(temp, O_RDWR);
        for (;;) {
            struct input_event iel[32];
            int32_t readSize = read(fd, &iel, sizeof(struct input_event));
            if (readSize <= 0 || (readSize % sizeof(struct input_event)) != 0) {
                continue;
            }
            size_t count = size_t(readSize) / sizeof(struct input_event);
            for (int j = 0; j < count; j++) {
                auto ie = iel[j];
                if (ie.type == EV_KEY && ie.code == KEY_VOLUMEUP && ie.value == 0) {
                    showMenu = !showMenu;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }).detach();
    // 下音量键
    // std::thread([]() {
    //     int fd;
    //     char temp[19];
    //     sprintf(temp, "/dev/input/event%d",
    //             get_volume_event_num(KEY_VOLUMEDOWN));
    //     // printf("%s\n", temp);
    //     fd =
    //         open(temp, O_RDWR);
    //     for (;;) {
    //         struct input_event iel[32];
    //         int32_t readSize = read(fd, &iel, sizeof(struct input_event));
    //         if (readSize <= 0 || (readSize % sizeof(struct input_event)) != 0) {
    //             continue;
    //         }
    //         size_t count = size_t(readSize) / sizeof(struct input_event);
    //         for (int j = 0; j < count; j++) {
    //             auto ie = iel[j];
    //             static int num = 0;
    //             if (ie.type == EV_KEY && ie.code == KEY_VOLUMEDOWN && ie.value == 0) {
    //                 num++;
    //                 if (num >= 3) {
    //                     // printf("%d %d %d\n", ie.type, ie.code, ie.value);
    //                     showMenu = !showMenu;
    //                     num = 0;
    //                 }
    //             }
    //         }
    //         std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //     }
    // }).detach();
}