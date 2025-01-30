#ifndef ZYGISK_IMGUI_MODMENU_UTILS_H
#define ZYGISK_IMGUI_MODMENU_UTILS_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE(x, '0', '9') ? (x - '0') : ((x & (~0x20)) - 'A' + 0xa))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))

#if defined(__arm__)
#define process_vm_readv_syscall 376
#define process_vm_writev_syscall 377
#elif defined(__aarch64__)
#define process_vm_readv_syscall 270
#define process_vm_writev_syscall 271
#elif defined(__i386__)
#define process_vm_readv_syscall 347
#define process_vm_writev_syscall 348
#else
#define process_vm_readv_syscall 310
#define process_vm_writev_syscall 311
#endif

namespace utils {
    struct module_info {
        void* start_address = nullptr;
        void* end_address  = nullptr;
        size_t size = 0;
        std::string name;

        int perms;
    };
    int get_android_api_level();
    bool read_address(void *address, void *out_buffer, size_t length);
    bool write_address(void *address, void *in_buffer, size_t length);
    module_info find_module(const char *name);
    std::vector<module_info> find_modules(void *address);
    uintptr_t find_pattern(uint8_t* start, const size_t length, const char* pattern);
    uintptr_t find_pattern_in_module(const char* lib_name, const char* pattern);
}

#endif //ZYGISK_IMGUI_MODMENU_UTILS_H