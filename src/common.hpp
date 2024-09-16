#ifndef HGS_COMMON_HPP_
#define HGS_COMMON_HPP_

#include <cassert>

constexpr int PipelineStage = 12;
constexpr int DRAMWordsPerCycle = 2;
constexpr int TaskQueueSize = 12;
constexpr int PENum = 4;

// for DCache
constexpr int BankNum = 4;
constexpr int BankSize = (TaskQueueSize + PENum) / BankNum;
constexpr int PortWordsPerCycle = 4;
constexpr int PortNum = 4;
constexpr int CacheWordsPerCycle = PortWordsPerCycle * PortNum;
// for Task
constexpr int MaxTaskSize = 32;
// for Scheduler
constexpr int MaxLeafBufferSize = 32;

#endif // HGS_COMMON_HPP_