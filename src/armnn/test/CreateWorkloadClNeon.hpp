//
// Copyright © 2017 Arm Ltd. All rights reserved.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include "CreateWorkload.hpp"
#include "backends/RefWorkloadFactory.hpp"

#if ARMCOMPUTECL_ENABLED
#include "backends/ClTensorHandle.hpp"
#endif

#if ARMCOMPUTENEON_ENABLED
#include "backends/NeonTensorHandle.hpp"
#endif


using namespace armnn;

namespace
{

using namespace std;

template<typename IComputeTensorHandle>
boost::test_tools::predicate_result CompareTensorHandleShape(IComputeTensorHandle*               tensorHandle,
                                                             std::initializer_list<unsigned int> expectedDimensions)
{
    arm_compute::ITensorInfo* info = tensorHandle->GetTensor().info();

    auto infoNumDims = info->num_dimensions();
    auto numExpectedDims = expectedDimensions.size();
    if (infoNumDims != numExpectedDims)
    {
        boost::test_tools::predicate_result res(false);
        res.message() << "Different number of dimensions [" << info->num_dimensions()
                      << "!=" << expectedDimensions.size() << "]";
        return res;
    }

    size_t i = info->num_dimensions() - 1;

    for (unsigned int expectedDimension : expectedDimensions)
    {
        if (info->dimension(i) != expectedDimension)
        {
            boost::test_tools::predicate_result res(false);
            res.message() << "Different dimension [" << info->dimension(i) << "!=" << expectedDimension << "]";
            return res;
        }

        i--;
    }

    return true;
}

template<template <DataType> class CopyFromCpuWorkload, template <DataType> class CopyToCpuWorkload,
    typename IComputeTensorHandle>
void CreateMemCopyWorkloads(IWorkloadFactory& factory)
{
    Graph graph;
    RefWorkloadFactory refFactory;

    // create the layers we're testing
    Layer* const layer1 = graph.AddLayer<MemCopyLayer>("layer1");
    Layer* const layer2 = graph.AddLayer<MemCopyLayer>("layer2");

    // create extra layers
    Layer* const input = graph.AddLayer<InputLayer>(0, "input");
    Layer* const output = graph.AddLayer<OutputLayer>(0, "output");

    // connect up
    TensorInfo tensorInfo({2, 3}, DataType::Float32);
    Connect(input, layer1, tensorInfo);
    Connect(layer1, layer2, tensorInfo);
    Connect(layer2, output, tensorInfo);

    input->CreateTensorHandles(graph, refFactory);
    layer1->CreateTensorHandles(graph, factory);
    layer2->CreateTensorHandles(graph, refFactory);
    output->CreateTensorHandles(graph, refFactory);

    // make the workloads and check them
    auto workload1 = MakeAndCheckWorkload<CopyFromCpuWorkload<DataType::Float32>>(*layer1, graph, factory);
    auto workload2 = MakeAndCheckWorkload<CopyToCpuWorkload<DataType::Float32>>(*layer2, graph, refFactory);

    MemCopyQueueDescriptor queueDescriptor1 = workload1->GetData();
    BOOST_TEST(queueDescriptor1.m_Inputs.size() == 1);
    BOOST_TEST(queueDescriptor1.m_Outputs.size() == 1);
    auto inputHandle1  = boost::polymorphic_downcast<ConstCpuTensorHandle*>(queueDescriptor1.m_Inputs[0]);
    auto outputHandle1 = boost::polymorphic_downcast<IComputeTensorHandle*>(queueDescriptor1.m_Outputs[0]);
    BOOST_TEST((inputHandle1->GetTensorInfo() == TensorInfo({2, 3}, DataType::Float32)));
    BOOST_TEST(CompareTensorHandleShape<IComputeTensorHandle>(outputHandle1, {2, 3}));


    MemCopyQueueDescriptor queueDescriptor2 = workload2->GetData();
    BOOST_TEST(queueDescriptor2.m_Inputs.size() == 1);
    BOOST_TEST(queueDescriptor2.m_Outputs.size() == 1);
    auto inputHandle2  = boost::polymorphic_downcast<IComputeTensorHandle*>(queueDescriptor2.m_Inputs[0]);
    auto outputHandle2 = boost::polymorphic_downcast<CpuTensorHandle*>(queueDescriptor2.m_Outputs[0]);
    BOOST_TEST(CompareTensorHandleShape<IComputeTensorHandle>(inputHandle2, {2, 3}));
    BOOST_TEST((outputHandle2->GetTensorInfo() == TensorInfo({2, 3}, DataType::Float32)));
}

}