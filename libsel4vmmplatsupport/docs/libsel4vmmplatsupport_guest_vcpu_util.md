<!--
     Copyright 2020, Data61
     Commonwealth Scientific and Industrial Research Organisation (CSIRO)
     ABN 41 687 119 230.

     This software may be distributed and modified according to the terms of
     the BSD 2-Clause license. Note that NO WARRANTY is provided.
     See "LICENSE_BSD2.txt" for details.

     @TAG(DATA61_BSD)
-->

## Interface `guest_vcpu_util.h`

The guest vcpu util interface provides abstractions and helpers for managing libsel4vm vcpus.

### Brief content:

**Functions**:

> [`create_vmm_plat_vcpu(vm, priority)`](#function-create_vmm_plat_vcpuvm-priority)


## Functions

The interface `guest_vcpu_util.h` defines the following functions.

### Function `create_vmm_plat_vcpu(vm, priority)`

Create a new platform vcpu. This is a wrapper around the libsel4vm function `vm_create_vcpu` however
further intialises and configures the vcpu with platform specific attributes

**Parameters:**

- `vm {vm_t *}`: A handle to the VM
- `priority {int}`: Priority of the new vcpu thread

**Returns:**

- NULL for error, otherwise pointer to created vm_vcpu_t object

Back to [interface description](#module-guest_vcpu_utilh).


Back to [top](#).

