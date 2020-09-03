#pragma once
#include <thread>

using ThreadID = std::thread::id;

pid_t RawTid();

ThreadID Tid();
uint64_t Tid_64();
uint64_t Tid_64(const ThreadID& id);

bool IsMainThread();
