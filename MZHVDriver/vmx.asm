.code

  VMX_vmcall PROC
    VMCALL
    RET
  VMX_vmcall ENDP

  VMX_inveptAll PROC
    MOV QWORD PTR [RSP - 16], 0
    MOV QWORD PTR [RSP - 8], 0
    MOV RCX, 2
    INVEPT RCX, OWORD PTR [RSP - 16]
    RET
  VMX_inveptAll ENDP

END