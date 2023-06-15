section .text
[BITS 64]

global test_asm
test_asm: 
	  mov r12, 1
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, r8
	  mov rdi, r9
	  syscall
	  ret

global create_proc
create_proc:
      mov r12, 2
	  mov r13, rcx
	  mov r14, 10
	  mov r15, 10
	  mov rdi, 10
	  syscall
	  ret

global exit_proc
exit_proc:
      mov r12, 5
	  mov r13, 0
	  mov r14, 0
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret

global wait_proc
wait_proc:
      mov r12, 6
	  mov r13, rcx
	  mov r14, 0
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret

global get_pid
get_pid:
      xor rax,rax
      mov r12, 4
	  mov r13, 0
	  mov r14, 0
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret

global fin_load_proc
fin_load_proc:
      mov r12, 7
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret

global proc_load_exec
proc_load_exec:
      mov r12, 8
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, r8
	  mov rdi, r9
	  syscall
	  ret

global create_shm
create_shm:
      xor rax, rax
      mov r12, 9
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, r8
	  mov rdi, 0
	  syscall
	  ret

global obtain_shm
obtain_shm:
      xor rax, rax
      mov r12, 10
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, r8
	  mov rdi, 0
	  syscall
	  ret

global unmap_shm
unmap_shm:
      xor rax, rax
      mov r12, 11
	  mov r13, rcx
	  mov r14, 0
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret

global openfile
openfile:
      xor rax,rax
	  mov r12, 12
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret

global mmap
mmap:
      xor rax, rax
	  mov r12, 13
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, r8
	  mov rdi, r9
	  syscall
	  ret

global munmap
munmap:
      xor rax,rax
	  mov r12, 14
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret
      
global get_heap_mem
get_heap_mem:
      xor rax,rax
	  mov r12, 15
	  mov r13, rcx
	  mov r14, 0
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret

global read_file
read_file:
      xor rax,rax
	  mov r12, 16
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, r8
	  mov rdi, 0
	  syscall
	  ret

global write_file
write_file:
      xor rax,rax
	  mov r12, 17
	  mov r13, rcx
	  mov r14, rdx
	  mov r15, r8
	  mov rdi, 0
	  syscall
	  ret

global create_dir
create_dir:
      xor rax,rax
	  mov r12, 18
	  mov r13, rcx
	  mov r14, 0
	  mov r15, 0
	  mov rdi, 0
	  syscall
	  ret
      
     
       

