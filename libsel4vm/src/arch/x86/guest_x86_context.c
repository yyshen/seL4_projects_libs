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

#include <sel4/sel4.h>

#include <sel4vm/guest_vm.h>
#include <sel4vm/guest_x86_context.h>
#include <sel4vm/vmcs.h>

#include "guest_state.h"

int vm_set_thread_context(vm_vcpu_t *vcpu, seL4_VCPUContext context) {
    MACHINE_STATE_DIRTY(vcpu->vcpu_arch.guest_state->machine.context);
    vcpu->vcpu_arch.guest_state->machine.context = context;
    return 0;
}

int vm_set_thread_context_reg(vm_vcpu_t *vcpu, vcpu_context_reg_t reg, uint32_t value) {
    MACHINE_STATE_DIRTY(vcpu->vcpu_arch.guest_state->machine.context);
    (&vcpu->vcpu_arch.guest_state->machine.context.eax)[reg] = value;
    return 0;
}

int vm_get_thread_context(vm_vcpu_t *vcpu, seL4_VCPUContext *context) {
    if(IS_MACHINE_STATE_UNKNOWN(vcpu->vcpu_arch.guest_state->machine.context)) {
        ZF_LOGE("Failed to get thread context: Context is unsynchronised. The VCPU hasn't exited?");
        return -1;
    }
    *context = vcpu->vcpu_arch.guest_state->machine.context;
    return 0;
}

int vm_get_thread_context_reg(vm_vcpu_t *vcpu, unsigned int reg, uint32_t *value) {
    if(IS_MACHINE_STATE_UNKNOWN(vcpu->vcpu_arch.guest_state->machine.context)) {
        ZF_LOGE("Failed to get thread context register: Context is unsynchronised. The VCPU hasn't exited?");
        return -1;
    }
    *value = (&vcpu->vcpu_arch.guest_state->machine.context.eax)[reg];
    return 0;
}

int vm_set_vmcs_field(vm_vcpu_t *vcpu, seL4_Word field, uint32_t value) {
    int err = 0;
    switch(field) {
    case VMX_GUEST_CR0:
        vmm_guest_state_set_cr0(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_CR3:
        vmm_guest_state_set_cr3(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_CR4:
        vmm_guest_state_set_cr4(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_RFLAGS:
        vmm_guest_state_set_rflags(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_IDTR_BASE:
        vmm_guest_state_set_idt_base(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_IDTR_LIMIT:
        vmm_guest_state_set_idt_limit(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_GDTR_BASE:
        vmm_guest_state_set_gdt_base(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_GDTR_LIMIT:
        vmm_guest_state_set_gdt_limit(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_CS_SELECTOR:
        vmm_guest_state_set_cs_selector(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_GUEST_RIP:
        vmm_guest_state_set_eip(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_CONTROL_PRIMARY_PROCESSOR_CONTROLS:
        vmm_guest_state_set_control_ppc(vcpu->vcpu_arch.guest_state, value);
        break;
    case VMX_CONTROL_ENTRY_INTERRUPTION_INFO:
        vmm_guest_state_set_control_entry(vcpu->vcpu_arch.guest_state, value);
        break;
    default:
        /* Write through to VMCS */
        err = vmm_vmcs_write(vcpu->vcpu.cptr, field, value);
    }
    return err;
}

int vm_get_vmcs_field(vm_vcpu_t *vcpu, seL4_Word field, uint32_t *value) {
    int err = 0;
    uint32_t val;
    switch(field) {
    case VMX_GUEST_CR0:
        val = vmm_guest_state_get_cr0(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_CR3:
        val = vmm_guest_state_get_cr3(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_CR4:
        val = vmm_guest_state_get_cr4(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_RFLAGS:
        val = vmm_guest_state_get_rflags(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_IDTR_BASE:
        val = vmm_guest_state_get_idt_base(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_IDTR_LIMIT:
        val = vmm_guest_state_get_idt_limit(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_GDTR_BASE:
        val = vmm_guest_state_get_gdt_base(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_GDTR_LIMIT:
        val = vmm_guest_state_get_gdt_limit(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_CS_SELECTOR:
        val = vmm_guest_state_get_cs_selector(vcpu->vcpu_arch.guest_state, vcpu->vcpu.cptr);
        break;
    case VMX_GUEST_RIP:
        val = vmm_guest_state_get_eip(vcpu->vcpu_arch.guest_state);
        break;
    case VMX_CONTROL_PRIMARY_PROCESSOR_CONTROLS:
        val = vmm_guest_state_get_control_ppc(vcpu->vcpu_arch.guest_state);
        break;
    case VMX_CONTROL_ENTRY_INTERRUPTION_INFO:
        val = vmm_guest_state_get_control_entry(vcpu->vcpu_arch.guest_state);
        break;
    default:
        /* Write through to VMCS */
        err = vmm_vmcs_read(vcpu->vcpu.cptr, field, &val);
    }
    *value = val;
    return err;
}