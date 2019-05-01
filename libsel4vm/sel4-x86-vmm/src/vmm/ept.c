/*
 * Copyright 2017, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(DATA61_GPL)
 */

/*vm exits related with ept violations*/

#include <stdio.h>
#include <stdlib.h>

#include <sel4/sel4.h>

#include "sel4vm/debug.h"
#include "sel4vm/vmm.h"
#include "sel4vm/platform/vmcs.h"
#include "sel4vm/mmio.h"

/* Handling EPT violation VMExit Events. */
int vmm_ept_violation_handler(vm_vcpu_t *vcpu) {

    uintptr_t guest_phys = vmm_guest_exit_get_physical(&vcpu->vcpu_arch.guest_state);
    unsigned int qualification = vmm_guest_exit_get_qualification(&vcpu->vcpu_arch.guest_state);

    int e = vmm_mmio_exit_handler(vcpu, guest_phys, qualification);

    if (e == 0) {
        DPRINTF(5, "EPT violation handled by mmio\n");
    } else {
        /* Read linear address that guest is trying to access. */
        unsigned int linear_address = vmm_vmcs_read(vcpu->vm_vcpu.cptr, VMX_DATA_GUEST_LINEAR_ADDRESS);
        printf(COLOUR_R "!!!!!!!! ALERT :: GUEST OS PAGE FAULT !!!!!!!!\n");
        printf("    Guest OS VMExit due to EPT Violation:\n");
        printf("        Linear address 0x%x.\n", linear_address);
        printf("        Guest-Physical address 0x%x.\n", vmm_guest_exit_get_physical(&vcpu->vcpu_arch.guest_state));
        printf("        Instruction pointer 0x%x.\n", vmm_guest_state_get_eip(&vcpu->vcpu_arch.guest_state));
        printf("    This is most likely due to a bug or misconfiguration.\n" COLOUR_RESET);
    }

#ifndef CONFIG_VMM_IGNORE_EPT_VIOLATION
    printf(COLOUR_R "    The faulting Guest OS thread will now be blocked forever.\n" COLOUR_RESET);
    return -1;
#else
    vmm_guest_exit_next_instruction(&vcpu->vcpu_arch.guest_state, vcpu->vm_vcpu.cptr);
    return 0;
#endif
}