#include "AIchat.h"
#include "utils.h"

#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <atomic>
#include <iomanip>

bool AIchat::init(void){
    return true;
}

std::atomic<bool> isRecording(false);
std::atomic<bool> shouldExit(false);
#define SAMPLE_RATE 16000
#define FRAMES_PER_BUFFER 512
#define NUM_CHANNELS 2
#define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;

// extern volatile int QUIT_FLAG;
struct UserData {
    std::vector<float> snowboyBuffer;  // 持续给Snowboy的缓冲区
    std::vector<float> recordingBuffer; // 录音缓冲区
    bool isFirstRecording = true;      // 是否是第一次录音
    };
// WAV文件头结构
struct WavHeader {
    char riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t overall_size;
    char wave[4] = {'W', 'A', 'V', 'E'};
    char fmt[4] = {'f', 'm', 't', ' '};
    uint32_t fmt_size = 16;
    uint16_t audio_format = 1; // PCM
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data[4] = {'d', 'a', 't', 'a'};
    uint32_t data_size;
};
void saveToWav(const std::vector<float>& samples,  const std::string& filename, 
    int sampleRate, int numChannels);
std::string generateUniqueFilename(const std::string& baseName, const std::string& extension);
void keyboardListener();
// 录音回调函数
static int audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) 
{
// 用户数据结构


    UserData* data = (UserData*)userData;
    const float* rptr = (const float*)inputBuffer;
    const float gain = 1.0f;
    // 1. 始终将音频数据提供给Snowboy
    // for (unsigned int i = 0; i < framesPerBuffer * 2; i++) {
    //     data->snowboyBuffer.push_back(rptr[i]);
    // }

    // 2. 如果正在录音，保存数据到录音缓冲区
    if (isRecording) {
        if (data->isFirstRecording) {
            data->recordingBuffer.clear();
            data->isFirstRecording = false;
        }
        for (unsigned int i = 0; i < framesPerBuffer * 2; i++) {
            // 应用增益并限制在-1.0到1.0之间
            float amplifiedSample = rptr[i] * gain;
            // amplifiedSample = std::max(-1.0f, std::min(1.0f, amplifiedSample));
            data->recordingBuffer.push_back(amplifiedSample);
        }
    }else {
    // 3. 如果刚结束录音，保存文件
        if (!data->isFirstRecording) {
            std::string filename = generateUniqueFilename("recording", ".wav");
            saveToWav(data->recordingBuffer, filename, 16000, 2);
            data->isFirstRecording = true;
        }
    }
    return paContinue;
}

// void AIchat_thread()
// {
//     PaStream* inputStream;
//     UserData userData;
//     const std::string BASE_FILENAME = "recording";
//     const std::string FILE_EXTENSION = ".wav";

//     std::thread keyboardThread(keyboardListener);
//     //port audio init
//     PaError err = Pa_Initialize();
//     if(err != paNoError) {
//         std::cerr << "PortAudio初始化错误: " << Pa_GetErrorText(err) << std::endl;
//         return ;
//     }
//     //open stream
//     err = Pa_OpenDefaultStream(&inputStream, 
//         NUM_CHANNELS, // 输入通道数
//         0,            // 无输出
//         paFloat32,    
//         SAMPLE_RATE,
//         FRAMES_PER_BUFFER,
//         audioCallback,
//         &userData);
//     if(err != paNoError) {
//         std::cerr << "打开输入流错误: " << Pa_GetErrorText(err) << std::endl;
//         Pa_Terminate();
//         return ;
//     }
//     //start stream
//     std::cout << "预热麦克风..." << std::endl;
//     Pa_Sleep(3000);  // 等待3秒
//     std::cout << "开始正式录音" << std::endl;
//     err = Pa_StartStream(inputStream);
//     if(err != paNoError) {
//         std::cerr << "启动流错误: " << Pa_GetErrorText(err) << std::endl;
//         Pa_CloseStream(inputStream);
//         Pa_Terminate();
//         return ;
//     }
//     std::cout << "程序运行中... 按住R键录音,释放停止并保存" << std::endl;
//     std::cout << "按Ctrl+C退出程序" << std::endl;

//     while (!QUIT_FLAG) {
//         // 这里可以添加Snowboy的处理逻辑
//         // 例如: snowboy->RunDetection(userData.snowboyBuffer.data(), userData.snowboyBuffer.size()/2);
        
//         // 清空Snowboy缓冲区(模拟处理)
//         // if (userData.snowboyBuffer.size() > SAMPLE_RATE) { // 保持约1秒的缓冲区
//         //     userData.snowboyBuffer.erase(userData.snowboyBuffer.begin(), 
//         //                                userData.snowboyBuffer.end() - SAMPLE_RATE * 2);
//         // }
        
//         Pa_Sleep(10);
//     }

//     err = Pa_StopStream(inputStream);
//     Pa_CloseStream(inputStream);
//     shouldExit = true;
//     keyboardThread.join();
// }

// 非阻塞键盘输入检测
void keyboardListener() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (!shouldExit) {
        char c = getchar();
        std::cout << "c= " << std::endl;
        if (c == 'r' || c == 'R') {
            if (!isRecording) {
                std::cout << "\n开始录音..." << std::endl;
                isRecording = true;
            }
        } else {
            if (isRecording) {
                std::cout << "\n停止录音" << std::endl;
                isRecording = false;
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void saveToWav(const std::vector<float>& samples,  const std::string& filename, 
    int sampleRate, int numChannels) {
    std::ofstream out(filename, std::ios::binary);

    if(!out.is_open()) {
    std::cerr << "无法打开文件 " << filename << " 用于写入" << std::endl;
    return;
    }

    WavHeader header;
    header.num_channels = numChannels;
    header.sample_rate = sampleRate;
    header.bits_per_sample = 16; // 我们将把float转换为16位PCM
    header.byte_rate = header.sample_rate * header.num_channels * header.bits_per_sample / 8;
    header.block_align = header.num_channels * header.bits_per_sample / 8;
    header.data_size = samples.size() * sizeof(int16_t); // 因为我们转换为16位

    // RIFF块大小是文件总大小减去8字节
    header.overall_size = 36 + header.data_size;

    // 写入头部
    out.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // 将float样本转换为16位PCM并写入
    for(float sample : samples) {
    // 将-1.0到1.0的float转换为-32768到32767的16位整数
    int16_t pcmSample = static_cast<int16_t>(sample * 32767.0f);
    out.write(reinterpret_cast<const char*>(&pcmSample), sizeof(int16_t));
    }

    out.close();
    std::cout << "已保存录音到 " << filename << std::endl;
}

// 检查文件是否存在
bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// 生成不重复的文件名(从000开始)
std::string generateUniqueFilename(const std::string& baseName, const std::string& extension) {
    int counter = 0;
    std::string filename;
    
    do {
        std::ostringstream oss;
        oss << baseName << "_" << std::setw(3) << std::setfill('0') << counter << extension;
        filename = oss.str();
        counter++;
    } while (fileExists(filename));
    
    return filename;
}


void AIchat::moduleThreadFunc()
{
    port_audio_driver.startRecording();
    while (!quit_flag)
    {

    }
    port_audio_driver.stopRecording();
}