//
// Created by Admin on 24.04.2024.
//
#define _GNU_SOURCE
#include <cstdio>
#include <cstdlib>
#include <link.h>

struct dl_error
{
    const char *error;

    dl_error() { error = ("cannot find"); }
    dl_error(const char *e) { error = e; }
};


struct section_data
{
    const char *name;
    uintptr_t address;
    bool founded;

    section_data (const char *n) { name = n; }
};

class Library
{
private:
    const char *library_name;
    uintptr_t address;
    bool founded;
    std::vector<dl_error *> errors;
public:

    Library(const char *library_name);

    static int callback(struct dl_phdr_info *info, size_t size, void *data);

    virtual void GetLibrary();

    virtual uintptr_t GetAddress();

    virtual std::vector<dl_error *> GetErrors();

    virtual int phdr_iterator(int (*callback)(dl_phdr_info *info, size_t size, void *data), section_data *data);

    virtual bool Loaded()
    {
        return founded;
    }
};

int Library::callback(struct dl_phdr_info *info, size_t size, void *data)
{
    const char *name = info->dlpi_name;
    section_data *sectionData = (section_data *) data;
    if (strstr(name, sectionData->name))
    {
        sectionData->address = info->dlpi_addr;
        sectionData->founded = true;
        return 1;
    }

    return 0;
}

int Library::phdr_iterator(int (*callback)(dl_phdr_info *info, size_t size, void *data), section_data *data)
{
    return dl_iterate_phdr(callback, (void *) data);
}

void Library::GetLibrary()
{
    section_data *data = new section_data(this->library_name);

    if (!this->phdr_iterator(callback, data))
    {
        dl_error *error = new dl_error();
        this->errors.push_back(error);
    }
    else
    {
        this->address = data->address;
        this->founded = data->founded;
    }

    delete data;
}

Library::Library(const char *library_name)
{
    this->library_name = library_name;

    this->GetLibrary();
}

uintptr_t Library::GetAddress()
{
    return this->address;
}

std::vector<dl_error *> Library::GetErrors()
{
    return this->errors;
}