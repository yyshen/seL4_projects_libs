/*
 * Copyright 2019, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

int
vm_install_ram_only_device(vm_t *vm, const struct device* device) {
    struct device d;
    uintptr_t paddr;
    int err;
    d = *device;
    for (paddr = d.pstart; paddr - d.pstart < d.size; paddr += 0x1000) {
        void* addr;
        addr = map_vm_ram(vm, paddr);
        if (!addr) {
            return -1;
        }
    }
    err = vm_add_device(vm, &d);
    assert(!err);
    return err;
}

static memory_fault_result_t
passthrough_device_fault(vm_t* vm, uintptr_t fault_addr, size_t fault_length, void *cookie) {
    ZF_LOGE("Fault occured on passthrough device");
    return FAULT_ERROR;
}

int
vm_install_passthrough_device(vm_t* vm, const struct device* device)
{
    struct device d;
    uintptr_t paddr;
    int err;
    d = *device;
    for (paddr = d.pstart; paddr - d.pstart < d.size; paddr += 0x1000) {
        void* addr;
        vm_memory_reservation_t *reservation;
        reservation = vm_reserve_memory_at(vm, paddr, 0x1000, passthrough_device_fault, NULL);
        if (!reservation) {
            return -1;
        }
        err = map_ut_alloc_reservation(vm, reservation);
#ifdef PLAT_EXYNOS5
        if (err && paddr == MCT_ADDR) {
            printf("*****************************************\n");
            printf("*** Linux will try to use the MCT but ***\n");
            printf("*** the kernel is not exporting it!   ***\n");
            printf("*****************************************\n");
            /* VMCT is not fully functional yet */
//            err = vm_install_vmct(vm);
            return -1;
        }
#endif
        if (err) {
            return -1;
        }
    }
    err = vm_add_device(vm, &d);
    assert(!err);
    return err;
}

static int
handle_listening_fault(struct device* d, vm_t* vm,
                       fault_t* fault)
{
    volatile uint32_t *reg;
    int offset;
    void** map;

    assert(d->priv);
    map = (void**)d->priv;
    offset = fault_get_address(fault) - d->pstart;

    reg = (volatile uint32_t*)(map[offset >> 12] + (offset & MASK(12)));

    printf("[Listener/%s] ", d->name);
    if (fault_is_read(fault)) {
        printf("read ");
        fault_set_data(fault, *reg);
    } else {
        printf("write");
        *reg = fault_emulate(fault, *reg);
    }
    printf(" ");
    fault_print_data(fault);
    printf(" address %p @ pc %p\n", (void *) fault_get_address(fault),
           (void *) fault_get_ctx(fault)->pc);
    return advance_fault(fault);
}


int
vm_install_listening_device(vm_t* vm, const struct device* dev_listening)
{
    struct device d;
    int pages;
    int i;
    void** map;
    int err;
    pages = dev_listening->size >> 12;
    d = *dev_listening;
    d.handle_page_fault = handle_listening_fault;
    /* Build device memory map */
    map = (void**)malloc(sizeof(void*) * pages);
    if (map == NULL) {
        return -1;
    }
    d.priv = map;
    for (i = 0; i < pages; i++) {
        map[i] = map_device(&vm->mem.vmm_vspace, vm->vka, vm->simple,
                            d.pstart + (i << 12), 0, seL4_AllRights);
    }
    err = vm_add_device(vm, &d);
    return err;
}


static int
handle_listening_ram_fault(struct device* d, vm_t* vm, fault_t* fault)
{
    volatile uint32_t *reg;
    int offset;

    assert(d->priv);
    offset = fault_get_address(fault) - d->pstart;

    reg = (volatile uint32_t*)(d->priv + offset);

    if (fault_is_read(fault)) {
        fault_set_data(fault, *reg);
    } else {
        *reg = fault_emulate(fault, *reg);
    }
    printf("Listener pc%p| %s%p:%p\n", (void *) fault_get_ctx(fault)->pc,
                                       fault_is_read(fault) ? "r" : "w",
                                       (void *) fault_get_address(fault),
                                       (void *) fault_get_data(fault));
    return advance_fault(fault);
}


const struct device dev_listening_ram = {
    .devid = DEV_CUSTOM,
    .name = "<listing_ram>",
    .pstart = 0x0,
    .size = 0x1000,
    .handle_page_fault = handle_listening_ram_fault,
    .priv = NULL
};


int
vm_install_listening_ram(vm_t* vm, uintptr_t addr, size_t size)
{
    struct device d;
    int err;
    d = dev_listening_ram;
    d.pstart = addr;
    d.size = size;
    d.priv = malloc(0x1000);
    assert(d.priv);
    if (!d.priv) {
        printf("malloc failed\n");
        return -1;
    }
    err = vm_add_device(vm, &d);
    assert(!err);
    if (err) {
        printf("alloc failed\n");
    }
    return err;
}