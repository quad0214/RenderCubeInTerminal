#include <chrono>
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

void TestSIMD() {

    srand(time(NULL));

    // 1 : same opearation with base datatype
    {
        alignas(16) float a, b, c, d;
        a = b = c = d = rand();

        float r = rand();

        a *= r;
        b *= r;
        c *= r;
        d *= r;
        cout << a << endl;
    }

    // 2 : same operation with array
    {
        float r1 = rand();
        float r2 = rand();
        alignas(16) float a[4]{r1, r1, r1, r1};

        a[0] *= r2;
        a[1] *= r2;
        a[2] *= r2;
        a[3] *= r2;
        cout << a[3] << endl;
    }

    // 3 : same operation with m128
    {
        float r1 = rand();
        float r2 = rand();

        alignas(16) float a[4]{ r1, r1, r1, r1 };
        alignas(16) float b[4]{ r2, r2, r2, r2 };        
        __m128 m1 = _mm_load_ps(a);
        __m128 m2 = _mm_load_ps(b);
        m1.m128_f32[0] *= m1.m128_f32[2];
        m1.m128_f32[1] *= m1.m128_f32[2];
        m1.m128_f32[2] *= m1.m128_f32[2];
        m1.m128_f32[3] *= m1.m128_f32[2];
        cout << m1.m128_f32[3] << endl;
    }

    // 4 : simd
    {
        float r1 = rand();
        float r2 = rand();

        alignas(16) float a[4]{ r1, r1, r1, r1 };
        alignas(16) float b[4]{ r2, r2, r2, r2 };
        __m128 m1 = _mm_load_ps(a);
        __m128 m2 = _mm_load_ps(b);
        __m128 m3 = _mm_mul_ps(m1, m2);
        alignas(16) float r[4];
        _mm_store_ps(r, m3);
        cout << r[3] << endl;
    }

}

int main() { 
    TestSIMD();

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