#pragma once

#include <cstddef>

// 内嵌 DLL 数据（用脚本从 Payload.dll 生成 payload_dll_data.cpp 后编译即可）
extern const unsigned char g_embedded_payload_dll[];
extern const size_t g_embedded_payload_dll_size;
