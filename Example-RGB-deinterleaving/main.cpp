/*
 * @Author: kkchen
 * @Date: 2021-11-16 20:47:39
 * @LastEditors: kkchen
 * @LastEditTime: 2022-02-19 18:53:52
 * @Email: 1649490996@qq.com
 * @Description: file content
 */
#include "arm_neon.h"
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <arm_neon.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
using namespace chrono;

void timeTest(function<void()>& fun, string& userData){
    auto start = steady_clock::now();
    fun();
    auto end   = steady_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    std::cout << userData << " cost: " << duration.count() / 1000.0 << " ms" << std::endl;
}

void kernelByHand(vector<uint8_t*>& inputVec, uint8_t* data, int height, int width){
    for(int i = 0 ; i < height; i++){
        for(int j = 0; j < width; j++){
            int index = j + i * width;
            uint8_t* pixelPtr = data + index;
            for(int c = 0; c < 3; c++){
                *(index + inputVec[c]) = *(pixelPtr + c);
            }
        }
    }
}

void kernelByNeon(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *rgb, int len_color) {
    /*
     * Take the elements of "rgb" and store the individual colors "r", "g", and "b"
     */
    int num8x16 = len_color / 16;
    uint8x16x3_t intlv_rgb;
    for (int i=0; i < num8x16; i++) {
        intlv_rgb = vld3q_u8(rgb+3*16*i);
        
        vst1q_u8(r+16*i, intlv_rgb.val[0]);
        vst1q_u8(g+16*i, intlv_rgb.val[1]);
        vst1q_u8(b+16*i, intlv_rgb.val[2]);
    }
}



int main(int argc, char* argv[]){
    
    cout << "argv[0] = " << argv[0] << " argv[1] = " << argv[1] << endl;

    string imgPath = argv[1];
    Mat imgOri = imread(imgPath);
    
    //get basic information of the image
    int heightOfOri = imgOri.rows;
    int widthOfOri  = imgOri.cols;

    //malloc the buffer for the bgr seperate
    vector<uint8_t*> inputVec;
    for(int i = 0; i < 3; i++){
        inputVec.push_back((uint8_t*)malloc(heightOfOri * widthOfOri));
    }

    function<void()> fun1 = bind(kernelByHand, inputVec, (uint8_t*)imgOri.data, heightOfOri, widthOfOri);
    string expOneName = "手动实现";
    timeTest(fun1, expOneName);

    function<void()> fun2 = bind(kernelByNeon, inputVec[0], inputVec[1], inputVec[2], (uint8_t*)imgOri.data, heightOfOri * widthOfOri);
    string expTwoName = "Neon优化";
    timeTest(fun2, expTwoName);
    //free the buffer 
    for(auto& it : inputVec){
        free(it);
    }


    return 0;
}
