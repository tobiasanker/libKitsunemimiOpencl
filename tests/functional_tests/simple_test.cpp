#include "simple_test.h"

#include <libKitsunemimiOpencl/opencl.h>

namespace Kitsunemimi
{
namespace Opencl
{

SimpleTest::SimpleTest()
    : Kitsunemimi::CompareTestHelper("SimpleTest")
{
    simple_test();
}

void
SimpleTest::simple_test()
{
    const size_t N = 1 << 27;

    // example kernel for task: c = a + b.
    const std::string kernelCode =
        "__kernel void add(\n"
        "       __global const float* a,\n"
        "       ulong n1,\n"
        "       __global const float* b,\n"
        "       ulong n2,\n"
        "       __global float* c,\n"
        "       ulong out\n"
        "       )\n"
        "{\n"
        "    __local float temp[512];\n"
        "    size_t globalId_x = get_global_id(0);\n"
        "    int localId_x = get_local_id(0);\n"
        "    size_t globalSize_x = get_global_size(0);\n"
        "    size_t globalSize_y = get_global_size(1);\n"
        "    \n"
        "    size_t globalId = get_global_id(0) + get_global_size(0) * get_global_id(1);\n"
        "    if (globalId < n1)\n"
        "    {\n"
        "       temp[localId_x] = b[globalId];\n"
        "       c[globalId] = a[globalId] + temp[localId_x];"
        "    }\n"
        "}\n";

    Kitsunemimi::Opencl::Opencl ocl;

    // create config-object
    Kitsunemimi::Opencl::OpenClConfig config;
    config.kernelCode = kernelCode;
    config.kernelName = "add";

    // create data-object
    Kitsunemimi::Opencl::OpenClData data;

    data.numberOfWg.x = N / 512;
    data.numberOfWg.y = 2;
    data.threadsPerWg.x = 256;

    // init empty buffer
    data.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer(N, sizeof(float), false, true));
    data.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer(N, sizeof(float), false, true));
    data.buffer.push_back(Kitsunemimi::Opencl::WorkerBuffer(N, sizeof(float), true, true));

    // convert pointer
    float* a = static_cast<float*>(data.buffer[0].data);
    float* b = static_cast<float*>(data.buffer[1].data);

    // write intput dat into buffer
    for(uint32_t i = 0; i < N; i++)
    {
        a[i] = 1.0f;
        b[i] = 2.0f;
    }

    // run
    TEST_EQUAL(ocl.init(config), true);
    ocl.copyToDevice(data);
    ocl.run(data);
    ocl.copyFromDevice(data);

    // check result
    float* outputValues = static_cast<float*>(data.buffer[2].data);
    TEST_EQUAL(outputValues[42], 3.0f);;

    // update data on host
    for(uint32_t i = 0; i < N; i++)
    {
        a[i] = 5.0f;
    }

    // update data on device
    TEST_EQUAL(ocl.updateBuffer(data.buffer[0]), true);

    // second run
    TEST_EQUAL(ocl.run(data), true);
    // copy new output back
    TEST_EQUAL(ocl.copyFromDevice(data), true);

    // check new result
    outputValues = static_cast<float*>(data.buffer[2].data);
    TEST_EQUAL(outputValues[42], 7.0f);

    TEST_NOT_EQUAL(ocl.getLocalMemorySize(), 0);
    TEST_NOT_EQUAL(ocl.getGlobalMemorySize(), 0);
    TEST_NOT_EQUAL(ocl.getMaxMemAllocSize(), 0);

    TEST_NOT_EQUAL(ocl.getMaxWorkGroupSize(), 0);
    TEST_NOT_EQUAL(ocl.getMaxWorkItemDimension(), 0);
    TEST_NOT_EQUAL(ocl.getMaxWorkItemSize().x, 0);
    TEST_NOT_EQUAL(ocl.getMaxWorkItemSize().y, 0);
    TEST_NOT_EQUAL(ocl.getMaxWorkItemSize().z, 0);
}

}
}
