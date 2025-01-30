#include <cstring>
#include <thread>
#include "includes/plutonium.h"
#include "includes/zygisk.hpp"
#include "includes/game.h"
#include "includes/log.h"
#include "dlfcn.h"
#include <sys/mman.h>
#include <unistd.h>
#include "includes/utils.h"
#include <linux/elf.h>
#include <sys/prctl.h>

#include <dobby/dobby.h>

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

__attribute__((__always_inline__))
constexpr size_t page_size() {
    return PAGE_SIZE;
}

// Returns the address of the page containing address 'x'.
__attribute__((__always_inline__))
inline uintptr_t page_start(uintptr_t x) {
    return x & ~(page_size() - 1);
}

// Returns the offset of address 'x' in its page.
__attribute__((__always_inline__))
inline uintptr_t page_offset(uintptr_t x) {
    return x & (page_size() - 1);
}

// Returns the address of the next page after address 'x', unless 'x' is
// itself at the start of a page.
__attribute__((__always_inline__))
inline uintptr_t page_end(uintptr_t x) {
    return page_start(x + page_size() - 1);
}
#include <sys/user.h>

size_t phdr_table_get_load_size(const Elf64_Phdr* phdr_table, size_t phdr_count, Elf64_Addr* out_min_vaddr, Elf64_Addr* out_max_vaddr)
{
    Elf64_Addr min_vaddr = UINTPTR_MAX;
    Elf64_Addr max_vaddr = 0;
    bool found_pt_load = false;
    for (size_t i = 0; i < phdr_count; ++i) {
        const Elf64_Phdr* phdr = &phdr_table[i];
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        found_pt_load = true;
        if (phdr->p_vaddr < min_vaddr) {
            min_vaddr = phdr->p_vaddr;
        }
        if (phdr->p_vaddr + phdr->p_memsz > max_vaddr) {
            max_vaddr = phdr->p_vaddr + phdr->p_memsz;
        }
    }
    if (!found_pt_load) {
        min_vaddr = 0;
    }
    min_vaddr = page_start(min_vaddr);
    max_vaddr = page_end(max_vaddr);
    if (out_min_vaddr != nullptr) {
        *out_min_vaddr = min_vaddr;
    }
    if (out_max_vaddr != nullptr) {
        *out_max_vaddr = max_vaddr;
    }
    return max_vaddr - min_vaddr;
}

uintptr_t find_in_memory(const char* str, const char* substr, size_t str_len)
{
    if (!str || !substr) { return 0x0; }

    size_t substr_len = strlen(substr);

    if (substr_len > str_len)
    {
        return 0x0;
    }

    for (size_t i = 0; i <= str_len - substr_len; ++i)
    {
        size_t j;
        for (j = 0; j < substr_len; ++j)
        {
            if (str[i + j] != substr[j])
            {
                break;
            }
        }

        if (j == substr_len)
        {
            return (uintptr_t) (str + i + substr_len);
        }
    }

    return 0x0;
}

#include "includes/shellcode.h"
#include "plutonium/hooks/plthook.h"
#include <map>
#include <functional>
#include <elf.h>
#include <sys/wait.h>
#include <dirent.h>
#include "includes/shellcode_exec.h"

uintptr_t load_bias{};

#include <inttypes.h>
void hook_syscalls(void *start_address, size_t size, void *_new, void **orig)
{
    LOGD("start address: %p", start_address);
    uint8_t svc[] = { 0x01, 0x00, 0x00, 0xd4};
    uint8_t mov[] = { 0x08, 0x07, 0x80, 0x52};

    if (start_address)
    {
        for (int i{}; i < size; i++)
        {
            uint8_t opcode = * (uint8_t *) ((uintptr_t) start_address + i);

            if (opcode == svc[0])
            {
                if (* (uint8_t *) ((uintptr_t) start_address + i + 1) != svc[1]) { continue; }
                if (* (uint8_t *) ((uintptr_t) start_address + i + 2) != svc[2]) { continue; }
                if (* (uint8_t *) ((uintptr_t) start_address + i + 3) != svc[3]) { continue; }

                opcode = * (uint8_t *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 3);

                if (opcode == mov[0])
                {
                    if (* (uint8_t *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 3 + 1) != mov[1]) { continue; }
                    if (* (uint8_t *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 3 + 2) != mov[2]) { continue; }
                    if (* (uint8_t *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 3 + 3) != mov[3]) { continue; }

                    LOGD("founded syscall, hooking offset: %p", i - sizeof(uint32_t) * 3);

                    DobbyHook((void *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 3), _new, orig);
                }

                opcode = * (uint8_t *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 2);

                if (opcode == mov[0])
                {
                    if (* (uint8_t *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 2 + 1) != mov[1]) { continue; }
                    if (* (uint8_t *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 2 + 2) != mov[2]) { continue; }
                    if (* (uint8_t *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 2 + 3) != mov[3]) { continue; }

                    LOGD("founded syscall, hooking offset: %p", i - sizeof(uint32_t) * 2);
                    DobbyHook((void *) ((uintptr_t) start_address + i - sizeof(uint32_t) * 2), _new, orig);
                }
            }
        }
    }
}

void *cheat_thread(void *remap_ptr)
{
    LOGD("%s called", "shellcode_exec_thread");

    shellcode_executor::allocate_shellcode();
    void *shellcode = shellcode_executor::get_shellcode();
    typedef void *(*shellcode_function_t)(void *);
    auto shellcode_function = (shellcode_function_t) shellcode;

    if (!shellcode)
    {
        LOGE("shellcode is NULL");
        abort();
    }

    shellcode_function(remap_ptr);

    auto *_remap    = (remap_t *) remap_ptr;
    load_bias = (uintptr_t) _remap->zygisk_module_base;
    LOGD("load bias: %p", load_bias);

    auto *founded_text = (char *) find_in_memory(reinterpret_cast<const char *>(load_bias), "libpenig_gisk_zygisk.so", _remap->zygisk_module_size);
    uint32_t    offset = founded_text - (char *) load_bias;
    LOGD("text offset: %p", offset);

    mprotect(reinterpret_cast<void *>(load_bias), offset, 7);
    memset(reinterpret_cast<void *const>(load_bias), 0, offset);
    mprotect(reinterpret_cast<void *>(load_bias), offset, 5);

    munmap(remap_ptr, sizeof(remap_t));
    munmap(shellcode, sizeof(function_shellcode));
    LOGD("shellcode addr: %p", shellcode);

    hack_prepare();

    return nullptr;
}

class plutonium_remapper : public zygisk::ModuleBase
{
public:
    void onLoad(Api *api, JNIEnv *env) override
    {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override
    {
        auto package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        auto app_data_dir = env->GetStringUTFChars(args->app_data_dir, nullptr);
        preSpecialize(package_name, app_data_dir);
        env->ReleaseStringUTFChars(args->nice_name, package_name);
        env->ReleaseStringUTFChars(args->app_data_dir, app_data_dir);
    }

    void postAppSpecialize(const AppSpecializeArgs *) override
    {
        if (enable_hack)
        {
            Dl_info info;
            dladdr((void *) remap_thread_shellcode, &info);
            LOGD("module base: %p", info.dli_fbase);

            std::vector<utils::module_info> modules = utils::find_modules(info.dli_fbase);
            auto *segments = (segment_t *) malloc(sizeof(segment_t) * modules.size());

            uint32_t zygisk_module_size = (uintptr_t) modules[modules.size() - 1].end_address - (uintptr_t) modules[0].start_address;
            LOGD("module size: %d (%p)", zygisk_module_size, zygisk_module_size);

            void *zygisk_module_bytes = malloc(zygisk_module_size);
            LOGD("module bytes allocated at %p", zygisk_module_bytes);

            for (int i{}; i < modules.size(); i++)
            {
                segment_t _segment = {
                        .segment_start = modules[i].start_address,
                        .segment_size  = modules[i].size,
                        .perms         = modules[i].perms
                };

                if (_segment.perms == 0)
                {
                    mprotect(_segment.segment_start, _segment.segment_size, PROT_READ);
                    LOGD("mprotected region %p", (void *) ((uintptr_t) zygisk_module_bytes + modules[i - 1].size));
                }

                if (_segment.perms == 5)
                {
                    //mprotect(_segment.segment_start, _segment.segment_size, 7);
                    //LOGD("mprotected region %p to rwxp", (void *) ((uintptr_t) zygisk_module_bytes + modules[i - 1].size));
                }

                LOGD("[%d] segment info: \n  start: %p\n   size: %d\n   perms: %d", i, _segment.segment_start, _segment.segment_size, _segment.perms);

                segments[i] = _segment;
            }

            memcpy(zygisk_module_bytes, info.dli_fbase, zygisk_module_size);

            char *buffer = (char *) zygisk_module_bytes;
            auto *header = (Elf64_Ehdr *) buffer;
            auto *phdr   = (Elf64_Phdr *) (buffer + header->e_phoff);

            size_t pt_dynamic_offset = 0;
            size_t pt_dynamic_filesz = 0;
            for (int i = 0; i < header->e_phnum; i++)
            {
                if (phdr[i].p_type == PT_DYNAMIC)
                {
                    pt_dynamic_offset = phdr[i].p_offset;
                    pt_dynamic_filesz = phdr[i].p_filesz;
                }
            }

            LOGD("dynamic offset %d", pt_dynamic_offset);
            auto *dynamic = (Elf64_Dyn *) ((uintptr_t) info.dli_fbase + pt_dynamic_offset);

            void **fini_array{};
            size_t fini_array_count{};

            for (int i{};;i++)
            {
                if (dynamic[i].d_tag == DT_FINI_ARRAY)
                {
                    fini_array = (void **) ((uintptr_t) info.dli_fbase + dynamic[i].d_un.d_ptr); // DT_INIT_ARRAY will be relocated!!! So we will use it from address space of our mapped lib not ours where we mapped file into our memory
                    LOGD("[+] DT_FINI_ARRAY: %p", fini_array);
                }

                if (dynamic[i].d_tag == DT_FINI_ARRAYSZ)
                {
                    fini_array_count = dynamic[i].d_un.d_val / sizeof(Elf64_Addr);
                    LOGD("[+] DT_FINI_ARRAYSZ: %zu", fini_array_count);
                    break;
                }
            }

            mprotect(ALLIGN(fini_array), PAGE_SIZE * 2, 7);

            for (int i{}; i < fini_array_count; i++)
            {
                LOGD("replacing fini number %d", i);
                fini_array[i] = nullptr;
            }

            mprotect(ALLIGN(fini_array), PAGE_SIZE * 2, 5);

            Elf64_Addr min_vaddr;
            size_t load_size = phdr_table_get_load_size(phdr, header->e_phnum, &min_vaddr, nullptr);
            LOGD("load size: %d, vaddr: %p", load_size, min_vaddr);

            const char *bss_str      = ".bss\00";
            size_t      bss_str_size = strlen(bss_str) * sizeof(char);

            void* bss_vma_name = malloc(bss_str_size);
            memcpy(bss_vma_name, bss_str, bss_str_size);

            remap_t remap = {
                    .mmap_function     = (void *) dlsym(RTLD_DEFAULT, "mmap"),
                    .mmap64_function   = (void *) dlsym(RTLD_DEFAULT, "mmap64"),
                    .mprotect_function = (void *) dlsym(RTLD_DEFAULT, "mprotect"),
                    .mincore_function  = (void *) dlsym(RTLD_DEFAULT, "mincore"),
                    .malloc_function   = (void *) dlsym(RTLD_DEFAULT, "malloc"),
                    .usleep_function   = (void *) dlsym(RTLD_DEFAULT, "usleep"),
                    .prctl_function    = (void *) dlsym(RTLD_DEFAULT, "prctl"),

                    .zygisk_module_bytes = zygisk_module_bytes,
                    .zygisk_module_base  = info.dli_fbase,
                    .return_address      = (void *) cheat_thread,
                    .zygisk_module_size  = zygisk_module_size,

                    .segments       = segments,
                    .segments_count = modules.size(),

                    .load_size       = load_size,
                    .bss_vma_name    = (const char *) std::string("g").c_str(),

                    //по названию виртуального адреса в мапсах ща детектит ну если рандомное имя сделать мб норм будет и проживет чуть чуть мб еще права на rwxp поменять, не будет банить, но не долго
                    .module_vma_name = (const char *) std::string("p").c_str()
            };

            auto *remap_ptr = (remap_t *) malloc(sizeof(remap_t));
            memcpy(remap_ptr, &remap, sizeof(remap_t));

            LOGI("remap info: %p\n"
                 "  mmap: %p\n"
                 "  mprotect: %p\n"
                 "  mincore: %p\n"
                 "  malloc: %p\n"
                 "  usleep: %p\n"
                 "  module bytes: %p\n"
                 "  module base: %p\n"
                 "  module size: %d\n"
                 "  return address: %p\n"
                 "  segments count: %d",
                 remap_ptr,
                 remap_ptr->mmap_function,
                 remap_ptr->mprotect_function,
                 remap_ptr->mincore_function,
                 remap_ptr->malloc_function,
                 remap_ptr->usleep_function,
                 remap_ptr->zygisk_module_bytes,
                 remap_ptr->zygisk_module_base,
                 remap_ptr->zygisk_module_size,
                 remap_ptr->return_address,
                 remap_ptr->segments_count);

            pthread_t ptid;
            int result = pthread_create(&ptid, nullptr, ((void *(*)(void *)) cheat_thread), remap_ptr);

            LOGD("pthread created with result %d, unloading module", result);
            sleep(1);
        }

        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

private:
    Api *api;
    JNIEnv *env;
    bool enable_hack;
    char *game_data_dir;

    void preSpecialize(const char *package_name, const char *app_data_dir)
    {
        if (strcmp(package_name, GamePackageName) == 0)
        {
            LOGI("detect game: %s", package_name);
            enable_hack = true;
            game_data_dir = new char[strlen(app_data_dir) + 1];
            strcpy(game_data_dir, app_data_dir);
        }
    }
};

REGISTER_ZYGISK_MODULE(plutonium_remapper)