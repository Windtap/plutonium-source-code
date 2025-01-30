struct segment_t
{
    void  *segment_start;
    size_t segment_size;

    int perms;
};

struct remap_t
{
    void *mmap_function;
    void *mmap64_function;
    void *mprotect_function;
    void *mincore_function;
    void *malloc_function;

    void *usleep_function;

    void *prctl_function;

    void *zygisk_module_bytes;
    void *zygisk_module_base;

    void *return_address;

    uint32_t zygisk_module_size;

    segment_t *segments;
    size_t     segments_count;

    size_t load_size;

    const char *bss_vma_name;
    const char *module_vma_name;
};

void *remap_thread_shellcode(void *arg)
{
    auto *_remap = (remap_t *) arg;

    if (_remap)
    {
        void *retaddr = _remap->return_address;

        typedef void *(*mmap_function_t)(void *, size_t, int, int, int, off_t);
        typedef void *(*malloc_function_t)(size_t);

        typedef int (*mprotect_function_t)(const void *, size_t, int);
        typedef int (*mincore_function_t)(void *, size_t, unsigned char *);

        typedef int (*usleep_function_t)(useconds_t);
        typedef int (*prctl_function_t)(int option, ...);

        auto _mmap     = (mmap_function_t)     _remap->mmap_function;
        auto _mmap64   = (mmap_function_t)     _remap->mmap64_function;
        auto _malloc   = (malloc_function_t)   _remap->malloc_function;
        auto _mprotect = (mprotect_function_t) _remap->mprotect_function;
        auto _mincore  = (mincore_function_t)  _remap->mincore_function;

        auto _usleep   = (usleep_function_t)   _remap->usleep_function;
        auto _prctl    = (prctl_function_t)    _remap->prctl_function;

        void    *zygisk_module_base = _remap->zygisk_module_base;
        uint32_t zygisk_module_size = _remap->zygisk_module_size;

        segment_t *segments       = _remap->segments;
        size_t     segments_count = _remap->segments_count;

        char *buffer = (char *)  _remap->zygisk_module_bytes;
        auto *header = (Elf64_Ehdr *) buffer;
        auto *phdr   = (Elf64_Phdr *) (buffer + header->e_phoff);

        while (true)
        {
            if (zygisk_module_base)
            {
                auto *vec            = (unsigned char *) _malloc(sizeof(unsigned char));
                int   mincore_result = _mincore(zygisk_module_base, 4096, vec);

                if (mincore_result == -1 || vec[0] == 0)
                {
                    void *remapped_module = _mmap(zygisk_module_base, _remap->load_size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                    uintptr_t load_bias   = (uintptr_t) remapped_module;

                    auto *remapped_module_bytes = (uint8_t *) remapped_module;
                    auto *zygisk_module_bytes   = (uint8_t *) _remap->zygisk_module_bytes;

                    for (int i = 0; i < header->e_phnum; i++)
                    {
                        Elf64_Addr seg_start = phdr[i].p_vaddr + load_bias;
                        Elf64_Addr seg_end = seg_start + phdr[i].p_memsz;

                        Elf64_Addr seg_page_start = page_start(seg_start);
                        Elf64_Addr seg_page_end = page_end(seg_end);

                        Elf64_Addr seg_file_end = seg_start + phdr[i].p_filesz;

                        Elf64_Addr file_start = phdr[i].p_offset;
                        Elf64_Addr file_end = file_start + phdr[i].p_filesz;

                        Elf64_Addr file_page_start = page_start(file_start);
                        Elf64_Addr file_length = file_end - file_page_start;

                        // Allocate memory for a segment (rwx)
                        _mmap64(reinterpret_cast<void *>(seg_page_start), file_length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

                        // Copy section data

                        seg_file_end = page_end(seg_file_end);
                        if (seg_page_end > seg_file_end)
                        {
                            size_t zeromap_size = seg_page_end - seg_file_end;
                            void* zeromap = _mmap(reinterpret_cast<void*>(seg_file_end), zeromap_size,
                                                  PROT_READ | PROT_WRITE,
                                                  MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE,
                                                  -1,
                                                  0);

                            _prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, zeromap, zeromap_size, _remap->bss_vma_name);
                        }
                    }

                    for (size_t i{}; i < zygisk_module_size; i++)
                    {
                        remapped_module_bytes[i] = zygisk_module_bytes[i];
                    }

                    for (size_t i{}; i < segments_count; i++)
                    {
                        segment_t &_segment = segments[i];

                        if (_segment.perms & PROT_EXEC)
                        {
                            _prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, _segment.segment_start, _segment.segment_size, _remap->module_vma_name);
                        }

                        _mprotect(_segment.segment_start, _segment.segment_size, _segment.perms);
                    }

                    return retaddr;
                }
            }

            _usleep(100);
        }
    }

    return nullptr;
}

namespace shellcode_executor
{
    std::atomic<bool>  shellcode_called{};

    void *g_shellcode{};

    class shellcode_t
    {
    private:
        void *shellcode{nullptr};
        void *fake_shellcode{nullptr};
        void *fake_shellcode_1{nullptr};
        void *fake_shellcode_2{nullptr};
        void *fake_shellcode_3{nullptr};
        void *fake_shellcode_4{nullptr};
        bool fake_bool{false};
        void *fake_shellcode_5{nullptr};
        void *fake_shellcode_6{nullptr};

    public:
        void *get_shellcode()
        {
            if (!shellcode)
            {
                abort();
            }

            return shellcode;
        }

        void allocate_shellcode()
        {
            void *m_shellcode = mmap64(nullptr, sizeof(function_shellcode), PROT_READ | PROT_WRITE | PROT_EXEC, 34, -1, 0);
            memcpy(m_shellcode, function_shellcode, sizeof(function_shellcode));
            shellcode = m_shellcode;

            LOGD("shellcode: %p", m_shellcode);
            prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, shellcode, sizeof(function_shellcode), "libc_malloc");
        }

        shellcode_t() = default;
    };

    shellcode_t _shellcode;

    void *get_shellcode()
    {
        return _shellcode.get_shellcode();
    }

    void allocate_shellcode()
    {
        _shellcode.allocate_shellcode();
    }
}
