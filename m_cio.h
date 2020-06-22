#pragma once

#include <Windows.h>
#include <functional>

#define $$ &&

void setcolor(int color, int bgcolor){
    color &= 0xf;
    bgcolor &= 0xf;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (bgcolor << 4) | color);
}

void withcolor(int color, int bgcolor, std::function<void()> func) {
    setcolor(color, bgcolor);
    func();
    setcolor(7, 0);
    return;
}

void err(std::string message) {
    setcolor(4, 0);
    std::cout << message;
    setcolor(7, 0);
}

void scss(std::string message) {
    setcolor(10, 0);
    std::cout << message;
    setcolor(7, 0);
}