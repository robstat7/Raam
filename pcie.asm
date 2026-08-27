struc ENHANCED_CONFIG_BASE_STRUCT {
  .base_addr          dq ?
  .pci_seg_grp_num    dw ?
  .start_bus_num      db ?
  .end_bus_num        db ?
  .reserved           dd ?
}
struct ENHANCED_CONFIG_BASE_STRUCT

struc PCIE_ECAM_STRUCT {
  .base               dq ?
  .start_bus_num      db ?
  .end_bus_num        db ?
}
struct PCIE_ECAM_STRUCT


section '.text' code executable readable

;
; pcie_init
;
; this function gets and stores the PCIe ECAM base address and the
; starting and the ending bus numbers.
;
; args:
;   @rdi = mcfg table pointer
;
; returns:
;   nothing
;
pcie_init:
  ; in most systems there is only one PCI segment group -
	; (PCI segment group number 0). Hence use the 0 index.
  ; for e.g.,  mcfg->e[0].base_addr
  lea rax, [pcie_ecam]
  mov rbx, qword [rdi + MCFG_STRUCT.e.base_addr]
  mov qword [rax + PCIE_ECAM_STRUCT.base], rbx

  mov bl, byte [rdi + MCFG_STRUCT.e.start_bus_num]
  mov byte [rax + PCIE_ECAM_STRUCT.start_bus_num], bl

  mov bl, byte [rdi + MCFG_STRUCT.e.end_bus_num]
  mov byte [rax + PCIE_ECAM_STRUCT.end_bus_num], bl
  ret


section '.data' data readable writeable

pcie_ecam rb sizeof.PCIE_ECAM_STRUCT
