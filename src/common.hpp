#ifndef HGS_COMMON_HPP_
#define HGS_COMMON_HPP_

#include <cassert>
#include <iostream>

#include "half.hpp"
#include "types.hpp"

constexpr int PipelineStage = 18;
constexpr int CutSelectStage = 12;
constexpr int WeightCalcStage = PipelineStage - CutSelectStage;
constexpr int DRAMWordsPerCycle = 4;
constexpr int TaskQueueSize = 12;
constexpr int PENum = 4;

// for DCache
constexpr int BankNum = 4;
constexpr int BankSize = 8 * (TaskQueueSize + PENum) / BankNum;
constexpr int PortWordsPerCycle = 4;
constexpr int PortNum = 4;
constexpr int CacheWordsPerCycle = PortWordsPerCycle * PortNum;
constexpr int BufferCacheSize = 8;
// for Task
constexpr int MaxTaskSize = 32;
constexpr int MaxSubtaskSize = 32;
constexpr int SizeOfTask = sizeof(int) + sizeof(int) + sizeof(Box) + sizeof(int);
constexpr int SizeofBox = sizeof(Point4) + sizeof(Point4);
constexpr int SizeOfNode = sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(float) + sizeof(int) + sizeof(bool);
constexpr int SizeOfSubtask = MaxSubtaskSize * (sizeof(int) + sizeof(int) + sizeof(int));
constexpr int SizeOfCacheData = SizeOfTask + SizeOfSubtask + MaxTaskSize * SizeOfNode + MaxTaskSize * SizeofBox;
// for Scheduler
constexpr int MaxLeafBufferSize = 32;
#endif // HGS_COMMON_HPP_