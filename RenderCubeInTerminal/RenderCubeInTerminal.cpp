﻿#include <chrono>
#include "PseudoRenderer.h"
#include "Renderer.h"
#include "SWRasterizer.h"
#include "DynamicMemoryPool.hpp"
#include "Math.h"
#include "LinkedList.hpp"
#include "TimeStamper.h"

#include <Windows.h>

#include <mutex>
#include <thread>
#include <processthreadsapi.h>


using namespace std;

void ProcessPseudoRenderer();

int main() { 
    // main render loop
    Renderer renderer;
    renderer.Initialize();

    std::chrono::steady_clock::time_point prevFrameSec = std::chrono::high_resolution_clock::now();
    while (true) {
        prevFrameSec = renderer.Frame(prevFrameSec);
    }

    renderer.Terminate();
}


void ProcessPseudoRenderer() {
    PseudoRenderer renderer;
    renderer.Initialize();

    std::chrono::steady_clock::time_point prevFrameSec = std::chrono::high_resolution_clock::now();
    while (true) {
        prevFrameSec = renderer.Frame(prevFrameSec);
    }

    renderer.Terminate();
}